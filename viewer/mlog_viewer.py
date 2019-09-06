#!/usr/bin/env python3

# Run this script with this command:
#   $ bokeh serve --show ./mlog_viewer.py --args "trace_file"

import sys
import bisect
import random
import copy
from collections import OrderedDict
from bokeh.io import curdoc
from bokeh.plotting import figure
from bokeh.models import ColumnDataSource, RangeTool, LabelSet
from bokeh.layouts import column, row
from bokeh.palettes import viridis
from bokeh.transform import factor_cmap
from bokeh.models.widgets import CheckboxGroup, CheckboxButtonGroup, Slider

input_name = sys.argv[1]

NUM_RECT_SAMPLES = 10000
NUM_LABEL_SAMPLES = 1000
NUM_RANGETOOL_SAMPLES = 10000

worker_concurrency = 5

print("Reading file...")
with open(input_name) as f:
    data_lines = [ s.strip().split(",") for s in f.readlines() ]

print("Sorting...")
data_lines.sort(key=lambda x: x[1])

print("Preparing data...")

data_lists = {
    'rank0' : [int  (d[0]) for d in data_lines],
    't0'    : [float(d[1]) for d in data_lines],
    'rank1' : [int  (d[2]) for d in data_lines],
    't1'    : [float(d[3]) for d in data_lines],
    'kind'  : [      d[4]  for d in data_lines],
    'y'     : [int  (d[0]) for d in data_lines],
}

data_lists['duration'] = [t1-t0 for t0, t1 in zip(data_lists['t0'], data_lists['t1'])]

t1_idx_list = list(enumerate(data_lists['t1']))
t1_idx_list.sort(key=lambda x: x[1])
t1_idx_list_keys = [ t1 for i, t1 in t1_idx_list ]

kind_list = list(set(data_lists['kind']))
kind_list.sort()
kind_colors = viridis(len(kind_list))

x_min = min(data_lists['t0'])
x_max = max(data_lists['t1'])
num_ranks = max(max(data_lists['rank0']), max(data_lists['rank1']))+1

empty_data = {k: [] for k in data_lists.keys()}
rect_source = ColumnDataSource(data=copy.deepcopy(empty_data))
label_source = ColumnDataSource(data=copy.deepcopy(empty_data))
rangetool_source = ColumnDataSource(data=copy.deepcopy(empty_data))

color_mapper = factor_cmap(field_name='kind', palette=kind_colors, factors=kind_list)

TOOLTIPS = [
    #('index', "$index"),
    ("t", "(@t0,@t1)"),
    ("duration", "@duration"),
    ("rank", "(@rank0,@rank1)"),
    ("kind", "@kind")
]

p1 = figure(plot_width=1200, plot_height=800,
            x_range=(x_min, x_max),
            tools='hover,xwheel_zoom,ywheel_zoom,xpan,pan,save,help',
            active_drag='xpan', active_scroll="xwheel_zoom",
            tooltips=TOOLTIPS, output_backend='webgl')

p1.hbar(y="y", left="t0", right="t1", height=0.1, 
        legend='kind', color=color_mapper, source=rect_source)

labels = LabelSet(x='t0', y='y', text='kind',
                  x_offset=5, y_offset=5, source=label_source,
                  level='glyph', render_mode='canvas')

p1.add_layout(labels)

p2 = figure(plot_width=1200, plot_height=150,
            y_range=p1.y_range,
            toolbar_location=None, output_backend='webgl')

p2.hbar(y="y", left="t0", right="t1", height=0.5, color=color_mapper, source=rangetool_source)

range_tool = RangeTool(x_range=p1.x_range)
p2.add_tools(range_tool)

kind_checks = CheckboxGroup(labels=kind_list, active=list(range(len(kind_list))))

cur_x_start = x_min
cur_x_end   = x_max
X_PADDING_RATE = 0.01

visible_kinds = set(kind_list)

def get_sampled_data(d_lists, idx_list, max_num_samples):
    global visible_kinds
    if len(idx_list) > max_num_samples:
        idx_list = random.sample(idx_list, max_num_samples)

    return {k: [l[i] for i in idx_list if d_lists['kind'][i] in visible_kinds]
        for k, l in d_lists.items()}

def update_rangetool_data():
    rangetool_data = \
        get_sampled_data(data_lists, range(len(data_lists['t0'])), NUM_RANGETOOL_SAMPLES)
    rangetool_source.data = rangetool_data

def adjust_data_y(d_lists):
    global worker_concurrency
    cur_pos = [0] * num_ranks
    div_y = worker_concurrency+1
    for i in range(len(d_lists['t0'])):
        rank = d_lists['rank0'][i]
        d_lists['y'][i] = rank + cur_pos[rank] / div_y
        cur_pos[rank] = 0 if cur_pos[rank] >= (div_y-2) else cur_pos[rank]+1

def update_cur_data():
    global cur_x_start
    global cur_x_end

    idx_start = t1_idx_list[bisect.bisect_left(t1_idx_list_keys, cur_x_start)][0]
    idx_end = bisect.bisect_right(data_lists['t0'], cur_x_end)

    rect_data = get_sampled_data(data_lists, range(idx_start, idx_end), NUM_RECT_SAMPLES)
    adjust_data_y(rect_data)
    label_data = get_sampled_data(rect_data, range(len(rect_data['t0'])), NUM_LABEL_SAMPLES)

    rect_source.data = rect_data
    label_source.data = label_data

def update_x_range(attr, old, new):
    global cur_x_start
    global cur_x_end
    print(attr, old, new)
    if attr == 'start':
        cur_x_start = new
    elif attr == 'end':
        cur_x_end = new
    update_cur_data()

def update_kinds(active_list):
    global visible_kinds
    visible_kinds = set(kind_list[i] for i in active_list)
    update_rangetool_data()
    update_cur_data()

def update_all_kinds(active_list):
    kind_checks.active = list(range(len(kind_list))) if 0 in active_list else []

def update_worker_concurrency(attr, old, new):
    global worker_concurrency
    worker_concurrency = new
    update_rangetool_data()
    update_cur_data()

update_cur_data()
update_rangetool_data()

range_tool.x_range.on_change('start', update_x_range)
range_tool.x_range.on_change('end', update_x_range)

kind_checks.on_click(update_kinds)

kind_all_button = CheckboxButtonGroup(labels=["Show all"], active=[0])
kind_all_button.on_click(update_all_kinds)

worker_concurrency_slider = Slider(start=1, end=30, value=worker_concurrency, step=1, title="Worker concurrency")
worker_concurrency_slider.on_change('value', update_worker_concurrency)

curdoc().add_root(column(row(p1, column(kind_checks, kind_all_button, worker_concurrency_slider)), p2))


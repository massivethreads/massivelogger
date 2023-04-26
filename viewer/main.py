#!/usr/bin/env python3

# Run this script with this command:
#   $ bokeh serve --show ./main.py --args "trace_file"

import pkg_resources

try:
    bokeh_version = pkg_resources.get_distribution("bokeh").version
except pkg_resources.DistributionNotFound:
    print("bokeh is not installed")
    exit(1)

if bokeh_version.startswith("2."):
    import main_v2 as main_module
elif bokeh_version.startswith("3."):
    import main_v3 as main_module
else:
    print("Please install bokeh v2 or v3")
    exit(1)

main_module.run()

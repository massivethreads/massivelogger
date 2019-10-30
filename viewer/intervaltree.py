#!/usr/bin/env python3

# Lazy Interval Tree

import numpy

class IntervalTree:
    def __init__(self, t0s, t1s):
        self.root = IntervalTreeNode(t0s, t1s, numpy.arange(len(t0s)))

    def overlaps(self, query_t0, query_t1):
        return self.root.overlaps(query_t0, query_t1, 100)

class IntervalTreeNode:
    def __init__(self, t0s, t1s, ids):
        self.ids           = ids
        self.t0s           = t0s
        self.t1s           = t1s
        self.tmin          = t0s.min()
        self.tmax          = t1s.max()
        self.pivot         = numpy.median(t0s / 2 + t1s / 2)
        self.num           = len(t0s)
        self.left_child    = None
        self.right_child   = None
        self.center_t0s    = None
        self.center_t1s    = None
        self.center_t0_ids = None
        self.center_t1_ids = None

    def overlaps(self, query_t0, query_t1, leaf_size):
        if self.num <= leaf_size:
            # leaf node
            cond1 = query_t0 < self.t1s
            cond2 = self.t0s < query_t1
            mask  = numpy.logical_and(cond1, cond2)
            return self.ids[mask]
        else:
            if self.left_child == None:
                if query_t0 <= self.tmin and self.tmax <= query_t1:
                    # return all intervals
                    return self.ids
                else:
                    self.partition_children()

            if query_t1 < self.pivot:
                # only left
                left_ids = self.left_child.overlaps(query_t0, query_t1, leaf_size)
                # center
                boundary   = self.center_t0s.searchsorted(query_t1, side="right")
                center_ids = self.center_t0_ids[:boundary]
                return numpy.concatenate([left_ids, center_ids])
            elif self.pivot < query_t0:
                # only right
                right_ids = self.right_child.overlaps(query_t0, query_t1, leaf_size)
                # center
                boundary   = self.center_t1s.searchsorted(query_t0, side="right")
                center_ids = self.center_t1_ids[boundary:]
                return numpy.concatenate([right_ids, center_ids])
            else:
                # both
                left_ids  = self.left_child.overlaps(query_t0, query_t1, leaf_size)
                right_ids = self.right_child.overlaps(query_t0, query_t1, leaf_size)
                # center
                center_ids = self.center_t0_ids
                return numpy.concatenate([left_ids, right_ids, center_ids])

    def partition_children(self):
        # partition intervals
        left_mask   = self.t1s < self.pivot
        right_mask  = self.pivot < self.t0s
        center_mask = numpy.logical_not(numpy.logical_or(left_mask, right_mask))
        # left child
        left_t0s = self.t0s[left_mask]
        left_t1s = self.t1s[left_mask]
        left_ids = self.ids[left_mask]
        self.left_child = IntervalTreeNode(left_t0s, left_t1s, left_ids)
        # right child
        right_t0s = self.t0s[right_mask]
        right_t1s = self.t1s[right_mask]
        right_ids = self.ids[right_mask]
        self.right_child = IntervalTreeNode(right_t0s, right_t1s, right_ids)
        # center intervals
        center_t0s = self.t0s[center_mask]
        center_t1s = self.t1s[center_mask]
        center_ids = self.ids[center_mask]
        # sort center intervals
        def sort_zip(v1, v2):
            sorted_idxs = v1.argsort()
            return v1.take(sorted_idxs), v2.take(sorted_idxs)
        self.center_t0s, self.center_t0_ids = sort_zip(center_t0s, center_ids)
        self.center_t1s, self.center_t1_ids = sort_zip(center_t1s, center_ids)
        # free
        self.t0s = None
        self.t1s = None
        self.ids = None

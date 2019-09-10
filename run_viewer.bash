#!/bin/bash

SCRIPT_PATH=$(dirname $0)
bokeh serve --show $SCRIPT_PATH/viewer --args "$@"


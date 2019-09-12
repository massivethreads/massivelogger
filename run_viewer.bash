#!/bin/bash

SCRIPT_PATH=$(dirname $0)

# --check-unused-sessions and --unused-session-lifetime are set to 1 sec so that the server shuts down
# immediately when a tab on browsers is closed. Default value (15 sec) is too large.
# See also `viewer/server_lifecycle.py`
bokeh serve --show $SCRIPT_PATH/viewer --check-unused-sessions 1000 --unused-session-lifetime 1000 --args "$@"

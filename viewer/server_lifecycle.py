import os
import sys

def on_session_destroyed(session_context):
    if os.getenv("MLOG_VIEWER_ONESHOT", "true").lower() in ["true", "1"]:
        print("session closed; stopping server...")
        sys.exit(0)

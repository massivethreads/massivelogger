import sys

def on_session_destroyed(session_context):
    print("session closed; stopping server...")
    sys.exit(0)

# Compatibility shim mapping to Ladybird's built-in requests-like client
# This allows `import requests` to work inside Ladybird's Python runtime

from ladybird_py.requests import *  # noqa: F401,F403

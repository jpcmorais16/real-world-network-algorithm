"""
pytest configuration — ensures the python/ package is importable from tests.
"""

import sys
import os

# Add the project root to sys.path so `from python.network_generator import ...` works
sys.path.insert(0, os.path.join(os.path.dirname(__file__), '..'))

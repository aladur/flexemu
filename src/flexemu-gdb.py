from pathlib import Path
import os.path
import sys

# Add python subdirectory to module search path
path_root = os.path.join(Path(__file__).parents[1], "python");
sys.path.append(str(path_root))

import flexemu_common


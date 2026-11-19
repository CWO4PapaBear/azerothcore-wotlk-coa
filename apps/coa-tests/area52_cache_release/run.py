from pathlib import Path
import runpy

for name in ("weapon_slots.py", "item_queue.py"):
    runpy.run_path(str(Path(__file__).with_name(name)), run_name="__main__")

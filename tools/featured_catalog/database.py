from pathlib import Path
import re
import subprocess
from catalog_worker import QUERY, PRICING_QUERY


class Database:
    def __init__(self, config):
        self.config = config
        for name in ("characters", "world"):
            if not re.fullmatch(r"[A-Za-z0-9_]+", config[name]):
                raise ValueError("Invalid database name")
        credentials = Path(config["defaults_file"])
        if not credentials.is_file() or credentials.stat().st_mode & 0o077:
            raise ValueError("Database defaults file must be private")

    def __call__(self, query):
        if query not in (QUERY, PRICING_QUERY):
            raise ValueError("Unsupported catalog query")
        query = query.replace("acore_world.", self.config["world"] + ".")
        result = subprocess.run(
            [self.config.get("executable", "mysql"), "--defaults-extra-file=" + self.config["defaults_file"],
             "--batch", "--raw", "--skip-column-names", "--connect-timeout=10", self.config["characters"]],
            input=query, text=True, capture_output=True, timeout=60, check=False)
        if result.returncode:
            raise RuntimeError("Catalog database read failed")
        if len(result.stdout) > 32 * 1024 * 1024:
            raise ValueError("Catalog query limit exceeded")
        return result.stdout

from __future__ import annotations
import json
from pathlib import Path

def load_prior(path: str | Path) -> dict:
    return json.loads(Path(path).read_text())

def current_envelope(prior: dict, active_modules: int) -> dict:
    if active_modules < 0:
        raise ValueError("active_modules must be >= 0")
    e=prior["qualification_envelope_ma_per_active_module"]
    return {
        "active_modules": active_modules,
        "low_ma": e["low"] * active_modules,
        "nominal_ma": e["nominal_full_channel"] * active_modules,
        "high_ma": e["high_full_channel"] * active_modules,
        "rail_voltage_v": prior["rail_voltage_v"],
        "voltage_droop_v": None,
        "voltage_droop_status": prior["voltage_droop"]["status"]
    }

def build_count_table(prior: dict, counts=(1,2,4,8,16,24)) -> list[dict]:
    return [current_envelope(prior,c) for c in counts]

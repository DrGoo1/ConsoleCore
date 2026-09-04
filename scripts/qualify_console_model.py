#!/usr/bin/env python3
from __future__ import annotations

import argparse
import json
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
sys.path.insert(0, str(ROOT))
from qualification.topology_grounded import qualify

def main() -> int:
    ap = argparse.ArgumentParser(description="Fail-closed topology-grounded ConsoleCore qualification")
    ap.add_argument("evidence_card", type=Path)
    ap.add_argument("--report", type=Path)
    args = ap.parse_args()
    card = json.loads(args.evidence_card.read_text())
    result = qualify(card).to_dict()
    text = json.dumps(result, indent=2, sort_keys=True) + "\n"
    if args.report:
        args.report.parent.mkdir(parents=True, exist_ok=True)
        args.report.write_text(text)
    print(text, end="")
    return 0 if result["status"] == "PASS" else 2

if __name__ == "__main__":
    raise SystemExit(main())

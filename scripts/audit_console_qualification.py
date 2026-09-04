#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from qualification.topology_grounded import qualify

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--cards",type=Path,default=ROOT/"qualification/cards")
    ap.add_argument("--report",type=Path)
    ap.add_argument("--require-all-pass",action="store_true")
    ap.add_argument("--require-promoted-pass",action="store_true")
    args=ap.parse_args()
    results=[]
    for p in sorted(args.cards.glob("*.evidence.json")):
        r=qualify(json.loads(p.read_text())).to_dict(); r["card"]=str(p.relative_to(ROOT)); results.append(r)
    summary={"schema_version":"1.0.0","model_count":len(results),"pass_count":sum(r["status"]=="PASS" for r in results),
             "blocked_count":sum(r["status"]!="PASS" for r in results),"results":results}
    text=json.dumps(summary,indent=2,sort_keys=True)+"\n"
    if args.report: args.report.parent.mkdir(parents=True,exist_ok=True); args.report.write_text(text)
    print(text,end="")
    if args.require_all_pass and summary["blocked_count"]: return 2
    if args.require_promoted_pass:
        for r in results:
            card=json.loads((ROOT/r["card"]).read_text())
            if card.get("promotion",{}).get("requested") and r["status"] != "PASS": return 2
    return 0
if __name__=="__main__": raise SystemExit(main())

#!/usr/bin/env python3
from __future__ import annotations
import argparse, json, sys
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
sys.path.insert(0,str(ROOT))
from qualification.topology_grounded import REQUIRED_STAGES

def main():
    ap=argparse.ArgumentParser()
    ap.add_argument("--output",type=Path,default=ROOT/"qualification/cards")
    args=ap.parse_args()
    reg=json.loads((ROOT/"qualification/topology_registry.json").read_text())
    args.output.mkdir(parents=True,exist_ok=True)
    for profile in sorted((ROOT/"console_profiles/generic").glob("*.json")):
        model_id=profile.stem
        model=reg["models"].get(model_id,{"family":"unknown","claim_level":"behavior_family"})
        stages=[]
        for stage_id in REQUIRED_STAGES:
            et=reg["stage_defaults"].get(stage_id,"unverified")
            stages.append({"stage_id":stage_id,"topology_class":"unresolved","evidence":[{"type":et,"source_id":"baseline-profile-prior"}],"parameters":[]})
        card={"schema_version":"1.0.0","model_id":model_id,"claim_level":model["claim_level"],
              "topology":{"family":model["family"],"shared_state_model":"native_shared_analog_environment","notes":model.get("notes","")},
              "stages":stages,"behavior_matrix":[],
              "promotion":{"requested":False,"production_promotion_allowed":False}}
        (args.output/f"{model_id}.evidence.json").write_text(json.dumps(card,indent=2)+"\n")
    return 0
if __name__=="__main__": raise SystemExit(main())

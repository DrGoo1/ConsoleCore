#!/usr/bin/env python3
from __future__ import annotations
import argparse,csv,json,sys
from pathlib import Path
sys.path.insert(0,str(Path(__file__).resolve().parents[1]))
from qualification.system_behavior import classify_fixed_trim_curve, required_render_matrix

def main():
 p=argparse.ArgumentParser(); p.add_argument("--legacy-fixed-trim-json"); p.add_argument("--out-dir",default="qualification/system_behavior")
 a=p.parse_args(); out=Path(a.out_dir); out.mkdir(parents=True,exist_ok=True)
 evidence=[]
 if a.legacy_fixed_trim_json:
  src=Path(a.legacy_fixed_trim_json); payload=json.loads(src.read_text())
  evidence.append(classify_fixed_trim_curve(payload,source_id="legacy_fixed_trim_shared_sender_summer_20260615").to_card_row())
 (out/'evidence.json').write_text(json.dumps({"schema_version":"1.0.0","evidence":evidence},indent=2)+"\n")
 rows=required_render_matrix()
 with (out/'required_render_matrix.csv').open('w',newline='') as f:
  w=csv.DictWriter(f,fieldnames=list(rows[0])); w.writeheader(); w.writerows(rows)
 print(json.dumps({"evidence_rows":len(evidence),"required_render_rows":len(rows),"out_dir":str(out)},indent=2))
if __name__=='__main__': main()

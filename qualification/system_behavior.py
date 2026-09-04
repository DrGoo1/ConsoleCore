from __future__ import annotations

from dataclasses import dataclass, asdict
from typing import Any

REQUIRED_COUNTS=(1,2,4,8,16,24)

@dataclass(frozen=True)
class DomainEvidence:
    domain:str
    status:str
    reason:str
    source_id:str
    fixed_trim:bool=True
    rms_normalized_per_count:bool=False
    channel_counts:tuple[int,...]=()
    metrics:dict[str,Any]|None=None

    def to_card_row(self)->dict[str,Any]:
        row={
            "domain":self.domain,"status":self.status,"reason":self.reason,
            "source_id":self.source_id,"fixed_trim":self.fixed_trim,
            "rms_normalized_per_count":self.rms_normalized_per_count,
        }
        if self.channel_counts: row["channel_counts"]=list(self.channel_counts)
        if self.metrics is not None: row["metrics"]=self.metrics
        return row

def classify_fixed_trim_curve(payload:dict[str,Any], *, source_id:str)->DomainEvidence:
    counts=tuple(sorted(int(x) for x in payload.get("stem_counts",[])))
    per=payload.get("per_stem",[])
    if not per:
        return DomainEvidence("fixed_trim_channel_count","insufficient","no render rows",source_id,channel_counts=counts)
    if any(str(x.get("trim_mode","")).startswith("rms-normal") for x in per):
        return DomainEvidence("fixed_trim_channel_count","fail","per-count RMS normalization detected",source_id,False,True,counts)
    missing=[x for x in REQUIRED_COUNTS if x not in counts]
    deltas=[]
    for row in per:
        d=row.get("deltas_ths_minus_consolecore",{})
        if d: deltas.append({"stem_count":row.get("stem_count"),"peak_db":d.get("peak_db"),"rms_db":d.get("rms_db"),"crest_db":d.get("crest_db")})
    # Historical THS comparison is a regression benchmark, never topology truth.
    if missing:
        status="insufficient"
        reason=f"missing required channel counts: {missing}"
    else:
        # conservative behavioral limit for regression qualification
        worst=max((abs(float(x.get("crest_db") or 0)) for x in deltas), default=999)
        status="pass" if worst<=1.5 else "fail"
        reason=f"worst crest delta {worst:.3f} dB vs behavioral benchmark"
    return DomainEvidence("fixed_trim_channel_count",status,reason,source_id,True,False,counts,{"benchmark":"legacy_shared_sender_summer","deltas":deltas})

def required_render_matrix()->list[dict[str,Any]]:
    rows=[]
    stimuli=("multitone","transient_burst","lf_heavy","hf_heavy","two_tone_imd")
    distributions=("distributed","concentrated")
    for n in REQUIRED_COUNTS:
        for stim in stimuli:
            for dist in distributions:
                rows.append({"channel_count":n,"stimulus":stim,"distribution":dist,"fixed_trim":True,"rms_normalized_per_count":False})
    return rows

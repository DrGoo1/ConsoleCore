from __future__ import annotations
import hashlib, json, math, random
from dataclasses import dataclass, asdict
from typing import Any

SCHEMA_VERSION = '1.0.0'
ALLOWED_BASIS = {'authoritative_spec','hardware_measurement','schematic_exact','schematic_inferred','component_topology_prior','behavioral_reference','historical_documentation'}

@dataclass(frozen=True)
class InferenceResult:
    model_id: str
    status: str
    confidence: float
    fingerprint: str
    blockers: tuple[str, ...]
    high_sensitivity_parameters: tuple[str, ...]
    promotion_eligible: bool = False
    def to_dict(self) -> dict[str, Any]: return asdict(self)

def _fp(x: dict[str, Any]) -> str:
    return hashlib.sha256(json.dumps(x,sort_keys=True,separators=(',',':')).encode()).hexdigest()

def evaluate(spec: dict[str, Any]) -> InferenceResult:
    blockers=[]; sensitivities=[]
    if spec.get('schema_version') != SCHEMA_VERSION: blockers.append('unsupported schema_version')
    if not spec.get('model_id'): blockers.append('missing model_id')
    stages=spec.get('stages',[])
    if not stages: blockers.append('no inferred stages')
    scores=[]
    for s in stages:
        evidence=s.get('evidence',[])
        if not evidence: blockers.append(f"{s.get('stage_id')}: no evidence")
        for e in evidence:
            if e.get('type') not in ALLOWED_BASIS: blockers.append(f"{s.get('stage_id')}: invalid evidence basis")
        for p in s.get('parameters',[]):
            lo=p.get('min'); hi=p.get('max')
            if lo is None or hi is None or float(lo)>float(hi): blockers.append(f"{s.get('stage_id')}: invalid bounds for {p.get('name')}")
            if not p.get('source_id'): blockers.append(f"{s.get('stage_id')}: {p.get('name')} missing source_id")
            c=float(p.get('confidence',0)); scores.append(max(0,min(1,c)))
            if float(p.get('sensitivity',0)) >= .7: sensitivities.append(str(p.get('name')))
    confidence=sum(scores)/len(scores) if scores else 0.0
    status='BLOCKED' if blockers else ('BOUNDED' if confidence >= .35 else 'LOW_CONFIDENCE')
    return InferenceResult(str(spec.get('model_id','<missing>')),status,round(confidence,4),_fp(spec),tuple(blockers),tuple(sorted(set(sensitivities))),False)

def bounded_ensemble(parameter: dict[str, Any], samples: int=2048, seed: int=1701) -> dict[str,float]:
    rng=random.Random(seed); lo=float(parameter['min']); hi=float(parameter['max'])
    vals=[rng.uniform(lo,hi) for _ in range(samples)]; vals.sort()
    q=lambda f: vals[min(len(vals)-1,max(0,int(f*(len(vals)-1))))]
    return {'p05':q(.05),'median':q(.5),'p95':q(.95)}

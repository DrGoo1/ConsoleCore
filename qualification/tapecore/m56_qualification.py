from __future__ import annotations
import hashlib, json
from dataclasses import dataclass, asdict
from typing import Any

SCHEMA_VERSION='1.0.0'
REQUIRED_DOMAINS=(
 'machine_identity','speed_transport','bias_record_eq','playback_frequency_phase',
 'record_frequency_phase','level_transfer','harmonic_level_sweep','imd',
 'lf_head_bump_magnitude_phase','level_dependent_lf_weight','hysteresis_memory_recovery',
 'crest_transient','wow_flutter_spectrum','noise_spectrum','crosstalk','shared_transport_state'
)
STRONG_EVIDENCE={'hardware_measurement','authoritative_spec','manufacturer_manual','manufacturer_paper','schematic_exact'}
BEHAVIOR_ONLY={'commercial_reference','listening_memory','behavioral_reference'}

@dataclass(frozen=True)
class M56QualificationResult:
    status:str; claim_level:str; fingerprint:str; blockers:tuple[str,...]; warnings:tuple[str,...]; production_promotion_allowed:bool
    def to_dict(self)->dict[str,Any]: return asdict(self)

def _fp(card:dict[str,Any])->str:
    return hashlib.sha256(json.dumps(card,sort_keys=True,separators=(',',':')).encode()).hexdigest()

def qualify_m56(card:dict[str,Any])->M56QualificationResult:
    blockers=[]; warnings=[]
    if card.get('schema_version')!=SCHEMA_VERSION: blockers.append('unsupported schema_version')
    claim=str(card.get('claim_level',''))
    if claim not in {'m56_behavior_family','m56_topology_inferred','m56_hardware_faithful'}: blockers.append('invalid claim_level')
    if card.get('machine_id')!='3m.m56': blockers.append('machine_id must be 3m.m56')
    if card.get('track_count')!=16 or card.get('tape_width_in')!=2.0: blockers.append('M56 identity requires 16-track 2-inch format')
    speeds=set(card.get('supported_speeds_ips',[]))
    if not {15.0,30.0}.issubset(speeds): blockers.append('15 and optional 30 ips variants must be represented')
    domains={x.get('domain'):x for x in card.get('evidence_domains',[]) if isinstance(x,dict)}
    for d in REQUIRED_DOMAINS:
        row=domains.get(d)
        if not row: blockers.append(f'missing domain: {d}'); continue
        if row.get('status')!='pass': blockers.append(f'domain not passed: {d}')
        sources=row.get('evidence',[])
        if not sources: blockers.append(f'{d}: no evidence')
    if claim=='m56_hardware_faithful':
        for d in ('machine_identity','speed_transport','bias_record_eq','playback_frequency_phase','record_frequency_phase','level_transfer','harmonic_level_sweep','wow_flutter_spectrum'):
            types={e.get('type') for e in domains.get(d,{}).get('evidence',[])}
            if not (types & STRONG_EVIDENCE): blockers.append(f'{d}: hardware-faithful claim needs strong evidence')
        # Documentation can prove limits and topology, but it cannot prove the exact sonic signature.
        for d in ('lf_head_bump_magnitude_phase','level_dependent_lf_weight','harmonic_level_sweep','hysteresis_memory_recovery','crest_transient','noise_spectrum','crosstalk'):
            types={e.get('type') for e in domains.get(d,{}).get('evidence',[])}
            if 'hardware_measurement' not in types:
                blockers.append(f'{d}: hardware-faithful sonic claim requires direct hardware measurement')
    for p in card.get('parameters',[]):
        if p.get('status')=='legacy_unverified' and p.get('promotion_eligible') is True:
            blockers.append(f"legacy parameter cannot promote: {p.get('name')}")
    if any(row.get('evidence') and all(e.get('type') in BEHAVIOR_ONLY for e in row.get('evidence',[])) for row in domains.values() if isinstance(row,dict)):
        warnings.append('one or more domains are behavioral-reference-only')
    promo=card.get('promotion',{})
    allowed=bool(promo.get('requested')) and bool(promo.get('production_promotion_allowed')) and not blockers
    return M56QualificationResult('PASS' if not blockers else 'BLOCKED',claim,_fp(card),tuple(blockers),tuple(warnings),allowed)

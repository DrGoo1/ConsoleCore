from __future__ import annotations

import hashlib
import json
from dataclasses import dataclass, asdict
from typing import Any, Iterable

SCHEMA_VERSION = "1.0.0"
REQUIRED_STAGES = (
    "console_sender_line_amp", "console_source_impedance", "console_summing_node",
    "console_shared_rail", "console_bus_amp", "console_output_stage",
    "console_dc_coupling_servo", "console_crosstalk_field",
)
REQUIRED_BEHAVIORS = (
    "level_transfer", "harmonic_level_sweep", "imd", "crest_transient",
    "fixed_trim_channel_count", "lf_load_rail_stress", "recovery_memory",
    "phase_group_delay", "crosstalk", "shared_state_interaction",
)
REQUIRED_CHANNEL_COUNTS = (1, 2, 4, 8, 16, 24)
PARAMETER_DERIVATIONS = {
    "measured", "authoritative_spec", "schematic_exact", "schematic_inferred",
    "component_calculated", "fitted_to_measurement", "bounded_topology_prior", "behavioral_fit",
}
STRONG_PARAMETER_DERIVATIONS = {
    "measured", "authoritative_spec", "schematic_exact", "schematic_inferred", "component_calculated", "fitted_to_measurement",
}

EVIDENCE_RANK = {
    "unverified": 0, "computational_prior": 1, "behavioral_reference": 2,
    "component_topology_prior": 2, "schematic_inferred": 3,
    "authoritative_spec": 4, "schematic_exact": 4, "hardware_measurement": 5,
}

@dataclass(frozen=True)
class QualificationResult:
    status: str
    model_id: str
    claim_level: str
    fingerprint: str
    blockers: tuple[str, ...]
    warnings: tuple[str, ...]
    production_promotion_allowed: bool

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)

def _fingerprint(card: dict[str, Any]) -> str:
    payload = json.dumps(card, sort_keys=True, separators=(",", ":")).encode()
    return hashlib.sha256(payload).hexdigest()

def _evidence_types(stage: dict[str, Any]) -> set[str]:
    return {str(e.get("type")) for e in stage.get("evidence", [])}

def qualify(card: dict[str, Any]) -> QualificationResult:
    blockers: list[str] = []
    warnings: list[str] = []
    model_id = str(card.get("model_id", "<missing>"))
    claim = str(card.get("claim_level", "<missing>"))
    if card.get("schema_version") != SCHEMA_VERSION:
        blockers.append("unsupported schema_version")
    if claim not in {"behavior_family", "topology_faithful", "exact_hardware"}:
        blockers.append("invalid claim_level")
    topology = card.get("topology", {})
    if topology.get("shared_state_model") != "native_shared_analog_environment":
        blockers.append("shared-state model must use native SharedAnalogEnvironment")

    stages = {s.get("stage_id"): s for s in card.get("stages", []) if isinstance(s, dict)}
    for stage_id in REQUIRED_STAGES:
        stage = stages.get(stage_id)
        if not stage:
            blockers.append(f"missing required stage: {stage_id}")
            continue
        if stage.get("physically_absent") is True:
            if not stage.get("absence_basis"):
                blockers.append(f"{stage_id}: physically_absent requires absence_basis")
            continue
        types = _evidence_types(stage)
        if not types:
            blockers.append(f"{stage_id}: no evidence")
            continue
        best = max((EVIDENCE_RANK.get(t, -1) for t in types), default=-1)
        if best < 0:
            blockers.append(f"{stage_id}: unknown evidence type")
        if claim in {"topology_faithful", "exact_hardware"}:
            grounded = types & {"component_topology_prior","schematic_inferred","authoritative_spec","schematic_exact","hardware_measurement"}
            if not grounded:
                blockers.append(f"{stage_id}: topology/circuit evidence required; weak or behavioral evidence cannot solely ground {claim}")
        params = stage.get("parameters", [])
        if claim in {"topology_faithful", "exact_hardware"}:
            if not params:
                blockers.append(f"{stage_id}: parameter provenance is required for {claim}")
            for param in params:
                derivation = str(param.get("derivation", ""))
                if derivation not in PARAMETER_DERIVATIONS:
                    blockers.append(f"{stage_id}: invalid parameter derivation for {param.get('name','<unnamed>')}")
                if not param.get("source_id"):
                    blockers.append(f"{stage_id}: parameter {param.get('name','<unnamed>')} missing source_id")
            if params and all(str(x.get("derivation")) == "behavioral_fit" for x in params):
                blockers.append(f"{stage_id}: behavioral fit cannot solely determine topology-specific parameters")
        if claim == "exact_hardware":
            if best < 3:
                blockers.append(f"{stage_id}: exact_hardware requires schematic-inferred or stronger evidence")
            weak = [x for x in params if str(x.get("derivation")) not in STRONG_PARAMETER_DERIVATIONS]
            if weak:
                blockers.append(f"{stage_id}: exact_hardware contains weak parameter derivations")

    if claim == "exact_hardware":
        strong = 0
        for stage in stages.values():
            if _evidence_types(stage) & {"hardware_measurement","schematic_exact","authoritative_spec"}:
                strong += 1
        if strong < 4:
            blockers.append("exact_hardware requires strong evidence on at least four stages")

    behaviors = {b.get("domain"): b for b in card.get("behavior_matrix", []) if isinstance(b, dict)}
    for domain in REQUIRED_BEHAVIORS:
        row = behaviors.get(domain)
        if not row:
            blockers.append(f"missing behavior domain: {domain}")
            continue
        if row.get("status") != "pass":
            blockers.append(f"behavior domain not passed: {domain}")
        if row.get("fixed_trim") is not True:
            blockers.append(f"{domain}: qualification must be fixed-trim")
        if row.get("rms_normalized_per_count") is True:
            blockers.append(f"{domain}: per-count RMS normalization is prohibited")
    count_row = behaviors.get("fixed_trim_channel_count", {})
    counts = tuple(sorted(set(int(x) for x in count_row.get("channel_counts", []) if isinstance(x, (int,float)))))
    missing_counts = [x for x in REQUIRED_CHANNEL_COUNTS if x not in counts]
    if missing_counts:
        blockers.append(f"fixed_trim_channel_count missing counts: {missing_counts}")
    shared = behaviors.get("shared_state_interaction", {})
    if shared and shared.get("distribution_comparison") not in {"pass", True}:
        blockers.append("shared_state_interaction must prove concentrated-vs-distributed energy behavior")

    promo = card.get("promotion", {})
    requested = bool(promo.get("requested", False))
    declared = bool(promo.get("production_promotion_allowed", False))
    if declared and blockers:
        warnings.append("card declares promotion allowed but qualification blockers exist")
    allowed = requested and declared and not blockers
    return QualificationResult(
        status="PASS" if not blockers else "BLOCKED",
        model_id=model_id, claim_level=claim, fingerprint=_fingerprint(card),
        blockers=tuple(blockers), warnings=tuple(warnings),
        production_promotion_allowed=allowed,
    )

import unittest

from qualification.topology_grounded import REQUIRED_BEHAVIORS, REQUIRED_STAGES, qualify

def complete_card(claim="topology_faithful"):
    stages=[]
    for idx, stage in enumerate(REQUIRED_STAGES):
        evidence_type="schematic_inferred"
        if idx < 4:
            evidence_type="hardware_measurement"
        stages.append({"stage_id":stage,"topology_class":"test-stage","evidence":[{"type":evidence_type,"promotion_eligible":True,"source_id":f"test-{idx}"}],"parameters":[{"name":"test_parameter","value":0.5,"unit":"normalized","derivation":"measured","source_id":f"test-{idx}"}]})
    matrix=[]
    for domain in REQUIRED_BEHAVIORS:
        row={"domain":domain,"status":"pass","fixed_trim":True,"rms_normalized_per_count":False}
        if domain=="fixed_trim_channel_count": row["channel_counts"]=[1,2,4,8,16,24]
        if domain=="shared_state_interaction": row["distribution_comparison"]="pass"
        matrix.append(row)
    return {"schema_version":"1.0.0","model_id":"test-only","claim_level":claim,
            "topology":{"family":"test","shared_state_model":"native_shared_analog_environment"},
            "stages":stages,"behavior_matrix":matrix,
            "promotion":{"requested":True,"production_promotion_allowed":True}}

class QualificationTests(unittest.TestCase):
    def test_complete_topology_card_passes(self):
        r=qualify(complete_card())
        self.assertEqual(r.status,"PASS")
        self.assertTrue(r.production_promotion_allowed)

    def test_behavioral_reference_alone_cannot_claim_topology(self):
        card=complete_card()
        card["stages"][0]["evidence"]=[{"type":"behavioral_reference","source_id":"ref"}]
        r=qualify(card)
        self.assertEqual(r.status,"BLOCKED")
        self.assertTrue(any("cannot solely ground" in x for x in r.blockers))

    def test_unreviewed_schematic_evidence_does_not_ground_topology(self):
        card=complete_card()
        card["stages"][0]["evidence"]=[{"type":"schematic_inferred","promotion_eligible":False,"source_id":"sandbox"}]
        r=qualify(card)
        self.assertEqual(r.status,"BLOCKED")
        self.assertTrue(any("topology/circuit evidence required" in x for x in r.blockers))

    def test_per_count_rms_normalization_fails(self):
        card=complete_card()
        card["behavior_matrix"][0]["rms_normalized_per_count"]=True
        self.assertEqual(qualify(card).status,"BLOCKED")

    def test_missing_24_channel_proof_fails(self):
        card=complete_card()
        row=next(x for x in card["behavior_matrix"] if x["domain"]=="fixed_trim_channel_count")
        row["channel_counts"]=[1,2,4,8,16]
        self.assertEqual(qualify(card).status,"BLOCKED")

    def test_exact_hardware_needs_strong_stage_coverage(self):
        card=complete_card("exact_hardware")
        for s in card["stages"]:
            s["evidence"]=[{"type":"schematic_inferred","promotion_eligible":True,"source_id":"weak"}]
        r=qualify(card)
        self.assertEqual(r.status,"BLOCKED")
        self.assertTrue(any("at least four stages" in x for x in r.blockers))

if __name__ == "__main__":
    unittest.main()

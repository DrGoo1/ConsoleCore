import json, pathlib, unittest

ROOT=pathlib.Path(__file__).resolve().parents[1]

class SharedLoadProvenanceTests(unittest.TestCase):
    def test_all_shared_load_parameters_have_provenance(self):
        payload=json.loads((ROOT/"qualification/evidence/shared_load_parameter_provenance.json").read_text())
        rows={r["parameter"]:r for r in payload["parameters"]}
        required={"channelCountRailLoadFactor","railVoltageDroopFactor","channelCountHeadroomFactor","channelDistributionSensitivity"}
        self.assertEqual(required,set(rows))
        for name in required:
            self.assertIn("status",rows[name])
            self.assertIn("promotion_eligible",rows[name])
            self.assertIn("required_evidence",rows[name])
            self.assertFalse(rows[name]["promotion_eligible"])

    def test_distribution_parameter_stays_zero_until_evidence(self):
        payload=json.loads((ROOT/"qualification/evidence/shared_load_parameter_provenance.json").read_text())
        row=next(r for r in payload["parameters"] if r["parameter"]=="channelDistributionSensitivity")
        self.assertEqual(0.0,row["current_value"])
        self.assertIn("blocked",row["status"])

if __name__=="__main__": unittest.main()

import csv, json, unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class LunaReferencePolicyTests(unittest.TestCase):
    def test_commercial_refs_cannot_be_topology_truth(self):
        d=json.loads((ROOT/"qualification/reference_benchmarks/luna_console_reference_policy.json").read_text())
        for r in d["references"]:
            self.assertIn("topology_truth",r["forbidden_roles"])
            self.assertIn("exact_hardware_claim",r["forbidden_roles"])
    def test_matrix_covers_all_refs_counts_stimuli_distributions(self):
        with (ROOT/"qualification/reference_benchmarks/luna_console_capture_matrix.csv").open() as f:
            rows=list(csv.DictReader(f))
        self.assertEqual(len(rows),180)
        self.assertEqual({int(r["channel_count"]) for r in rows},{1,2,4,8,16,24})
        self.assertTrue(all(r["trim_mode"]=="fixed" and r["output_normalization"]=="no" for r in rows))
if __name__=="__main__": unittest.main()

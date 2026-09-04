import unittest
from qualification.system_behavior import classify_fixed_trim_curve,required_render_matrix
class SystemBehaviorTests(unittest.TestCase):
 def test_matrix_has_all_counts_and_distributions(self):
  rows=required_render_matrix(); self.assertEqual(len(rows),60)
  self.assertEqual({r["channel_count"] for r in rows},{1,2,4,8,16,24})
  self.assertEqual({r["distribution"] for r in rows},{"distributed","concentrated"})
  self.assertTrue(all(r["fixed_trim"] and not r["rms_normalized_per_count"] for r in rows))
 def test_old_1_to_8_curve_is_insufficient(self):
  p={"stem_counts":[1,2,4,8],"per_stem":[{"stem_count":1,"deltas_ths_minus_consolecore":{"crest_db":-16.5}}]}
  r=classify_fixed_trim_curve(p,source_id="x"); self.assertEqual(r.status,"insufficient"); self.assertIn("16",r.reason)
 def test_complete_bad_crest_curve_fails(self):
  rows=[{"stem_count":n,"deltas_ths_minus_consolecore":{"crest_db":2.1}} for n in [1,2,4,8,16,24]]
  r=classify_fixed_trim_curve({"stem_counts":[1,2,4,8,16,24],"per_stem":rows},source_id="x")
  self.assertEqual(r.status,"fail")
if __name__=='__main__': unittest.main()

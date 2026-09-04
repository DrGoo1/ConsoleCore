import json, pathlib, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class HardwareEvidenceTests(unittest.TestCase):
    def test_two_channel_hardware_anchor_is_scoped(self):
        d=json.load(open(ROOT/'qualification/evidence/neve_1073_two_channel_hardware_summing_20260610.json'))
        self.assertTrue(d['promotion_eligible'])
        self.assertEqual(d['capture_count'],18)
        self.assertEqual(d['clipped_samples_total'],0)
        self.assertIn('quiescent_channel_count_loading_above_2',d['blocked_domains'])
        self.assertLess(d['constraints']['baseline_repeatability_spread_db'],0.25)
    def test_card_uses_hardware_without_claiming_shared_rail(self):
        d=json.load(open(ROOT/'qualification/cards/British_ClassA_Discrete.evidence.json'))
        rail=next(x for x in d['stages'] if x['stage_id']=='console_shared_rail')
        self.assertFalse(any(e.get('source_id')=='HW_CLASS_A_TWO_CHANNEL_SUMMING_A_20260610' for e in rail['evidence']))
        summing=next(x for x in d['stages'] if x['stage_id']=='console_summing_node')
        self.assertTrue(any(e.get('source_id')=='HW_CLASS_A_TWO_CHANNEL_SUMMING_A_20260610' for e in summing['evidence']))
if __name__=='__main__': unittest.main()

import pathlib, unittest
from qualification.quiescent_supply import load_prior, current_envelope, build_count_table
ROOT=pathlib.Path(__file__).resolve().parents[1]
class QuiescentSupplyPriorTests(unittest.TestCase):
    def setUp(self):
        self.prior=load_prior(ROOT/'qualification/priors/british_class_a_quiescent_supply.json')
    def test_24_channel_current_scale_is_explicit(self):
        r=current_envelope(self.prior,24)
        self.assertEqual(r['low_ma'],1920.0)
        self.assertEqual(r['nominal_ma'],2544.0)
        self.assertEqual(r['high_ma'],2784.0)
    def test_voltage_droop_remains_blocked(self):
        for r in build_count_table(self.prior):
            self.assertIsNone(r['voltage_droop_v'])
            self.assertEqual(r['voltage_droop_status'],'blocked')
    def test_prior_cannot_promote_model(self):
        self.assertFalse(self.prior['promotion_eligible'])
if __name__=='__main__': unittest.main()

import json, pathlib, unittest
ROOT=pathlib.Path(__file__).resolve().parents[1]
class SharedLoadStimulusSpecTests(unittest.TestCase):
    def test_spec_is_matched_energy_and_waveform_equivalent(self):
        s=json.loads((ROOT/"qualification/system_behavior/shared_load_stimulus_spec.json").read_text())
        self.assertEqual([1,2,4,8,16,24],s["channel_counts"])
        self.assertEqual(-18.0,s["target_sum_rms_dbfs"])
        self.assertTrue(s["fixed_trim"])
        self.assertFalse(s["output_normalization_allowed"])
        self.assertTrue(s["pair_waveform_equivalence_required"])
        self.assertLessEqual(s["max_pair_quantization_error_samples_normalized"],2e-6)
        self.assertGreaterEqual(s["minimum_pair_correlation"],0.9999999999)
if __name__=="__main__": unittest.main()

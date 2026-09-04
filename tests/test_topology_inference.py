import unittest
from qualification.topology_inference import evaluate, bounded_ensemble

class TopologyInferenceTests(unittest.TestCase):
    def test_bounded_inference_never_promotes(self):
        spec={'schema_version':'1.0.0','model_id':'x','stages':[{'stage_id':'amp','evidence':[{'type':'component_topology_prior'}],'parameters':[{'name':'rail_v','min':20,'max':30,'source_id':'s','confidence':.6,'sensitivity':.8}]}]}
        r=evaluate(spec); self.assertEqual(r.status,'BOUNDED'); self.assertFalse(r.promotion_eligible); self.assertIn('rail_v',r.high_sensitivity_parameters)
    def test_missing_bounds_block(self):
        spec={'schema_version':'1.0.0','model_id':'x','stages':[{'stage_id':'amp','evidence':[{'type':'behavioral_reference'}],'parameters':[{'name':'x','source_id':'s'}]}]}
        self.assertEqual(evaluate(spec).status,'BLOCKED')
    def test_ensemble_is_deterministic_and_bounded(self):
        p={'min':1.0,'max':2.0}; a=bounded_ensemble(p); b=bounded_ensemble(p)
        self.assertEqual(a,b); self.assertGreaterEqual(a['p05'],1); self.assertLessEqual(a['p95'],2)
if __name__=='__main__': unittest.main()

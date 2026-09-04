import copy,json,unittest
from pathlib import Path
from qualification.tapecore.m56_qualification import qualify_m56
ROOT=Path(__file__).resolve().parents[1]
CARD=json.loads((ROOT/'qualification/tapecore/m56_evidence_card.json').read_text())
class M56QualificationTests(unittest.TestCase):
 def test_current_m56_card_is_correctly_blocked(self):
  r=qualify_m56(CARD); self.assertEqual(r.status,'BLOCKED'); self.assertFalse(r.production_promotion_allowed); self.assertTrue(any('lf_head_bump' in x for x in r.blockers))
 def test_legacy_coefficients_cannot_promote(self):
  d=copy.deepcopy(CARD); d['parameters'][2]['promotion_eligible']=True
  self.assertTrue(any('legacy parameter cannot promote' in x for x in qualify_m56(d).blockers))
 def test_behavioral_reference_cannot_complete_hardware_faithful_claim(self):
  d=copy.deepcopy(CARD); d['claim_level']='m56_hardware_faithful'
  for row in d['evidence_domains']: row['status']='pass'
  r=qualify_m56(d); self.assertEqual(r.status,'BLOCKED'); self.assertTrue(any('hardware-faithful sonic claim requires direct hardware measurement' in x for x in r.blockers))
 def test_identity_requires_2inch_16track(self):
  d=copy.deepcopy(CARD); d['track_count']=24
  self.assertIn('M56 identity requires 16-track 2-inch format',qualify_m56(d).blockers)
if __name__=='__main__': unittest.main()

#!/usr/bin/env python3
from __future__ import annotations
import argparse, hashlib, json, math, wave
from pathlib import Path
import numpy as np

FS=96000
COUNTS=(1,2,4,8,16,24)
STIMULI=("multitone","transient_burst","lf_heavy","hf_heavy","two_tone_imd")
DISTRIBUTIONS=("distributed","concentrated")
TARGET_SUM_RMS=10**(-18/20)
DURATION_S=2.0

def base_signal(kind:str, idx:int, n:int)->np.ndarray:
    t=np.arange(n,dtype=np.float64)/FS
    if kind=="multitone":
        freqs=(82.41,164.81,329.63,659.25,1318.51,2637.02,5274.04)
        x=sum(np.sin(2*math.pi*f*t+0.31*j) for j,f in enumerate(freqs))/len(freqs)
    elif kind=="transient_burst":
        x=0.16*np.sin(2*math.pi*220*t)
        spacing=FS//20
        for k in range(FS//40,n,spacing):
            m=min(FS//120,n-k); env=np.exp(-np.arange(m)/(FS*0.0025)); x[k:k+m]+=0.72*env*np.sin(2*math.pi*1200*np.arange(m)/FS)
    elif kind=="lf_heavy": x=0.75*np.sin(2*math.pi*55*t)+0.25*np.sin(2*math.pi*110*t+0.4)
    elif kind=="hf_heavy": x=0.6*np.sin(2*math.pi*6000*t)+0.4*np.sin(2*math.pi*11000*t+0.7)
    else: x=0.5*np.sin(2*math.pi*60*t)+0.5*np.sin(2*math.pi*7000*t+0.2)
    return x.astype(np.float64)

def write_pcm24(path:Path,x:np.ndarray):
    y=np.clip(x,-0.999999,0.999999); q=np.round(y*8388607).astype(np.int32)
    b=bytearray()
    for v in q:
        if v<0: v=(1<<24)+int(v)
        b.extend((v&255,(v>>8)&255,(v>>16)&255))
    with wave.open(str(path),'wb') as w:
        w.setnchannels(1); w.setsampwidth(3); w.setframerate(FS); w.writeframes(bytes(b))

def main():
    ap=argparse.ArgumentParser(); ap.add_argument('--out',required=True); a=ap.parse_args(); out=Path(a.out); out.mkdir(parents=True,exist_ok=True)
    manifest=[]; n=int(FS*DURATION_S)
    for count in COUNTS:
      for kind in STIMULI:
       target=base_signal(kind,0,n)
       rms=float(np.sqrt(np.mean(target*target)))
       target=target*(TARGET_SUM_RMS/max(rms,1e-12))
       if float(np.max(np.abs(target))) >= 0.95:
        raise RuntimeError(f"canonical stimulus peak too high for {kind}: {np.max(np.abs(target))}")
       for dist in DISTRIBUTIONS:
        if dist=="distributed": stems=[target/count for _ in range(count)]
        else: stems=[target.copy()]+[np.zeros(n) for _ in range(count-1)]
        case=f'{count:02d}ch_{kind}_{dist}'; case_dir=out/case; case_dir.mkdir(exist_ok=True)
        hashes=[]
        for i,s in enumerate(stems,1):
            p=case_dir/f'ch{i:02d}.wav'; write_pcm24(p,s); hashes.append(hashlib.sha256(p.read_bytes()).hexdigest())
        check=np.sum(stems,axis=0); final_rms=20*math.log10(max(float(np.sqrt(np.mean(check*check))),1e-20))
        manifest.append({"case":case,"channel_count":count,"stimulus":kind,"distribution":dist,"target_sum_rms_dbfs":-18.0,"actual_sum_rms_dbfs":final_rms,"stem_sha256":hashes})
    (out/'manifest.json').write_text(json.dumps({"schema_version":"1.0.0","sample_rate_hz":FS,"duration_seconds":DURATION_S,"fixed_trim":True,"rms_normalized_per_count":False,"normalization":"stimulus construction only: each case summed RMS matched before processing; output is never normalized","cases":manifest},indent=2)+'\n')
    print(json.dumps({"cases":len(manifest),"out":str(out)},indent=2))
if __name__=='__main__': main()

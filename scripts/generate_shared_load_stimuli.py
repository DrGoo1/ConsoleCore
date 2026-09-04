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
    phase=(idx*0.61803398875)%1.0*2*math.pi
    if kind=="multitone":
        freqs=(82.41,164.81,329.63,659.25,1318.51,2637.02,5274.04)
        x=sum(np.sin(2*math.pi*f*t+phase*(j+1)) for j,f in enumerate(freqs))/len(freqs)
    elif kind=="transient_burst":
        x=np.zeros(n); spacing=FS//4
        for k in range(FS//8,n,spacing):
            m=min(FS//50,n-k); env=np.exp(-np.arange(m)/(FS*0.006)); x[k:k+m]+=env*np.sin(2*math.pi*(120+idx*7)*np.arange(m)/FS+phase)
    elif kind=="lf_heavy": x=0.75*np.sin(2*math.pi*(55+idx*0.7)*t+phase)+0.25*np.sin(2*math.pi*(110+idx)*t+phase*0.3)
    elif kind=="hf_heavy": x=0.6*np.sin(2*math.pi*(6000+idx*53)*t+phase)+0.4*np.sin(2*math.pi*(11000+idx*71)*t+phase*0.7)
    else: x=0.5*np.sin(2*math.pi*60*t+phase)+0.5*np.sin(2*math.pi*7000*t+phase*0.2)
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
       raw=[base_signal(kind,i,n) for i in range(count)]
       for dist in DISTRIBUTIONS:
        stems=[]
        if dist=="distributed": stems=[r.copy() for r in raw]
        else:
            dominant=sum(raw)/max(1,count)
            stems=[dominant]+[np.zeros(n) for _ in range(count-1)]
        summed=np.sum(stems,axis=0); rms=float(np.sqrt(np.mean(summed*summed)))
        scale=TARGET_SUM_RMS/max(rms,1e-12); stems=[s*scale for s in stems]
        case=f'{count:02d}ch_{kind}_{dist}'; case_dir=out/case; case_dir.mkdir(exist_ok=True)
        hashes=[]
        for i,s in enumerate(stems,1):
            p=case_dir/f'ch{i:02d}.wav'; write_pcm24(p,s); hashes.append(hashlib.sha256(p.read_bytes()).hexdigest())
        check=np.sum(stems,axis=0); final_rms=20*math.log10(max(float(np.sqrt(np.mean(check*check))),1e-20))
        manifest.append({"case":case,"channel_count":count,"stimulus":kind,"distribution":dist,"target_sum_rms_dbfs":-18.0,"actual_sum_rms_dbfs":final_rms,"stem_sha256":hashes})
    (out/'manifest.json').write_text(json.dumps({"schema_version":"1.0.0","sample_rate_hz":FS,"duration_seconds":DURATION_S,"fixed_trim":True,"rms_normalized_per_count":False,"normalization":"stimulus construction only: each case summed RMS matched before processing; output is never normalized","cases":manifest},indent=2)+'\n')
    print(json.dumps({"cases":len(manifest),"out":str(out)},indent=2))
if __name__=='__main__': main()

#include "../../modules/analog_physics/AnalogPhysicsCore.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

static constexpr double PI=3.14159265358979323846;
static void makeStimulus(std::vector<float>& l,std::vector<float>& r,double fs,const std::string& kind){
    for(size_t i=0;i<l.size();++i){ double t=double(i)/fs; float x=0.0f;
        if(kind=="multitone") x=0.10f*(std::sin(2*PI*80*t)+std::sin(2*PI*997*t)+std::sin(2*PI*6000*t));
        else if(kind=="transient_burst") x=(i%2400<48)?0.65f*std::sin(2*PI*1000*t):0.0f;
        else if(kind=="lf_heavy") x=0.28f*std::sin(2*PI*55*t)+0.06f*std::sin(2*PI*440*t);
        else if(kind=="hf_heavy") x=0.10f*std::sin(2*PI*997*t)+0.18f*std::sin(2*PI*9000*t);
        else x=0.14f*std::sin(2*PI*60*t)+0.14f*std::sin(2*PI*7000*t);
        l[i]=x; r[i]=0.97f*x;
    }
}
static void metrics(const std::vector<float>& l,const std::vector<float>& r,float& peak,float& rms,float& crest){
    double ss=0; peak=0; size_t n=l.size()+r.size();
    for(float x:l){peak=std::max(peak,std::abs(x)); ss+=double(x)*x;} for(float x:r){peak=std::max(peak,std::abs(x));ss+=double(x)*x;}
    rms=float(std::sqrt(ss/double(n))); crest=20.0f*std::log10(std::max(peak,1e-12f)/std::max(rms,1e-12f));
}
int main(int argc,char**argv){
    const std::string out=(argc>1)?argv[1]:"system_behavior_actual.csv"; std::ofstream f(out); if(!f){std::cerr<<"cannot open output\n";return 2;}
    f<<"channel_count,stimulus,distribution,sensitivity,peak,rms,crest_db,rail_voltage,rail_stress,memory,headroom_gain,concentration,distribution_stress\n";
    const int counts[]={1,2,4,8,16,24}; const char* stimuli[]={"multitone","transient_burst","lf_heavy","hf_heavy","two_tone_imd"}; const char* dists[]={"distributed","concentrated"};
    constexpr double fs=96000.0; constexpr int n=96000;
    for(float sensitivity: {0.0f,0.25f}) for(int c:counts) for(auto stim:stimuli) for(auto dist:dists){
        std::vector<float> l(n),r(n); makeStimulus(l,r,fs,stim); AnalogPhysicsCore core; core.prepare(fs,n); AnalogPhysicsParameters p; p.channelDistributionSensitivity=sensitivity; core.setParameters(p);
        ChannelLoadSummary s; s.concentration=(std::string(dist)=="concentrated" || c==1)?1.0f:1.0f/float(c); s.maxRms=s.concentration; s.meanRms=1.0f/float(std::max(c,1));
        core.processBlock(l.data(),r.data(),n,c,s); float peak,rms,crest; metrics(l,r,peak,rms,crest); auto t=core.getTelemetry();
        f<<c<<','<<stim<<','<<dist<<','<<sensitivity<<','<<peak<<','<<rms<<','<<crest<<','<<t.railVoltage<<','<<t.railStress<<','<<t.memory<<','<<t.headroomGain<<','<<t.channelConcentration<<','<<t.channelDistributionStress<<'\n';
    }
    std::cout<<"wrote "<<out<<" (120 rows: 60 legacy-preserving + 60 sensitivity probe)\n"; return 0;
}

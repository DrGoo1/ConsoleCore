#include "TopologyAwareMixEngine.h"
#include "ConsoleProfileLibrary.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
using namespace consolecore;
static constexpr double PI=3.14159265358979323846;
static void canonical(std::vector<float>& x,double fs,const std::string& k){
 for(size_t i=0;i<x.size();++i){double t=double(i)/fs; float v=0;
  if(k=="multitone") v=.08f*(std::sin(2*PI*82.41*t)+std::sin(2*PI*997*t)+std::sin(2*PI*5274*t));
  else if(k=="transient_burst") v=(i%24000<256)?0.35f*std::exp(-double(i%24000)/(fs*.004))*std::sin(2*PI*997*t):0;
  else if(k=="lf_heavy") v=.18f*std::sin(2*PI*55*t)+.04f*std::sin(2*PI*110*t);
  else if(k=="hf_heavy") v=.08f*std::sin(2*PI*997*t)+.12f*std::sin(2*PI*9000*t);
  else v=.10f*std::sin(2*PI*60*t)+.10f*std::sin(2*PI*7000*t); x[i]=v; }
 double ss=0; for(float v:x) ss+=double(v)*v; double rms=std::sqrt(ss/x.size()); float g=float(std::pow(10.0,-18.0/20.0)/std::max(rms,1e-12)); for(auto&v:x)v*=g;
}
static void metrics(const std::vector<float>& l,const std::vector<float>& r,float& peak,float& rms,float& crest){double ss=0;peak=0;for(float v:l){peak=std::max(peak,std::abs(v));ss+=double(v)*v;}for(float v:r){peak=std::max(peak,std::abs(v));ss+=double(v)*v;}rms=float(std::sqrt(ss/(l.size()+r.size())));crest=20*std::log10(std::max(peak,1e-12f)/std::max(rms,1e-12f));}
int main(int argc,char**argv){std::string out=argc>1?argv[1]:"topology_family_actual.csv";std::ofstream f(out);if(!f)return 2;f<<"family,channel_count,stimulus,distribution,peak,rms,crest_db,rail_sag,activity\n";
 constexpr double fs=96000; constexpr int n=96000; int counts[]={1,2,4,8,16,24}; const char* kinds[]={"multitone","transient_burst","lf_heavy","hf_heavy","two_tone_imd"}; const char* ds[]={"distributed","concentrated"};
 auto family=ConsoleFamily::BritishClassADiscrete; auto profile=ConsoleProfileLibrary::makeProfile(family);
 for(int c:counts)for(auto k:kinds)for(auto d:ds){std::vector<float>x(n);canonical(x,fs,k);std::vector<std::vector<float>> chans(2*c,std::vector<float>(n));for(int s=0;s<c;++s){float scale=std::string(d)=="distributed"?1.0f/c:(s==0?1.0f:0.0f);for(int i=0;i<n;++i)chans[2*s][i]=chans[2*s+1][i]=x[i]*scale;}std::vector<const float*> ptrs;for(auto&v:chans)ptrs.push_back(v.data());ConsoleTopology topo;topo.inputChannelCount=2*c;topo.outputChannelCount=2;for(int s=0;s<c;++s){StemConfig st;st.stemId=s;st.inputLeft=2*s;st.inputRight=2*s+1;topo.stems.push_back(st);}std::vector<float>l(n),r(n);TopologyAwareMixEngine e;e.prepare(fs,n,2*c);e.setTopology(topo);e.setProfile(profile);e.setDrive(.28f);e.setStress(.42f);e.setCrosstalk(.08f);e.setTransformerWeight(.48f);e.setQualificationPhysicalMode(true);e.process(ptrs.data(),2*c,l.data(),r.data(),n);float p,rm,cr;metrics(l,r,p,rm,cr);f<<"British_ClassA_Discrete,"<<c<<','<<k<<','<<d<<','<<p<<','<<rm<<','<<cr<<','<<e.getRailSag()<<','<<e.getActivity()<<'\n';}
 std::cout<<"wrote "<<out<<"\n";}

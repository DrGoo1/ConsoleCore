#include "TopologyAwareMixEngine.h"
#include "ConsoleProfileLibrary.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <vector>
using namespace consolecore;

static ConsoleTopology makeTopology(bool reverse) {
    ConsoleTopology t; t.inputChannelCount=4; t.outputChannelCount=2;
    StemConfig a; a.stemId=1; a.name="A"; a.inputLeft=0; a.inputRight=1;
    StemConfig b; b.stemId=2; b.name="B"; b.inputLeft=2; b.inputRight=3;
    t.stems = reverse ? std::vector<StemConfig>{b,a} : std::vector<StemConfig>{a,b};
    return t;
}
static std::vector<float> render(bool reverse) {
    constexpr int n=4096; constexpr double fs=96000.0;
    std::vector<float> a(n), b(n), c(n), d(n), l(n), r(n);
    for(int i=0;i<n;++i){ double tt=double(i)/fs; a[i]=b[i]=0.12f*std::sin(2*M_PI*83.0*tt); c[i]=d[i]=0.07f*std::sin(2*M_PI*997.0*tt+0.7); }
    const float* ins[]={a.data(),b.data(),c.data(),d.data()};
    TopologyAwareMixEngine e; e.prepare(fs,n,4); e.setTopology(makeTopology(reverse));
    e.setProfile(ConsoleProfileLibrary::makeProfile(ConsoleFamily::BritishClassADiscrete));
    e.setDrive(0.28f); e.setStress(0.42f); e.setCrosstalk(0.08f); e.setTransformerWeight(0.48f); e.setQualificationPhysicalMode(true);
    e.process(ins,4,l.data(),r.data(),n); l.insert(l.end(),r.begin(),r.end()); return l;
}
int main(){
    auto x=render(false), y=render(true); float maxd=0.0f;
    for(size_t i=0;i<x.size();++i) maxd=std::max(maxd,std::abs(x[i]-y[i]));
    if(maxd>1e-6f){ std::cerr<<"stem-order dependence: "<<maxd<<"\n"; return 1; }

    constexpr int n=4096; constexpr double fs=96000.0;
    std::vector<float> a(n), b(n), l(n), r(n);
    for(int i=0;i<n;++i) a[i]=b[i]=1.0e-4f*std::sin(2*M_PI*997.0*double(i)/fs);
    const float* ins[]={a.data(),b.data()}; ConsoleTopology t; StemConfig st; st.inputLeft=0; st.inputRight=1; t.stems={st};
    ConsoleModelProfile clean; clean.busSagDepth=0.0; clean.transformerWeight=0.0; clean.crosstalkDb=-120.0;
    TopologyAwareMixEngine e; e.prepare(fs,n,2); e.setTopology(t); e.setProfile(clean); e.setDrive(0.5f); e.setStress(0.0f); e.setCrosstalk(0.0f); e.setTransformerWeight(0.0f); e.setQualificationPhysicalMode(true); e.process(ins,2,l.data(),r.data(),n);
    double inss=0,outss=0; for(int i=0;i<n;++i){inss+=double(a[i])*a[i]; outss+=double(l[i])*l[i];}
    const double expectedPan=std::sqrt(0.5); const double ratio=std::sqrt(outss/inss)/expectedPan;
    if(std::abs(ratio-1.0)>0.002){ std::cerr<<"small-signal slope not unity: "<<ratio<<"\n"; return 2; }
    std::cout<<"qualification physical mode order invariant; max diff="<<maxd<<" small-signal ratio="<<ratio<<"\n"; return 0;
}

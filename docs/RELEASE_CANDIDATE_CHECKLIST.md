# Release Candidate Checklist

## Build
- [ ] Clean CMake configure
- [ ] Clean Release build
- [ ] Apple Silicon tested
- [ ] Intel Mac tested if supported

## Audio
- [ ] Loopback routing works
- [ ] No dropouts at 48k/256
- [ ] No NaN/Inf in stress tests
- [ ] Latency calibration works

## Plugins
- [ ] AU validation
- [ ] VST3 validation
- [ ] Sender/Return recall
- [ ] Automation test

## Product
- [ ] Installer built
- [ ] Signed
- [ ] Notarized
- [ ] Docs complete
- [ ] Beta feedback form ready
# Git Checkpoint Instructions

```bash
git checkout -b integrate-tapecore-engine
git add .
git commit -m "Integrate TapeCore as ConsoleCore analog path subsystem"
```

Recommended follow-up commits:

```bash
git commit -am "Wire TapeCore into ProductionDspEngine"
git commit -am "Add tape profile loading and runtime settings"
git commit -am "Expose tape telemetry to ConsoleCore UI state"
git commit -am "Add per-stem tape enable and print tape routing"
```

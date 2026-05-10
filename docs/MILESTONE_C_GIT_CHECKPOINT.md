# Git Checkpoint Instructions

After copying Milestone C files into the ConsoleCore repo:

```bash
git checkout -b milestone-c-analog-physics
git add .
git commit -m "Milestone C: add distributed analog physics core"
git status
```

Recommended follow-up commits:

```bash
git commit -am "Integrate analog physics core into main audio callback"
git commit -am "Add offline physics stability tests"
git commit -am "Add generic console physics profiles"
```
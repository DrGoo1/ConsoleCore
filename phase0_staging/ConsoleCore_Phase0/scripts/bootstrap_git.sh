#!/usr/bin/env bash
set -euo pipefail

if [ ! -d .git ]; then
  git init
fi

git add .
git commit -m "Initial ConsoleCore Phase 0 scaffold" || true

echo "Git repository initialized."
echo "Next: create an empty GitHub repo, then run:"
echo "git remote add origin git@github.com:YOUR_ACCOUNT/ConsoleCore.git"
echo "git branch -M main"
echo "git push -u origin main"

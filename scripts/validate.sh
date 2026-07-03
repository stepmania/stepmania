#!/usr/bin/env bash
set -euo pipefail

bash -n autogen.sh Utils/*.sh Xcode/Libraries/guard-ffmpeg.sh src/libtomcrypt/*.sh
git diff --check

if [[ -d build ]]; then
  cmake --build build -- -j2
else
  echo "No build directory found; skipping native build check."
fi

echo "StepMania validation passed."

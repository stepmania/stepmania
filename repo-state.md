# repo-state.md

- repo: stepmania
- path: /home/lancer1977/code/stepmania
- updated_utc: 2026-07-03T00:00:00Z
- canonical_tracker: GitHub Issues
- operational_surface: Repo-local docs
- stable_map: docs/index.md
- local_validation: bash scripts/validate.sh

## Current policy

- Durable work lives in GitHub Issues.
- Repo-local docs stay compact and point to the current entrypoints.
- Generated or disposable artifacts belong under `.devstudio/`.
- `scripts/validate.sh` is the lightweight local gate for shell helper syntax,
  whitespace errors, and an optional native build when a `build/` tree exists.
- 2026-07-03 validation: `bash scripts/validate.sh` passed; no `build/` tree
  was present, so the native build check was skipped by design.

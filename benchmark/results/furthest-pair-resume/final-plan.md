# Adopted-header confirmation

Initial layout/wrapper evidence is retained unchanged in `initial/`.
The adopted production SHA256 is
`b2e6e51f3dd36fb7cd7e8e5883847d02d86d2d2e0e4836a337ac97dfda9c5cef`;
the baseline is the initial snapshot
`37dc37add43d42f2a8710852c0f3cb35e0c737baab00080a4362448b3219df4d`.
Only the N<=2/all-equal fast return and explicit matching type/bounds checks
change. General hull/calipers/index recovery remain the same algorithm.

The paired experiment uses direct `blueberry::furthest_pair` calls from those
exact headers, not the initial wrapper. Common C++ harness, convex_hull and
FastIO are byte-identical between include roots. Final sources are copied from
the frozen live production files. Both the just-copied source/header hashes and
live files at the end of preparation are checked against requested hashes;
current live dependencies must equal the final snapshots before metadata seals.
Compilation remains sequential, one compiler at a time.

Approved bounded matrix: same19 kernel cells (nine large distributions at
N2048/200000 plus tiny batch), GCC14/Clang19, release/assert with exactly the
initial flags, baseline/final, one warmup +five measured alternating samples.
This is912 profiles:152 warmups,760 measured,76 before/after comparisons.
All raw rows and min/median/max are retained. Construction/calipers/recovery
proxy fields still emitted by the unchanged harness are retained but are not
interpreted as modified production stages. Only direct full-call elapsed is
used to confirm adoption. No repeat of the unrelated layout or I/O matrix.

Twelve binaries: eight uninstrumented timing binaries and four separate GCC
allocation binaries. Each receives the1,258-case all-pairs gate;120 small public
call profiles cross-check input, objective and H identities. Six assert binaries
must specifically SIGABRT for an invalid singleton. Memory/allocation diagnostic
is bounded to all-same/tiny/late-different, both modes and both headers:12 rows.
Instrumented timing values are never used for performance conclusions.

The same quiet-window protocol and shared-host limitations apply as in
`plan.md`. Do not start timing while the root's full/focused correctness jobs
are active. Production tests bind to the same frozen header. The initial
measurement is not pooled with this paired follow-up.

Commands executed (prepare only before final timing authorization):

```sh
source /tmp/blueberry-setup/env.sh
ulimit -c 0
python3 benchmark/furthest-pair-final.py prepare --production-sha256 b2e6e51f3dd36fb7cd7e8e5883847d02d86d2d2e0e4836a337ac97dfda9c5cef
# After all other workers are quiet and the parent authorizes:
python3 benchmark/furthest-pair-final.py run
python3 benchmark/furthest-pair-final.py diagnostics
python3 benchmark/furthest-pair-report.py
```

The runner intentionally refuses an existing final experiment or measurement.
The exact compiler/check/sample commands and snapshots in `final/` are the
reproduction specification; do not overwrite saved evidence to replay commands.

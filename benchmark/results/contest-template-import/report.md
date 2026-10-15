# Contest template import measurements

5 runs, fixed seed12345, WSL2; see environment.json and validation.json for machine/compiler/flags.
Allocation+construction and pre-generated query loop are timed separately. No parse/format time is included in table timings.
For compression, the build column is the complete compression operation, including rank preparation and remapping; the input-fixture copy is outside the timed region.
Compression has no query phase: preserved legacy CSV zeros in query_seconds were absent-phase placeholders, not measured zero durations. They are reported as not_applicable/null and shown as — below. Future CSV output leaves that field empty.
This is a reporting-only correction; timers and preserved measurements are unchanged and no benchmark rerun was performed.
Shared desktop host timings are descriptive, especially small differences; each source/flag/input set is kept separate.

| compiler/revision | variant | construction or full compression median / min (ms) | query median / min (ms) |
| --- | --- | ---: | ---: |
| g++-before-raw | original-com | 12.938 / 12.568 | 9.972 / 9.764 |
| g++-before-raw | acl-dynamic-combinations | 8.003 / 7.663 | 7.541 / 6.771 |
| g++-before-raw | acl-static-combinations | 5.606 / 5.557 | 6.655 / 5.028 |
| g++-before-raw | original-map-compress | 58.962 / 57.464 | — |
| g++-before-raw | sorted-compress | 25.129 / 24.849 | — |
| g++-raw-candidate | original-com | 13.213 / 12.969 | 10.678 / 9.715 |
| g++-raw-candidate | acl-dynamic-combinations | 7.976 / 7.518 | 6.947 / 6.332 |
| g++-raw-candidate | acl-static-combinations | 5.617 / 5.584 | 6.448 / 5.330 |
| g++-raw-candidate | original-map-compress | 65.738 / 57.075 | — |
| g++-raw-candidate | sorted-compress | 25.568 / 24.847 | — |
| g++-final | original-com | 16.095 / 14.953 | 11.917 / 11.072 |
| g++-final | acl-dynamic-combinations | 9.149 / 8.999 | 7.996 / 7.529 |
| g++-final | acl-static-combinations | 6.899 / 6.589 | 7.347 / 6.499 |
| g++-final | original-map-compress | 73.590 / 70.016 | — |
| g++-final | sorted-compress | 30.350 / 28.704 | — |
| clang++-before-raw | original-com | 12.891 / 12.343 | 10.661 / 9.737 |
| clang++-before-raw | acl-dynamic-combinations | 9.356 / 9.118 | 6.598 / 6.338 |
| clang++-before-raw | acl-static-combinations | 7.744 / 7.450 | 5.368 / 5.036 |
| clang++-before-raw | original-map-compress | 36.245 / 35.294 | — |
| clang++-before-raw | sorted-compress | 16.689 / 16.447 | — |
| clang++-raw-candidate | original-com | 12.301 / 11.816 | 10.653 / 10.237 |
| clang++-raw-candidate | acl-dynamic-combinations | 10.223 / 9.636 | 6.356 / 6.338 |
| clang++-raw-candidate | acl-static-combinations | 7.939 / 7.864 | 4.676 / 4.588 |
| clang++-raw-candidate | original-map-compress | 39.928 / 37.237 | — |
| clang++-raw-candidate | sorted-compress | 17.531 / 17.375 | — |
| clang++-final | original-com | 12.232 / 11.888 | 9.441 / 9.264 |
| clang++-final | acl-dynamic-combinations | 10.123 / 9.429 | 6.726 / 6.127 |
| clang++-final | acl-static-combinations | 7.949 / 7.633 | 4.470 / 4.310 |
| clang++-final | original-map-compress | 38.182 / 37.962 | — |
| clang++-final | sorted-compress | 17.815 / 17.299 | — |

Same release flags / official maximum case; 3 runs with official checker after each run:

| driver revision | variant | median / min (ms) | peak RSS median (KiB) |
| --- | --- | ---: | ---: |
| fastest-local-before-driver-io.json | fastest-223248 | 215.365 / 210.612 | 81740 |
| fastest-local-before-driver-io.json | blueberry-driver | 605.943 / 431.463 | 89196 |
| fastest-local.json | fastest-223248 | 330.375 / 277.057 | 81716 |
| fastest-local.json | blueberry-driver | 400.175 / 336.531 | 89220 |

The final driver retains query storage (roughly 8 MB for T=1e6) to size the generic table to actual maximum n.
The fastest source has AVX2 target pragmas and a fixed common-prime table bound. Its public code is research-only and is not redistributed.
The initial unmatched assert-flags experiment and ACL multi-TU failure are preserved but excluded from the performance comparison.

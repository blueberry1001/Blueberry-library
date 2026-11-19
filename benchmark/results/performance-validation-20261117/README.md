# Targeted verification, 2026-11-17

The recorded run used an isolated `git archive` of `efa8b4e` with the modified
`aggregate-queue.hpp`, `factorize.hpp`, their documentation and the added math
boundary test copied in. It ran each of the three listed official targets once.
The revision printed by the metrics tool identifies the enclosing repository's
HEAD, not a commit containing these uncommitted changes. Other working-tree
changes were not included. This is not a full `make verify` run.

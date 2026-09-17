# Legacy migration implementation plan

1. Inspect legacy contracts and modern replacements; check clean main, remote and open PRs.
2. Create codex/legacy-library-migration. Record design, split source ownership.
3. Write failing boundary/random tests, then implement each replacement with complete API docs.
4. Add official verifiers and independently check numeric, lazy, ownership and sparse-memory contracts.
5. Compare relevant alternatives under identical compiler/input conditions; retain raw logs and provenance.
6. Register headers and search aliases; add migration guide and update current docs/policy.
7. After replacement checks, remove seven obsolete root headers, update discovery and references.
8. Run GCC/Clang C++20/23 checks, sanitizer, all official verifiers, docs generation and site checks.
9. Independent code review; fix issues and rerun affected checks. Commit cohesive changes.
10. Push, create PR, inspect CI, merge when green, verify main deployment and public migration links.

No breaking change outside the explicitly authorized obsolete root headers is intended.

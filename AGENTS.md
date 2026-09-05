# Contributor instructions

Read IMPLEMENTATION_POLICY.md before changing a library. Standard ACL features
are ACL-first; prioritize non-ACL functionality. Preserve legacy compatibility.

For every new or changed public API:

- Maintain docs/<category>/<name>.md with documentation_of pointing to the header.
- Use templates/library.md.example: overview/preconditions/memory, a compilable
  minimal C++ main with assertions, and an operation table with call + complexity.
- Document every public constructor, overload, operation and exposed field.
- Provide a <details class="api-operation" id="..." markdown="1"> per operation
  (including <summary>signature — complexity</summary>, example and caveats).
- State bounds, return values, empty cases, type/algebra assumptions, overflow,
  lifetime/invalidation and amortized complexity when relevant.
- Wrap fenced code examples with Liquid raw/endraw tags outside the fences, so
  C++ nested initialization braces are not interpreted as Liquid templates.
- Register the library in .verify-helper/docs/static/_data/libraries.yml.
- Add official verify coverage and boundary/randomized tests as appropriate.
- Run make test, make verify and make docs. Do not claim unrun tests passed.
- Judge performance only under comparable inputs/compiler/environment; preserve
  measurement logs and do not replace failed/missing measurements with zero.

Keep GitHub Pages/verification-helper as the publishing system. Do not restore
the upstream minimal-theme fixed sidebar or its inline theme_fix width rules.
Never auto-merge a new PR without explicit user authorization for that PR.

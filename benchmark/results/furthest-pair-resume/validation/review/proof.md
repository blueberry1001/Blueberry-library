# Independent furthest_pair proof review

Reviewed production header SHA256 `37dc37add43d42f2a8710852c0f3cb35e0c737baab00080a4362448b3219df4d` at checkpoint `da912761b09e4c4cf6d67d2c154c03672ca8658b`. Its dependency convex-hull.hpp has SHA256 `cdc6aaa2c1cd1c913be5f3b0fe2864295e0677f82740419c08ed03bbd299859e`.

No correctness, API, include or complexity defect was found. This is proof/source review plus a bounded independent check, not final batch validation or benchmark clearance.

## Preconditions and reduction

convex_hull instantiates its signed-integral, at-most-64-bit static assertion regardless of input size. furthest_pair asserts N <= INT_MAX before copying; convex_hull checks every coordinate before any empty/singleton return. The early returns therefore do not evade the coordinate precondition. In release these remain caller preconditions.

The hull removes duplicate coordinates and all intermediate collinear boundary points. For H >= 3 it is strict, CCW, with consecutive edge directions turning through angles strictly between 0 and pi. A maximum squared distance on a convex polygon is attained by a pair of vertices (maximize the convex squared-distance function over one endpoint at a time). Thus removing interior, duplicate and collinear intermediate coordinates preserves the maximum value.

## Support invariant and advance bound

Write h_i for the strict hull in CCW order and e_i = h_(i+1)-h_i. Extend its edge angles as strictly increasing real values theta_i with theta_(i+H)=theta_i+2pi. Every consecutive gap is in (0,pi). The index j in code is a wrapped representation of an implicitly unwrapped index J.

At the end of the while loop for edge i, J is the first edge index with theta_J >= theta_i+pi. Equivalently, the support-height sequence A_i(k)=cross(e_i,h_k-h_i) has its first maximum at J. Its successive difference is exactly cross(e_i,e_k), the expression computed by change(k).

For i=0, J starts at 1, before the opposite support because theta_1-theta_0 is in (0,pi). Cross products are positive until the first direction reaches theta_0+pi. For the next edge, the threshold increases; the old J never exceeds the new first crossing and remains ahead of the new starting edge. It therefore either stays or advances. The angular gap of a step is less than pi, so it cannot skip to the same-direction minimum or advance through a complete revolution for a single edge.

In particular i+1 <= J < i+H. Across all H edges, J is nondecreasing, starts at 1 and ends below 2H-1. The number of increments is consequently less than 2H (indeed at most 2H-3 for integer indices). Alternatively, if J0 is the first edge's support, the last support is at most J0+H; initialization plus the sweep is at most H+J0-1 < 2H. Each edge performs O(1) work outside the while loop, so the total caliper work is O(H). No claim of at most one revolution measured from the initial j=1 is needed.

This uses support-height monotonicity, not distance unimodality. Pairwise distances around a convex hull need not be unimodal; the official hack_uso_caliper example directly targets that incorrect assumption.

## Ties and diameter coverage

At this invariant's crossing, change(j)==0 means theta_J=theta_i+pi: the opposite support is an edge parallel to the current one. It is not the same-direction minimum. Strictness excludes three collinear hull vertices, so its plateau has exactly two endpoints. The implementation considers both current-edge endpoints against both opposite-edge endpoints. Without a plateau it considers both against the sole support vertex.

Every diameter pair p,q has parallel supporting lines perpendicular to p-q. No third distinct vertex can share either support line at that orientation: its distance to the other diameter endpoint would be strictly larger. Rotate the support direction while retaining p and q until one endpoint's support first acquires an incident edge. The opposite support still contains the other endpoint, possibly on a tied edge. That event is one of the edge/support combinations considered above. Thus a diameter is included among the candidates, including parallel-support ties.

## Exact arithmetic

Let B=2^62-1 and D=2B=2^63-2. Every coordinate subtraction is widened first and has magnitude <=D. Each product in hull orientation, support change and squared distance has magnitude <=D^2<2^126. Each two-product subtraction or sum has magnitude <=2D^2=2^127-2^66+8, strictly below signed __int128's positive maximum. Negative cross products are likewise above its minimum. The support change is computed as a cross product of two edges, so it does not require a potentially larger intermediate subtraction of already formed areas.

## Degeneracy, original indices and ownership

N<2 returns {-1,-1}. If N>=2 and H=1, every coordinate is equal and {0,1} gives distinct original indices. H=2 chooses the distinct collinear extremes. For H>=3 the maximum distance is positive, hence its selected coordinates are distinct. Both coordinates came from the original input; the recovery scan therefore finds two distinct original indices and normalizes their order. No lexicographically minimal diameter pair is promised.

With N<=INT_MAX, the cast and loop increment remain representable even at N=INT_MAX. Input is passed by const reference and hull construction copies it, so no input element, reference or iterator is modified. All directly used standard facilities have direct includes; the dependency include is intentional. The total bound is O(N log(N+1)) time and O(N) auxiliary memory, with O(1) returned indices.

## Independent executable evidence

`exhaustive.cpp` calls the actual production header, compares results against an independent int256 all-original-index-pairs oracle, and checks input preservation and normalized indices. It covers every subset of two 4x4 grids (small and near +/-B), reversed/duplicated variants, and all subsets of the official non-unimodal-distance trap under eight signed coordinate permutations at large scale. A separate exact-integer height model checks each selected support against a full hull scan, verifies strict/supporting hull edges against every original point, checks antipodal candidate coverage and counts fewer than 2H advances.

One sequential GCC14 gnu++20 build with -O1, warnings-as-errors, UBSan and no sanitizer recovery succeeded; its run passed 197120 cases, 624344 support checks, 306560 parallel ties and 131030 wrapped sweeps. The exhaustive grids produced hulls up to eight vertices. Exact command, source hashes, output and exit statuses are in exhaustive-result.json, compile.log and run.log. This evidence is not an exhaustive proof over all inputs, a four-mode compiler matrix, or a comparative timing result.

An independent collaborating proof reviewer separately examined the frozen header and confirmed the invariant, plateau coverage and arithmetic bounds without code edits or execution. The cached Library Checker correct.cpp and official info.toml were read as primary reference context; no network requests were made.

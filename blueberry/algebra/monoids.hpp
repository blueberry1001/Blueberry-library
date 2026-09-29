#pragma once

#include <algorithm>
#include <cassert>

namespace blueberry::monoid {

template <class T> struct AffineSum {
    struct S { T sum; long long len; };
    struct F { T a, b; };
    static S op(S x, S y) { return {x.sum + y.sum, x.len + y.len}; }
    static S e() { return {T(0), 0}; }
    static S leaf(T x) { return {x, 1}; }
    static S mapping(F f, S x) { return {f.a * x.sum + f.b * T(x.len), x.len}; }
    static F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
    static F id() { return {T(1), T(0)}; }
};

// Absolute indices keep an arithmetic-progression update independent of tree nodes.
template <class T> struct IndexAffineSum {
    struct S { T sum, index_sum; long long len; };
    struct F { T a, b, c; };
    static S op(S x, S y) {
        return {x.sum + y.sum, x.index_sum + y.index_sum, x.len + y.len};
    }
    static S e() { return {T(0), T(0), 0}; }
    static S leaf(T x, long long i) { return {x, T(i), 1}; }
    static S mapping(F f, S x) {
        return {f.a * x.sum + f.b * x.index_sum + f.c * T(x.len), x.index_sum, x.len};
    }
    static F composition(F f, F g) {
        return {f.a * g.a, f.a * g.b + f.b, f.a * g.c + f.c};
    }
    static F id() { return {T(1), T(0), T(0)}; }
};

template <class T> struct AffineSumSquares {
    struct S { T sum, square_sum; long long len; };
    struct F { T a, b; };
    static S op(S x, S y) {
        return {x.sum + y.sum, x.square_sum + y.square_sum, x.len + y.len};
    }
    static S e() { return {T(0), T(0), 0}; }
    static S leaf(T x) { return {x, x * x, 1}; }
    static S mapping(F f, S x) {
        return {f.a * x.sum + f.b * T(x.len),
                f.a * f.a * x.square_sum + T(2) * f.a * f.b * x.sum +
                    f.b * f.b * T(x.len), x.len};
    }
    static F composition(F f, F g) { return {f.a * g.a, f.a * g.b + f.b}; }
    static F id() { return {T(1), T(0)}; }
};

struct BinaryFlipInversions {
    struct S { long long zero, one, inversions; };
    using F = bool;
    static S op(S x, S y) {
        return {x.zero + y.zero, x.one + y.one,
                x.inversions + y.inversions + x.one * y.zero};
    }
    static S e() { return {0, 0, 0}; }
    static S leaf(bool x) { return {x ? 0 : 1, x ? 1 : 0, 0}; }
    static S mapping(F f, S x) {
        return f ? S{x.one, x.zero, x.zero * x.one - x.inversions} : x;
    }
    static F composition(F f, F g) { return f != g; }
    static F id() { return false; }
};

// Empty subarrays are allowed: all-negative intervals have best == 0.
template <class T> struct MaxSubarray {
    struct S { T sum, prefix, suffix, best; };
    static S op(S x, S y) {
        return {x.sum + y.sum, std::max(x.prefix, x.sum + y.prefix),
                std::max(y.suffix, y.sum + x.suffix),
                std::max(std::max(x.best, y.best), x.suffix + y.prefix)};
    }
    static S e() { return {T(0), T(0), T(0), T(0)}; }
    static S leaf(T x) {
        T positive = std::max(T(0), x);
        return {x, positive, positive, positive};
    }
};

struct Bracket {
    struct S { long long sum, min_prefix; };
    static S op(S x, S y) {
        return {x.sum + y.sum, std::min(x.min_prefix, x.sum + y.min_prefix)};
    }
    static S e() { return {0, 0}; }
    static S leaf(char c) {
        assert(c == '(' || c == ')');
        return c == '(' ? S{1, 0} : S{-1, -1};
    }
};

template <class T> struct AffineComposition {
    struct S { T a, b; };
    // Concatenation evaluates the left interval first, then the right interval.
    static S op(S x, S y) { return {y.a * x.a, y.a * x.b + y.b}; }
    static S e() { return {T(1), T(0)}; }
    static S leaf(T a, T b) { return {a, b}; }
};

template <class T> struct MaxCount {
    struct S { T value; long long count; };
    static S op(S x, S y) {
        if (x.count == 0) return y;
        if (y.count == 0) return x;
        if (x.value < y.value) return y;
        if (y.value < x.value) return x;
        return {x.value, x.count + y.count};
    }
    static S e() { return {T{}, 0}; }
    static S leaf(T x) { return {x, 1}; }
};

} // namespace blueberry::monoid

#pragma once

// Backward-compatible include path.  New code should include the namespaced
// header directly.
#include "math/formal-power-series.hpp"

using blueberry::FormalPowerSeries;
using atcoder::modint998244353;
using mint = atcoder::modint998244353;
using FPS = blueberry::FormalPowerSeries<mint>;
using sfps = FPS::Sparse;

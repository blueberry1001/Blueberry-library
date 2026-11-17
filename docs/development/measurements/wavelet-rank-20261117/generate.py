from pathlib import Path

root=Path.cwd()
here=root/'.build/wavelet-rank-study'
source=(root/'docs/development/measurements/wavelet-rank-20261117/current.hpp.txt').read_text()
old='''  bool direct_ = false;
  std::vector<int> dense_;'''
new='''  struct RankBlock { std::uint64_t bits = 0; int prefix = 0; };
  bool direct_ = false;
  std::size_t dense_span_ = 0;
  std::vector<RankBlock> dense_;'''
assert old in source
packed=source.replace(old,new)
old='''      return d >= dense_.size() ? static_cast<int>(values_.size()) : dense_[static_cast<std::size_t>(d)];'''
new='''      if (d > dense_span_) return static_cast<int>(values_.size());
      const auto& block = dense_[static_cast<std::size_t>(d) / 64];
      return block.prefix + std::popcount(block.bits & ((std::uint64_t{1} << (static_cast<std::size_t>(d) % 64)) - 1));'''
assert old in packed
packed=packed.replace(old,new)
old='''            std::vector<int> mapping(static_cast<std::size_t>(remaining) + 1, -1);
            for (const auto& value : a) mapping[U(static_cast<U>(value) - base)] = 0;
            int distinct = 0;
            for (int& value : mapping) { bool present = value == 0; value = distinct; distinct += present; }
            for (int i = 0; i < n_; ++i) { current[i] = mapping[U(static_cast<U>(a[i]) - base)]; values_[current[i]] = a[i]; }
            values_.resize(distinct);
            if (static_cast<std::size_t>(distinct) != mapping.size()) dense_.swap(mapping);'''
new='''            const std::size_t span = static_cast<std::size_t>(remaining);
            std::vector<RankBlock> mapping(span / 64 + 1);
            for (const auto& value : a) {
              const std::size_t d = U(static_cast<U>(value) - base);
              mapping[d / 64].bits |= std::uint64_t{1} << (d % 64);
            }
            int distinct = 0;
            for (auto& block : mapping) { block.prefix = distinct; distinct += std::popcount(block.bits); }
            for (int i = 0; i < n_; ++i) {
              const std::size_t d = U(static_cast<U>(a[i]) - base);
              const auto& block = mapping[d / 64];
              current[i] = block.prefix + std::popcount(block.bits & ((std::uint64_t{1} << (d % 64)) - 1));
              values_[current[i]] = a[i];
            }
            values_.resize(distinct);
            if (static_cast<std::size_t>(distinct) != span + 1) { dense_span_ = span; dense_.swap(mapping); }'''
assert old in packed
packed=packed.replace(old,new)
old='''    return count_less(l, r, hi, first) - count_less(l, r, lo, first);'''
new='''    const auto& level = levels_[first];
    const int ones_l = level.rank(l), ones_r = level.rank(r);
    const int zl = l - ones_l, zr = r - ones_r;
    return zr - zl - count_less(zl, zr, lo, first + 1)
      + count_less(level.zeros + ones_l, level.zeros + ones_r, hi, first + 1);'''
assert old in source
divergence=source.replace(old,new)
combined=packed.replace(old,new)
hybrid=combined.replace('  std::vector<RankBlock> dense_;', '  std::vector<RankBlock> dense_;\n  std::vector<int> small_;')
hybrid=hybrid.replace('    if (direct_ || !dense_.empty()) {', '    if (direct_ || !dense_.empty() || !small_.empty()) {')
hybrid=hybrid.replace('''      if (d > dense_span_) return static_cast<int>(values_.size());
      const auto& block = dense_[static_cast<std::size_t>(d) / 64];''','''      if (d > dense_span_) return static_cast<int>(values_.size());
      if (!small_.empty()) return small_[static_cast<std::size_t>(d)];
      const auto& block = dense_[static_cast<std::size_t>(d) / 64];''')
hybrid=hybrid.replace('''            if (static_cast<std::size_t>(distinct) != span + 1) { dense_span_ = span; dense_.swap(mapping); }''','''            if (static_cast<std::size_t>(distinct) != span + 1) {
              dense_span_ = span;
              if (distinct <= 256) {
                std::vector<int> table(span + 1);
                std::size_t start = 0;
                int rank = 0;
                for (T value : values_) {
                  const std::size_t end = U(static_cast<U>(value) - base);
                  std::fill(table.begin() + start, table.begin() + end + 1, rank++);
                  start = end + 1;
                }
                small_.swap(table);
              } else dense_.swap(mapping);
            }''')
hybrid4096=hybrid.replace('distinct <= 256', 'distinct <= 4096')
hybrid_eq=hybrid.replace('    const int lo = lower(lower_value), hi = lower(upper);', '    if (lower_value == upper) return 0;\n    const int lo = lower(lower_value), hi = lower(upper);')
hybrid4096_eq=hybrid4096.replace('    const int lo = lower(lower_value), hi = lower(upper);', '    if (lower_value == upper) return 0;\n    const int lo = lower(lower_value), hi = lower(upper);')
for name,text in [('current',source),('packed',packed),('divergence',divergence),('combined',combined),('hybrid',hybrid),('hybrid4096',hybrid4096),('hybrid-eq',hybrid_eq),('hybrid4096-eq',hybrid4096_eq)]:
    directory=here/name
    directory.mkdir(parents=True,exist_ok=True)
    (directory/'wavelet-matrix.hpp').write_text(text,newline='\n')

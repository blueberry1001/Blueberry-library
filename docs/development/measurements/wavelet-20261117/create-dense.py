from pathlib import Path

root=Path.cwd()
base=(root/'docs/development/measurements/wavelet-20261117/final-prerank.hpp.txt').read_text()
old='''  int lower(const T& x) const {
    return static_cast<int>(std::lower_bound(values_.begin(), values_.end(), x) - values_.begin());
  }'''
new='''  int lower(const T& x) const {
    if (direct_) {
      if (x < values_.front()) return 0;
      using U = std::make_unsigned_t<T>;
      U d = U(U(x) - U(values_.front()));
      return d >= values_.size() ? static_cast<int>(values_.size()) : static_cast<int>(d);
    }
    return static_cast<int>(std::lower_bound(values_.begin(), values_.end(), x) - values_.begin());
  }'''
assert old in base
contiguous=base.replace('  int height_ = 0;', '  int height_ = 0;\n  bool direct_ = false;').replace(old,new)
key='    height_ = std::bit_width(static_cast<unsigned>(values_.size() - 1));'
assign='''    using DirectU = std::make_unsigned_t<T>;
    direct_ = DirectU(DirectU(values_.back()) - DirectU(values_.front())) == DirectU(values_.size() - 1);
'''
assert key in contiguous
contiguous=contiguous.replace(key,assign+key)
dense=contiguous.replace('  bool direct_ = false;', '  bool direct_ = false;\n  std::vector<int> dense_;')
dense=dense.replace('''    if (direct_) {
      if (x < values_.front()) return 0;
      using U = std::make_unsigned_t<T>;
      U d = U(U(x) - U(values_.front()));
      return d >= values_.size() ? static_cast<int>(values_.size()) : static_cast<int>(d);
    }''','''    if (direct_ || !dense_.empty()) {
      if (x < values_.front()) return 0;
      using U = std::make_unsigned_t<T>;
      U d = U(U(x) - U(values_.front()));
      if (direct_) return d >= values_.size() ? static_cast<int>(values_.size()) : static_cast<int>(d);
      return d >= dense_.size() ? static_cast<int>(values_.size()) : dense_[static_cast<std::size_t>(d)];
    }''')
dense=dense.replace('std::uint64_t(n_) * 2','std::uint64_t(n_) * 8')
dense=dense.replace('''            int distinct = 0;
            for (int& value : mapping) if (value == 0) value = distinct++;
            for (int i = 0; i < n_; ++i) { current[i] = mapping[U(static_cast<U>(a[i]) - base)]; values_[current[i]] = a[i]; }
            values_.resize(distinct);''','''            int distinct = 0;
            for (int& value : mapping) { bool present = value == 0; value = distinct; distinct += present; }
            for (int i = 0; i < n_; ++i) { current[i] = mapping[U(static_cast<U>(a[i]) - base)]; values_[current[i]] = a[i]; }
            values_.resize(distinct);
            if (static_cast<std::size_t>(distinct) != mapping.size()) dense_.swap(mapping);''')
for name,text in [('contiguous',contiguous),('dense',dense)]:
  directory=root/'.build/wavelet-investigation'/name
  directory.mkdir(parents=True,exist_ok=True)
  (directory/'wavelet-matrix.hpp').write_text(text,newline='\n')

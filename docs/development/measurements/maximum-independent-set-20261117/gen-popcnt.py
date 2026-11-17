from pathlib import Path
p=Path('.')
s=p.joinpath('components.hpp').read_text()
s=s.replace('inline std::vector<int> maximum_independent_set(int n, const std::vector<std::pair<int, int>>& edges) {', '''#if defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
__attribute__((always_inline))
#endif
inline std::vector<int> maximum_independent_set_impl(int n, const std::vector<std::pair<int, int>>& edges) {''')
assert s.endswith('}\n}\n')
s=s[:-2]+'''#if defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
__attribute__((target("popcnt"))) inline std::vector<int> maximum_independent_set_popcnt(int n, const std::vector<std::pair<int, int>>& edges) {
  return maximum_independent_set_impl(n, edges);
}
#endif
inline std::vector<int> maximum_independent_set(int n, const std::vector<std::pair<int, int>>& edges) {
#if defined(__clang__) && (defined(__x86_64__) || defined(__i386__))
  if (__builtin_cpu_supports("popcnt")) return maximum_independent_set_popcnt(n, edges);
#endif
  return maximum_independent_set_impl(n, edges);
}
}
'''
p.joinpath('components-popcnt.hpp').write_text(s,encoding='utf-8',newline='\n')

#!/usr/bin/env bash
set -euo pipefail
root="$PWD"
logs="$root/.build/fast-io-validation"
mkdir -p "$logs"
stage=$(mktemp -d /tmp/blueberry-fast-io-validation-XXXXXX)
printf '%s\n' "$stage" > "$logs/staging-directory.txt"
git clone -q --shared "$root" "$stage/repo"
trap 'if [ -f "$stage/repo/.verification/current.json" ]; then cp "$stage/repo/.verification/current.json" "$logs/official-results.json"; fi' EXIT
git ls-files -z | tar --null -T - -cf - | tar -xf - -C "$stage/repo"
for path in benchmark/fast-io-throughput.cpp benchmark/fast-io-throughput.py; do
  cp "$root/$path" "$stage/repo/$path"
done
cp "$root/docs/development/fast-io-throughput.md" "$stage/repo/docs/development/"
cp -a "$root/docs/development/measurements/fast-io-throughput" "$stage/repo/docs/development/measurements/"
cd "$stage/repo"
mkdir -p .deps
cp -a "$root/.deps/ac-library" .deps/ac-library
mkdir -p .verify-helper
ln -s "$root/.verify-helper/cache" .verify-helper/cache
run() {
  local name="$1"; shift
  printf '%s\n' "Running $name"
  set +e
  "$@" > "$logs/$name.log" 2>&1
  local status=$?
  set -e
  printf '%s\n' "$status" > "$logs/$name.exit"
  if [ "$status" -ne 0 ]; then tail -60 "$logs/$name.log"; return "$status"; fi
  tail -3 "$logs/$name.log"
}
export CPPFLAGS="-isystem $PWD/.deps/ac-library"
git rev-parse HEAD > "$logs/revision.txt"
sha256sum blueberry/utility/fast-io.hpp > "$logs/header.sha256"
run make-check make check COMPILE_JOBS=4
run release-include make include-test CXXFLAGS=-DNDEBUG COMPILE_JOBS=4
run make-verify make verify VERIFY_REPEATS=3 VERIFY_TIMEOUT=1200 'VERIFY_PATHS=verify/utility/fast-io.test.cpp verify/utility/fast-io-128bit.test.cpp verify/utility/contest-fast-io.test.cpp verify/utility/contest-fast-io-128bit.test.cpp verify/utility/contest-template-128bit.test.cpp verify/utility/all-headers.test.cpp'
cp .verification/current.json "$logs/official-results.json"
mv .verification/current.json .verification/scoped-current.json
run make-docs make docs OJ_VERIFY_JOBS=4
cp "$root/../../../.verify-helper/markdown/Gemfile.lock" .verify-helper/markdown/Gemfile.lock
run pages-build env JEKYLL_ENV=production PAGES_REPO_NWO=blueberry1001/Blueberry-library BUNDLE_PATH=/home/blueberry/.cache/blueberry-gems BUNDLE_GEMFILE="$PWD/.verify-helper/markdown/Gemfile" bundle exec github-pages build --source "$PWD/.verify-helper/markdown" --destination "$PWD/.build/site"
run check-site python3 scripts/check_site.py --without-metrics

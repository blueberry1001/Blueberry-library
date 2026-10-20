#!/usr/bin/env bash
set -euo pipefail
root="$PWD"
logs="$root/.build/fast-io-validation"
stage=$(cat "$logs/staging-directory.txt")
cp "$root/.verify-helper/docs/_config.yml" "$stage/repo/.verify-helper/docs/_config.yml"
cp "$root/docs/development/fast-io-throughput.md" "$stage/repo/docs/development/"
cp -a "$root/docs/development/measurements/fast-io-throughput/." "$stage/repo/docs/development/measurements/fast-io-throughput/"
cd "$stage/repo"
export PATH="$HOME/.local/bin:$PATH"
export CPPFLAGS="-isystem $PWD/.deps/ac-library"
run() {
  local name="$1"; shift
  printf '%s\n' "Running $name"
  set +e
  "$@" > "$logs/$name.log" 2>&1
  local status=$?
  set -e
  printf '%s\n' "$status" > "$logs/$name.exit"
  if [ "$status" -ne 0 ]; then tail -50 "$logs/$name.log"; return "$status"; fi
  tail -3 "$logs/$name.log"
}
run make-docs make docs OJ_VERIFY_JOBS=4
cp "$root/../../../.verify-helper/markdown/Gemfile.lock" .verify-helper/markdown/Gemfile.lock
run pages-build env JEKYLL_ENV=production PAGES_REPO_NWO=blueberry1001/Blueberry-library BUNDLE_PATH=/home/blueberry/.cache/blueberry-gems BUNDLE_GEMFILE="$PWD/.verify-helper/markdown/Gemfile" bundle exec github-pages build --source "$PWD/.verify-helper/markdown" --destination "$PWD/.build/site"
run check-site python3 scripts/check_site.py --without-metrics
printf '0\n' > "$logs/completed.exit"

OJ_VERIFY_JOBS ?= 2
VERIFY_REPEATS ?= 3
CXX ?= g++
CXX_STANDARD ?= gnu++20
COMPILE_JOBS ?= 2
RANDOM_RUNS ?= 20
RANDOM_SEED ?= 1
RANDOM_TIMEOUT ?= 5
ACL_ROOT ?= .deps/ac-library
CPPFLAGS ?= -I$(ACL_ROOT)

.PHONY: setup setup-acl check test compile-test include-test verify-compile random-test verify benchmark benchmark-smoke benchmark-baseline research-static-rmq docs bundle

setup: setup-acl
	python3 -m pip install --requirement requirements-dev.txt

setup-acl:
	@test -d "$(ACL_ROOT)/atcoder" || git clone --depth 1 https://github.com/atcoder/ac-library.git "$(ACL_ROOT)"

check: test compile-test random-test

compile-test:
	CPPFLAGS="$(CPPFLAGS)" python3 scripts/check_cpp.py compile --compiler "$(CXX)" --standard "$(CXX_STANDARD)" --jobs "$(COMPILE_JOBS)"

include-test:
	CPPFLAGS="$(CPPFLAGS)" python3 scripts/check_cpp.py headers --compiler "$(CXX)" --standard "$(CXX_STANDARD)" --jobs "$(COMPILE_JOBS)"

verify-compile:
	CPPFLAGS="$(CPPFLAGS)" python3 scripts/check_cpp.py verify --compiler "$(CXX)" --standard "$(CXX_STANDARD)" --jobs "$(COMPILE_JOBS)"

random-test:
	CPPFLAGS="$(CPPFLAGS)" python3 scripts/check_cpp.py random --compiler "$(CXX)" --standard "$(CXX_STANDARD)" --jobs "$(COMPILE_JOBS)" --random-runs "$(RANDOM_RUNS)" --random-seed "$(RANDOM_SEED)" --random-timeout "$(RANDOM_TIMEOUT)"

verify:
	CPLUS_INCLUDE_PATH="$(abspath $(ACL_ROOT))$(if $(CPLUS_INCLUDE_PATH),:$(CPLUS_INCLUDE_PATH))" python3 scripts/verify_with_metrics.py --repeats $(VERIFY_REPEATS)

benchmark:
	python3 scripts/run_benchmarks.py

benchmark-smoke:
	python3 scripts/run_benchmarks.py --flag-set portable --repeats 1 --warmups 0 \
		--target 'static-rmq-random-*' --output .benchmark/results/smoke

benchmark-baseline:
	CPLUS_INCLUDE_PATH="$(abspath $(ACL_ROOT))$(if $(CPLUS_INCLUDE_PATH),:$(CPLUS_INCLUDE_PATH))" python3 scripts/verify_with_metrics.py --repeats $(VERIFY_REPEATS) --save-baseline

research-static-rmq:
	python3 scripts/fetch_lc_submissions.py --problem staticrmq --user blueberry1001 \
		--limit 5 --analyze-top 1 --output .benchmark/library-checker/staticrmq

test:
	python3 -m unittest discover -s tests -v
	CXX="$(CXX)" CXX_STANDARD="$(CXX_STANDARD)" CPPFLAGS="$(CPPFLAGS)" python3 scripts/check_docs.py

docs:
	CPLUS_INCLUDE_PATH="$(abspath $(ACL_ROOT))$(if $(CPLUS_INCLUDE_PATH),:$(CPLUS_INCLUDE_PATH))" python3 scripts/generate_docs.py -j $(OJ_VERIFY_JOBS)
	python3 scripts/publish_metrics.py

bundle:
	@test -n "$(SOURCE)" || (echo "usage: make bundle SOURCE=main.cpp"; exit 1)
	oj-bundle "$(SOURCE)" -I . -I "$(ACL_ROOT)"

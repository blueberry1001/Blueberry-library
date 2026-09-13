OJ_VERIFY_JOBS ?= 2
VERIFY_REPEATS ?= 3

.PHONY: setup test verify benchmark benchmark-smoke benchmark-baseline research-static-rmq docs bundle

setup:
	python3 -m pip install --requirement requirements-dev.txt

verify:
	python3 scripts/verify_with_metrics.py --repeats $(VERIFY_REPEATS)

benchmark:
	python3 scripts/run_benchmarks.py

benchmark-smoke:
	python3 scripts/run_benchmarks.py --flag-set portable --repeats 1 --warmups 0 \
		--target 'static-rmq-random-*' --output .benchmark/results/smoke

benchmark-baseline:
	python3 scripts/verify_with_metrics.py --repeats $(VERIFY_REPEATS) --save-baseline

research-static-rmq:
	python3 scripts/fetch_lc_submissions.py --problem staticrmq --user blueberry1001 \
		--limit 5 --analyze-top 1 --output .benchmark/library-checker/staticrmq

test:
	python3 -m unittest discover -s tests -v
	python3 scripts/check_docs.py

docs:
	python3 scripts/generate_docs.py -j $(OJ_VERIFY_JOBS)
	python3 scripts/publish_metrics.py

bundle:
	@test -n "$(SOURCE)" || (echo "usage: make bundle SOURCE=main.cpp"; exit 1)
	oj-bundle "$(SOURCE)" -I .

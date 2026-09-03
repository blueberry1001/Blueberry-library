OJ_VERIFY_JOBS ?= 2

.PHONY: setup verify docs bundle

setup:
	python3 -m pip install --requirement requirements-dev.txt

verify:
	oj-verify run -j $(OJ_VERIFY_JOBS)

docs:
	oj-verify docs -j $(OJ_VERIFY_JOBS)

bundle:
	@test -n "$(SOURCE)" || (echo "usage: make bundle SOURCE=main.cpp"; exit 1)
	oj-bundle "$(SOURCE)" -I .

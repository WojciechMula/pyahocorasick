.SUFFIXES:
.PHONY: test test_unicode test_bytes build build_unicode build_bytes clean valgrind benchmark

DEPS=src/*.c \
     src/*.h \
     setup.py \
     tests/*.py

build: build_unicode

build_unicode: $(DEPS) venv/bin/activate
	@rm -rf dist build *.so
	AHOCORASICK_UNICODE=yes venv/bin/pip install -e .

build_bytes: $(DEPS) venv/bin/activate
	@rm -rf dist build *.so
	AHOCORASICK_BYTES=yes venv/bin/pip install -e .

test:  test_unicode

test_unicode:  $(DEPS) build_unicode
	@rm -rf dist build *.so
	AHOCORASICK_UNICODE=yes venv/bin/pip install -e .[testing]
	venv/bin/pytest -vvs

test_bytes: $(DEPS) build_bytes
	@rm -rf dist build *.so
	AHOCORASICK_BYTES=yes venv/bin/pip install -e .[testing]
	venv/bin/pytest -vvs

venv/virtualenv.pyz:
	@mkdir -p venv
	@curl -o venv/virtualenv.pyz https://bootstrap.pypa.io/virtualenv/virtualenv.pyz

venv/bin/activate: venv/virtualenv.pyz
	python3 venv/virtualenv.pyz venv

benchmark: etc/benchmarks/benchmark.py build
	python3 $^

valgrind:
	python -c "import sys;print(sys.version)"
	valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log venv/bin/pytest -vvs

clean:
	rm -f stamp/*
	rm -rf dist build venv

.SUFFIXES:
.PHONY: test clean valgrind benchmark

DEPS=src/*.c \
     src/*.h \
     setup.py \
     tests/*.py

test:  $(DEPS) build 
	venv/bin/pytest -vvs

build: $(DEPS) venv/bin/activate
	venv/bin/pip install -e .[testing]

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

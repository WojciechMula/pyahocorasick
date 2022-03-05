.SUFFIXES:
.PHONY: test clean valgrind

export PYTHONPATH := .:$(PYTHONPATH):$(PATH)

DEPS=src/*.c \
     src/*.h \
     setup.py \
     tests/test_unit.py

test3: stamp/regression_py3

test: stamp/regression_py2 stamp/regression_py3

stamp/build_py2: $(DEPS)
	python2 setup.py build_ext --inplace
	touch $@

stamp/unittests_py2: stamp/build_py2
	python2 tests/test_unit.py
	touch $@

stamp/regression_py2: stamp/unittests_py2 
	python2 tests/test_issue_5.py
	python2 tests/test_issue_8.py
	python2 tests/test_issue_9.py
	python2 tests/test_issue_10.py
	python2 tests/test_issue_26.py
	python2 tests/test_issue_56.py
	touch $@


stamp/build_py3: $(DEPS)
	python3 setup.py build_ext --inplace
	touch $@

stamp/unittests_py3: stamp/build_py3
	python3 tests/test_unit.py
	touch $@

stamp/regression_py3: stamp/unittests_py3
	python3 tests/test_issue_5.py
	python3 tests/test_issue_8.py
	python3 tests/test_issue_9.py
	python3 tests/test_issue_10.py
	python3 tests/test_issue_26.py
	python3 tests/test_issue_56.py
	touch $@


benchmark: etc/benchmarks/benchmark.py stamp/build_py3
	python3 $^

devbuild2:
	python2 setup.py build_ext --inplace

devbuild3:
	python3 setup.py build_ext --inplace

valgrind:
	python -c "import sys;print(sys.version)"
	valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log python tests/test_unit.py

pip-release:
	python setup.py sdist upload

dist:
	python setup.py sdist

clean:
	rm -f stamp/*
	rm -rf dist build

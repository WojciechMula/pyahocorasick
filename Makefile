.SUFFIXES:
.PHONY: test clean valgrind

export PYTHONPATH := .:$(PYTHONPATH):$(PATH)

DEPS=*.c \
     *.h \
     setup.py \
     unittests.py

test: stamp/regression_py2 stamp/regression_py3

stamp/build_py2: $(DEPS)
	python2 setup.py build_ext --inplace
	touch $@

stamp/unittests_py2: stamp/build_py2
	python2 unittests.py
	touch $@

stamp/regression_py2: stamp/unittests_py2 
	python2 regression/issue_5.py
	python2 regression/issue_8.py
	python2 regression/issue_9.py
	python2 regression/issue_10.py
	python2 regression/issue_26.py
	python2 regression/issue_56.py
	touch $@


stamp/build_py3: $(DEPS)
	python3 setup.py build_ext --inplace
	touch $@

stamp/unittests_py3: stamp/build_py3
	python3 unittests.py
	touch $@

stamp/regression_py3: stamp/unittests_py3
	python3 regression/issue_5.py
	python3 regression/issue_8.py
	python3 regression/issue_9.py
	python3 regression/issue_10.py
	python3 regression/issue_26.py
	python3 regression/issue_56.py
	touch $@


benchmark: benchmarks/benchmark.py stamp/build_py2
	python2 $^

devbuild2:
	python2 setup.py build_ext --inplace

devbuild3:
	python3 setup.py build_ext --inplace

valgrind:
	python -c "import sys;print(sys.version)"
	valgrind --leak-check=full --track-origins=yes --log-file=valgrind.log python unittests.py

pip-release:
	python setup.py sdist upload

clean:
	rm -f stamp/*
	rm -rf dist build

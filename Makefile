.SUFFIXES:
.PHONY: test clean

export PYTHONPATH := .:$(PYTHONPATH):$(PATH)

test: stamp/regression_py2 stamp/regression_py3

stamp/build_py2:
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
	touch $@


stamp/build_py3:
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
	touch $@


clean:
	rm -f stamp/*

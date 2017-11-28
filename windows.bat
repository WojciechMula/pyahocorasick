@echo off
@rem A python interperter must be available through PATH.

SET PYTHONPATH=.

IF [%1]==[clean] (
    del /Q stamp\*_pyW
    exit /B
)

IF NOT EXIST stamp\build_pyW (
    python setup.py build_ext --inplace
    IF %ERRORLEVEL% NEQ 0 EXIT /B
    type nul > stamp\build_pyW
) ELSE echo the extension was built

IF NOT EXIST stamp\unittests_pyW (
    python unittests.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B
    type nul > stamp\unittests_pyW
) ELSE echo unittests were run

IF NOT EXIST stamp\regression_pyW (
	python regression/issue_5.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B

	python regression/issue_8.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B

	python regression/issue_9.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B

	python regression/issue_10.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B

	python regression/issue_26.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B

	python regression/issue_56.py
    IF %ERRORLEVEL% NEQ 0 EXIT /B

    type nul > stamp\regression_pyW
) ELSE echo regression tests were run

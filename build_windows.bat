@echo off

SET varsall="%LOCALAPPDATA%\Programs\Common\Microsoft\Visual C++ for Python\9.0\vcvarsall.bat"

IF NOT EXIST %varsall% (
	ECHO "It seems that Microsoft compiler is missing from your computer"
	ECHO "Please install it from https://www.microsoft.com/en-us/download/details.aspx?id=44266"
	EXIT /B 1
) 

WHERE /Q python
IF %ERRORLEVEL% NEQ 0 (
	ECHO "Python can't be found. Add it to your PATH"
	EXIT /B 2
)

rem setup compiler
call %varsall% amd64

rem setuptools settings
SET DISTUTILS_USE_SDK=1
SET MSSdk=1

python setup.py build
IF %ERRORLEVEL% NEQ 0 (
	ECHO "Build error"
	EXIT /B 3
)

python setup.py install
IF %ERRORLEVEL% NEQ 0 (
	ECHO "Install error"
	EXIT /B 3
)

python unittests.py
IF %ERRORLEVEL% NEQ 0 (
	ECHO "Tests failed"
	EXIT /B 3
)

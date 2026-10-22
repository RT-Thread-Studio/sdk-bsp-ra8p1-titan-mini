@echo off
setlocal
set "PYTHONHOME="
set "PYTHONPATH="
set "SCONS_LIB_DIR="
set "OPENMV_BOOTSTRAP=%TITAN_OPENMV_PYTHON%"
if defined OPENMV_BOOTSTRAP goto run_python

rem Eclipse's builder PATH may contain neither python nor py.
set "OPENMV_BOOTSTRAP=%SystemRoot%\py.exe"
if exist "%OPENMV_BOOTSTRAP%" goto run_launcher
set "OPENMV_BOOTSTRAP=%LocalAppData%\Programs\Python\Launcher\py.exe"
if exist "%OPENMV_BOOTSTRAP%" goto run_launcher

set "OPENMV_BOOTSTRAP="
for /d %%D in ("%LocalAppData%\Programs\Python\Python*") do if exist "%%~fD\python.exe" set "OPENMV_BOOTSTRAP=%%~fD\python.exe"
if defined OPENMV_BOOTSTRAP goto run_python
for %%P in (python.exe) do set "OPENMV_BOOTSTRAP=%%~$PATH:P"
if defined OPENMV_BOOTSTRAP goto run_python

echo OpenMV requires Python 3.10+ with SCons. Set TITAN_OPENMV_PYTHON to its full python.exe path. 1>&2
exit /b 1

:run_launcher
"%OPENMV_BOOTSTRAP%" -3 "%~dp0studio_build.py" %*
exit /b %errorlevel%

:run_python
"%OPENMV_BOOTSTRAP%" "%~dp0studio_build.py" %*
exit /b %errorlevel%

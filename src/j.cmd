@echo off

REM // Launching script for 'jumpdir' instructions. This script works on
REM // Windows NT-based operating systems (including WindowsXP).

if "%os%" equ "Windows_NT" goto :do_commands
@echo jumpdir: %0 script requires Windows-NT based operating systems.
exit /b 1

:do_commands
for /f "delims=" %%s in ('jumpdir %*') do %%s

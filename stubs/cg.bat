@echo off

if "%1" EQU "" (
c:
cd \coding\games
goto end
)

if "%1" EQU "u4" (
c:
cd \coding\games\u4map
goto end
)

if "%1" EQU "u5" (
c:
cd \coding\games\u5map
goto end
)

if EXIST "c:\coding\games\%1" (
c:
cd \coding\games\%1
goto end
)

echo Didn't find anything for %1.

:end
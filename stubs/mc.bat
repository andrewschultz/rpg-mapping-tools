@echo off

if "%2" EQU "" (
c:\coding\games\mapcpy\mapcpy %1.txt
c:\coding\games\mapconv\mapconv -u -x %1.nmr
goto end
)

c:\coding\games\mapcpy\mapcpy %1.txt
c:\coding\games\mapconv\mapconv -u -x %1.nmr

::end
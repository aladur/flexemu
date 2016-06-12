@echo off

REM REMOVE.BAT
REM
REM Utility for mingw32 make to delete files
REM

:DELETE
shift
if '%0' == '' goto END
DEL /F/Q %0
goto DELETE


:END
exit

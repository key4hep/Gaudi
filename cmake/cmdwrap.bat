@echo off

set p=%1
rem echo PATH=%p:~1,-1%
rem set PATH=%p:~1,-1%;%PATH%
echo PATH=%p:[:]=;%
set PATH=%p:[:]=;%;%PATH%

shift 
set cmd=%1
:getarg
shift
if "%1"=="" goto end
set cmd=%cmd% %1
goto getarg
:end
echo CMD=%cmd%
%cmd%



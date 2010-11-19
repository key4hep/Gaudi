@echo off
call %1
shift
set d=%1
set d=%d:/=\%
cd/d %d%
shift

set cmd=%1
:getarg
shift
if "%1"=="" goto end
set cmd=%cmd% %1
goto getarg
:end
%cmd%



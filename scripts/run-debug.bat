@echo off
set EXE_NAME=raze.exe
pushd %~dp0\..\build\raze\Debug
if exist %EXE_NAME% (
    %EXE_NAME%
) else (
    echo Debug executable "%EXE_NAME%" not found!
)
popd

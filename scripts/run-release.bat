@echo off
set EXE_NAME=raze.exe
pushd %~dp0\..\build\raze\Release
if exist %EXE_NAME% (
    %EXE_NAME%
) else (
    echo Release executable "%EXE_NAME%" not found!
)
popd

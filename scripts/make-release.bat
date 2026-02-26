@echo off
pushd %~dp0\..
if not exist build mkdir build
pushd build
cmake --build . --config Release
popd
popd

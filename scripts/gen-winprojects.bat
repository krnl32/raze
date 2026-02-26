@echo off
pushd %~dp0\..\
mkdir build
pushd build
cmake ..
popd
popd

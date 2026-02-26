@echo off
pushd %~dp0\..\
mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=Debug ..
cmake --build . --config Debug
popd
popd

@echo off
pushd %~dp0\..\
mkdir build
pushd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
popd
popd

@echo off

IF NOT EXIST build mkdir build
pushd build

	cl /Zi /EHsc ..\src\main.cpp user32.lib

popd
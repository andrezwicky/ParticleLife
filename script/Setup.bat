@echo off

pushd %~dp0..
vendor\bin\premake5.exe vs2022
popd
pause
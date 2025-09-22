@echo off
pushd %~dp0
fltmc unload ignis
sc query ignis
popd
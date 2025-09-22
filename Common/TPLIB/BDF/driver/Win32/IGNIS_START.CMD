@echo off
pushd %~dp0
net start ignis
sc query ignis
popd
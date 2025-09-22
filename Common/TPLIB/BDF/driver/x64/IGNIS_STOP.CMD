@echo off
pushd %~dp0
fltmc unload ignis
reg delete "HKLM\System\CurrentControlSet\Services\Ignis" /f
sc query ignis
popd
@echo off
pushd %~dp0
RunDLL32.Exe syssetup,SetupInfObjectInstallAction DefaultInstall 128 %CD%\ignis.inf
reg add "HKLM\System\CurrentControlSet\Services\Ignis" /v Start /t REG_DWORD /d 0 /f
net start ignis
popd
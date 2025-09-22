@echo off

rem registering 32 bit machine DLL===========================================================================================================================================================================================================

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WrdWizAVUI.exe"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WrdWizAVUI.exe"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZUSBDETECTUI.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZUSBDETECTUI.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZCLOSEALL.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZCLOSEALL.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZCOMMSRV.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZCOMMSRV.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZALUSRV.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZALUSRV.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZTRAY.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZTRAY.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZUSBDETECT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZUSBDETECT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZEXTRACT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZEXTRACT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZREGISTERDATA.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZREGISTERDATA.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZREGISTRATION.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZREGISTRATION.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZREPAIRDLL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZREPAIRDLL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZRESOURCE.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZRESOURCE.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZRKSCN.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZRKSCN.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZSCANDLL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZSCANDLL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZSENDEMAIL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZSENDEMAIL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZSETUPDLL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZSETUPDLL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Unicode Release MinDependency\Win32\Binaries\WRDWIZEMAILADDIN.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Unicode Release MinDependency\Win32\Binaries\WRDWIZEMAILADDIN.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZLAUNCH.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZLAUNCH.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZCRASHRPT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZCRASHRPT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZHASH.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDWIZHASH.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDCRASHSENDER.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\Win32\Binaries\WRDCRASHSENDER.EXE"

rem registering 64 bit machine DLL ===========================================================================================================================================================================================================

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WrdWizAVUI.exe"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WrdWizAVUI.exe"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZUSBDETECTUI.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZUSBDETECTUI.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZHASH.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZHASH.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZCLOSEALL.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZCLOSEALL.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZCOMMSRV.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZCOMMSRV.EXE"


signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZALUSRV.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZALUSRV.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZTRAY.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZTRAY.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZUSBDETECT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZUSBDETECT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZEXTRACT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZEXTRACT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZREGISTERDATA.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZREGISTERDATA.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZREGISTRATION.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZREGISTRATION.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZREPAIRDLL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZREPAIRDLL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZRESOURCE.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZRESOURCE.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZRKSCN.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZRKSCN.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZSCANDLL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZSCANDLL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZSENDEMAIL.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZSENDEMAIL.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release MinDependency\x64\Binaries\WRDWIZEMAILADDIN.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release MinDependency\x64\Binaries\WRDWIZEMAILADDIN.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZCRASHRPT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDWIZCRASHRPT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDCRASHSENDER.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\Release\x64\Binaries\WRDCRASHSENDER.EXE"

rem outlook plugin 32 bit DLL's===========================================================================================================================================================================================================

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZEMAILADDIN32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZEMAILADDIN32.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZEXTRACT32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZEXTRACT32.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZSCANDLL32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZSCANDLL32.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZREPAIRDLL32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZREPAIRDLL32.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZREGISTERDATA32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZREGISTERDATA32.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZREGISTRATION32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\ReleaseOutlookPlugin\Binaries\WRDWIZREGISTRATION32.DLL"

rem registereing common binaries===========================================================================================================================================================================================================

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\UNRAR\x86\UNRAR.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\UNRAR\x86\UNRAR.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\UNRAR\x86\UNRAR32.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\UNRAR\x86\UNRAR32.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\UNRAR\x64\UNRAR.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\UNRAR\x64\UNRAR.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\LIBCLAMAV.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigixtalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\LIBCLAMAV.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\WRDWIZSCANNER.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\WRDWIZSCANNER.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\EVALREGDLLS\x86\WRDWIZEVALREG.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\EVALREGDLLS\x86\WRDWIZEVALREG.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\EVALREGDLLS\x64\WRDWIZEVALREG.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\RequiredFiles\EVALREGDLLS\x64\WRDWIZEVALREG.DLL"

rem crach sender ------------------------------------------------------------------------------------------------------------------------------------------

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDCRASHSENDER.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDCRASHSENDER.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDWIZCRASHRPT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDWIZCRASHRPT.DLL"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDCRASHSENDER.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDCRASHSENDER.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDWIZCRASHRPT.DLL"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\WardWizCrashHandler\x86\bin\WRDWIZCRASHRPT.DLL"

rem ------------------------------------------------------------------------------------------------------------------------------------------


rem ===========================================================================================================================================================================================================
pause

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZPROSETUPX86.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZPROSETUPX86.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZPROSETUPX64.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZPROSETUPX64.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZESSENTIALSETUPX86.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZESSENTIALSETUPX86.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZESSENTIALSETUPX64.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZESSENTIALSETUPX64.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZBASICSETUPX86.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZBASICSETUPX86.EXE"

signtool sign /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZBASICSETUPX64.EXE"
signtool sign /t http://timestamp.digicert.com /f "D:\WardWiz_Developement\SetupRequirements\DigitalSignature\OS201406256286.pfx" /p "mfk923414441" "D:\WardWiz_Developement\SetupRequirements\Output\WARDWIZBASICSETUPX64.EXE"

rem process completed sucessfully...


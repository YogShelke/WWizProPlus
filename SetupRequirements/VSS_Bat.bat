SET SSDIR=\\192.168.2.99\WardWizDevelopement

mkdir F:\WardWiz_Development

"C:\Program Files (x86)\Microsoft Visual SourceSafe\ss.exe" Get -R -I -Y -W $/WardWizDevelopment -GLF:\WardWiz_Development

"F:\WardWiz_Development\WardWizALUpd.exe" rb

@echo off
xcopy ..\x64\Release\cminor.exe ..\bin /Y
xcopy ..\x64\Release\cminor2html.exe ..\bin /Y
xcopy ..\x64\Release\cminorbuildsys.exe ..\bin /Y
xcopy ..\x64\Debug\cminorbuildsysd.exe ..\bin /Y
xcopy ..\x64\Release\cminorc.exe ..\bin /Y
xcopy ..\x64\Debug\cminorcd.exe ..\bin /Y
xcopy ..\x64\Release\cminordb.exe ..\bin /Y
xcopy ..\x64\Release\cminordump.exe ..\bin /Y
xcopy ..\x64\Release\cminormachine.dll ..\bin /Y
xcopy ..\x64\Release\cminorrt.dll ..\bin /Y
xcopy ..\x64\Release\cminorpg.exe ..\bin /Y
xcopy ..\x64\Release\cminorvm.exe ..\bin /Y
xcopy ..\x64\Debug\cminorvmd.exe ..\bin /Y
xcopy "C:\Programming\llvm-3.9.1-release\build\Release\bin\llc.exe" ..\bin /Y
xcopy "C:\Programming\llvm-3.9.1-release\build\Release\bin\opt.exe" ..\bin /Y
xcopy "C:\Programming\llvm-3.9.1-release\build\Release\bin\lld-link.exe" ..\bin /Y
xcopy ..\x64\Release\cminormachine.lib ..\lib /Y
xcopy ..\x64\Release\cminorrt.lib ..\lib /Y

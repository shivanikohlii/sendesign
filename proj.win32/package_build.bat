@echo off

@setlocal
set TITLE=CSP_RELEASE
if EXIST %TITLE% ( rmdir %TITLE% /S /Q )

mkdir %TITLE%

cd Release.win32

xcopy "../../Resources" "../%TITLE%" /E /Y /Q /I

copy "*.exe" "../%TITLE%"
copy "*.dll" "../%TITLE%"
if NOT EXIST "../%TITLE%/CSP" mkdir "../%TITLE%/CSP"

copy "*.cpp" "../%TITLE%"
copy "build.bat" "../%TITLE%/build.bat"
xcopy "CSP" "../%TITLE%/CSP" /E /Y /Q /I


set TITLE=CSP_DEBUG
cd ../Debug.win32

xcopy "../../Resources" "../%TITLE%" /E /Y /Q /I

copy "*.exe" "../%TITLE%"
copy "*.dll" "../%TITLE%"
copy "*.pdb" "../%TITLE%"
if NOT EXIST "../%TITLE%/CSP" mkdir "../%TITLE%/CSP"

copy "*.cpp" "../%TITLE%"
copy "build.bat" "../%TITLE%/build.bat"
xcopy "CSP" "../%TITLE%/CSP" /E /Y /Q /I

cd ../%TITLE%
del libcocos2d.pdb
del librecast.pdb
del libSpine.pdb
del vc141.pdb
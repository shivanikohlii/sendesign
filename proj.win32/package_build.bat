@echo off
@setlocal

set TITLE=CSP

if EXIST %TITLE% ( rmdir %TITLE% /S /Q )
mkdir %TITLE%
cd Release.win32
xcopy "../../Resources" "../%TITLE%" /E /Y /Q /I
copy "*.exe" "../%TITLE%"
copy "*.dll" "../%TITLE%"
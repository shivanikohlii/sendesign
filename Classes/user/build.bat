@setlocal
@echo off

set BASE_NAME=game
set RELEASE=0
set BUILD_32_BIT=1
set CLEANUP=1
set NAME_BY_ARCHITECTURE=0

for %%x in (%*) do (
    if %%~x == -r set RELEASE=1
    if %%~x == -x86 set BUILD_32_BIT=1
    if %%~x == -n set NAME_BY_ARCHITECTURE=1
)

REM if %BUILD_32_BIT% == 1 call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvars32.bat"

if exist ".vs" (rmdir /S /Q ".vs")
if exist "Debug" (rmdir /S /Q "Debug")

if %NAME_BY_ARCHITECTURE% == 1 (
if %BUILD_32_BIT% == 1 (
set PROGRAM_NAME=%BASE_NAME%32
) else (
set PROGRAM_NAME=%BASE_NAME%64
)
) else (
set PROGRAM_NAME=%BASE_NAME%
)

set LIBPATHS=
set INCLUDEPATHS=
set LIBFILES= user32.lib
set EXTRA_COMPILE_FLAGS=

set LIBFILES= %LIBFILES%
if %BUILD_32_BIT% == 1 (
set EXTRA_LINK_FLAGS= -SUBSYSTEM:WINDOWS,5.01
) else (
set EXTRA_LINK_FLAGS= -SUBSYSTEM:WINDOWS,5.02
)

if NOT %RELEASE% == 1 (
set EXTRA_LINK_FLAGS= %EXTRA_LINK_FLAGS% -PDB:"%BASE_NAME%_%date:~-4,4%%date:~-10,2%%date:~-7,2%_%time:~0,2%%time:~3,2%%time:~6,2%.pdb"
)

set DISABLED_WARNINGS= -wd4201 -wd4100 -wd4189 -wd4996 -wd4505 -wd4312
set DEBUG_INFO= -Z7
if %RELEASE% == 1 (
set COMPILE_FLAGS= -EHsc -LD -nologo -Gm- -GR- -Ox -W3 %DISABLED_WARNINGS% -DDEBUG_CONSOLE=0 -DDEBUG=0 -DDEBUG_MODE=0 -DNDEBUG=1 -FC -Fe%PROGRAM_NAME% -EHa- %DEBUG_INFO% %INCLUDEPATHS%
) else (
set COMPILE_FLAGS= -EHsc -LDd -nologo -Gm- -GR- -Oi -W3 -Fe%PROGRAM_NAME% -DDEBUG_MODE=1 -DDEBUG=1 -DUSE_EDITOR=1 -DDEBUG_CONSOLE=1 %DISABLED_WARNINGS% -FC -EHa- %EXTRA_COMPILE_FLAGS% %DEBUG_INFO% %INCLUDEPATHS%
)

set LINK_FLAGS= -link -opt:ref %EXTRA_LINK_FLAGS% %LIBPATHS%

if NOT %RELEASE% == 1 (
del *.pdb > NUL 2> NUL
)

cl %COMPILE_FLAGS% %BASE_NAME%.cpp %LIBFILES% %LINK_FLAGS%

if %CLEANUP% == 1 (
del *.exp
del *.lib
del *.obj
if %RELEASE% == 1 ( del *.pdb )
)

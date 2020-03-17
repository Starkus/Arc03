@echo off

cls

REM -- Flags

set Release=0
set Tools=0

:processargs
set ARG=%1
IF DEFINED ARG (
	IF "%ARG%"=="-r" set Release=1
	IF "%ARG%"=="-t" set Tools=1
	SHIFT
	GOTO processargs
)

set UserPath=%HomeDrive%%HomePath%
set LibPath=%UserPath%\source\libraries
set SrcPath=..\src

IF %Tools% EQU 0 (
	echo BUILDING CODE
	set SourceFiles=%SrcPath%\Unity.cpp
	set CompilerFlags=-Fe: Arc03.exe -MDd -nologo -GR- -Oi -EHa- -W4 -wd4530 -wd4701 -FC -Z7 -std:c++17
	set LinkerFlags=-opt:ref -incremental:no -NODEFAULTLIB:MSVCRT
	set Libraries=vulkan-1.lib glfw3.lib user32.lib Gdi32.lib winmm.lib shell32.lib
	set IncludePaths=-I %SrcPath% -I C:\VulkanSDK\1.2.131.2\Include -I %LibPath%\glfw-3.3.2\include -I %LibPath%\glm -I %LibPath%\stb -I %LibPath%\tinyobjloader
	set LibPaths=-LIBPATH:C:\VulkanSDK\1.2.131.2\Lib -LIBPATH:%LibPath%\glfw-3.3.2\bin\Release
) ELSE (
	echo BUILDING TOOLS
	set SourceFiles=..\tools\bake.cpp
	set CompilerFlags=-MDd -nologo -GR- -Oi -W4 -FC -Z7 -std:c++17
	set LinkerFlags=-opt:ref -incremental:no -NODEFAULTLIB:MSVCRT
	set Libraries=user32.lib Gdi32.lib winmm.lib shell32.lib
	set IncludePaths=-I %LibPath%\glm -I %LibPath%\stb -I %LibPath%\tinyobjloader
	set LibPaths=
)

REM -- DEBUG
set DebugCompilerFlags=-Od
REM -- RELEASE
set ReleaseCompilerFlags=-O2

IF NOT EXIST .\build mkdir .\build
pushd .\build

IF %Release% EQU 0 (
	echo DEBUG build selected
	cl %CompilerFlags% %DebugCompilerFlags% %SourceFiles% %Libraries% %IncludePaths% -link %LinkerFlags% %LibPaths%
) ELSE (
	echo RELEASE build selected
	cl %CompilerFlags% %ReleaseCompilerFlags% %SourceFiles% %Libraries% %IncludePaths% -link %LinkerFlags% %LibPaths%
)
IF %ERRORLEVEL% NEQ 0 echo Compilation [31mfailed![0m
IF %ERRORLEVEL% EQU 0 echo Compilation [32msuccess[0m

popd

echo.

echo Generating tags...
IF EXIST "C:\Program Files\ctags\ctags.exe" (
	"C:\Program Files\ctags\ctags.exe" -R --use-slash-as-filename-separator=yes .
	IF %ERRORLEVEL% NEQ 0 echo Tag generation [31mfailed![0m
	IF %ERRORLEVEL% EQU 0 echo Done generating tags
) ELSE (
	echo Ctags not found!
)

@echo off

cls

set UserPath=%HomeDrive%%HomePath%
set LibPath=%UserPath%\source\libraries

set SourceFiles=..\src\Arc03.cpp
set CompilerFlags=-MDd -nologo -GR- -Oi -EHa- -W4 -wd4530 -wd4701 -FC -Z7 -std:c++17
set LinkerFlags=-opt:ref -incremental:no -NODEFAULTLIB:MSVCRT
set Libraries=vulkan-1.lib glfw3.lib user32.lib Gdi32.lib winmm.lib shell32.lib
set IncludePaths=-I C:\VulkanSDK\1.2.131.2\Include -I %LibPath%\glfw-3.3.2\include -I %LibPath%\glm -I %LibPath%\stb -I %LibPath%\tinyobjloader
set LibPaths=-LIBPATH:C:\VulkanSDK\1.2.131.2\Lib -LIBPATH:%LibPath%\glfw-3.3.2\bin\Release

REM - DEBUG
set DebugCompilerFlags=-Od
REM - RELEASE
set ReleaseCompilerFlags=-O2

IF NOT EXIST .\build mkdir .\build
pushd .\build

echo BUILDING CODE
IF "%~1"=="-d" (
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

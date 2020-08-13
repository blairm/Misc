@echo off

call "%ProgramFiles(x86)%\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall" x64

for %%F in ("%cd%") do set "folder=%%~nxF"
start /b "sublime" "%ProgramFiles%\Sublime Text 3\sublime_text.exe" --project %folder%.sublime-project -n

call devenv Build\Win32\Win32_main.exe
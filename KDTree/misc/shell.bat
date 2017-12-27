@echo off

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall" x64

set path=%CD%\misc;%path%

start /b "sublime" "C:\Program Files\Sublime Text 3\sublime_text.exe" --project KDTree.sublime-project -n

call devenv build\main.exe
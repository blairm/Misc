@echo off

for %%F in ("%cd%") do set "folder=%%~nxF"
start /b "sublime" "%ProgramFiles%\Sublime Text 3\sublime_text.exe" --project %folder%.sublime-project -n

start /b "android studio" "%ProgramFiles%\Android\Android Studio\bin\studio64" "Src/Android/AndroidStudioProject"

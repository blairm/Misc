@echo off

IF NOT EXIST MusicMP3 mkdir MusicMP3
pushd MusicMP3

	for /D %%i in ("../Music/*") do (
		IF NOT EXIST "%%i" mkdir "%%i"
		pushd "%%i"
		
			IF EXIST "../../Music/%%i/Albums.txt" xcopy /y "..\..\Music\%%i\Albums.txt" "."
		
			for /D %%j in ("../../Music/%%i/*") do (
				IF NOT EXIST "%%j" mkdir "%%j"
				pushd "%%j"
				
					forfiles /p "..\..\..\Music\%%i\%%j\\" /c "cmd /c if @ext==\"jpg\" xcopy /y /h \"..\..\..\Music\%%i\%%j\@file\" \"..\..\..\MusicMP3\%%i\%%j\\""
					forfiles /p "..\..\..\Music\%%i\%%j\\" /c "cmd /c if @ext==\"png\" xcopy /y /h \"..\..\..\Music\%%i\%%j\@file\" \"..\..\..\MusicMP3\%%i\%%j\\""
				
					for %%k in ("../../../Music/%%i/%%j/*.FLAC") do (
					 	"../../../ffmpeg" -y -i "../../../Music/%%i/%%j/%%k" -ab 320k "%%~nk.mp3"
					)
				
				popd
			)
		
		popd
	)

popd
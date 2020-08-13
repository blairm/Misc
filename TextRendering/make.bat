@echo off

IF NOT EXIST Build mkdir Build
pushd Build

    IF NOT EXIST Win32 mkdir Win32
    pushd Win32

        SET compileFlags=/GR- /Oi /W3 /Gm- /FC /EHa- /Fmbuild.map /nologo /D _CRT_SECURE_NO_WARNINGS /D OPENGL /D PLATFORM_WIN32 /D UNICODE /D _UNICODE

        SET debugCompileFlags=/MTd /Od /Zi /D _DEBUG
        SET debugCompileFlags=%debugCompileFlags% /D OPENGL_DEBUG_CONTEXT
        SET releaseCompileFlags=/MT /GL /GS- /arch:AVX2 /fp:fast /O2 /D NDEBUG
        REM SET releaseCompileFlags=%releaseCompileFlags% /Qvec-report:2

        SET compileFlags=%debugCompileFlags% %compileFlags%
        REM SET compileFlags=%releaseCompileFlags% %compileFlags%

        SET files=..\..\Src\Win32\Win32_main.cpp
        SET files=%files% ..\..\Src\Shared\App.cpp
        SET files=%files% ..\..\Src\Shared\C3dMaths.cpp

        SET linkFlags=/incremental:no /opt:ref user32.lib Gdi32.lib opengl32.lib

        cl %compileFlags% %files% /link %linkFlags%

    popd

popd
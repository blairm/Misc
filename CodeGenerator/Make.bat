@echo off

IF NOT EXIST Build mkdir Build
pushd Build

    IF NOT EXIST Win32 mkdir Win32
    pushd Win32

        SET compileFlags=/GR- /Oi /W3 /Gm- /FC /EHa- /Fmbuild.map /nologo /D _CRT_SECURE_NO_WARNINGS

        SET debugCompileFlags=/MTd /Od /Zi /D _DEBUG
        SET releaseCompileFlags=/MT /GL /GS- /arch:AVX2 /fp:fast /O2 /D NDEBUG
        REM SET releaseCompileFlags=%releaseCompileFlags% /Qvec-report:2

        SET compileFlags=%debugCompileFlags% %compileFlags%
        REM SET compileFlags=%releaseCompileFlags% %compileFlags%

        SET files=..\..\Src\Main.cpp

        SET linkFlags=/incremental:no /opt:ref user32.lib

        cl %compileFlags% %files% /link %linkFlags%

    popd

popd
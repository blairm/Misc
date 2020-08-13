#!/bin/sh

DIR="$( dirname "$(readlink -f "$0")")"
cd $DIR

if [ ! -d "Build" ]; then
	mkdir "Build"
fi

cd Build

	if [ ! -d "Linux" ]; then
		mkdir "Linux"
	fi

	cd Linux

		compileFlags="-Wall -fno-rtti -fno-exceptions -Wno-write-strings -Wno-unused-function -Wno-unused-result -no-pie -pipe -march=x86-64 -o Linux_main -DOPENGL -DPLATFORM_LINUX -D__gl_h_"

		debugCompileFlags="-O0 -g3 -D_DEBUG"
		#debugCompileFlags="$debugCompileFlags -DOPENGL_DEBUG_CONTEXT"
		releaseCompileFlags="-O3 -g0 -flto -ffast-math -mavx -DNDEBUG"
		#releaseCompileFlags="$releaseCompileFlags -fopt-info-vec-optimized"

		compileFlags="$debugCompileFlags $compileFlags"
		#compileFlags="$releaseCompileFlags $compileFlags"

		files="../../Src/Linux/Linux_main.cpp"
		files="$files ../../Src/Shared/C3dMaths.cpp"
		files="$files ../../Src/Shared/App.cpp"

		includes="-I/usr/include/freetype2"
		includes="$includes -I/usr/include/harfbuzz"
		includes="$includes -I/usr/include/cairo"
		includes="$includes -I/usr/include/glib-2.0/"
		includes="$includes -I/usr/lib/x86_64-linux-gnu/glib-2.0/include"
		includes="$includes -I/usr/include/pango-1.0"

		libs="-lstdc++ -lm -lX11 -lGL -lpthread -lfreetype -lcairo -lgobject-2.0 -lfontconfig -lpango-1.0 -lpangocairo-1.0"

		linkFlags="-Map=build.map -Wno-undef"

		gcc $compileFlags $files $includes $libs -Xlinker $linkFlags
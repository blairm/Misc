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

		compileFlags="-Wall -fno-rtti -fno-exceptions -Wno-write-strings -Wno-unused-function -Wno-unused-result -no-pie -pipe -march=x86-64 -o Linux_main"

		debugCompileFlags="-O0 -g3 -D_DEBUG"
		releaseCompileFlags="-O3 -g0 -flto -ffast-math -mavx -DNDEBUG"
		#releaseCompileFlags="$releaseCompileFlags -fopt-info-vec-optimized"

		compileFlags="$debugCompileFlags $compileFlags"
		#compileFlags="$releaseCompileFlags $compileFlags"

		files="../../Src/Main.cpp"

		includes=""

		libs="-lstdc++ -lm -lX11"

		linkFlags="-Map=build.map -Wno-undef"

		gcc $compileFlags $files $includes $libs -Xlinker $linkFlags
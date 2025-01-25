#!/bin/sh

DEBUG=0
RELEASE=1
BUILD_MODE=${DEBUG}

PRX_BUILD=0
ENCRYPT_PRX=0

DISC_ID="UCES-00000"
TITLE="PSPTest"
VERSION="1.00"

DIR="$( dirname "$(readlink -f "$0")")"
cd $DIR

ICON_IMAGE_FILE="${DIR}/Packing/IconStatic.png"
ICON_VIDEO_FILE="NULL"										#ICON1.pmf - this and music file need to be < 500kb
DESC_IMAGE_FILE="${DIR}/Packing/ContentDescription.png"
BG_IMAGE_FILE="${DIR}/Packing/ContentBackground.png"
MUSIC_FILE="NULL"											#SND0.at3 - this and icon video file need to be < 500kb
ARCHIVE_FILE="NULL"											#psar - game data goes here

TARGET="PSP_main"

PSPSDK="../../../pspdev"

if [ ! -d "Build" ]; then
	mkdir "Build"
fi

cd Build

	if [ ! -d "PSP" ]; then
		mkdir "PSP"
	fi

	cd PSP

		compileFlags="-Wall -fno-rtti -fno-exceptions -Wno-write-strings -Wno-unused-function -Wno-unused-result -no-pie -pipe -DPSP -D__PSP__ -D_PSP_FW_VERSION=200 -o ${TARGET}"

		debugCompileFlags="-O0 -g3 -D_DEBUG"
		releaseCompileFlags="-O3 -g0 -flto -ffast-math -DNDEBUG"


		case ${BUILD_MODE} in
			${DEBUG} )
				compileFlags="${debugCompileFlags} ${compileFlags}" ;;
			${RELEASE} )
				compileFlags="${releaseCompileFlags} ${compileFlags}" ;;
			* )
				echo "Build mode doesn't exist" 1>&2
				exit 1 ;;
		esac


		libs="-lpspgu -lpspdisplay -lpspge -lpspctrl -lstdc++ -L${PSPSDK}/lib -L${PSPSDK}/psp/lib -L${PSPSDK}/psp/sdk/lib"


		files="${DIR}/Main.cpp"


		linkFlags="-Wno-undef -Wl,-zmax-page-size=128"

		if [ ${PRX_BUILD} = 1 ]; then
			echo "prx link flags"
			linkFlags="$linkFlags -specs=${PSPSDK}/psp/sdk/lib/prxspecs -Wl,-q,-T${PSPSDK}/psp/sdk/lib/linkfile.prx ${PSPSDK}/psp/sdk/lib/prxexports.o"
		fi


		"${PSPSDK}/bin/psp-g++" -I"${PSPSDK}/psp/include" -I"${PSPSDK}/psp/sdk/include" $compileFlags $files $libs $linkFlags


		#@NOTE psp-prxgen includes strip so don't do this if prx build
		if [ ${BUILD_MODE} = ${RELEASE} ] && [ ${PRX_BUILD} = 0 ]; then
			echo "strip"
			"${PSPSDK}/bin/psp-strip" "${TARGET}"
		fi


		"${PSPSDK}/bin/psp-fixup-imports" "${TARGET}"

		"${PSPSDK}/bin/mksfoex" "-d" "MEMSIZE=1" "-s" "DISC_ID=${DISC_ID}" "-s" "APP_VER=${VERSION}" "${TITLE}" PARAM.SFO


		if [ ${PRX_BUILD} = 1 ]; then
			"${PSPSDK}/bin/psp-prxgen" "${TARGET}" "${TARGET}.prx"

			if [ ${ENCRYPT_PRX} = 1 ]; then
				echo "encrypt prx"
				"${PSPSDK}/bin/PrxEncrypter" "${TARGET}.prx" "${TARGET}.prx"
			fi

			echo "pack pbp with prx"
			"${PSPSDK}/bin/pack-pbp" "EBOOT.PBP" "PARAM.SFO" "${ICON_IMAGE_FILE}" "${ICON_VIDEO_FILE}" "${DESC_IMAGE_FILE}" "${BG_IMAGE_FILE}" "${MUSIC_FILE}" "${TARGET}.prx" "${ARCHIVE_FILE}"
		else
			echo "pack pbp with elf"
			"${PSPSDK}/bin/pack-pbp" "EBOOT.PBP" "PARAM.SFO" "${ICON_IMAGE_FILE}" "${ICON_VIDEO_FILE}" "${DESC_IMAGE_FILE}" "${BG_IMAGE_FILE}" "${MUSIC_FILE}" "${TARGET}" "${ARCHIVE_FILE}"
		fi

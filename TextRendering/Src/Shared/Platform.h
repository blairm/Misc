#ifndef PLATFORM_H
#define PLATFORM_H


#include "Types.h"


#if defined( PLATFORM_WIN32 ) || defined( PLATFORM_LINUX )
    #define DATA_FOLDER "../../Data/"
#else
    #define DATA_FOLDER ""
#endif


namespace Platform
{
    struct File
    {
        u64 size;
        void* data;
    };


    extern void Log( char* string, ... );
    #define LOG( string, ... ) Platform::Log( string, ##__VA_ARGS__ )


    extern f64 GetTime();

    extern s32 GetProcessorCount();


    typedef void WorkCallback( s32 threadIndex, void* data );
    extern void AddWorkQueueEntry( WorkCallback* callback, void* data );
    extern void FinishWork();


    extern File ReadFile( char* filename );
    extern void FreeFile( File* file );


    extern u8* CreateTextBitmap( char* string,
                                    char* fontFilename,
                                    char* fontFacename,
                                    s32 fontSize,
                                    s32 kerning,
                                    HAlignment hAlignment,
                                    v2i* textSize,
                                    v2i* bitmapSize );
    extern void FreeTextBitmap( u8** textBitmap );

    extern u8* CreateFontAtlasBitmap( char* string,
                                        char* fontFilename,
                                        char* fontFacename,
                                        s32 fontSize,
                                        s32 kerning,
                                        s32 padding,
                                        v2i* bitmapSize,
                                        FontAtlas* fontAtlas );
    extern void FreeFontAtlasBitmap( u8** fontAtlasBitmap );
    extern void FreeFontAtlas( FontAtlas* fontAtlas );
}


#endif
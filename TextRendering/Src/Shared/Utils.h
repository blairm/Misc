#ifndef UTILS_H
#define UTILS_H


#include "Types.h"
#include "C3dMaths.h"
#include "Platform.h"


#include <stdlib.h>


#define ARRAY_COUNT( array ) ( sizeof( ( array ) ) / sizeof( ( array )[ 0 ] ) )


#ifdef _DEBUG
    #ifdef _MSC_VER
        #define ASSERT( expression ) { if( !( expression ) ) { *( s32* ) 0 = 0; } }
    #elif defined __GNUC__
        #define ASSERT( expression ) { if( !( expression ) ) { __builtin_trap(); } }
    #else
        #error
    #endif
#else
    #define ASSERT( expression ) {}
#endif


#define CACHE_LINE_SIZE 64
#define NO_PER_CACHE_LINE( type ) ( CACHE_LINE_SIZE / sizeof( type ) )

#define S8_PER_CACHE_LINE NO_PER_CACHE_LINE( s8 )
#define U8_PER_CACHE_LINE NO_PER_CACHE_LINE( u8 )

#define S16_PER_CACHE_LINE NO_PER_CACHE_LINE( s16 )
#define U16_PER_CACHE_LINE NO_PER_CACHE_LINE( u16 )

#define S32_PER_CACHE_LINE NO_PER_CACHE_LINE( s32 )
#define U32_PER_CACHE_LINE NO_PER_CACHE_LINE( u32 )

#define S64_PER_CACHE_LINE NO_PER_CACHE_LINE( s64 )
#define U64_PER_CACHE_LINE NO_PER_CACHE_LINE( u64 )

#define F32_PER_CACHE_LINE NO_PER_CACHE_LINE( f32 )

#define F64_PER_CACHE_LINE NO_PER_CACHE_LINE( f64 )

#define ASSERT_ALIGNED_TO( pointer, alignment ) ASSERT( ( ( u64 ) ( pointer ) & ( alignment - 1 ) ) == 0 )
#define ASSERT_ALIGNED_TO_CACHE( pointer ) ASSERT_ALIGNED_TO( ( pointer ), CACHE_LINE_SIZE )

#ifdef _MSC_VER
    #define ALIGNED_ALLOC( size, alignment ) _aligned_malloc( ( size ), ( alignment ) )
    #define ALIGNED_FREE( memory ) _aligned_free( memory )
#elif defined __GNUC__
    #ifdef PLATFORM_ANDROID
        //@NOTE aligned_alloc doesn't appear to be in Androids libc++
        //memalign is basically aligned_alloc without checking size is multiple of alignment
        #define ALIGNED_ALLOC( size, alignment ) memalign( ( alignment ), ( size ) )
    #else
        #define ALIGNED_ALLOC( size, alignment ) aligned_alloc( ( alignment ), ( size ) )
    #endif
    #define ALIGNED_FREE( memory ) free( memory )
#else
    #error
#endif

#define ALIGNED_NEW( type, count, alignment ) ( type* ) ALIGNED_ALLOC( ( ( count ) * sizeof( type ) ), ( alignment ) )
#define NEW( type, count ) ( type* ) malloc( ( count ) * sizeof( type ) )
#define RENEW( type, memory, count ) ( type* ) realloc( ( memory ), ( count ) * sizeof( type ) )
#define FREE( memory ) free( memory )


namespace Utils
{
    struct TimedBlock
    {
        f64 startTime;
        f32* time;

        TimedBlock( f32* time )
        {
            this->time = time;
            startTime = Platform::GetTime();
        }

        ~TimedBlock()
        {
            *this->time = ( f32 ) ( Platform::GetTime() - startTime );
        }
    };
    #define TIMED_BLOCK( time ) Utils::TimedBlock timedBlock( time )


    extern bool IsPowerOf2( s32 value );
    extern s32 CeilToPower2( s32 value );
    extern v2i CeilToPower2( v2i value );

    extern f32 Randf01();
    extern s32 Rand0N( s32 maxValue );

    extern void GlyphPack( s32 padding,
                            s32 maxSize,
                            v2i* bitmapSize,
                            FontAtlas* fontAtlas );


    #ifdef UTILS_FUNCTIONS
        bool IsPowerOf2( s32 value )
        {
            bool result = value != 0;
            result &= ( value & ( value - 1 ) ) == 0;
            return result;
        }

        s32 CeilToPower2( s32 value )
        {
            --value;
            value |= ( value >>  1 );
            value |= ( value >>  2 );
            value |= ( value >>  4 );
            value |= ( value >>  8 );
            value |= ( value >> 16 );
            return ++value;
        }

        v2i CeilToPower2( v2i value )
        {
            value.x = CeilToPower2( value.x );
            value.y = CeilToPower2( value.y );
            return value;
        }


        f32 Randf01()
        {
            f32 result = ( f32 ) rand() / ( f32 ) RAND_MAX;
            return result;
        }

        s32 Rand0N( s32 maxValue )
        {
            s32 result = rand() % ( maxValue + 1 );
            return result;
        }


        void GlyphPack( s32 padding,
                        s32 maxSize,
                        v2i* bitmapSize,
                        FontAtlas* fontAtlas )
        {
            s32 glyphsFitted = 0;
            while( glyphsFitted != fontAtlas->glyphCount )
            {
                glyphsFitted = 0;
                s32 runningWidth = padding;
                s32 runningHeight = padding;
                s32 rowHeight = 0;

                for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                {
                    Glyph* glyph = &fontAtlas->glyphList[ i ];
                    if( runningWidth + glyph->width + padding < bitmapSize->x )
                    {
                        glyph->atlasX = runningWidth;
                        glyph->atlasY = runningHeight;
                        runningWidth += glyph->width + padding;
                        rowHeight = MAX( rowHeight, runningHeight + glyph->height );
                        ++glyphsFitted;
                    }
                    else if( runningHeight + rowHeight + padding + glyph->height + padding < bitmapSize->y )
                    {
                        runningWidth = padding;
                        runningHeight += rowHeight + padding;
                        rowHeight = glyph->height;

                        glyph->atlasX = runningWidth;
                        glyph->atlasY = runningHeight;

                        runningWidth += glyph->width + padding;
                        ++glyphsFitted;
                    }
                    else
                    {
                        break;
                    }
                }

                if( glyphsFitted < fontAtlas->glyphCount )
                {
                    if( bitmapSize->y < maxSize )
                    {
                        if( bitmapSize->x == bitmapSize->y )
                            bitmapSize->x *= 2;
                        else
                            bitmapSize->y *= 2;
                    }
                }
            }
        }
    #endif
}


#endif
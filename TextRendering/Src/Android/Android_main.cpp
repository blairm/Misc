#include "../Shared/App.h"
#include "../Shared/Types.h"
#include "../Shared/Platform.h"
#include "../Shared/Utils.h"
#include "../Shared/C3dMaths.h"
#define GLHELPER_PLATFORM_FUNCTIONS
#include "../Shared/GLHelper.h"

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <unistd.h>
#include <sys/sysinfo.h>
#include <pthread.h>
#include <semaphore.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <jni.h>
#include <android/log.h>
#include <android/native_window_jni.h>
#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

#include <atomic>
typedef std::atomic_uint au32;


namespace
{
    JavaVM *javaVM;
    jclass MainActivity;
    jobject mainActivity;


    EGLDisplay display  = EGL_NO_DISPLAY;
    EGLSurface surface  = EGL_NO_SURFACE;
    EGLContext context  = EGL_NO_CONTEXT;
    EGLConfig config    = 0;


    char*           TITLE           = "text rendering";
    volatile s32    WIDTH           = 1280;
    volatile s32    HEIGHT          = 720;
    s32             RED_BITS        = 8;
    s32             GREEN_BITS      = 8;
    s32             BLUE_BITS       = 8;
    s32             ALPHA_BITS      = 8;
    s32             COLOUR_BITS     = RED_BITS + GREEN_BITS + BLUE_BITS + ALPHA_BITS;
    s32             DEPTH_BITS      = 24;
    s32             STENCIL_BITS    = 8;


    AAssetManager* assetManager;


    bool    platformIsInitialised   = false;
    bool    appIsInitialised        = false;
    bool    surfaceIsCurrent        = false;

    volatile bool active            = false;
    volatile bool wasActive         = false;


    void createDisplay()
    {
        display = eglGetDisplay( EGL_DEFAULT_DISPLAY );
        if( display == EGL_NO_DISPLAY )
        {
            LOG( "eglGetDisplay failed" );
            return;
        }

        if( !eglInitialize( display, 0, 0 ) )
        {
            LOG( "eglInitialize failed" );
            return;
        }
    }

    void createContext()
    {
        EGLint configAttributes[] =
        {
            EGL_RENDERABLE_TYPE,    EGL_OPENGL_ES3_BIT,
            EGL_RED_SIZE,           RED_BITS,
            EGL_GREEN_SIZE,         GREEN_BITS,
            EGL_BLUE_SIZE,          BLUE_BITS,
            EGL_ALPHA_SIZE,         ALPHA_BITS,
            EGL_BUFFER_SIZE,        COLOUR_BITS,
            EGL_DEPTH_SIZE,         DEPTH_BITS,
            EGL_STENCIL_SIZE,       STENCIL_BITS,
            EGL_NONE
        };

        s32 configCount = 0;
        if( !eglChooseConfig( display,
                                configAttributes,
                                NULL,
                                0,
                                &configCount ) )
        {
            LOG( "eglChooseConfig failed" );
            return;
        }

        if( configCount < 1 )
        {
            LOG( "eglChooseConfig configCount < 1" );
            return;
        }

        EGLConfig* configArray = NEW( EGLConfig, configCount );

        if( !eglChooseConfig( display,
                                configAttributes,
                                configArray,
                                configCount,
                                &configCount ) )
        {
            LOG( "eglChooseConfig failed" );
            return;
        }

        config = configArray[ 0 ];
        for( s32 i = 0; i < configCount; ++i )
        {
            s32 value;
            if( eglGetConfigAttrib( display, configArray[ i ], EGL_RED_SIZE, &value ) && value >= RED_BITS &&
                eglGetConfigAttrib( display, configArray[ i ], EGL_GREEN_SIZE, &value ) && value >= GREEN_BITS &&
                eglGetConfigAttrib( display, configArray[ i ], EGL_BLUE_SIZE, &value ) && value >= BLUE_BITS &&
                eglGetConfigAttrib( display, configArray[ i ], EGL_ALPHA_SIZE, &value ) && value >= ALPHA_BITS &&
                eglGetConfigAttrib( display, configArray[ i ], EGL_DEPTH_SIZE, &value ) && value >= DEPTH_BITS &&
                eglGetConfigAttrib( display, configArray[ i ], EGL_STENCIL_SIZE, &value ) && value >= STENCIL_BITS )
            {
                config = configArray[ i ];
                break;
            }
        }

        FREE( configArray );


        EGLint contextAttributes[] =
        {
            EGL_CONTEXT_MAJOR_VERSION, 3,
            EGL_CONTEXT_MINOR_VERSION, 0,
            EGL_NONE
        };

        context = eglCreateContext( display, config, NULL, contextAttributes );
        if( context == EGL_NO_CONTEXT )
        {
            LOG( "eglCreateContext failed" );
            return;
        }
    }

    void createSurface( ANativeWindow *window )
    {
        surface = eglCreateWindowSurface( display, config, window, NULL );
        if( surface == EGL_NO_SURFACE )
        {
            LOG( "eglCreateWindowSurface failed" );
            return;
        }
    }

    void destroySurface()
    {
        if( surface != EGL_NO_SURFACE )
        {
            eglDestroySurface( display,
                                surface );
            surface = EGL_NO_SURFACE;
        }
    }

    void destroyContext()
    {
        if( context != EGL_NO_CONTEXT )
        {
            eglDestroyContext( display, context );
            context = EGL_NO_CONTEXT;
        }
    }

    void destroyDisplay()
    {
        if( display != EGL_NO_DISPLAY )
        {
            eglTerminate( display );
            display = EGL_NO_DISPLAY;
        }
    }


    struct ThreadInfo
    {
        s32 threadIndex;
        pthread_t threadId;
        bool isRunning;
    };

    struct WorkQueueEntry
    {
        Platform::WorkCallback* callback;
        void* data;
    };


    WorkQueueEntry workQueueEntry[ 256 ];
    const u32 QUEUE_ENTRY_MASK          = ARRAY_COUNT( workQueueEntry ) - 1;
    u32 workQueueEntryStartedCount      = 0;
    au32 workQueueEntryCompletedCount   = { 0 };
    au32 workQueueEntryReadIndex        = { 0 };
    au32 workQueueEntryWriteIndex       = { 0 };

    sem_t semaphoreMain;
    sem_t semaphore;
    ThreadInfo* threadInfo;
    s32 threadCount = 0;


    bool DoWork( ThreadInfo* info )
    {
        bool result = false;

        u32 readIndex = workQueueEntryReadIndex;
        if( readIndex != atomic_load( &workQueueEntryWriteIndex ) )
        {
            result = true;

            u32 nextReadIndex = ( readIndex + 1 ) & QUEUE_ENTRY_MASK;
            if( atomic_compare_exchange_weak( &workQueueEntryReadIndex, &readIndex, nextReadIndex ) )
            {
                WorkQueueEntry entry = workQueueEntry[ readIndex ];
                entry.callback( info->threadIndex, entry.data );

                ++workQueueEntryCompletedCount;
            }
        }

        return result;
    }

    void* ThreadProc( void* arg )
    {
        ThreadInfo *info = ( ThreadInfo* ) arg;

        while( info->isRunning )
        {
            if( !DoWork( info ) )
                sem_wait( &semaphore );
        };

        return 0;
    }

    void* MainThreadProc( void* arg )
    {
        ThreadInfo *info = ( ThreadInfo* ) arg;

        while( info->isRunning )
        {
            if( surfaceIsCurrent )
            {
                if( surface == EGL_NO_SURFACE )
                {
                    eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, context );
                    active = false;
                    surfaceIsCurrent = false;
                }
            }
            else
            {
                if( surface != EGL_NO_SURFACE )
                {
                    if( eglMakeCurrent( display, surface, surface, context ) )
                    {
                        surfaceIsCurrent = true;
                        eglSwapInterval( display, 1 );

                        if( appIsInitialised )
                        {
                            App::Resize( WIDTH, HEIGHT );
                        }
                        else
                        {
                            LogGLInfo();
                            App::Init( WIDTH, HEIGHT );
                            appIsInitialised = true;
                        }
                    }
                    else
                    {
                        LOG( "eglMakeCurrent failed" );
                        surface = EGL_NO_SURFACE;
                        active = false;
                    }
                }
            }
            

            if( active )
            {
                if( surfaceIsCurrent )
                {
                    App::Update();
                    App::Render();
                    eglSwapBuffers( display, surface );
                }
            }
            else
            {
                sem_wait( &semaphoreMain );
            }
        };

        eglMakeCurrent( display, EGL_NO_SURFACE, EGL_NO_SURFACE, context );
        active = false;
        surfaceIsCurrent = false;

        return 0;
    }

    void ResumeMainThread()
    {
        sem_post( &semaphoreMain );
    }

    void PauseMainThread()
    {
        s32 value = 1;
        while( value )
            sem_getvalue( &semaphoreMain, &value );
    }
}


namespace Platform
{
    void Log( char* string, ... )
    {
        const s32 OUTPUT_LENGTH = 1024;
        char output[ OUTPUT_LENGTH ];

        va_list args;
        va_start( args, string );
        vsnprintf( output, OUTPUT_LENGTH, string, args );
        __android_log_print( ANDROID_LOG_INFO, TITLE, "%s", output );
        va_end( args );
    }


    f64 GetTime()
    {
        f64 result = 0;

        timespec timeSpec;
        if( clock_gettime( CLOCK_MONOTONIC_RAW, &timeSpec ) == 0 )
            result = timeSpec.tv_sec + ( timeSpec.tv_nsec * 1e-9 );

        return result;
    }


    s32 GetProcessorCount()
    {
        s32 result = MAX( ( s32 ) sysconf( _SC_NPROCESSORS_ONLN ), 1 );
        return result;
    }

    void FinishWork()
    {
        ASSERT( pthread_equal( pthread_self(), threadInfo[ 0 ].threadId ) );

        while( atomic_load( &workQueueEntryCompletedCount ) < workQueueEntryStartedCount )
            DoWork( &threadInfo[ 0 ] );

        workQueueEntryStartedCount = 0;
        atomic_store( &workQueueEntryCompletedCount, 0u );
    }

    void AddWorkQueueEntry( WorkCallback* callback, void* data )
    {
        ASSERT( pthread_equal( pthread_self(), threadInfo[ 0 ].threadId ) );

        u32 nextWriteIndex = ( workQueueEntryWriteIndex + 1 ) & QUEUE_ENTRY_MASK;
        if( nextWriteIndex == atomic_load( &workQueueEntryReadIndex ) )
        {
            LOG( "Work queue full!" );
            FinishWork();
        }

        WorkQueueEntry* entry = &workQueueEntry[ workQueueEntryWriteIndex ];
        entry->callback = callback;
        entry->data = data;

        ++workQueueEntryStartedCount;
        atomic_store( &workQueueEntryWriteIndex, nextWriteIndex );

        sem_post( &semaphore );
    }

    File ReadFile( char* filename )
    {
        File file = {};

        AAsset* asset = AAssetManager_open( assetManager, filename, AASSET_MODE_BUFFER );

        if( asset )
        {
            file.size = AAsset_getLength64( asset );

            if( file.size > 0 )
            {
                //@NOTE for this project, if the file is more than 16MB, somethings wrong
                ASSERT( file.size < ( 0xffffff ) );
                file.data = ALIGNED_ALLOC( file.size, 1 );

                s32 bytesRead = AAsset_read( asset, file.data, file.size );

                if( bytesRead != file.size )
                {
                    LOG( "AAsset_read bytes read doesn't match file.size, bytes read: %d, file size: %lu", bytesRead, file.size );
                    FreeFile( &file );
                }
                else if( bytesRead < 0 )
                {
                    LOG( "AAsset_read failed" );
                    FreeFile( &file );
                }
            }
            else
            {
                LOG( "file.size is %ld", file.size );
                FreeFile( &file );
            }

            AAsset_close( asset );
        }
        else
        {
            LOG( "AAssetManager_open failed" );
        }

        return file;
    }

    void FreeFile( File* file )
    {
        ALIGNED_FREE( file->data );
        *file = {};
    }


    u8* CreateTextBitmap( char* string,
                            char* fontFilename,
                            char* fontFacename,
                            s32 fontSize,
                            s32 kerning,
                            HAlignment hAlignment,
                            v2i* textSize,
                            v2i* bitmapSize )
    {
        u8* textBitmap = 0;

        *bitmapSize = {};


        JNIEnv* env;
        if( javaVM->AttachCurrentThread( &env, NULL ) != JNI_OK )
        {
            LOG( "AttachCurrentThread failed" );
            return 0;
        }


        jmethodID getTextSizeKt = env->GetMethodID( MainActivity, "getTextSize", "(Ljava/lang/String;Ljava/lang/String;III[I)V" );
        if( getTextSizeKt )
        {
            jstring jString         = env->NewStringUTF( string );
            jstring jFontFilename   = env->NewStringUTF( fontFilename );
            jintArray jTextSize     = env->NewIntArray( 2 );
            env->SetIntArrayRegion( jTextSize, 0, 2, ( s32* ) textSize );

            env->CallVoidMethod( mainActivity,
                                    getTextSizeKt,
                                    jString,
                                    jFontFilename,
                                    fontSize,
                                    kerning,
                                    ( s32 ) hAlignment,
                                    jTextSize );

            env->GetIntArrayRegion( jTextSize, 0, 2, ( s32* ) textSize );

            s32 maxTextureSize = GLHelper::GetMaxTextureSize();
            bitmapSize->x = MIN( Utils::CeilToPower2( textSize->x ), maxTextureSize );
            bitmapSize->y = MIN( Utils::CeilToPower2( textSize->y ), maxTextureSize );

            textBitmap = NEW( u8, bitmapSize->x * bitmapSize->y );
            memset( textBitmap, 0, bitmapSize->x * bitmapSize->y );

            jmethodID getTextBitmapKt = env->GetMethodID( MainActivity, "getTextBitmap", "(Ljava/lang/String;Ljava/lang/String;IIIII[I[I)V" );
            if( getTextBitmapKt )
            {
                jintArray jPixels = env->NewIntArray( bitmapSize->x * bitmapSize->y );

                env->CallVoidMethod( mainActivity,
                                        getTextBitmapKt,
                                        jString,
                                        jFontFilename,
                                        fontSize,
                                        kerning,
                                        ( s32 ) hAlignment,
                                        bitmapSize->x,
                                        bitmapSize->y,
                                        jTextSize,
                                        jPixels );

                jint* jPixelsPtr = env->GetIntArrayElements( jPixels, NULL );

                for( s32 i = 0; i < bitmapSize->x * bitmapSize->y; ++i )
                    textBitmap[ i ] = ( jPixelsPtr[ i ] >> 24 ) & 0xff;

                env->ReleaseIntArrayElements( jPixels, jPixelsPtr, 0 );
                env->DeleteLocalRef( jPixels );
            }
            else
            {
                LOG( "GetMethodID( getTextBitmapKt ) failed" );
            }

            env->DeleteLocalRef( jString );
            env->DeleteLocalRef( jFontFilename );
            env->DeleteLocalRef( jTextSize );
        }
        else
        {
            LOG( "GetMethodID( getTextSizeKt ) failed" );
        }


        if( javaVM->DetachCurrentThread() != JNI_OK )
        {
            LOG( "DetachCurrentThread failed" );
            return 0;
        }

        return textBitmap;
    }

    void FreeTextBitmap( u8** textBitmap )
    {
        FREE( *textBitmap );
        *textBitmap = 0;
    }

    //@NOTE ASCII only
    u8* CreateFontAtlasBitmap( char* string,
                                char* fontFilename,
                                char* fontFacename,
                                s32 fontSize,
                                s32 kerning,
                                s32 padding,
                                v2i* bitmapSize,
                                FontAtlas* fontAtlas )
    {
        u8* fontAtlasBitmap = 0;

        *bitmapSize = {};

        ASSERT( fontAtlas->glyphList == 0 );
        fontAtlas->glyphCount   = ( s32 ) strlen( string );
        fontAtlas->glyphList    = NEW( Glyph, fontAtlas->glyphCount );
        fontAtlas->ascent       = 0;
        fontAtlas->descent      = 0;
        fontAtlas->kerning      = kerning;


        JNIEnv* env;
        if( javaVM->AttachCurrentThread( &env, NULL ) != JNI_OK )
        {
            LOG( "AttachCurrentThread failed" );
            return 0;
        }


        jmethodID getFontAtlasTextSizeKt = env->GetMethodID( MainActivity, "getFontAtlasTextSize", "(Ljava/lang/String;Ljava/lang/String;I[I[I[S[S)V" );
        if( getFontAtlasTextSizeKt )
        {
            jstring jString         = env->NewStringUTF( string );
            jstring jFontFilename   = env->NewStringUTF( fontFilename );
            jintArray jAscent       = env->NewIntArray( 1 );
            jintArray jDescent      = env->NewIntArray( 1 );
            jshortArray jGlyphX     = env->NewShortArray( fontAtlas->glyphCount );
            jshortArray jGlyphY     = env->NewShortArray( fontAtlas->glyphCount );

            env->CallVoidMethod( mainActivity,
                                    getFontAtlasTextSizeKt,
                                    jString,
                                    jFontFilename,
                                    fontSize,
                                    jAscent,
                                    jDescent,
                                    jGlyphX,
                                    jGlyphY );

            env->GetIntArrayRegion( jAscent, 0, 1, &fontAtlas->ascent );
            env->GetIntArrayRegion( jDescent, 0, 1, &fontAtlas->descent );

            jshort* jGlyphXPtr  = env->GetShortArrayElements( jGlyphX, NULL );
            jshort* jGlyphYPtr  = env->GetShortArrayElements( jGlyphY, NULL );

            for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
            {
                Glyph* glyph = &fontAtlas->glyphList[ i ];
                glyph->codepoint = string[ i ];
                glyph->width = jGlyphXPtr[ i ];
                glyph->height = jGlyphYPtr[ i ];
                bitmapSize->y = MAX( bitmapSize->y, glyph->height * 2 );
            }

            bitmapSize->x = Utils::CeilToPower2( bitmapSize->y );
            bitmapSize->y = bitmapSize->x;

            env->ReleaseShortArrayElements( jGlyphX, jGlyphXPtr, 0 );
            env->ReleaseShortArrayElements( jGlyphY, jGlyphYPtr, 0 );

            env->DeleteLocalRef( jAscent );
            env->DeleteLocalRef( jDescent );


            Utils::GlyphPack( padding,
                                GLHelper::GetMaxTextureSize(),
                                bitmapSize,
                                fontAtlas );


            fontAtlasBitmap = NEW( u8, bitmapSize->x * bitmapSize->y );
            memset( fontAtlasBitmap, 0, bitmapSize->x * bitmapSize->y );


            jmethodID getFontAtlasBitmapKt = env->GetMethodID( MainActivity, "getFontAtlasBitmap", "(Ljava/lang/String;Ljava/lang/String;III[S[S[I)V" );
            if( getFontAtlasBitmapKt )
            {
                jGlyphXPtr = env->GetShortArrayElements( jGlyphX, NULL );
                jGlyphYPtr = env->GetShortArrayElements( jGlyphY, NULL );

                for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                {
                    Glyph* glyph = &fontAtlas->glyphList[ i ];
                    jGlyphXPtr[ i ] = glyph->atlasX;
                    jGlyphYPtr[ i ] = glyph->atlasY;
                }

                env->ReleaseShortArrayElements( jGlyphX, jGlyphXPtr, 0 );
                env->ReleaseShortArrayElements( jGlyphY, jGlyphYPtr, 0 );

                jintArray jPixels = env->NewIntArray( bitmapSize->x * bitmapSize->y );

                env->CallVoidMethod( mainActivity,
                                        getFontAtlasBitmapKt,
                                        jString,
                                        jFontFilename,
                                        fontSize,
                                        bitmapSize->x,
                                        bitmapSize->y,
                                        jGlyphX,
                                        jGlyphY,
                                        jPixels );

                jint* jPixelsPtr = env->GetIntArrayElements( jPixels, NULL );

                for( s32 i = 0; i < bitmapSize->x * bitmapSize->y; ++i )
                    fontAtlasBitmap[ i ] = ( jPixelsPtr[ i ] >> 24 ) & 0xff;

                env->ReleaseIntArrayElements( jPixels, jPixelsPtr, 0 );
                env->DeleteLocalRef( jPixels );
            }
            else
            {
                LOG( "GetMethodID( getFontAtlasBitmapKt ) failed" );
            }


            env->DeleteLocalRef( jString );
            env->DeleteLocalRef( jFontFilename );
            env->DeleteLocalRef( jGlyphX );
            env->DeleteLocalRef( jGlyphY );
        }
        else
        {
            LOG( "GetMethodID( getFontAtlasTextSizeKt ) failed" );
        }


        if( javaVM->DetachCurrentThread() != JNI_OK )
        {
            LOG( "DetachCurrentThread failed" );
            return 0;
        }

        return fontAtlasBitmap;
    }

    void FreeFontAtlasBitmap( u8** fontAtlasBitmap )
    {
        FREE( *fontAtlasBitmap );
        *fontAtlasBitmap = 0;
    }

    void FreeFontAtlas( FontAtlas* fontAtlas )
    {
        FREE( fontAtlas->glyphList );
        *fontAtlas = {};
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_textrendering_MainActivity_init( JNIEnv* env,
                                                    jobject instance,
                                                    jobject assetManagerJava )
{
    if( !platformIsInitialised )
    {
        env->GetJavaVM( &javaVM );
        jclass instanceClass = env->GetObjectClass( instance );
        MainActivity = ( jclass ) env->NewGlobalRef( instanceClass );
        mainActivity = env->NewGlobalRef( instance );

        createDisplay();
        createContext();

        sem_init( &semaphoreMain, 0, 0 );
        sem_init( &semaphore, 0, 0 );

        ASSERT( Utils::IsPowerOf2( ARRAY_COUNT( workQueueEntry ) ) );

        threadCount                 = Platform::GetProcessorCount();
        threadInfo                  = NEW( ThreadInfo, threadCount );
        threadInfo[ 0 ].threadIndex = 0;
        threadInfo[ 0 ].isRunning   = true;
        pthread_create( &threadInfo[ 0 ].threadId, NULL, MainThreadProc, &threadInfo[ 0 ] );

        for( s32 i = 1; i < threadCount; ++i )
        {
            threadInfo[ i ].threadIndex = i;
            threadInfo[ i ].isRunning   = true;
            pthread_create( &threadInfo[ i ].threadId, NULL, ThreadProc, &threadInfo[ i ] );
        }


        assetManager = AAssetManager_fromJava( env, assetManagerJava );


        platformIsInitialised = true;
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_textrendering_MainActivity_start( JNIEnv* env,
                                                    jobject /* this */ )
{
    active = true;
    wasActive = active;
    ResumeMainThread();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_textrendering_MainActivity_stop( JNIEnv* env,
                                                    jobject /* this */ )
{
    active = false;
    wasActive = active;
    PauseMainThread();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_textrendering_MainActivity_destroy( JNIEnv* env,
                                                        jobject /* this */ )
{
    if( surface != EGL_NO_SURFACE )
    {
        LOG( "surfaceDestroyed not called by android, destroying surface now" );
        destroySurface();
    }
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_textrendering_MainActivity_surfaceChanged( JNIEnv* env,
                                                            jobject /* this */,
                                                            jobject androidSurface,
                                                            jint width,
                                                            jint height )
{
    WIDTH = width;
    HEIGHT = height;

    ASSERT( display != EGL_NO_DISPLAY );
    ASSERT( context != EGL_NO_CONTEXT );

    
    ANativeWindow *window = ANativeWindow_fromSurface( env, androidSurface );
    if( !window )
    {
        LOG( "ANativeWindow_fromSurface failed" );
        return;
    }

    createSurface( window );

    ANativeWindow_release( window );


    active = wasActive;
    if( active )
        ResumeMainThread();
}

extern "C" JNIEXPORT void JNICALL
Java_com_example_textrendering_MainActivity_surfaceDestroyed( JNIEnv* env,
                                                                jobject /* this */ )
{
    wasActive = active;
    active = false;
    PauseMainThread();

    destroySurface();
}
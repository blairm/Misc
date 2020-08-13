#include "../Shared/App.h"
#include "../Shared/Platform.h"
#include "../Shared/Types.h"
#include "../Shared/Utils.h"
#include "../Shared/C3dMaths.h"
#define GLHELPER_PLATFORM_FUNCTIONS
#include "../Shared/GLHelper.h"

#include <vector>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <sys/sysinfo.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <X11/Xlib.h>
#include <GL/glx.h>

#include <fontconfig/fontconfig.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <pango/pangocairo.h>

#include <atomic>
typedef std::atomic_uint au32;


namespace
{
    typedef GLXContext APIENTRY GLXCreateContextAttribsARB( Display* display, GLXFBConfig config, GLXContext shareContext, bool direct, const int* attrib_list );
    GLXCreateContextAttribsARB* glXCreateContextAttribsARB = NULL;

    typedef void APIENTRY GLXSwapIntervalEXT( Display* display, GLXDrawable drawable, int interval );
    GLXSwapIntervalEXT* _glXSwapIntervalEXT = NULL;
    typedef int APIENTRY GLXSwapIntervalMESA( unsigned int interval );
    GLXSwapIntervalMESA* _glXSwapIntervalMESA = NULL;
    typedef int APIENTRY GLXSwapIntervalSGI( int interval );
    GLXSwapIntervalSGI* _glXSwapIntervalSGI = NULL;

    enum GLXSwapIntervalMode
    {
        GLXSwapIntervalMode_None = 0,
        GLXSwapIntervalMode_Ext,
        GLXSwapIntervalMode_MESA,
        GLXSwapIntervalMode_SGI
    };

    GLXSwapIntervalMode glXSwapIntervalMode = GLXSwapIntervalMode_None;


    Display*    display         = NULL;
    s32         screenId        = 0;
    Window      window;
    GLXContext  glXContext      = 0;
    Atom        wmDeleteWindow;

    char*       TITLE           = "Text Rendering";
    s32         WIDTH           = 1280;
    s32         HEIGHT          = 720;
    s32         COLOUR_BITS     = 32;
    s32         DEPTH_BITS      = 24;
    s32         STENCIL_BITS    = 8;

    bool        active          = true;
    bool        fullscreen      = false;


    bool HasGLXExtension( char* extension )
    {
        char* extensionString = ( char* ) glXQueryExtensionsString( display, screenId );

        bool result = strstr( extensionString, extension );
        return result;
    }

    void glXSwapInterval( s32 interval )
    {
        switch( glXSwapIntervalMode )
        {
            case GLXSwapIntervalMode_Ext:   _glXSwapIntervalEXT( display, window, interval );   break;
            case GLXSwapIntervalMode_MESA:  _glXSwapIntervalMESA( interval );                   break;
            case GLXSwapIntervalMode_SGI:   _glXSwapIntervalSGI( interval );                    break;
            default: break;
        }
    }
    

    bool ProcessEvent( XEvent* xev )
    {
        switch( xev->type )
        {
            case KeyPress:
            {
                return true;
            }
            case KeyRelease:
            {
                return true;
            }
            case ButtonPress:
            {
                //s32 delta = 0;

                XButtonEvent* event = ( XButtonEvent* ) xev;
                if( event->button == 1
                    || event->button == 2
                    || event->button == 3 )
                {
                    //s32 x = event->x;
                    //s32 y = event->y;
                }
                else if( event->button == 4 )
                {
                    //delta = 1;
                }
                else if( event->button == 5 )
                {
                    //delta = -1;
                }

                return true;
            }
            case ButtonRelease:
            {
                XButtonEvent* event = ( XButtonEvent* ) xev;
                if( event->button == 1
                    || event->button == 2
                    || event->button == 3 )
                {
                    //s32 x = event->x;
                    //s32 y = event->y;
                }

                return true;
            }
            case MotionNotify:
            {
                //XMotionEvent* event = ( XMotionEvent* ) xev;
                //s32 x = event->x;
                //s32 y = event->y;
                return true;
            }
            case UnmapNotify:
            {
                active = false;
                return true;
            }
            case MapNotify:
            {
                active = true;
                return true;
            }
            case ConfigureNotify:
            {
                XConfigureEvent* event = ( XConfigureEvent* ) xev;
                WIDTH = event->width;
                HEIGHT = event->height;
                App::Resize( WIDTH, HEIGHT );
                return true;
            }
        }

        return false;
    }

    void DestroyWindowGL()
    {
        if( !glXMakeCurrent( display, None, NULL ) )
            LOG( "glXMakeCurrent( display, None, NULL ) failed\n" );

        glXDestroyContext( display, glXContext );

        XDestroyWindow( display, window );

        XCloseDisplay( display );
        display = NULL;
    }

    bool CreateWindowGL( bool fullscreenFlag )
    {
        fullscreen = fullscreenFlag;

        display = XOpenDisplay( NULL );
        if( !display )
        {
            LOG( "XOpenDisplay( NULL ) failed\n" );
            return false;
        }

        Window root = DefaultRootWindow( display );

        s32 glXMajor;
        s32 glXMinor;
        if( !glXQueryVersion( display, &glXMajor, &glXMinor ) )
        {
            LOG( "glXQueryVersion( display, &glXMajor, &glXMinor ) failed\n" );
            return false;
        }

        if( glXMajor < 1 || ( glXMajor == 1 && glXMinor < 3 ) )
        {
            LOG( "glX version needs to be 1.3 or higher, got %d.%d\n", glXMajor, glXMinor );
            return false;
        }

        s32 bitsPerColour           = COLOUR_BITS / 4;
        GLint visualAttributes[]    = { GLX_X_RENDERABLE,   GL_TRUE,
                                        GLX_DRAWABLE_TYPE,  GLX_WINDOW_BIT,
                                        GLX_RENDER_TYPE,    GLX_RGBA_BIT,
                                        GLX_X_VISUAL_TYPE,  GLX_TRUE_COLOR,
                                        GLX_RED_SIZE,       bitsPerColour,
                                        GLX_GREEN_SIZE,     bitsPerColour,
                                        GLX_BLUE_SIZE,      bitsPerColour,
                                        GLX_ALPHA_SIZE,     bitsPerColour,
                                        GLX_DEPTH_SIZE,     DEPTH_BITS,
                                        GLX_STENCIL_SIZE,   STENCIL_BITS,
                                        GLX_DOUBLEBUFFER,   GL_TRUE,
                                        None };

        s32 configCount;
        GLXFBConfig* fbConfigArray = glXChooseFBConfig( display, DefaultScreen( display ), visualAttributes, &configCount );

        if( !fbConfigArray )
        {
            LOG( "glXChooseFBConfig( display, DefaultScreen( display ), visualAttributes, &configCount )\n" );
            return false;
        }

        GLXFBConfig fbConfig = fbConfigArray[ 0 ];
        XFree( fbConfigArray );

        XVisualInfo* visualInfoPtr = glXGetVisualFromFBConfig( display, fbConfig );

        if( !visualInfoPtr )
        {
            LOG( "glXGetVisualFromFBConfig( display, fbConfig ) failed\n" );
            return false;
        }

        XVisualInfo visualInfo = *visualInfoPtr;
        XFree( visualInfoPtr );

        if( fullscreen )
        {
            Screen* screen = XScreenOfDisplay( display, screenId );
            if( screen )
            {
                WIDTH = XWidthOfScreen( screen );
                HEIGHT = XHeightOfScreen( screen );
            }
            else
            {
                LOG( "XScreenOfDisplay( display, screenId ) failed\n" );
                fullscreen = false;
            }
        }

        Colormap colourMap = XCreateColormap( display, root, visualInfo.visual, AllocNone );

        XSetWindowAttributes windowAttributes   = {};
        windowAttributes.event_mask             = ExposureMask;
        windowAttributes.event_mask             |= StructureNotifyMask;
        windowAttributes.event_mask             |= KeyPressMask | KeyReleaseMask;
        windowAttributes.event_mask             |= PointerMotionMask | ButtonPressMask | ButtonReleaseMask;
        windowAttributes.colormap               = colourMap;

        window = XCreateWindow( display,
                                root,
                                0, 0,
                                WIDTH, HEIGHT,
                                0,
                                visualInfo.depth,
                                InputOutput,
                                visualInfo.visual,
                                CWEventMask | CWColormap | CWCursor,
                                &windowAttributes );

        XMapWindow( display, window );
        XStoreName( display, window, TITLE );

        if( fullscreen )
        {
            Atom wmState                = XInternAtom( display, "_NET_WM_STATE", false );
            Atom wmStateFullscreen      = XInternAtom( display, "_NET_WM_STATE_FULLSCREEN", false );

            XEvent xev                  = {};
            xev.type                    = ClientMessage;
            xev.xclient.window          = window;
            xev.xclient.message_type    = wmState;
            xev.xclient.format          = 32;
            xev.xclient.data.l[0]       = 1;
            xev.xclient.data.l[1]       = wmStateFullscreen;
            xev.xclient.data.l[2]       = 0;

            XSendEvent( display,
                        root,
                        false,
                        SubstructureRedirectMask | SubstructureNotifyMask,
                        &xev );

            XFlush( display );
        }

        if( HasGLXExtension( "GLX_ARB_create_context" ) )
        {
            glXCreateContextAttribsARB = ( GLXCreateContextAttribsARB* ) glXGetProcAddress( ( GLubyte* ) "glXCreateContextAttribsARB" );

            s32 attribs[] = { GLX_CONTEXT_MAJOR_VERSION_ARB, OPENGL_VERSION_WANTED_MAJOR,
                                GLX_CONTEXT_MINOR_VERSION_ARB, OPENGL_VERSION_WANTED_MINOR,
                                #ifdef OPENGL_DEBUG_CONTEXT
                                    GLX_CONTEXT_FLAGS_ARB, GLX_CONTEXT_DEBUG_BIT_ARB,// | GLX_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                                #endif
                                GLX_CONTEXT_PROFILE_MASK_ARB,
                                #ifdef OPENGL_CORE
                                    GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
                                #else
                                    GLX_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                                #endif
                                0 };
            glXContext = glXCreateContextAttribsARB( display, fbConfig, NULL, GL_TRUE, attribs );
            if( glXContext )
            {
                if( !glXMakeCurrent( display, window, glXContext ) )
                    LOG( "glXMakeCurrent( display, window, glXContext ) with attribs failed\n" );
            }
        }

        if( !glXContext )
        {
            glXContext = glXCreateContext( display, &visualInfo, NULL, GL_TRUE );
            if( !glXContext )
            {
                LOG( "glXCreateContext( display, &visualInfo, NULL, GL_TRUE ) failed\n" );
                return false;
            }

            if( !glXMakeCurrent( display, window, glXContext ) )
            {
                LOG( "glXMakeCurrent( display, window, glXContext ) failed\n" );
                return false;
            }
        }
        

        LoadGLFunctions();

        if( !GLVersionMeets( OPENGL_VERSION_WANTED_MAJOR, OPENGL_VERSION_WANTED_MINOR ) )
        {
            LOG( "doesn't support required opengl version\n" );
            return false;
        }

        #ifdef OPENGL_DEBUG_CONTEXT
            s32 flags;
            glGetIntegerv( GL_CONTEXT_FLAGS, &flags );
            if( ( flags & GL_CONTEXT_FLAG_DEBUG_BIT ) == 0 )
            {
                LOG( "failed to get a debug context\n" );
                return false;
            }
        #endif

        LogGLInfo();


        //*@TODO test on actual hardware not vm
        if( HasGLXExtension( "GLX_EXT_swap_control" ) )
        {
            _glXSwapIntervalEXT = ( GLXSwapIntervalEXT* ) glXGetProcAddress( ( GLubyte* ) "glXSwapIntervalEXT" );
            glXSwapIntervalMode = GLXSwapIntervalMode_Ext;
        }
        else if( HasGLXExtension( "GLX_MESA_swap_control" ) )
        {
            _glXSwapIntervalMESA = ( GLXSwapIntervalMESA* ) glXGetProcAddress( ( GLubyte* ) "glXSwapIntervalMESA" );
            glXSwapIntervalMode = GLXSwapIntervalMode_MESA;
        }
        else if( HasGLXExtension( "GLX_SGI_swap_control" ) )
        {
            _glXSwapIntervalSGI = ( GLXSwapIntervalSGI* ) glXGetProcAddress( ( GLubyte* ) "glXSwapIntervalSGI" );
            glXSwapIntervalMode = GLXSwapIntervalMode_SGI;
        }

        if( HasGLXExtension( "GLX_EXT_swap_control_tear" ) )
            glXSwapInterval( -1 );
        else
            glXSwapInterval( 1 );
        //*/

        wmDeleteWindow = XInternAtom( display, "WM_DELETE_WINDOW", false );
        XSetWMProtocols( display, window, &wmDeleteWindow, 1 );

        return true;
    }


    struct ThreadInfo
    {
        s32 threadIndex;
        pthread_t threadId;
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

    sem_t semaphore;
    ThreadInfo* threadInfo;


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

        while( true )
        {
            if( !DoWork( info ) )
                sem_wait( &semaphore );
        };
    }


    s32 FTFontUnitsToPixels( s32 fontUnits, s32 unitsPerEM, s32 fontSize )
    {
        s32 result = SROUND( ( fontUnits / ( f32 ) unitsPerEM ) * fontSize );
        return result;
    }

    void BlitGlyph( u8* dst, v2i dstSize, u8* src, v2i srcPos, v2i srcSize )
    {
        v2i dstPos = {};

        for( s32 j = 0; j < srcSize.y; ++j )
        {
            dstPos.y = srcPos.y + j;
            if( dstPos.y >= 0 && dstPos.y < dstSize.y )
            {
                for( s32 i = 0; i < srcSize.x; ++i )
                {
                    dstPos.x = srcPos.x + i;

                    if( dstPos.x < 0 )
                    {
                        i += dstPos.x - 1;
                        continue;
                    }
                    else if( dstPos.x >= dstSize.x )
                    {
                        break;
                    }

                    u8* pixel = &dst[ dstPos.y * dstSize.x + dstPos.x ];
                    *pixel = ( u8 ) MIN( ( u16 ) *pixel + src[ j * srcSize.x + i ], U8_MAX );
                }
            }
            else if( dstPos.y < 0 )
            {
                j += -dstPos.y - 1;
                continue;
            }
            else
            {
                break;
            }
        }
    }
}

namespace Platform
{
    void Log( char* string, ... )
    {
        va_list args;
        va_start( args, string );
        vprintf( string, args );
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
        s32 result = MAX( get_nprocs(), 1 );
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
            LOG( "Work queue full!\n" );
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

        s32 fileDescriptor = open( filename, O_RDONLY );

        if( fileDescriptor >= 0 )
        {
            struct stat buf;
            if( fstat( fileDescriptor, &buf) >= 0 )
            {
                //@NOTE for this project, if the file is more than 16MB, somethings wrong
                ASSERT( buf.st_size < ( 0xffffff ) );
                file.size = buf.st_size;
                file.data = ALIGNED_ALLOC( file.size, 1 );

                u64 bytesRead = read( fileDescriptor, file.data, file.size );

                if( bytesRead != file.size )
                {
                    LOG( "read bytes read doesn't match file.size, bytes read: %d, file size: %lu", bytesRead, file.size );
                    FreeFile( &file );
                }
                else if( bytesRead < 0 )
                {
                    LOG( "read failed" );
                    FreeFile( &file );
                }
            }
            else
            {
                LOG( "fstat failed\n" );
            }

            if( close( fileDescriptor ) < 0 )
                LOG( "close failed\n" );
        }
        else
        {
            LOG( "open failed\n" );
        }

        return file;
    }

    void FreeFile( File* file )
    {
        ALIGNED_FREE( file->data );
        *file = {};
    }
    

    //@TODO check Pango is creating surfaces, font descriptions etc. without issue
    //@TODO thai line height not high enough, manually set line height for thai?
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

        cairo_surface_t* layoutSurface = cairo_image_surface_create( CAIRO_FORMAT_A8, 0, 0 );
        cairo_t* layoutContext = cairo_create( layoutSurface );
        cairo_surface_destroy( layoutSurface );

        PangoLayout* pangoLayout = pango_cairo_create_layout( layoutContext );
        pango_layout_set_width( pangoLayout, textSize->x * PANGO_SCALE );
        pango_layout_set_auto_dir( pangoLayout, false );

        if( hAlignment == HAlignment::Left )
            pango_layout_set_alignment( pangoLayout, PangoAlignment::PANGO_ALIGN_LEFT );
        else if( hAlignment == HAlignment::Centre )
            pango_layout_set_alignment( pangoLayout, PangoAlignment::PANGO_ALIGN_CENTER );
        else
            pango_layout_set_alignment( pangoLayout, PangoAlignment::PANGO_ALIGN_RIGHT );

        pango_layout_set_wrap( pangoLayout, PangoWrapMode::PANGO_WRAP_WORD_CHAR );

        FcConfigAppFontAddFile( NULL, ( FcChar8* ) fontFilename );
        PangoFontDescription* fontDescription = pango_font_description_from_string( fontFacename );
        pango_font_description_set_absolute_size( fontDescription, fontSize * PANGO_SCALE );
        pango_layout_set_font_description( pangoLayout, fontDescription );

        //@NOTE attribute destroyed with attribute list
        PangoAttrList* layoutAttribs = pango_attr_list_new();
        pango_attr_list_insert( layoutAttribs, pango_attr_insert_hyphens_new( false ) );
        pango_layout_set_attributes( pangoLayout, layoutAttribs );

        pango_layout_set_text( pangoLayout, string, -1 );

        pango_layout_get_pixel_size( pangoLayout, &textSize->x, &textSize->y );

        *bitmapSize = Utils::CeilToPower2( *textSize );
                        
        textBitmap = NEW( u8, bitmapSize->x * bitmapSize->y );
        memset( textBitmap, 0, bitmapSize->x * bitmapSize->y );

        cairo_surface_t* renderSurface = cairo_image_surface_create_for_data( textBitmap, CAIRO_FORMAT_A8, bitmapSize->x, bitmapSize->y, bitmapSize->x );
        cairo_t* renderContext = cairo_create( renderSurface );

        if( hAlignment == HAlignment::Centre )
            cairo_translate( renderContext, SROUND( ( textSize->x - bitmapSize->x ) * 0.5f ), 0 );
        else if( hAlignment == HAlignment::Right )
            cairo_translate( renderContext, textSize->x - bitmapSize->x, 0 );

        pango_cairo_show_layout( renderContext, pangoLayout );

        g_object_unref( pangoLayout );
        cairo_destroy( layoutContext );
        pango_font_description_free( fontDescription );
        pango_attr_list_unref( layoutAttribs );
        cairo_destroy( renderContext );
        cairo_surface_destroy( renderSurface );

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
        f32 ft16_16Scale = 1.0f / 65536.0f;

        u8* fontAtlasBitmap = 0;

        *bitmapSize = {};

        ASSERT( fontAtlas->glyphList == 0 );
        fontAtlas->glyphCount   = ( s32 ) strlen( string );
        fontAtlas->glyphList    = NEW( Glyph, fontAtlas->glyphCount );
        fontAtlas->ascent       = 0;
        fontAtlas->descent      = 0;
        fontAtlas->kerning      = kerning;

        FT_Library ftLib;
        if( FT_Init_FreeType( &ftLib ) == FT_Err_Ok )
        {
            FT_Face ftFace;
            if( FT_New_Face( ftLib, fontFilename, 0, &ftFace ) == FT_Err_Ok )
            {
                FT_Set_Pixel_Sizes( ftFace, 0, fontSize );


                fontAtlas->ascent   = FTFontUnitsToPixels( ftFace->ascender, ftFace->units_per_EM, fontSize );
                fontAtlas->descent  = FTFontUnitsToPixels( ftFace->descender, ftFace->units_per_EM, fontSize );


                for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                {
                    FT_Load_Glyph( ftFace, FT_Get_Char_Index( ftFace, string[ i ] ), FT_LOAD_DEFAULT );
                    FT_GlyphSlot ftGlyphSlot = ftFace->glyph;

                    Glyph* glyph = &fontAtlas->glyphList[ i ];
                    glyph->codepoint = string[ i ];
                    glyph->width = ( s16 ) ( ftGlyphSlot->linearHoriAdvance * ft16_16Scale );
                    glyph->height = ( s16 ) ( fontAtlas->ascent - fontAtlas->descent );
                    bitmapSize->y = MAX( bitmapSize->y, glyph->height * 2 );
                }

                bitmapSize->x = Utils::CeilToPower2( bitmapSize->y );
                bitmapSize->y = bitmapSize->x;


                Utils::GlyphPack( padding,
                                    GLHelper::GetMaxTextureSize(),
                                    bitmapSize,
                                    fontAtlas );

                fontAtlasBitmap = NEW( u8, bitmapSize->x * bitmapSize->y );
                memset( fontAtlasBitmap, 0, bitmapSize->x * bitmapSize->y );


                for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                {
                    FT_Load_Glyph( ftFace, FT_Get_Char_Index( ftFace, string[ i ] ), FT_LOAD_DEFAULT );
                    FT_GlyphSlot ftGlyphSlot = ftFace->glyph;
                    FT_Render_Glyph( ftGlyphSlot, FT_RENDER_MODE_NORMAL );
                    FT_Bitmap ftBitmap = ftGlyphSlot->bitmap;

                    Glyph* glyph = &fontAtlas->glyphList[ i ];

                    BlitGlyph( fontAtlasBitmap,
                                *bitmapSize,
                                ftBitmap.buffer,
                                v2i{ glyph->atlasX + ftGlyphSlot->bitmap_left, glyph->atlasY + fontAtlas->ascent - ftGlyphSlot->bitmap_top },
                                v2i{ ( s32 ) ftBitmap.width, ( s32 ) ftBitmap.rows } );
                }

                
                FT_Done_Face( ftFace );
            }
            else
            {
                LOG( "FT_New_Face( ftLib, fontFilename, 0, &ftFace ) failed\n" );
            }


            FT_Done_FreeType( ftLib );
        }
        else
        {
            LOG( "FT_Init_FreeType( &ftLib ) failed\n" );
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


s32 main()
{
    ASSERT( Utils::IsPowerOf2( ARRAY_COUNT( workQueueEntry ) ) );

    s32 threadCount             = Platform::GetProcessorCount();
    threadInfo                  = NEW( ThreadInfo, threadCount );
    threadInfo[ 0 ].threadIndex = 0;
    threadInfo[ 0 ].threadId    = pthread_self();

    sem_init( &semaphore, 0, 0 );

    for( s32 i = 1; i < threadCount; ++i )
    {
        threadInfo[ i ].threadIndex = i;

        pthread_create( &threadInfo[ i ].threadId, NULL, ThreadProc, &threadInfo[ i ] );
    }


	if( !CreateWindowGL( fullscreen ) )
        return 0;


    App::Init( WIDTH, HEIGHT );


    XEvent xev;
    bool done = false;
    while( !done )
    {
        while( XPending( display ) )
        {
            XNextEvent( display, &xev );

            if( !ProcessEvent( &xev ) )
            {
                if( xev.type == ClientMessage && ( u64 ) xev.xclient.data.l[0] == wmDeleteWindow )
                {
                    done = true;
                    active = false;
                    break;
                }
            }
        }

        if( active )
        {
            App::Update();
            App::Render();
            glXSwapBuffers( display, window );
        }
    }

	return 0;
}
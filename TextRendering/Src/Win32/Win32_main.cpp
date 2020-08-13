#include "../Shared/App.h"
#include "../Shared/Platform.h"
#include "../Shared/Types.h"
#include "../Shared/Utils.h"
#include "../Shared/C3dMaths.h"
#define GLHELPER_PLATFORM_FUNCTIONS
#include "../Shared/GLHelper.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0601
#include <windows.h>
#include <windowsx.h>

#include <atomic>
typedef std::atomic_uint au32;


#define WGL_CONTEXT_MAJOR_VERSION_ARB               0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB               0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB                 0x2093
#define WGL_CONTEXT_FLAGS_ARB                       0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB                0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                   0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB      0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB            0x0001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB   0x0002


namespace
{
    f64 INV_COUNTER_FREQUENCY = 1.0 / 1000.0;


    typedef HGLRC APIENTRY WGLCreateContextAttribsARB( HDC hDC, HGLRC hshareContext, const int *attribList );
    WGLCreateContextAttribsARB* wglCreateContextAttribsARB = NULL;
    typedef char* APIENTRY WGLGetExtensionsStringARB();
    WGLGetExtensionsStringARB* wglGetExtensionsStringARB = NULL;
    typedef void APIENTRY WGLSwapIntervalEXT( int interval );
    WGLSwapIntervalEXT* wglSwapIntervalEXT = NULL;


    bool HasGLExtension( char* extension )
    {
        char* extensionString = ( char* ) glGetString( GL_EXTENSIONS );

        bool result = strstr( extensionString, extension );
        return result;
    }

    bool HasWGLExtension( char* extension )
    {
        if( wglGetExtensionsStringARB )
        {
            char* extensionString = ( char* ) wglGetExtensionsStringARB();

            bool result = strstr( extensionString, extension );
            return result;
        }

        return false;
    }


    HGLRC       hRC                     = NULL;
    HDC         hDC                     = NULL;
    HWND        hWnd                    = NULL;
    HINSTANCE   hInstance               = NULL;

    wchar_t*    TITLE                   = L"Text Rendering";
    s32         WIDTH                   = 1280;
    s32         HEIGHT                  = 720;
    s32         COLOUR_BITS             = 32;
    s32         DEPTH_BITS              = 24;
    s32         STENCIL_BITS            = 8;

    bool        active                  = true;
    bool        fullscreen              = false;


    LRESULT CALLBACK WndProc( HWND    hWnd,
                              UINT    uMsg,
                              WPARAM  wParam,
                              LPARAM  lParam )
    {
        switch( uMsg )
        {
            case WM_SIZE:
            {
                WIDTH = LOWORD( lParam );
                HEIGHT = HIWORD( lParam );
                App::Resize( WIDTH, HEIGHT );
                return 0;
            }
            case WM_ACTIVATE:
            {
                active = !HIWORD( wParam );
                return 0;
            }
            case WM_CLOSE:
            {
                PostQuitMessage( 0 );
                return 0;
            }
            case WM_KEYDOWN:
            {
                return 0;
            }
            case WM_KEYUP:
            {
                return 0;
            }
            case WM_SYSCOMMAND:
            {
                switch( wParam )
                {
                    case SC_SCREENSAVE:
                    case SC_MONITORPOWER:
                        return 0;
                }

                break;
            }
            case WM_MOUSEMOVE:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_LBUTTONDOWN:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_LBUTTONUP:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_RBUTTONDOWN:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_RBUTTONUP:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_MBUTTONDOWN:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_MBUTTONUP:
            {
                s32 x = GET_X_LPARAM( lParam );
                s32 y = GET_Y_LPARAM( lParam );
                return 0;
            }
            case WM_MOUSEWHEEL:
            {
                s32 delta = GET_WHEEL_DELTA_WPARAM( wParam );
                return 0;
            }
        }
        
        return DefWindowProc( hWnd, uMsg, wParam, lParam );
    }

    void DestroyWindowGL()
    {
        if( fullscreen )
        {
            ChangeDisplaySettings( NULL, 0 );
            ShowCursor( true );
        }
        
        if( !wglMakeCurrent( NULL, NULL ) )
            LOG( "wglMakeCurrent( NULL, NULL ) failed\n" );

        if( hRC && !wglDeleteContext( hRC ) )
        {
            LOG( "wglDeleteContext( hRC ) failed\n" );
            hRC = NULL;
        }

        if( hDC && !ReleaseDC( hWnd, hDC ) )
        {
            LOG( "ReleaseDC( hWnd, hDC ) failed\n" );
            hDC = NULL;
        }

        if( hWnd && !DestroyWindow( hWnd ) )
        {
            LOG( "DestroyWindow( hWnd ) failed\n" );
            hWnd = NULL;
        }

        if( !UnregisterClass( TITLE, hInstance ) )
        {
            LOG( "UnregisterClass( TITLE, hInstance ) failed\n" );
            hInstance = NULL;
        }
    }

    bool CreateWindowGL( bool fullscreenFlag )
    {
        fullscreen  = fullscreenFlag;
        hInstance   = GetModuleHandle( NULL );

        WNDCLASS wc         = {};
        wc.style            = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
        wc.lpfnWndProc      = ( WNDPROC ) WndProc;
        wc.cbClsExtra       = 0;
        wc.cbWndExtra       = 0;
        wc.hInstance        = hInstance;
        wc.hIcon            = LoadIcon( hInstance, NULL );
        wc.hCursor          = LoadCursor( hInstance, IDC_ARROW );
        wc.hbrBackground    = NULL;
        wc.lpszMenuName     = NULL;
        wc.lpszClassName    = TITLE;

        if( !RegisterClass( &wc ) )
        {
            LOG( "RegisterClass( &wc ) failed\n" );
            return false;
        }

        if( fullscreen )
        {
            s32 displayHz = 0;
            DEVMODE currentScreenSettings       = {};
            currentScreenSettings.dmSize        = sizeof( currentScreenSettings );
            currentScreenSettings.dmDriverExtra = 0;
            if( EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &currentScreenSettings ) )
            {
                WIDTH       = currentScreenSettings.dmPelsWidth;
                HEIGHT      = currentScreenSettings.dmPelsHeight;
                displayHz   = currentScreenSettings.dmDisplayFrequency;

                DEVMODE screenSettings              = {};
                screenSettings.dmSize               = sizeof( screenSettings );
                screenSettings.dmPelsWidth          = WIDTH;
                screenSettings.dmPelsHeight         = HEIGHT;
                screenSettings.dmBitsPerPel         = COLOUR_BITS;
                screenSettings.dmDisplayFrequency   = displayHz;
                screenSettings.dmFields             = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT | DM_DISPLAYFREQUENCY;

                if( ChangeDisplaySettings( &screenSettings , CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
                {
                    LOG( "ChangeDisplaySettings( &screenSettings , CDS_FULLSCREEN ) failed\n" );
                    fullscreen = false;
                }
            }
            else
            {
                LOG( "EnumDisplaySettings( NULL, ENUM_CURRENT_SETTINGS, &currentScreenSettings ) failed\n" );
                fullscreen = false;
            }
        }

        DWORD dwStyle;
        DWORD dwExStyle;

        if( fullscreen )
        {
            dwStyle     = WS_POPUP;
            dwExStyle   = WS_EX_APPWINDOW;
        }
        else
        {
            dwStyle     = WS_OVERLAPPEDWINDOW;
            dwExStyle   = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
        }

        RECT windowRect     = {};
        windowRect.left     = ( s64 ) 0;
        windowRect.right    = ( s64 ) WIDTH;
        windowRect.top      = ( s64 ) 0;
        windowRect.bottom   = ( s64 ) HEIGHT;
        AdjustWindowRectEx( &windowRect, dwStyle, false, dwExStyle );

        hWnd = CreateWindowEx( dwExStyle,
                                TITLE,
                                TITLE,
                                WS_CLIPSIBLINGS | WS_CLIPCHILDREN | dwStyle,
                                0, 0,
                                windowRect.right - windowRect.left,
                                windowRect.bottom - windowRect.top,
                                NULL,
                                NULL,
                                hInstance,
                                NULL );
        if( !hWnd )
        {
            LOG( "CreateWindowEx() failed\n" );
            return false;
        }

        static PIXELFORMATDESCRIPTOR pfd =
        {
            sizeof( PIXELFORMATDESCRIPTOR ),
            1,
            PFD_DRAW_TO_WINDOW | PFD_DOUBLEBUFFER | PFD_SUPPORT_OPENGL,
            PFD_TYPE_RGBA,
            ( BYTE ) COLOUR_BITS,
            0, 0, 0, 0, 0, 0,
            0,
            0,
            0,
            0, 0, 0, 0,
            ( BYTE ) DEPTH_BITS,
            ( BYTE ) STENCIL_BITS,
            0,
            PFD_MAIN_PLANE,
            0,
            0, 0, 0
        };

        hDC = GetDC( hWnd );
        if( !hDC )
        {
            LOG( "GetDC( hWnd ) failed\n" );
            return false;
        }

        s32 pixelFormat = ChoosePixelFormat( hDC, &pfd );
        if( !pixelFormat )
        {
            LOG( "ChoosePixelFormat( hDC, &pfd ) failed\n" );
            return false;
        }

        if( !SetPixelFormat( hDC, pixelFormat, &pfd ) )
        {
            LOG( "SetPixelFormat( hDC, pixelFormat, &pfd ) failed\n" );
            return false;
        }

        hRC = wglCreateContext( hDC );
        if( !hRC )
        {
            LOG( "wglCreateContext( hDC ) failed\n" );
            return false;
        }

        if( !wglMakeCurrent( hDC, hRC ) )
        {
            LOG( "wglMakeCurrent( hDC, hRC ) failed\n" );
            return false;
        }


        wglGetExtensionsStringARB = ( WGLGetExtensionsStringARB* ) wglGetProcAddress( "wglGetExtensionsStringARB" );
        if( !wglGetExtensionsStringARB )
        {
            LOG( "wglGetProcAddress( \"wglGetExtensionsStringARB\" ) failed\n" );
            return false;
        }

        wglCreateContextAttribsARB = ( WGLCreateContextAttribsARB* ) wglGetProcAddress( "wglCreateContextAttribsARB" );
        if( wglCreateContextAttribsARB )
        {
            s32 attribs[] = { WGL_CONTEXT_MAJOR_VERSION_ARB, OPENGL_VERSION_WANTED_MAJOR,
                                WGL_CONTEXT_MINOR_VERSION_ARB, OPENGL_VERSION_WANTED_MINOR,
                                #ifdef OPENGL_DEBUG_CONTEXT
                                    WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_DEBUG_BIT_ARB,// | WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
                                #endif
                                WGL_CONTEXT_PROFILE_MASK_ARB,
                                #ifdef OPENGL_CORE
                                    WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
                                #else
                                    WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
                                #endif
                                0 };
            HGLRC hCoreContext = wglCreateContextAttribsARB( hDC, 0, attribs );

            if( hCoreContext )
            {
                if( wglMakeCurrent( hDC, hCoreContext ) )
                {
                    wglDeleteContext( hRC );
                    hRC = hCoreContext;
                }
                else
                {
                    LOG( "wglMakeCurrent( hDC, hCoreContext ) failed\n" );
                }
            }
        }
        else
        {
            LOG( "wglCreateContextAttribsARB not available\n" );
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
        

        wglGetExtensionsStringARB = ( WGLGetExtensionsStringARB* ) wglGetProcAddress( "wglGetExtensionsStringARB" );
        if( wglGetExtensionsStringARB )
        {
            char* swapControlExt = "WGL_EXT_swap_control";
            if( HasWGLExtension( swapControlExt ) || HasGLExtension( swapControlExt ) )
            {
                wglSwapIntervalEXT = ( WGLSwapIntervalEXT* ) wglGetProcAddress( "wglSwapIntervalEXT" );

                char* adaptiveVSyncExt = "WGL_EXT_swap_control_tear";
                if( HasWGLExtension( adaptiveVSyncExt ) )
                    wglSwapIntervalEXT( -1 );
                else
                    wglSwapIntervalEXT( 1 );
            }
        }

        ShowWindow( hWnd, SW_SHOW );
        SetForegroundWindow( hWnd );
        SetFocus( hWnd );

        return true;
    }


    struct ThreadInfo
    {
        s32 threadIndex;
        DWORD threadId;
    };

    struct WorkQueueEntry
    {
        Platform::WorkCallback* callback;
        void* data;
    };


    WorkQueueEntry workQueueEntry[ 256 ];
    const u32 QUEUE_ENTRY_MASK          = ARRAY_COUNT( workQueueEntry ) - 1;
    u32 workQueueEntryStartedCount      = 0;
    au32 workQueueEntryCompletedCount   = 0;
    au32 workQueueEntryReadIndex        = 0;
    au32 workQueueEntryWriteIndex       = 0;

    HANDLE hSemaphore;
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

    DWORD WINAPI ThreadProc( LPVOID lpParameter )
    {
        ThreadInfo *info = ( ThreadInfo* ) lpParameter;

        while( true )
        {
            if( !DoWork( info ) )
                WaitForSingleObjectEx( hSemaphore, INFINITE, false );
        };
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
        va_end( args );

        OutputDebugStringA( output );
    }


    f64 GetTime()
    {
        f64 result = 0;

        LARGE_INTEGER performanceCounter;
        if( QueryPerformanceCounter( &performanceCounter ) != 0 )
            result = ( f64 ) performanceCounter.QuadPart * INV_COUNTER_FREQUENCY;

        return result;
    }

    s32 GetProcessorCount()
    {
        SYSTEM_INFO systemInfo;
        GetSystemInfo( &systemInfo );

        s32 result = MAX( ( s32 ) systemInfo.dwNumberOfProcessors, 1 );
        return result;
    }


    void FinishWork()
    {
        ASSERT( GetCurrentThreadId() == threadInfo[ 0 ].threadId );

        while( atomic_load( &workQueueEntryCompletedCount ) < workQueueEntryStartedCount )
            DoWork( &threadInfo[ 0 ] );

        workQueueEntryStartedCount = 0;
        atomic_store( &workQueueEntryCompletedCount, 0u );
    }

    void AddWorkQueueEntry( WorkCallback* callback, void* data )
    {
        ASSERT( GetCurrentThreadId() == threadInfo[ 0 ].threadId );

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

        ReleaseSemaphore( hSemaphore, 1, 0 );
    }


    File ReadFile( char* filename )
    {
        File file = {};

        HANDLE hFile = CreateFileA( filename,
                                    GENERIC_READ,
                                    FILE_SHARE_READ,
                                    NULL,
                                    OPEN_EXISTING,
                                    NULL,
                                    NULL );

        if( hFile != INVALID_HANDLE_VALUE )
        {
            LARGE_INTEGER fileSize;
            if( GetFileSizeEx( hFile, &fileSize ) )
            {
                //@NOTE for this project, if the file is more than 16MB, somethings wrong
                ASSERT( fileSize.QuadPart < ( 0xffffff ) );
                file.size = fileSize.QuadPart;
                file.data = ALIGNED_ALLOC( file.size, 1 );

                DWORD bytesRead;
                if( ::ReadFile( hFile,
                                file.data,
                                ( u32 ) file.size,
                                &bytesRead,
                                NULL ) )
                {
                    if( file.size != bytesRead )
                    {
                        LOG( "ReadFile bytes read doesn't match file.size, bytes read: %u, file size: %lu\n", bytesRead, file.size );
                        FreeFile( &file );
                    }
                }
                else
                {
                    LOG( "ReadFile failed\n" );
                    FreeFile( &file );
                }
            }
            else
            {
                LOG( "GetFileSizeEx failed\n" );
            }

            if( !CloseHandle( hFile ) )
                LOG( "CloseHandle failed\n" );
        }
        else
        {
            LOG( "CreateFile failed\n" );
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


        u32 stringLength = MultiByteToWideChar( CP_UTF8, 0, string, ( s32 ) strlen( string ), 0, 0 );
        wchar_t *wString = NEW( wchar_t, stringLength + 1 );
        MultiByteToWideChar( CP_UTF8, 0, string, ( s32 ) strlen( string ), wString, ( s32 ) stringLength );
        wString[ stringLength ] = 0;


        u32 textFlags = DT_TOP | DT_WORDBREAK | DT_NOCLIP;

        if( hAlignment == HAlignment::Left )
            textFlags |= DT_LEFT;
        else if( hAlignment == HAlignment::Centre )
            textFlags |= DT_CENTER;
        else
            textFlags |= DT_RIGHT;


        HDC context = CreateCompatibleDC( GetDC( 0 ) );

        if( context )
        {
            SetBkColor( context, RGB( 0, 0, 0 ) );
            SetTextColor( context, RGB( 255, 255, 255 ) );
            SetTextCharacterExtra( context, kerning );

            //@NOTE if we can't add our custom font, fallback to whatever the default is
            s32 fontsAdded = 0;
            if( *fontFilename != 0 )
            {
                fontsAdded = AddFontResourceExA( fontFilename, FR_PRIVATE, 0 );
                if( fontsAdded == 0 )
                    LOG( "AddFontResourceExA failed for %s\n", fontFilename );
            }


            HFONT hFont = CreateFontA( -fontSize,
                                        0,
                                        0,
                                        0,
                                        FW_REGULAR,
                                        false,
                                        false,
                                        false,
                                        DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS,
                                        CLIP_DEFAULT_PRECIS,
                                        PROOF_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE,
                                        fontFacename );

            if( hFont )
            {
                SelectObject( context, hFont );

                RECT rect = { 0, 0, textSize->x, textSize->y };
                DrawTextW( context, wString, -1, &rect, textFlags | DT_CALCRECT );
                textSize->x = rect.right;
                textSize->y = rect.bottom;

                s32 maxTextureSize = GLHelper::GetMaxTextureSize();
                bitmapSize->x = MIN( Utils::CeilToPower2( textSize->x ), maxTextureSize );
                bitmapSize->y = MIN( Utils::CeilToPower2( textSize->y ), maxTextureSize );

                BITMAPINFO bitmapInfo;
                bitmapInfo.bmiHeader.biSize             = sizeof( BITMAPINFO );
                bitmapInfo.bmiHeader.biWidth            = bitmapSize->x;
                bitmapInfo.bmiHeader.biHeight           = -bitmapSize->y;
                bitmapInfo.bmiHeader.biPlanes           = 1;
                bitmapInfo.bmiHeader.biBitCount         = 24;
                bitmapInfo.bmiHeader.biCompression      = BI_RGB;
                bitmapInfo.bmiHeader.biSizeImage        = 0;
                bitmapInfo.bmiHeader.biXPelsPerMeter    = 0;
                bitmapInfo.bmiHeader.biYPelsPerMeter    = 0;
                bitmapInfo.bmiHeader.biClrUsed          = 0;
                bitmapInfo.bmiHeader.biClrImportant     = 0;

                u8* bits;
                HBITMAP hBitmap = CreateDIBSection( context,
                                                    &bitmapInfo,
                                                    DIB_RGB_COLORS,
                                                    ( VOID** ) &bits,
                                                    NULL,
                                                    0 );

                if( hBitmap )
                {
                    SelectObject( context, hBitmap );

                    DrawTextW( context, wString, -1, &rect, textFlags );
                    GdiFlush();

                    textBitmap = NEW( u8, bitmapSize->x * bitmapSize->y );
                    memset( textBitmap, 0, bitmapSize->x * bitmapSize->y );

                    for( s32 i = 0; i < bitmapSize->x * bitmapSize->y; ++i )
                    {
                        u8* pixel = bits + i * 3;
                        textBitmap[ i ] = *pixel;
                    }

                    DeleteObject( hBitmap );
                }
                else
                {
                    LOG( "CreateDIBSection failed\n" );
                    bitmapSize = {};
                }

                DeleteObject( hFont );
            }
            else
            {
                LOG( "CreateFontA failed for %s\n", fontFilename );
            }


            if( fontsAdded > 0 )
            {
                if( !RemoveFontResourceExA( fontFilename, FR_PRIVATE, 0 ) )
                    LOG( "RemoveFontResourceExA failed for %s\n", fontFilename );
            }

            if( !DeleteDC( context ) )
                LOG( "DeleteDC failed\n" );
        }
        else
        {
            LOG( "CreateCompatibleDC failed\n" );
        }

        FREE( wString );

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


        HDC context = CreateCompatibleDC( GetDC( 0 ) );

        if( context )
        {
            SetBkColor( context, RGB( 0, 0, 0 ) );
            SetTextColor( context, RGB( 255, 255, 255 ) );

            //@NOTE if we can't add our custom font, fallback to whatever the default is
            s32 fontsAdded = 0;
            if( *fontFilename != 0 )
            {
                fontsAdded = AddFontResourceExA( fontFilename, FR_PRIVATE, 0 );
                if( fontsAdded == 0 )
                    LOG( "AddFontResourceExA failed for %s\n", fontFilename );
            }


            HFONT hFont = CreateFontA( -fontSize,
                                        0,
                                        0,
                                        0,
                                        FW_REGULAR,
                                        false,
                                        false,
                                        false,
                                        DEFAULT_CHARSET,
                                        OUT_DEFAULT_PRECIS,
                                        CLIP_DEFAULT_PRECIS,
                                        PROOF_QUALITY,
                                        DEFAULT_PITCH | FF_DONTCARE,
                                        fontFacename );

            if( hFont )
            {
                SelectObject( context, hFont );

                TEXTMETRIC textMetric;
                GetTextMetrics( context, &textMetric );
                fontAtlas->ascent = textMetric.tmAscent;
                fontAtlas->descent = textMetric.tmDescent;


                char codepointString[ 2 ];
                codepointString[ 1 ] = 0;
                SIZE size;

                for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                {
                    Glyph* glyph = &fontAtlas->glyphList[ i ];
                    glyph->codepoint = string[ i ];

                    codepointString[ 0 ] = string[ i ];
                    GetTextExtentPoint32A( context, codepointString, 1, &size );
                    glyph->width = ( s16 ) size.cx;
                    glyph->height = ( s16 ) size.cy;
                    bitmapSize->y = MAX( bitmapSize->y, glyph->height * 2 );
                }

                bitmapSize->x = Utils::CeilToPower2( bitmapSize->y );
                bitmapSize->y = bitmapSize->x;


                Utils::GlyphPack( padding,
                                    GLHelper::GetMaxTextureSize(),
                                    bitmapSize,
                                    fontAtlas );


                BITMAPINFO bitmapInfo;
                bitmapInfo.bmiHeader.biSize             = sizeof( BITMAPINFO );
                bitmapInfo.bmiHeader.biWidth            = bitmapSize->x;
                bitmapInfo.bmiHeader.biHeight           = -bitmapSize->y;
                bitmapInfo.bmiHeader.biPlanes           = 1;
                bitmapInfo.bmiHeader.biBitCount         = 24;
                bitmapInfo.bmiHeader.biCompression      = BI_RGB;
                bitmapInfo.bmiHeader.biSizeImage        = 0;
                bitmapInfo.bmiHeader.biXPelsPerMeter    = 0;
                bitmapInfo.bmiHeader.biYPelsPerMeter    = 0;
                bitmapInfo.bmiHeader.biClrUsed          = 0;
                bitmapInfo.bmiHeader.biClrImportant     = 0;

                u8* bits;
                HBITMAP hBitmap = CreateDIBSection( context,
                                                    &bitmapInfo,
                                                    DIB_RGB_COLORS,
                                                    ( VOID** ) &bits,
                                                    NULL,
                                                    0 );

                if( hBitmap )
                {
                    SelectObject( context, hBitmap );

                    for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                    {
                        Glyph* glyph = &fontAtlas->glyphList[ i ];
                        codepointString[ 0 ] = glyph->codepoint;
                        TextOutA( context, glyph->atlasX, glyph->atlasY, codepointString, 1 );
                    }

                    GdiFlush();

                    fontAtlasBitmap = NEW( u8, bitmapSize->x * bitmapSize->y );
                    memset( fontAtlasBitmap, 0, bitmapSize->x * bitmapSize->y );

                    for( s32 i = 0; i < bitmapSize->x * bitmapSize->y; ++i )
                    {
                        u8* pixel = bits + i * 3;
                        fontAtlasBitmap[ i ] = *pixel;
                    }

                    DeleteObject( hBitmap );
                }
                else
                {
                    LOG( "CreateDIBSection failed\n" );
                    bitmapSize = {};
                }

                DeleteObject( hFont );
            }
            else
            {
                LOG( "CreateFontA failed for %s\n", fontFilename );
            }


            if( fontsAdded > 0 )
            {
                if( !RemoveFontResourceExA( fontFilename, FR_PRIVATE, 0 ) )
                    LOG( "RemoveFontResourceExA failed for %s\n", fontFilename );
            }

            if( !DeleteDC( context ) )
                LOG( "DeleteDC failed\n" );
        }
        else
        {
            LOG( "CreateCompatibleDC failed\n" );
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


s32 WINAPI WinMain( HINSTANCE hInstance,
                    HINSTANCE hPrevInstance,
                    LPSTR     lpCmdLine,
                    s32       nCmdShow )
{
    LARGE_INTEGER wallClockFrequency;
    QueryPerformanceFrequency( &wallClockFrequency );
    INV_COUNTER_FREQUENCY = 1.0 / ( f64 ) wallClockFrequency.QuadPart;


    ASSERT( Utils::IsPowerOf2( ARRAY_COUNT( workQueueEntry ) ) );

    s32 threadCount             = Platform::GetProcessorCount();
    threadInfo                  = NEW( ThreadInfo, threadCount );
    threadInfo[ 0 ].threadIndex = 0;
    threadInfo[ 0 ].threadId    = GetCurrentThreadId();

    hSemaphore = CreateSemaphore( 0, 0, threadCount, 0 );

    for( s32 i = 1; i < threadCount; ++i )
    {
        threadInfo[ i ].threadIndex = i;

        u32 stackSize = 0;
        HANDLE hThread = CreateThread( 0, stackSize, ThreadProc, &threadInfo[ i ], 0, &threadInfo[ i ].threadId );
        CloseHandle( hThread );
    }


    if( !CreateWindowGL( fullscreen ) )
        return 0;


    App::Init( WIDTH, HEIGHT );


    MSG msg;
    bool done = false;
    while( !done )
    {
        while( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) )
        {
            if( msg.message != WM_QUIT )
            {
                TranslateMessage( &msg );
                DispatchMessage( &msg );
            }
            else
            {
                done = true;
                active = false;
                break;
            }
        }
        
        if( active )
        {
            App::Update();
            App::Render();
            SwapBuffers( hDC );
        }
    }

    return 0;
}
#ifndef GLHELPER_H
#define GLHELPER_H


#include "Types.h"


#ifdef OPENGL
    #ifdef OPENGL_DEBUG_CONTEXT
        #define OPENGL_VERSION_WANTED_MAJOR 4
        #define OPENGL_VERSION_WANTED_MINOR 3
    #else
        #define OPENGL_VERSION_WANTED_MAJOR 3
        #define OPENGL_VERSION_WANTED_MINOR 0
    #endif

    #ifdef PLATFORM_WIN32
        #define APIENTRY __stdcall
        #define WINGDIAPI __declspec(dllimport)
    #endif

    #ifdef GLHELPER_PLATFORM_FUNCTIONS
        #define GLAPI
    #endif

    #define OPENGL_CORE
    #ifdef OPENGL_CORE
        #include <GL/glcorearb.h>
    #else
        #include <GL/gl.h>
        #include <GL/glext.h>
    #endif

    #if defined( PLATFORM_WIN32 ) || defined( PLATFORM_LINUX )
        #ifdef __gl_glcorearb_h_
            //@NOTE 1.0
            GLAPI PFNGLGETSTRINGPROC glGetString;

            GLAPI PFNGLCLEARPROC glClear;
            GLAPI PFNGLCLEARCOLORPROC glClearColor;
            GLAPI PFNGLCLEARSTENCILPROC glClearStencil;
            GLAPI PFNGLCLEARDEPTHPROC glClearDepth;

            GLAPI PFNGLVIEWPORTPROC glViewport;

            GLAPI PFNGLENABLEPROC glEnable;

            GLAPI PFNGLCULLFACEPROC glCullFace;

            GLAPI PFNGLBLENDFUNCPROC glBlendFunc;

            GLAPI PFNGLTEXPARAMETERIPROC glTexParameteri;
            GLAPI PFNGLTEXIMAGE2DPROC glTexImage2D;

            GLAPI PFNGLGETINTEGERVPROC glGetIntegerv;

            GLAPI PFNGLGETERRORPROC glGetError;

            //@NOTE 1.1
            GLAPI PFNGLDRAWARRAYSPROC glDrawArrays;
            GLAPI PFNGLDRAWELEMENTSPROC glDrawElements;

            GLAPI PFNGLGENTEXTURESPROC glGenTextures;
            GLAPI PFNGLBINDTEXTUREPROC glBindTexture;
            GLAPI PFNGLDELETETEXTURESPROC glDeleteTextures;
        #endif

        //@NOTE 1.5
        GLAPI PFNGLBINDBUFFERPROC glBindBuffer;
        GLAPI PFNGLGENBUFFERSPROC glGenBuffers;
        GLAPI PFNGLBUFFERDATAPROC glBufferData;
        GLAPI PFNGLBUFFERSUBDATAPROC glBufferSubData;
        GLAPI PFNGLDELETEBUFFERSPROC glDeleteBuffers;

        //@NOTE 2.0
        GLAPI PFNGLCREATESHADERPROC glCreateShader;
        GLAPI PFNGLSHADERSOURCEPROC glShaderSource;
        GLAPI PFNGLCOMPILESHADERPROC glCompileShader;
        GLAPI PFNGLATTACHSHADERPROC glAttachShader;
        GLAPI PFNGLDETACHSHADERPROC glDetachShader;
        GLAPI PFNGLDELETESHADERPROC glDeleteShader;

        GLAPI PFNGLCREATEPROGRAMPROC glCreateProgram;
        GLAPI PFNGLLINKPROGRAMPROC glLinkProgram;
        GLAPI PFNGLVALIDATEPROGRAMPROC glValidateProgram;
        GLAPI PFNGLUSEPROGRAMPROC glUseProgram;
        GLAPI PFNGLDELETEPROGRAMPROC glDeleteProgram;

        GLAPI PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
        GLAPI PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

        GLAPI PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
        GLAPI PFNGLGETPROGRAMIVPROC glGetProgramiv;
        GLAPI PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
        GLAPI PFNGLGETSHADERIVPROC glGetShaderiv;
        GLAPI PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
        GLAPI PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;

        GLAPI PFNGLUNIFORM4FVPROC glUniform2fv;
        GLAPI PFNGLUNIFORM4FVPROC glUniform3fv;
        GLAPI PFNGLUNIFORM4FVPROC glUniform4fv;
        GLAPI PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;

        GLAPI PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;

        //@NOTE 3.0
        GLAPI PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
        GLAPI PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
        GLAPI PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;

        #ifdef OPENGL_DEBUG
            //@NOTE 4.3
            GLAPI PFNGLDEBUGMESSAGECONTROLPROC glDebugMessageControl;
            GLAPI PFNGLDEBUGMESSAGEINSERTPROC glDebugMessageInsert;
            GLAPI PFNGLDEBUGMESSAGECALLBACKPROC glDebugMessageCallback;
        #endif


        #ifdef PLATFORM_WIN32
            #undef APIENTRY
            #undef WINGDIAPI

            #ifdef GLHELPER_PLATFORM_FUNCTIONS
                #define WIN32_LEAN_AND_MEAN
                #define _WIN32_WINNT 0x0601
                #include <windows.h>

                #define GET_PROC_ADDRESS( name ) wglGetProcAddress( ( name ) )
                #define GET_PROC_ADDRESS_BASE( library, name ) GetProcAddress( ( library ), ( name ) )
            #endif
        #elif PLATFORM_LINUX
            #ifdef GLHELPER_PLATFORM_FUNCTIONS
                #include <GL/glx.h>

                #define GET_PROC_ADDRESS( name ) glXGetProcAddress( ( ( GLubyte* ) name ) )
                #define GET_PROC_ADDRESS_BASE( library, name ) GET_PROC_ADDRESS( name )
            #endif
        #endif

        #define SET_UP_GL_FUNCTION( name ) name = ( decltype( name ) ) GET_PROC_ADDRESS( ( #name ) )
        #define SET_UP_GL_FUNCTION_BASE( library, name ) name = ( decltype( name ) ) GET_PROC_ADDRESS_BASE( ( library ), ( #name ) )


        #ifdef GLHELPER_PLATFORM_FUNCTIONS
            #ifdef OPENGL_DEBUG
                void APIENTRY GLDebugMessageCallback( GLenum source,
                                                         GLenum type,
                                                         GLuint id,
                                                         GLenum severity,
                                                         GLsizei length,
                                                         const GLchar* message,
                                                         const void* userParam )
                {
                    char* sourceString;
                    switch( source )
                    {
                        case GL_DEBUG_SOURCE_API:               sourceString = "GL_DEBUG_SOURCE_API"; break;
                        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:     sourceString = "GL_DEBUG_SOURCE_WINDOW_SYSTEM"; break;
                        case GL_DEBUG_SOURCE_SHADER_COMPILER:   sourceString = "GL_DEBUG_SOURCE_SHADER_COMPILER"; break;
                        case GL_DEBUG_SOURCE_THIRD_PARTY:       sourceString = "GL_DEBUG_SOURCE_THIRD_PARTY"; break;
                        case GL_DEBUG_SOURCE_APPLICATION:       sourceString = "GL_DEBUG_SOURCE_APPLICATION"; break;
                        case GL_DEBUG_SOURCE_OTHER:             sourceString = "GL_DEBUG_SOURCE_OTHER"; break;
                    }

                    char* typeString;
                    switch( type )
                    {
                        case GL_DEBUG_TYPE_ERROR:               typeString = "GL_DEBUG_TYPE_ERROR"; break;
                        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: typeString = "GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR"; break;
                        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  typeString = "GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR"; break; 
                        case GL_DEBUG_TYPE_PORTABILITY:         typeString = "GL_DEBUG_TYPE_PORTABILITY"; break;
                        case GL_DEBUG_TYPE_PERFORMANCE:         typeString = "GL_DEBUG_TYPE_PERFORMANCE"; break;
                        case GL_DEBUG_TYPE_MARKER:              typeString = "GL_DEBUG_TYPE_MARKER"; break;
                        case GL_DEBUG_TYPE_PUSH_GROUP:          typeString = "GL_DEBUG_TYPE_PUSH_GROUP"; break;
                        case GL_DEBUG_TYPE_POP_GROUP:           typeString = "GL_DEBUG_TYPE_POP_GROUP"; break;
                        case GL_DEBUG_TYPE_OTHER:               typeString = "GL_DEBUG_TYPE_OTHER"; break;
                    }

                    char* severityString;
                    switch( severity )
                    {
                        case GL_DEBUG_SEVERITY_HIGH:            severityString = "GL_DEBUG_SEVERITY_HIGH"; break;
                        case GL_DEBUG_SEVERITY_MEDIUM:          severityString = "GL_DEBUG_SEVERITY_MEDIUM"; break;
                        case GL_DEBUG_SEVERITY_LOW:             severityString = "GL_DEBUG_SEVERITY_LOW"; break;
                        case GL_DEBUG_SEVERITY_NOTIFICATION:    severityString = "GL_DEBUG_SEVERITY_NOTIFICATION"; break;
                    }

                    LOG( "source: %s, type: %s, severity: %s, id: %u, message: ", sourceString, typeString, severityString, id );
                    LOG( ( char* ) message );
                    LOG( "\n" );
                }
            #endif

            void LoadGLFunctions()
            {
                #ifdef __gl_glcorearb_h_
                    #ifdef PLATFORM_WIN32
                        HMODULE glModule = LoadLibraryA( "opengl32.dll" );
                    #else
                        #define glModule 0
                    #endif

                    //@NOTE 1.0
                    SET_UP_GL_FUNCTION_BASE( glModule, glGetString );

                    SET_UP_GL_FUNCTION_BASE( glModule, glClear );
                    SET_UP_GL_FUNCTION_BASE( glModule, glClearColor );
                    SET_UP_GL_FUNCTION_BASE( glModule, glClearStencil );
                    SET_UP_GL_FUNCTION_BASE( glModule, glClearDepth );

                    SET_UP_GL_FUNCTION_BASE( glModule, glViewport );

                    SET_UP_GL_FUNCTION_BASE( glModule, glEnable );

                    SET_UP_GL_FUNCTION_BASE( glModule, glCullFace );

                    SET_UP_GL_FUNCTION_BASE( glModule, glBlendFunc );

                    SET_UP_GL_FUNCTION_BASE( glModule, glTexParameteri );
                    SET_UP_GL_FUNCTION_BASE( glModule, glTexImage2D );

                    SET_UP_GL_FUNCTION_BASE( glModule, glGetIntegerv );

                    SET_UP_GL_FUNCTION_BASE( glModule, glGetError );

                    //@NOTE 1.1
                    SET_UP_GL_FUNCTION_BASE( glModule, glDrawArrays );
                    SET_UP_GL_FUNCTION_BASE( glModule, glDrawElements );

                    SET_UP_GL_FUNCTION_BASE( glModule, glGenTextures );
                    SET_UP_GL_FUNCTION_BASE( glModule, glBindTexture );
                    SET_UP_GL_FUNCTION_BASE( glModule, glDeleteTextures );

                    #ifdef PLATFORM_WIN32
                        FreeLibrary( glModule );
                    #endif
                #endif

                //@NOTE 1.5
                SET_UP_GL_FUNCTION( glBindBuffer );
                SET_UP_GL_FUNCTION( glGenBuffers );
                SET_UP_GL_FUNCTION( glBufferData );
                SET_UP_GL_FUNCTION( glBufferSubData );
                SET_UP_GL_FUNCTION( glDeleteBuffers );

                //@NOTE 2.0
                SET_UP_GL_FUNCTION( glCreateShader );
                SET_UP_GL_FUNCTION( glShaderSource );
                SET_UP_GL_FUNCTION( glCompileShader );
                SET_UP_GL_FUNCTION( glAttachShader );
                SET_UP_GL_FUNCTION( glDetachShader );
                SET_UP_GL_FUNCTION( glDeleteShader );

                SET_UP_GL_FUNCTION( glCreateProgram );
                SET_UP_GL_FUNCTION( glLinkProgram );
                SET_UP_GL_FUNCTION( glValidateProgram );
                SET_UP_GL_FUNCTION( glUseProgram );
                SET_UP_GL_FUNCTION( glDeleteProgram );

                SET_UP_GL_FUNCTION( glDisableVertexAttribArray );
                SET_UP_GL_FUNCTION( glEnableVertexAttribArray );

                SET_UP_GL_FUNCTION( glGetAttribLocation );
                SET_UP_GL_FUNCTION( glGetProgramiv );
                SET_UP_GL_FUNCTION( glGetProgramInfoLog );
                SET_UP_GL_FUNCTION( glGetShaderiv );
                SET_UP_GL_FUNCTION( glGetShaderInfoLog );
                SET_UP_GL_FUNCTION( glGetUniformLocation );

                SET_UP_GL_FUNCTION( glUniform2fv );
                SET_UP_GL_FUNCTION( glUniform3fv );
                SET_UP_GL_FUNCTION( glUniform4fv );
                SET_UP_GL_FUNCTION( glUniformMatrix4fv );

                SET_UP_GL_FUNCTION( glVertexAttribPointer );

                //@NOTE 3.0
                SET_UP_GL_FUNCTION( glGenVertexArrays );
                SET_UP_GL_FUNCTION( glBindVertexArray );
                SET_UP_GL_FUNCTION( glDeleteVertexArrays );

                #ifdef OPENGL_DEBUG
                    //@NOTE 4.3
                    SET_UP_GL_FUNCTION( glDebugMessageControl );
                    SET_UP_GL_FUNCTION( glDebugMessageInsert );
                    SET_UP_GL_FUNCTION( glDebugMessageCallback );

                    glEnable( GL_DEBUG_OUTPUT );
                    glEnable( GL_DEBUG_OUTPUT_SYNCHRONOUS ); 
                    glDebugMessageControl( GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, 0, GL_TRUE );
                    glDebugMessageCallback( GLDebugMessageCallback, 0 );
                #endif
            }

            bool GLVersionMeets( s32 major, s32 minor )
            {
                char* glVersion = ( char* ) glGetString( GL_VERSION );
                s32 glMajor = glVersion[ 0 ] - 48;
                s32 glMinor = glVersion[ 2 ] - 48;

                bool result = false;
                if( glMajor > major )
                    result = true;
                else if( glMajor == major && glMinor >= minor )
                    result = true;

                return result;
            }
        #endif
    #endif
#elif defined OPENGL_ES
    #include <GLES3/gl3.h>
#endif


#ifdef GLHELPER_PLATFORM_FUNCTIONS
    #include "Platform.h"
    void LogGLInfo()
    {
        LOG( "%s\n", ( char* ) glGetString( GL_VENDOR ) );
        LOG( "%s\n", ( char* ) glGetString( GL_RENDERER ) );
        LOG( "%s\n", ( char* ) glGetString( GL_VERSION ) );
        LOG( "%s\n", ( char* ) glGetString( GL_SHADING_LANGUAGE_VERSION ) );
    }
#endif


#define GL_S8   GL_BYTE
#define GL_U8   GL_UNSIGNED_BYTE
#define GL_S16  GL_SHORT
#define GL_U16  GL_UNSIGNED_SHORT
#define GL_S32  GL_INT
#define GL_U32  GL_UNSIGNED_INT
#define GL_F32  GL_FLOAT


namespace
{
    void LogProgramInfo( u32 program, char* string )
    {
        s32 infoLength = 0;
        glGetProgramiv( program, GL_INFO_LOG_LENGTH, &infoLength );
        if( infoLength )
        {
            char* info = NEW( char, infoLength );
            glGetProgramInfoLog( program, infoLength, NULL, info );
            
            LOG( string, info );

            FREE( info );
        }
    }
}


namespace GLHelper
{
    extern s32 GetMaxTextureSize();

    extern void LogGLError( char* file, char* function, s32 line );
    #define LOG_GL_ERROR() GLHelper::LogGLError( ( char* ) __FILE__, ( char* ) __FUNCTION__, __LINE__ )

    extern u32 CreateShader( s32 shaderType, char* shaderSource );
    extern void DeleteShader( u32 program, u32 shader );

    extern u32 CreateProgram( u32 vertexShader, u32 fragmentShader );
    extern u32 CreateProgram( char* vertexShaderSource, char* fragmentShaderSource );
    extern bool ValidateProgram( u32 program );
    extern void DeleteProgram( u32 program );


    #ifdef GLHELPER_FUNCTIONS
        s32 GetMaxTextureSize()
        {
            s32 result;
            glGetIntegerv( GL_MAX_TEXTURE_SIZE, &result );
            return result;
        }

        void LogGLError( char* file, char* function, s32 line )
        {
            #ifdef _DEBUG
                GLenum error = glGetError();
                switch( error )
                {
                    case GL_NO_ERROR:                       break;
                    case GL_INVALID_ENUM:                   LOG( "GL_INVALID_ENUM in %s, line %d, file %s", function, line, file );                     break;
                    case GL_INVALID_VALUE:                  LOG( "GL_INVALID_VALUE in %s, line %d, file %s", function, line, file );                    break;
                    case GL_INVALID_OPERATION:              LOG( "GL_INVALID_OPERATION in %s, line %d, file %s", function, line, file );                break;
                    case GL_INVALID_FRAMEBUFFER_OPERATION:  LOG( "GL_INVALID_FRAMEBUFFER_OPERATION in %s, line %d, file %s", function, line, file );    break;
                    case GL_OUT_OF_MEMORY:                  LOG( "GL_OUT_OF_MEMORY in %s, line %d, file %s", function, line, file );                    break;
                }
            #endif
        }

        u32 CreateShader( s32 shaderType, char* shaderSource )
        {
            u32 shader = glCreateShader( shaderType );
            if( shader )
            {
                glShaderSource( shader, 1, &shaderSource, NULL );

                s32 wasCompiled = 0;
                glCompileShader( shader );
                glGetShaderiv( shader, GL_COMPILE_STATUS, &wasCompiled );

                if( !wasCompiled )
                {
                    s32 infoLength = 0;
                    glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &infoLength );
                    if( infoLength )
                    {
                        char* info = NEW( char, infoLength );
                        glGetShaderInfoLog( shader, infoLength, NULL, info );

                        if( shaderType == GL_VERTEX_SHADER )
                            LOG( "vertex shader compile failed: %s", info );
                        else if( shaderType == GL_FRAGMENT_SHADER )
                            LOG( "fragment shader compile failed: %s", info );
                        else
                            LOG( "shaderType not supported" );

                        FREE( info );
                    }

                    glDeleteShader( shader );
                    shader = 0;
                }
            }

            return shader;
        }

        void DeleteShader( u32 program, u32 shader )
        {
            glDetachShader( program, shader );
            glDeleteShader( shader );
        }

        u32 CreateProgram( u32 vertexShader, u32 fragmentShader )
        {
            u32 program = glCreateProgram();
            if( program )
            {
                glAttachShader( program, vertexShader );
                glAttachShader( program, fragmentShader );
                glLinkProgram( program );

                s32 wasLinked = 0;
                glGetProgramiv( program, GL_LINK_STATUS, &wasLinked );

                if( !wasLinked )
                {
                    LogProgramInfo( program, "program link failed: %s" );

                    glDeleteProgram( program );
                    program = 0;
                }
            }

            return program;
        }

        u32 CreateProgram( char* vertexShaderSource, char* fragmentShaderSource )
        {
            u32 vertexShader    = CreateShader( GL_VERTEX_SHADER, vertexShaderSource );
            u32 fragmentShader  = CreateShader( GL_FRAGMENT_SHADER, fragmentShaderSource );

            u32 program = CreateProgram( vertexShader, fragmentShader );

            DeleteShader( program, vertexShader );
            DeleteShader( program, fragmentShader );

            return program;
        }

        bool ValidateProgram( u32 program )
        {
            bool result = false;

            s32 wasValidated = 0;
            glGetProgramiv( program, GL_VALIDATE_STATUS, &wasValidated );

            if( wasValidated )
            {
                result = true;
            }
            else
            {
                LogProgramInfo( program, "program validation failed: %s" );
            }

            return result;
        }

        void DeleteProgram( u32 program )
        {
            glDeleteProgram( program );
        }
    #endif
}


#endif
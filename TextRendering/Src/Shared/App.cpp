#include "App.h"
#include "Platform.h"
#define UTILS_FUNCTIONS
#include "Utils.h"
#include "C3dMaths.h"
#define GLHELPER_FUNCTIONS
#include "GLHelper.h"
#define MATERIALS_FUNCTIONS
#include "Materials.h"


#include <stdarg.h>
#include <stdio.h>


#define PI      3.14159265f
#define INV_PI  1.0f / PI
#define TAU     6.28318530f
#define INV_TAU 1.0f / TAU


//@TODO word wrapping for examples on linux doesn't quite match android / win32
//@TODO default alignment so LTR is left aligned and RTL is right aligned?
//@TODO load font once and use for multiple textfields instead of loading / freeing for each textfield
//@TODO use Freetype, Pango for text rendering on every platform?
namespace
{
    v2i windowSize;
    f32 aspectRatio;


    #define MAX_CHARACTER_COUNT 512


    struct RenderObject
    {
        Materials::Material* material;
        u32 renderMode;
        u32 indexCount;
        u32 VAO;
        u32 VBO[ 2 ];
    };

    struct TextField
    {
        RenderObject renderObject;
    };

    struct DynamicTextField
    {
        RenderObject renderObject;
        f32* vertexData;
        s32 maxCharacterCount;
    };


    Materials::MaterialShared textSharedMaterial;
    TextField englishTF;
    TextField englishLeftTF;
    TextField englishCentreTF;
    TextField englishRightTF;
    TextField arabicTF;
    TextField arabicLeftTF;
    TextField arabicCentreTF;
    TextField arabicRightTF;
    TextField thaiTF;
    TextField thaiLeftTF;
    TextField thaiCentreTF;
    TextField thaiRightTF;

    Materials::Material dynamicTextMaterial;
    FontAtlas openSans20Atlas;
    DynamicTextField dynamicTextField;
    f64 runningTime = 0;


    //@NOTE vertex followed by texCoord
    RenderObject CreateRenderObject( Materials::Material* material,
                                        f32* vertexData,
                                        u32 vertexDataUsage,
                                        u32 vertexDataCount,
                                        u8 vertexDimensions,
                                        u8 texCoordDimensions,
                                        u16* indices,
                                        u32 indicesUsage,
                                        u32 indexCount,
                                        u32 renderMode )
    {
        u32 VAO;
        glGenVertexArrays( 1, &VAO );
        glBindVertexArray( VAO );

        u32 VBO[ 2 ];
        glGenBuffers( 2, VBO );
        glBindBuffer( GL_ARRAY_BUFFER, VBO[ 0 ] );
        glBufferData( GL_ARRAY_BUFFER, vertexDataCount * sizeof( f32 ), vertexData, vertexDataUsage );

        u32 stride = ( vertexDimensions + texCoordDimensions ) * sizeof( f32 );
        glEnableVertexAttribArray( material->shared->inPosition );
        glVertexAttribPointer( material->shared->inPosition, vertexDimensions, GL_F32, false, stride, 0 );

        u64 vertexOffset = vertexDimensions * sizeof( f32 );

        if( texCoordDimensions > 0 )
        {
            glEnableVertexAttribArray( material->shared->inTexCoord );
            glVertexAttribPointer( material->shared->inTexCoord, texCoordDimensions, GL_F32, false, stride, ( void* ) vertexOffset );
            vertexOffset += texCoordDimensions * sizeof( f32 );
        }
        
        glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, VBO[ 1 ] );
        glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof( u16 ), indices, indicesUsage );

        RenderObject renderObject = { material, renderMode, indexCount, VAO, { VBO[ 0 ], VBO[ 1 ] } };
        return renderObject;
    }

    void FreeRenderObject( RenderObject* renderObject )
    {
        glDeleteVertexArrays( 1, &renderObject->VAO );
        glDeleteBuffers( 2, renderObject->VBO );
        *renderObject = {};
    }

    void Render( RenderObject* renderObject,
                    m44* cameraMatrix,
                    m44* modelViewMatrix )
    {
        Materials::Apply( renderObject->material );
        glBindVertexArray( renderObject->VAO );

        m44 MVP = C3dMaths::MatrixMultiply( cameraMatrix, modelViewMatrix );
        glUniformMatrix4fv( renderObject->material->shared->inMVP, 1, false, ( f32* ) &MVP );

        glDrawElements( renderObject->renderMode, renderObject->indexCount, GL_U16, NULL );
    }


    DynamicTextField CreateDynamicTextField( s32 characterCount,
                                                Materials::Material* material )
    {
        ASSERT( characterCount <= MAX_CHARACTER_COUNT );

        u32 vertexSize      = ( 4 + 4 ) * 2;
        u32 indicesPerChar  = 6;


        u16* indexData = NEW( u16, characterCount * indicesPerChar );
        u16* indices = indexData;
        for( s32 i = 0; i < characterCount; ++i )
        {
            s32 index = i * 4;
            indices[ 0 ] = index;       indices[ 1 ] = index + 1;   indices[ 2 ] = index + 2;
            indices[ 3 ] = index + 2;   indices[ 4 ] = index + 3;   indices[ 5 ] = index;
            indices += indicesPerChar;
        }

        RenderObject renderObject = CreateRenderObject( material,
                                                        0,
                                                        GL_DYNAMIC_DRAW,
                                                        characterCount * vertexSize,
                                                        2,
                                                        2,
                                                        indexData,
                                                        GL_STATIC_DRAW,
                                                        characterCount * indicesPerChar,
                                                        GL_TRIANGLES );

        FREE( indexData );


        DynamicTextField dynamicTextField = { renderObject,
                                                ALIGNED_NEW( f32, characterCount * vertexSize, 16 * sizeof( f32 ) ),
                                                characterCount };

        return dynamicTextField;
    }

    void UpdateDynamicTextField( DynamicTextField* dynamicTextField,
                                    v2i* textFieldSize,
                                    char* string,
                                    ... )
    {
        va_list args;
        va_start( args, string );
        char output[ MAX_CHARACTER_COUNT ];
        vsnprintf( output, MAX_CHARACTER_COUNT, string, args );
        va_end( args );


        u32 vertexSize      = ( 4 + 4 ) * 2;
        u32 indicesPerChar  = 6;

        FontAtlas* fontAtlas = dynamicTextField->renderObject.material->Text.fontAtlas;


        v2 position = { 0.0f, 0.0f };

        RenderObject* renderObject = &dynamicTextField->renderObject;
        Materials::Material* material = renderObject->material;
        f32* vertexData = dynamicTextField->vertexData;


        s32 visibleCount = 0;
        char* currChar = output;
        while( *currChar && visibleCount < dynamicTextField->maxCharacterCount )
        {
            if( *currChar != ' ' )
            {
                Glyph* glyph = 0;
                for( s32 i = 0; i < fontAtlas->glyphCount; ++i )
                {
                    if( *currChar == fontAtlas->glyphList[ i ].codepoint )
                    {
                        glyph = &fontAtlas->glyphList[ i ];
                        break;
                    }
                }

                if( glyph )
                {
                    vertexData[  0 ] = position.x;                  vertexData[  1 ] = position.y;                  vertexData[  2 ] = glyph->atlasX / material->Text.textureSize.x;                     vertexData[  3 ] = glyph->atlasY / material->Text.textureSize.y;
                    vertexData[  4 ] = position.x;                  vertexData[  5 ] = position.y + glyph->height;  vertexData[  6 ] = glyph->atlasX / material->Text.textureSize.x;                     vertexData[  7 ] = ( glyph->atlasY + glyph->height ) / material->Text.textureSize.y;
                    vertexData[  8 ] = position.x + glyph->width;   vertexData[  9 ] = position.y + glyph->height;  vertexData[ 10 ] = ( glyph->atlasX + glyph->width ) / material->Text.textureSize.x;  vertexData[ 11 ] = ( glyph->atlasY + glyph->height ) / material->Text.textureSize.y;
                    vertexData[ 12 ] = position.x + glyph->width;   vertexData[ 13 ] = position.y;                  vertexData[ 14 ] = ( glyph->atlasX + glyph->width ) / material->Text.textureSize.x;  vertexData[ 15 ] = glyph->atlasY / material->Text.textureSize.y;
                    vertexData += vertexSize;

                    position.x = ROUND( position.x + glyph->width + fontAtlas->kerning );
                    ++visibleCount;
                }
                else
                {
                    LOG( "character %c missing from dynamic textfield\n", *currChar );
                }
            }
            else
            {
                position.x = ROUND( position.x + fontAtlas->ascent * 0.25f );
            }

            ++currChar;
        }

        glBindVertexArray( renderObject->VAO );
        glBindBuffer( GL_ARRAY_BUFFER, renderObject->VBO[ 0 ] );
        glBufferSubData( GL_ARRAY_BUFFER, 0, visibleCount * vertexSize * sizeof( f32 ), dynamicTextField->vertexData );

        renderObject->indexCount = visibleCount * indicesPerChar;
        *textFieldSize = { ( s32 ) position.x - fontAtlas->kerning, fontAtlas->ascent - fontAtlas->descent };
    }

    TextField CreateTextFieldFromString( char* string,
                                            char* fontFilename,
                                            char* fontFacename,
                                            s32 pointSize,
                                            s32 kerning,
                                            s32 width,
                                            v2 anchorPosition,
                                            v4 colour,
                                            HAlignment hAlignment,
                                            VAlignment vAlignment,
                                            Materials::MaterialShared* materialShared )
    {
        v2i textSize    = { width, 0 };
        v2i bitmapSize  = {};

        u8* textBitmap = Platform::CreateTextBitmap( string,
                                                        fontFilename,
                                                        fontFacename,
                                                        pointSize,
                                                        kerning,
                                                        hAlignment,
                                                        &textSize,
                                                        &bitmapSize );

        Materials::Material* material = NEW( Materials::Material, 1 );
        *material = Materials::CreateText( materialShared,
                                            anchorPosition,
                                            colour,
                                            textBitmap,
                                            bitmapSize,
                                            0 );

        Platform::FreeTextBitmap( &textBitmap );

        v2 position = { ( f32 ) -textSize.x, ( f32 ) -textSize.y };
        if( hAlignment == HAlignment::Left )
            position.x = 0.0f;
        else if( hAlignment == HAlignment::Centre )
            position.x = textSize.x * -0.5f;

        if( vAlignment == VAlignment::Top )
            position.y = 0.0f;
        else if( vAlignment == VAlignment::Centre )
            position.y = textSize.y * -0.5f;

        v2 uv = { textSize.x / ( f32 ) bitmapSize.x, textSize.y / ( f32 ) bitmapSize.y };
        f32 vertexData[] = { position.x,                    position.y,                 0.0f,   0.0f,
                                position.x,                 position.y + textSize.y,    0.0f,   uv.y,
                                position.x + textSize.x,    position.y + textSize.y,    uv.x,   uv.y,
                                position.x + textSize.x,    position.y,                 uv.x,   0.0f };

        u16 indices[] = { 0, 1, 2,
                            2, 3, 0 };

        RenderObject renderObject = CreateRenderObject( material,
                                                        vertexData,
                                                        GL_STATIC_DRAW,
                                                        ARRAY_COUNT( vertexData ),
                                                        2,
                                                        2,
                                                        indices,
                                                        GL_STATIC_DRAW,
                                                        ARRAY_COUNT( indices ),
                                                        GL_TRIANGLES );

        TextField textField = { renderObject };
        return textField;
    }

    void FreeTextField( TextField* textField )
    {
        Materials::Free( textField->renderObject.material );
        FreeRenderObject( &textField->renderObject );
        *textField = {};
    }

    void FreeDynamicTextField( DynamicTextField* dynamicTextField )
    {
        FreeRenderObject( &dynamicTextField->renderObject );
        ALIGNED_FREE( dynamicTextField->vertexData );
        *dynamicTextField = {};
    }
}

namespace App
{
    void Resize( s32 width, s32 height )
    {
        windowSize  = v2i{ width, height };
        aspectRatio = width / ( f32 ) height;

        glViewport( 0, 0, width, height );
    }

    void Init( s32 width, s32 height )
    {
        glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
        #ifdef OPENGL
            glClearDepth( 1.0 );
        #else
            glClearDepthf( 1.0f );
        #endif
        glClearStencil( 0 );

        glEnable( GL_CULL_FACE );
        glCullFace( GL_BACK );

        glEnable( GL_BLEND );
        glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

        
        Resize( width, height );


        textSharedMaterial = Materials::CreateTextShared();

        s32 staticFontSize  = 30;
        s32 staticKerning   = 0;
        s32 staticMaxWidth  = 128;
        v4 staticTextColour = v4{ 0.0f, 0.0f, 0.0f, 1.0f };

        englishTF = CreateTextFieldFromString( "English",
                                                DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                                "Open Sans",
                                                staticFontSize,
                                                staticKerning,
                                                staticMaxWidth,
                                                v2{},
                                                staticTextColour,
                                                HAlignment::Left,
                                                VAlignment::Top,
                                                &textSharedMaterial );
        englishLeftTF = CreateTextFieldFromString( "This text is left aligned",
                                                    DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                                    "Open Sans",
                                                    staticFontSize,
                                                    staticKerning,
                                                    staticMaxWidth,
                                                    v2{ 150.0f, 0.0f },
                                                    staticTextColour,
                                                    HAlignment::Left,
                                                    VAlignment::Top,
                                                    &textSharedMaterial );
        englishCentreTF = CreateTextFieldFromString( "This text is centre aligned",
                                                        DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                                        "Open Sans",
                                                        staticFontSize,
                                                        staticKerning,
                                                        staticMaxWidth,
                                                        v2{ 400.0f, 0.0f },
                                                        staticTextColour,
                                                        HAlignment::Centre,
                                                        VAlignment::Top,
                                                        &textSharedMaterial );
        englishRightTF = CreateTextFieldFromString( "This text is right aligned",
                                                    DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                                    "Open Sans",
                                                    staticFontSize,
                                                    staticKerning,
                                                    staticMaxWidth,
                                                    v2{ 650.0f, 0.0f },
                                                    staticTextColour,
                                                    HAlignment::Right,
                                                    VAlignment::Top,
                                                    &textSharedMaterial );

        arabicTF = CreateTextFieldFromString( "Arabic",
                                                DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                                "Open Sans",
                                                staticFontSize,
                                                staticKerning,
                                                staticMaxWidth,
                                                v2{},
                                                staticTextColour,
                                                HAlignment::Left,
                                                VAlignment::Top,
                                                &textSharedMaterial );
        arabicLeftTF = CreateTextFieldFromString( "هذا النص محاذاة إلى اليسار",
                                                    DATA_FOLDER"Markazi_Text/static/MarkaziText-Regular.ttf",
                                                    "Markazi Text",
                                                    staticFontSize,
                                                    staticKerning,
                                                    staticMaxWidth,
                                                    v2{ 150.0f, 0.0f },
                                                    staticTextColour,
                                                    HAlignment::Left,
                                                    VAlignment::Top,
                                                    &textSharedMaterial );
        arabicCentreTF = CreateTextFieldFromString( "هذا النص بمحاذاة الوسط",
                                                        DATA_FOLDER"Markazi_Text/static/MarkaziText-Regular.ttf",
                                                        "Markazi Text",
                                                        staticFontSize,
                                                        staticKerning,
                                                        staticMaxWidth,
                                                        v2{ 400.0f, 0.0f },
                                                        staticTextColour,
                                                        HAlignment::Centre,
                                                        VAlignment::Top,
                                                        &textSharedMaterial );
        arabicRightTF = CreateTextFieldFromString( "هذا النص محاذاة إلى اليمين",
                                                    DATA_FOLDER"Markazi_Text/static/MarkaziText-Regular.ttf",
                                                    "Markazi Text",
                                                    staticFontSize,
                                                    staticKerning,
                                                    staticMaxWidth,
                                                    v2{ 650.0f, 0.0f },
                                                    staticTextColour,
                                                    HAlignment::Right,
                                                    VAlignment::Top,
                                                    &textSharedMaterial );

        thaiTF = CreateTextFieldFromString( "Thai",
                                            DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                            "Open Sans",
                                            staticFontSize,
                                            staticKerning,
                                            staticMaxWidth,
                                            v2{},
                                            staticTextColour,
                                            HAlignment::Left,
                                            VAlignment::Top,
                                            &textSharedMaterial );
        thaiLeftTF = CreateTextFieldFromString( "ข้อความนี้จัดชิดซ้าย",
                                                DATA_FOLDER"Niramit/Niramit-Regular.ttf",
                                                "Niramit",
                                                staticFontSize,
                                                staticKerning,
                                                staticMaxWidth,
                                                v2{ 150.0f, 0.0f },
                                                staticTextColour,
                                                HAlignment::Left,
                                                VAlignment::Top,
                                                &textSharedMaterial );
        thaiCentreTF = CreateTextFieldFromString( "ข้อความนี้อยู่ในแนวกึ่งกลาง",
                                                    DATA_FOLDER"Niramit/Niramit-Regular.ttf",
                                                    "Niramit",
                                                    staticFontSize,
                                                    staticKerning,
                                                    staticMaxWidth,
                                                    v2{ 400.0f, 0.0f },
                                                    staticTextColour,
                                                    HAlignment::Centre,
                                                    VAlignment::Top,
                                                    &textSharedMaterial );
        thaiRightTF = CreateTextFieldFromString( "ข้อความนี้จัดชิดขวา",
                                                    DATA_FOLDER"Niramit/Niramit-Regular.ttf",
                                                    "Niramit",
                                                    staticFontSize,
                                                    staticKerning,
                                                    staticMaxWidth,
                                                    v2{ 650.0f, 0.0f },
                                                    staticTextColour,
                                                    HAlignment::Right,
                                                    VAlignment::Top,
                                                    &textSharedMaterial );


        s32 fontSize = 20;
        s32 kerning = 0;
        s32 characterCount = 20;
        s32 padding = 2;
        v2i bitmapSize = {};
        openSans20Atlas = {};
        u8* fontAtlasBitmap = Platform::CreateFontAtlasBitmap( "0123456789.fps",
                                                                DATA_FOLDER"Open_Sans/OpenSans-Regular.ttf",
                                                                "Open Sans",
                                                                fontSize,
                                                                kerning,
                                                                padding,
                                                                &bitmapSize,
                                                                &openSans20Atlas );

        dynamicTextMaterial = Materials::CreateText( &textSharedMaterial,
                                                        v2{},
                                                        v4{ 0.0f, 0.0f, 0.0f, 1.0f },
                                                        fontAtlasBitmap,
                                                        bitmapSize,
                                                        &openSans20Atlas );

        dynamicTextField = CreateDynamicTextField( characterCount, &dynamicTextMaterial );

        Platform::FreeFontAtlasBitmap( &fontAtlasBitmap );
    }

    void Update()
    {
    }

    void Render()
    {
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

        m44 cameraMatrix = C3dMaths::CreateOrthoMatrix( 0.0f, ( f32 ) windowSize.x, ( f32 ) windowSize.y, 0.0f, -1.0f, 1.0f );
        m44 modelViewMatrix = C3dMaths::CreateIdentityMatrix();


        C3dMaths::Translate( &modelViewMatrix, v3{ 20.0f, 20.0f, 0.0f } );
        Render( &englishTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &englishLeftTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &englishCentreTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &englishRightTF.renderObject, &cameraMatrix, &modelViewMatrix );

        modelViewMatrix = C3dMaths::CreateIdentityMatrix();
        C3dMaths::Translate( &modelViewMatrix, v3{ 20.0f, 170.0f, 0.0f } );
        Render( &arabicTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &arabicLeftTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &arabicCentreTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &arabicRightTF.renderObject, &cameraMatrix, &modelViewMatrix );

        modelViewMatrix = C3dMaths::CreateIdentityMatrix();
        C3dMaths::Translate( &modelViewMatrix, v3{ 20.0f, 320.0f, 0.0f } );
        Render( &thaiTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &thaiLeftTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &thaiCentreTF.renderObject, &cameraMatrix, &modelViewMatrix );
        Render( &thaiRightTF.renderObject, &cameraMatrix, &modelViewMatrix );


        f64 newRunningTime = Platform::GetTime();
        f32 fps = 1.0f / ( f32 ) ( newRunningTime - runningTime );
        runningTime = newRunningTime;

        v2i size;
        UpdateDynamicTextField( &dynamicTextField,
                                &size,
                                "%.2f fps",
                                fps );
        modelViewMatrix = C3dMaths::CreateIdentityMatrix();
        dynamicTextField.renderObject.material->Text.anchorPosition = v2{ ( f32 ) windowSize.x - size.x, 0.0f };
        Render( &dynamicTextField.renderObject, &cameraMatrix, &modelViewMatrix );
    }
}
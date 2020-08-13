#ifndef MATERIALS_H
#define MATERIALS_H


#include "Types.h"
#include "C3dMaths.h"
#include "GLHelper.h"


namespace Materials
{
    enum struct MaterialType
    {
        ColourUnlit     = 0,
        TextureUnlit    = 1,
        Text            = 2
    };

    struct MaterialShared
    {
        u32 shaderProgram;

        s32 inPosition;
        s32 inTexCoord;
        s32 inMVP;

        union
        {
            struct
            {
                s32 inColour;
            } ColourUnlit;
            struct
            {
            } TextureUnlit;
            struct
            {
                s32 anchorPosition;
                s32 inColour;
            } Text;
        };
    };
}

namespace
{
    char COLOUR_UNLIT_VERTEX_SHADER[] = "#version 100\n\
                                        \
                                        attribute vec4 inPosition;\
                                        uniform mat4 inMVP;\
                                        \
                                        uniform vec4 inColour;\
                                        \
                                        varying vec4 fragColour;\
                                        \
                                        void main()\
                                        {\
                                            gl_Position = inMVP * inPosition;\
                                            fragColour = inColour;\
                                        }";

    char COLOUR_UNLIT_FRAGMENT_SHADER[] =   "#version 100\n\
                                            \
                                            precision mediump float;\
                                            \
                                            varying vec4 fragColour;\
                                            \
                                            void main()\
                                            {\
                                                gl_FragColor = fragColour;\
                                            }";


    char TEXTURE_UNLIT_VERTEX_SHADER[] = "#version 100\n\
                                          \
                                          attribute vec4 inPosition;\
                                          attribute vec2 inTexCoord;\
                                          uniform mat4 inMVP;\
                                          \
                                          varying vec2 fragTexCoord;\
                                          \
                                          void main()\
                                          {\
                                             gl_Position     = inMVP * inPosition;\
                                             fragTexCoord    = inTexCoord;\
                                          }";

    char TEXTURE_UNLIT_FRAGMENT_SHADER[] = "#version 100\n\
                                            \
                                            precision mediump float;\
                                            \
                                            varying vec2 fragTexCoord;\
                                            \
                                            uniform sampler2D texSampler;\
                                            \
                                            void main()\
                                            {\
                                                gl_FragColor = vec4( vec3( 0, 0, 0 ), texture2D( texSampler, fragTexCoord ).r );\
                                            }";


    char TEXT_VERTEX_SHADER[] = "#version 100\n\
                                 \
                                 attribute vec4 inPosition;\
                                 attribute vec2 inTexCoord;\
                                 uniform mat4 inMVP;\
                                 \
                                 uniform vec2 anchorPosition;\
                                 uniform vec4 inColour;\
                                 \
                                 varying vec2 fragTexCoord;\
                                 varying vec4 fragColour;\
                                 \
                                 void main()\
                                 {\
                                    gl_Position     = inMVP * ( vec4( anchorPosition, 0, 0 ) + inPosition );\
                                    fragTexCoord    = inTexCoord;\
                                    fragColour      = inColour;\
                                 }";

    char TEXT_FRAGMENT_SHADER[] = "#version 100\n\
                                   \
                                   precision mediump float;\
                                   \
                                   varying vec2 fragTexCoord;\
                                   varying vec4 fragColour;\
                                   \
                                   uniform sampler2D texSampler;\
                                   \
                                   void main()\
                                   {\
                                      gl_FragColor = vec4( fragColour.rgb, texture2D( texSampler, fragTexCoord ).r );\
                                   }";


    void CreateSharedBase( Materials::MaterialShared* materialShared, char* vertexSource, char* fragmentSource )
    {
        materialShared->shaderProgram   = GLHelper::CreateProgram( vertexSource, fragmentSource );

        materialShared->inPosition      = glGetAttribLocation( materialShared->shaderProgram,   "inPosition" );
        materialShared->inTexCoord      = glGetAttribLocation( materialShared->shaderProgram,   "inTexCoord" );
        materialShared->inMVP           = glGetUniformLocation( materialShared->shaderProgram,  "inMVP" );
    }
}


namespace Materials
{
    struct Material
    {
        MaterialType type;
        MaterialShared* shared;

        union
        {
            struct
            {
                v4 colour;
            } ColourUnlit;
            struct
            {
                u32 textureId;
            } TextureUnlit;
            struct
            {
                v2 anchorPosition;
                v4 colour;
                v2 textureSize;
                u32 fontTextureId;
                FontAtlas* fontAtlas;
            } Text;
        };
    };

    
    extern MaterialShared CreateColourUnlitShared();
    extern Material CreateColourUnlit( MaterialShared* sharedMaterial, v4 colour );

    extern MaterialShared CreateTextureUnlitShared();
    extern Material CreateTextureUnlit( MaterialShared* sharedMaterial,
                                        u8* bitmap,
                                        v2i bitmapSize );

    extern MaterialShared CreateTextShared();
    extern Material CreateText( MaterialShared* sharedMaterial,
                                v2 anchorPosition,
                                v4 colour,
                                u8* bitmap,
                                v2i bitmapSize );

    extern void Apply( Material* material );

    extern void Free( MaterialShared* sharedMaterial );
    extern void Free( Material* material );


    #ifdef MATERIALS_FUNCTIONS
        MaterialShared CreateColourUnlitShared()
        {
            MaterialShared material;
            CreateSharedBase( &material, COLOUR_UNLIT_VERTEX_SHADER, COLOUR_UNLIT_FRAGMENT_SHADER );
            material.ColourUnlit.inColour = glGetUniformLocation( material.shaderProgram, "inColour" );
            return material;
        }

        Material CreateColourUnlit( MaterialShared* sharedMaterial, v4 colour )
        {
            Material material;
            material.type               = MaterialType::ColourUnlit;
            material.shared             = sharedMaterial;
            material.ColourUnlit.colour = colour;
            return material;
        }


        MaterialShared CreateTextureUnlitShared()
        {
            MaterialShared material;
            CreateSharedBase( &material, TEXTURE_UNLIT_VERTEX_SHADER, TEXTURE_UNLIT_FRAGMENT_SHADER );
            return material;
        }

        Material CreateTextureUnlit( MaterialShared* sharedMaterial,
                                        u8* bitmap,
                                        v2i bitmapSize )
        {
            u32 textureId;
            glGenTextures( 1, &textureId );
            glBindTexture( GL_TEXTURE_2D, textureId );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

            s32 level = 0;
            s32 border = 0;
            glTexImage2D( GL_TEXTURE_2D, level, GL_R8, bitmapSize.x, bitmapSize.y, border, GL_RED, GL_U8, bitmap );

            Material material;
            material.type                   = MaterialType::TextureUnlit;
            material.shared                 = sharedMaterial;
            material.TextureUnlit.textureId = textureId;
            return material;
        }


        MaterialShared CreateTextShared()
        {
            MaterialShared material;
            CreateSharedBase( &material, TEXT_VERTEX_SHADER, TEXT_FRAGMENT_SHADER );
            material.Text.anchorPosition = glGetUniformLocation( material.shaderProgram, "anchorPosition" );
            material.Text.inColour = glGetUniformLocation( material.shaderProgram, "inColour" );
            return material;
        }

        Material CreateText( MaterialShared* sharedMaterial,
                                v2 anchorPosition,
                                v4 colour,
                                u8* bitmap,
                                v2i bitmapSize,
                                FontAtlas* fontAtlas )
        {
            u32 textureId;
            glGenTextures( 1, &textureId );
            glBindTexture( GL_TEXTURE_2D, textureId );

            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
            glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );

            s32 level = 0;
            s32 border = 0;
            glTexImage2D( GL_TEXTURE_2D, level, GL_R8, bitmapSize.x, bitmapSize.y, border, GL_RED, GL_U8, bitmap );

            Material material;
            material.type                   = MaterialType::Text;
            material.shared                 = sharedMaterial;
            material.Text.anchorPosition    = anchorPosition;
            material.Text.colour            = colour;
            material.Text.textureSize       = v2{ ( f32 ) bitmapSize.x, ( f32 ) bitmapSize.y };
            material.Text.fontTextureId     = textureId;
            material.Text.fontAtlas         = fontAtlas;

            return material;
        }


        void Apply( Material* material )
        {
            glUseProgram( material->shared->shaderProgram );

            switch( material->type )
            {
                case MaterialType::ColourUnlit:
                    glUniform4fv( material->shared->ColourUnlit.inColour, 1, ( f32* ) &material->ColourUnlit.colour );
                    break;
                case MaterialType::TextureUnlit:
                    glBindTexture( GL_TEXTURE_2D, material->TextureUnlit.textureId );
                    break;
                case MaterialType::Text:
                    glBindTexture( GL_TEXTURE_2D, material->Text.fontTextureId );
                    glUniform2fv( material->shared->Text.anchorPosition, 1, ( f32* ) &material->Text.anchorPosition );
                    glUniform4fv( material->shared->Text.inColour, 1, ( f32* ) &material->Text.colour );
                    break;
            }
        }


        void Free( MaterialShared* sharedMaterial )
        {
            GLHelper::DeleteProgram( sharedMaterial->shaderProgram );
            *sharedMaterial = {};
        }

        void Free( Material* material )
        {
            switch( material->type )
            {
                case MaterialType::ColourUnlit:
                    break;
                case MaterialType::TextureUnlit:
                    glDeleteTextures( 1, &material->TextureUnlit.textureId );
                    break;
                case MaterialType::Text:
                    glDeleteTextures( 1, &material->Text.fontTextureId );
                    break;
            }

            *material = {};
        }
    #endif
}


#endif
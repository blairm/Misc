#ifndef C3DMATHS_H
#define C3DMATHS_H


#include "Types.h"


namespace C3dMaths
{
    s32 Max( s32 a, s32 b );
    #define MAX( a, b ) C3dMaths::Max( a, b )
    s32 Min( s32 a, s32 b );
    #define MIN( a, b ) C3dMaths::Min( a, b )

    f32 Round( f32 value );
    #define ROUND( value ) C3dMaths::Round( value )
    s32 SRound( f32 value );
    #define SROUND( value ) C3dMaths::SRound( value )


    m44 CreateIdentityMatrix();

    m44 CreateOrthoMatrix( f32 left,
                            f32 right,
                            f32 bottom,
                            f32 top,
                            f32 near,
                            f32 far );

    m44 MatrixMultiply( m44* m, m44* n );

    void Translate( m44* m, v3 t );
    void Scale( m44* m, v3 s );
    void Scale( m44* m, f32 s );
}


v2 operator *( v2 v, f32 s );
v2 operator *( f32 s, v2 v );
v3 operator *( v3 v, f32 s );
v3 operator *( f32 s, v3 v );
v4 operator *( v4 v, f32 s );
v4 operator *( f32 s, v4 v );


#endif
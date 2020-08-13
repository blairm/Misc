#include "C3dMaths.h"


#include <math.h>


namespace C3dMaths
{
    s32 Max( s32 a, s32 b )
    {
        s32 result = a > b ? a : b;
        return result;
    }

    s32 Min( s32 a, s32 b )
    {
        s32 result = a < b ? a : b;
        return result;
    }

    f32 Round( f32 value )
    {
        f32 result = roundf( value );
        return result;
    }

    s32 SRound( f32 value )
    {
        s32 result = ( s32 ) roundf( value );
        return result;
    }


    m44 CreateIdentityMatrix()
    {
        m44 identity = { 1.0f, 0.0f, 0.0f, 0.0f,
                         0.0f, 1.0f, 0.0f, 0.0f,
                         0.0f, 0.0f, 1.0f, 0.0f,
                         0.0f, 0.0f, 0.0f, 1.0f };
        return identity;
    }

    m44 CreateOrthoMatrix( f32 left,
                            f32 right,
                            f32 bottom,
                            f32 top,
                            f32 near,
                            f32 far )
    {
        m44 orthoMatrix = { 2.0f / ( right - left ),                  0.0f,                                     0.0f,                                0.0f,
                            0.0f,                                     2.0f / ( top - bottom ),                  0.0f,                                0.0f,
                            0.0f,                                     0.0f,                                     -2.0f / ( far - near ),              0.0f,
                            -( ( right + left ) / ( right - left ) ), -( ( top + bottom ) / ( top - bottom ) ), -( ( far + near ) / ( far - near) ), 1.0f };
        return orthoMatrix;
    }

    m44 MatrixMultiply( m44* m, m44* n )
    {
        m44 result;

        result.m[  0 ] = m->m[ 0 ] * n->m[  0 ] + m->m[ 4 ] * n->m[  1 ] + m->m[  8 ] * n->m[  2 ] + m->m[ 12 ] * n->m[  3 ];
        result.m[  1 ] = m->m[ 1 ] * n->m[  0 ] + m->m[ 5 ] * n->m[  1 ] + m->m[  9 ] * n->m[  2 ] + m->m[ 13 ] * n->m[  3 ];
        result.m[  2 ] = m->m[ 2 ] * n->m[  0 ] + m->m[ 6 ] * n->m[  1 ] + m->m[ 10 ] * n->m[  2 ] + m->m[ 14 ] * n->m[  3 ];
        result.m[  3 ] = m->m[ 3 ] * n->m[  0 ] + m->m[ 7 ] * n->m[  1 ] + m->m[ 11 ] * n->m[  2 ] + m->m[ 15 ] * n->m[  3 ];
        result.m[  4 ] = m->m[ 0 ] * n->m[  4 ] + m->m[ 4 ] * n->m[  5 ] + m->m[  8 ] * n->m[  6 ] + m->m[ 12 ] * n->m[  7 ];
        result.m[  5 ] = m->m[ 1 ] * n->m[  4 ] + m->m[ 5 ] * n->m[  5 ] + m->m[  9 ] * n->m[  6 ] + m->m[ 13 ] * n->m[  7 ];
        result.m[  6 ] = m->m[ 2 ] * n->m[  4 ] + m->m[ 6 ] * n->m[  5 ] + m->m[ 10 ] * n->m[  6 ] + m->m[ 14 ] * n->m[  7 ];
        result.m[  7 ] = m->m[ 3 ] * n->m[  4 ] + m->m[ 7 ] * n->m[  5 ] + m->m[ 11 ] * n->m[  6 ] + m->m[ 15 ] * n->m[  7 ];
        result.m[  8 ] = m->m[ 0 ] * n->m[  8 ] + m->m[ 4 ] * n->m[  9 ] + m->m[  8 ] * n->m[ 10 ] + m->m[ 12 ] * n->m[ 11 ];
        result.m[  9 ] = m->m[ 1 ] * n->m[  8 ] + m->m[ 5 ] * n->m[  9 ] + m->m[  9 ] * n->m[ 10 ] + m->m[ 13 ] * n->m[ 11 ];
        result.m[ 10 ] = m->m[ 2 ] * n->m[  8 ] + m->m[ 6 ] * n->m[  9 ] + m->m[ 10 ] * n->m[ 10 ] + m->m[ 14 ] * n->m[ 11 ];
        result.m[ 11 ] = m->m[ 3 ] * n->m[  8 ] + m->m[ 7 ] * n->m[  9 ] + m->m[ 11 ] * n->m[ 10 ] + m->m[ 15 ] * n->m[ 11 ];
        result.m[ 12 ] = m->m[ 0 ] * n->m[ 12 ] + m->m[ 4 ] * n->m[ 13 ] + m->m[  8 ] * n->m[ 14 ] + m->m[ 12 ] * n->m[ 15 ];
        result.m[ 13 ] = m->m[ 1 ] * n->m[ 12 ] + m->m[ 5 ] * n->m[ 13 ] + m->m[  9 ] * n->m[ 14 ] + m->m[ 13 ] * n->m[ 15 ];
        result.m[ 14 ] = m->m[ 2 ] * n->m[ 12 ] + m->m[ 6 ] * n->m[ 13 ] + m->m[ 10 ] * n->m[ 14 ] + m->m[ 14 ] * n->m[ 15 ];
        result.m[ 15 ] = m->m[ 3 ] * n->m[ 12 ] + m->m[ 7 ] * n->m[ 13 ] + m->m[ 11 ] * n->m[ 14 ] + m->m[ 15 ] * n->m[ 15 ];

        return result;
    }

    void Translate( m44* m, v3 t )
    {
        m->m[ 12 ] = m->m[ 0 ] * t.v[ 0 ] + m->m[ 4 ] * t.v[ 1 ] + m->m[  8 ] * t.v[ 2 ] + m->m[ 12 ];
        m->m[ 13 ] = m->m[ 1 ] * t.v[ 0 ] + m->m[ 5 ] * t.v[ 1 ] + m->m[  9 ] * t.v[ 2 ] + m->m[ 13 ];
        m->m[ 14 ] = m->m[ 2 ] * t.v[ 0 ] + m->m[ 6 ] * t.v[ 1 ] + m->m[ 10 ] * t.v[ 2 ] + m->m[ 14 ];
        m->m[ 15 ] = m->m[ 3 ] * t.v[ 0 ] + m->m[ 7 ] * t.v[ 1 ] + m->m[ 11 ] * t.v[ 2 ] + m->m[ 15 ];
    }

    void Scale( m44* m, v3 s )
    {
        m->m[  0 ] *= s.v[ 0 ];
        m->m[  1 ] *= s.v[ 0 ];
        m->m[  2 ] *= s.v[ 0 ];
        m->m[  3 ] *= s.v[ 0 ];
        m->m[  4 ] *= s.v[ 1 ];
        m->m[  5 ] *= s.v[ 1 ];
        m->m[  6 ] *= s.v[ 1 ];
        m->m[  7 ] *= s.v[ 1 ];
        m->m[  8 ] *= s.v[ 2 ];
        m->m[  9 ] *= s.v[ 2 ];
        m->m[ 10 ] *= s.v[ 2 ];
        m->m[ 11 ] *= s.v[ 2 ];
    }

    void Scale( m44* m, f32 s )
    {
        m->m[  0 ] *= s;
        m->m[  1 ] *= s;
        m->m[  2 ] *= s;
        m->m[  3 ] *= s;
        m->m[  4 ] *= s;
        m->m[  5 ] *= s;
        m->m[  6 ] *= s;
        m->m[  7 ] *= s;
        m->m[  8 ] *= s;
        m->m[  9 ] *= s;
        m->m[ 10 ] *= s;
        m->m[ 11 ] *= s;
    }
}


v2 operator *( v2 v, f32 s ) { v2 result = v2{ v.x * s, v.y * s }; return result; }
v2 operator *( f32 s, v2 v ) { v2 result = v2{ v.x * s, v.y * s }; return result; }
v3 operator *( v3 v, f32 s ) { v3 result = v3{ v.x * s, v.y * s, v.z * s }; return result; }
v3 operator *( f32 s, v3 v ) { v3 result = v3{ v.x * s, v.y * s, v.z * s }; return result; }
v4 operator *( v4 v, f32 s ) { v4 result = v4{ v.x * s, v.y * s, v.z * s, v.w * s }; return result; }
v4 operator *( f32 s, v4 v ) { v4 result = v4{ v.x * s, v.y * s, v.z * s, v.w * s }; return result; }
#ifndef TYPES_H
#define TYPES_H


#if defined( _MSC_VER ) || defined( __GNUC__ )
    typedef char                s8;
    typedef unsigned char       u8;

    typedef short               s16;
    typedef unsigned short      u16;

    typedef int                 s32;
    typedef unsigned int        u32;

    typedef long long           s64;
    typedef unsigned long long  u64;

    typedef float               f32;

    typedef double              f64;
#else
    //@NOTE add s32, u32 etc. typedefs
    //for other compilers here
    #error
#endif


#define S8_MAX  0x7f
#define S8_MIN  ( s8 ) 0x80
#define U8_MAX  0xff

#define S16_MAX 0x7fff
#define S16_MIN ( s16 ) 0x8000
#define U16_MAX 0xffff

#define S32_MAX 0x7fffffff
#define S32_MIN ( s32 ) 0x80000000
#define U32_MAX 0xffffffff

#define S64_MAX 0x7fffffffffffffff
#define S64_MIN ( s64 ) 0x8000000000000000
#define U64_MAX 0xffffffffffffffff

#define F32_MAX 3.402823466e+38f
#define F32_MIN -F32_MAX

#define F64_MAX 1.7976931348623158e+308
#define F64_MIN -F64_MAX


struct v2_f32
{
    union
    {
        f32 v[ 2 ];
        struct
        {
            f32 x;
            f32 y;
        };
    };
};
typedef v2_f32 v2;

struct v2_s32
{
    union
    {
        s32 v[ 2 ];
        struct
        {
            s32 x;
            s32 y;
        };
    };
};
typedef v2_s32 v2i;

struct v3_f32
{
    union
    {
        f32 v[ 3 ];
        struct
        {
            f32 x;
            f32 y;
            f32 z;
        };
    };
};
typedef v3_f32 v3;

struct v4_f32
{
    union
    {
        f32 v[ 4 ];
        struct
        {
            f32 x;
            f32 y;
            f32 z;
            f32 w;
        };
    };
};
typedef v4_f32 v4;

//@NOTE column major
struct m44_f32
{
    union
    {
        f32 m[ 16 ];
        struct
        {
            v4 column0;
            v4 column1;
            v4 column2;
            v4 column3;
        };
    };
};
typedef m44_f32 m44;


#endif
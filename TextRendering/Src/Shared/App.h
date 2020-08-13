#ifndef APP_H
#define APP_H


#include "Types.h"


namespace App
{
    void Resize( s32 width, s32 height );
    void Init( s32 width, s32 height );
    void Update();
    void Render();
}


#endif
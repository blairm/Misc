#include <pspkernel.h>
#include <pspgu.h>
#include <pspdisplay.h>
#include <pspctrl.h>


//@NOTE name, attributes?, major, minor version
PSP_MODULE_INFO( "PSP Test", 0, 1, 0 );
PSP_MAIN_THREAD_ATTR( PSP_THREAD_ATTR_USER );


#define WIDTH           480
#define HEIGHT          272
#define BUFFER_WIDTH    512
#define PIXEL_FORMAT    GU_PSM_8888                     //@NOTE GU_PSM_5650, GU_PSM_5551, GU_PSM_4444, GU_PSM_8888


bool done = false;

char displayList[ 0x10000 ] __attribute__ ( ( aligned( 64 ) ) );


int ExitCallback( int arg1, int arg2, void *common )
{
    done = true;
    return 0;
}

int ExitCallbackThread( SceSize args, void *argp )
{
    int callbackId = sceKernelCreateCallback( "Exit Callback", ExitCallback, 0 );
    sceKernelRegisterExitCallback( callbackId );
    sceKernelSleepThreadCB();
    return 0;
}

int main()
 {
    //@TODO figure out good values for these
    int threadPriority                      = 17;
    int stackSize                           = 4000;
    SceUInt threadAttribs                   = 0;            //@NOTE PspThreadAttributes
    SceKernelThreadOptParam *threadOptParam = 0;

    int threadId = sceKernelCreateThread( "ExitThread",
                                            ExitCallbackThread,
                                            threadPriority,
                                            stackSize,
                                            threadAttribs,
                                            threadOptParam );
    if( threadId >= 0 )
        sceKernelStartThread( threadId, 0, 0 );


    SceCtrlData pad;

    sceCtrlSetSamplingCycle( 0 );
    sceCtrlSetSamplingMode( PSP_CTRL_MODE_ANALOG );


    sceGuInit();

    sceGuStart( GU_DIRECT, displayList );


    int bytesPerPixel = 2;
    if( PIXEL_FORMAT == GU_PSM_8888 )
        bytesPerPixel = 4;

    void* drawBuffer    = 0;
    void* displayBuffer = ( void* ) ( BUFFER_WIDTH * HEIGHT * bytesPerPixel );
    sceGuDrawBuffer( PIXEL_FORMAT, drawBuffer, BUFFER_WIDTH );
    sceGuDispBuffer( WIDTH, HEIGHT, displayBuffer, BUFFER_WIDTH );


    //@NOTE vertices have to be within range 0 - 4095 otherwise primitive gets culled
    sceGuOffset( 2048 - ( WIDTH / 2 ), 2048 - ( HEIGHT / 2 ) );
    sceGuViewport( 2048, 2048, WIDTH, HEIGHT );


    sceGuEnable( GU_SCISSOR_TEST );
    sceGuScissor( 0, 0, WIDTH, HEIGHT );

    //@NOTE only applies to near plane?
    sceGuEnable( GU_CLIP_PLANES );


    float quadSpeed         = 3;
    float quadWidth         = 50;
    float quadHeight        = quadWidth;
    float quadX             = WIDTH / 2 - quadWidth / 2;
    float quadY             = HEIGHT / 2 - quadHeight / 2;
    float quadVertices[]    = { quadX, quadY, 0,
                                quadX + quadWidth, quadY + quadHeight, 0 };


    sceGuFinish();
    sceGuDisplay( true );

    
    while( !done )
    {
        sceCtrlReadBufferPositive( &pad, 1 );

        quadX += quadSpeed * ( ( pad.Lx - 128 ) / 255.0f );
        quadY += quadSpeed * ( ( pad.Ly - 128 ) / 255.0f );

        if( pad.Buttons & PSP_CTRL_CROSS )
        {
            quadX = WIDTH / 2 - quadWidth / 2;
            quadY = HEIGHT / 2 - quadHeight / 2;
        }

        quadVertices[ 0 ] = quadX;
        quadVertices[ 1 ] = quadY;
        quadVertices[ 3 ] = quadX + quadWidth;
        quadVertices[ 4 ] = quadY + quadHeight;

        sceKernelDcacheWritebackAll();


        sceGuStart( GU_DIRECT, displayList );
        sceGuClearColor( 0xFFFFFFFF );
        sceGuClear( GU_COLOR_BUFFER_BIT );


        sceGuColor( 0xFF00FFFF );
        sceGuDrawArray( GU_SPRITES, GU_VERTEX_BITS | GU_TRANSFORM_2D, 2, 0, quadVertices );


        sceGuFinish();
        sceGuSync( 0, 0 );
        sceDisplayWaitVblankStart();
        sceGuSwapBuffers();
    }


    sceGuDisplay( false );
    sceGuTerm();

    sceKernelExitGame();


    return 0;
}

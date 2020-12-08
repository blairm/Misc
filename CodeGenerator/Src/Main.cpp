#include "Types.h"
#include "Utils.h"

#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>


char TEMPLATE_TYPES[]   = "#TEMPLATETYPES";
char TEMPLATE_TAG[]     = "<T>";
char TEMPLATE_HEADER[]  = "#TEMPLATEHEADER";
char TEMPLATE_BODY[]    = "#TEMPLATEBODY";
char TEMPLATE_FOOTER[]  = "#TEMPLATEFOOTER";
char TEMPLATE_EXT[]     = ".template";


void skipWhiteSpace( char** data )
{
    while( **data == ' '
            || **data == '\t'
            || **data == '\n'
            || **data == 0 )
        ++*data;
}

int main( int argc,
          char** argv )
{
    char** input = argv;
    for( s32 i = 0; i < argc; ++i )
    {
        char* templateName = strstr( *input, TEMPLATE_EXT );
        if( templateName > 0
            && *( templateName + ARRAY_COUNT( TEMPLATE_EXT ) - 1 ) == 0 )
        {
            templateName = *input;

            struct stat statBuffer;
            s32 statResult = stat( templateName, &statBuffer );
            if( statResult == 0 )
            {
                s32 templateSize = statBuffer.st_size;
                printf( "%s, file size: %d bytes\n",
                        templateName,
                        templateSize );

                char* templateData = NEW( char, templateSize );

                FILE* templateFile = fopen( templateName, "rb" );
                if( templateFile )
                {
                    if( fread( templateData, 1, templateSize, templateFile ) == ( u32 ) templateSize )
                    {
                        s32 length = ( s32 ) strlen( templateName ) - ARRAY_COUNT( TEMPLATE_EXT ) + 2;  //+1 as strlen doesn't count null char and +1 as ARRAY_COUNT does
                        char* generatedName = NEW( char, length );
                        strncpy( generatedName, templateName, length - 1 );
                        generatedName[ length - 1 ] = 0;

                        FILE* generatedFile = fopen( generatedName, "wb" );
                        if( generatedFile )
                        {
                            if( strncmp( templateData, TEMPLATE_TYPES, ARRAY_COUNT( TEMPLATE_TYPES ) - 1 ) == 0 )
                            {
                                char* tType[ 16 ];                                                      //16 should be enough
                                s32 tTypeCount = 0;

                                char* dataCurr = strstr( templateData, "{" );
                                ++dataCurr;
                                char* token = strtok( dataCurr, " ,\n" );
                                while( token != 0 )
                                {
                                    printf( "%s\n", token );
                                    tType[ tTypeCount ] = token;
                                    ++tTypeCount;

                                    token = strtok( 0, " ,\n" );
                                    if( *token == '}' )
                                    {
                                        dataCurr = token + 1;
                                        break;
                                    }
                                }

                                skipWhiteSpace( &dataCurr );

                                if( strncmp( dataCurr, TEMPLATE_HEADER, ARRAY_COUNT( TEMPLATE_HEADER ) - 1 ) == 0 )
                                {
                                    dataCurr += ARRAY_COUNT( TEMPLATE_HEADER );
                                    skipWhiteSpace( &dataCurr );

                                    char* str = strstr( dataCurr, TEMPLATE_BODY );
                                    if( !str )
                                        str = strstr( dataCurr, TEMPLATE_FOOTER );
                                    
                                    s32 writeSize = 0;
                                    if( str )
                                        writeSize = ( s32 ) ( str - dataCurr );
                                    else
                                        writeSize = templateSize - ( s32 ) ( dataCurr - templateData );

                                    fwrite( dataCurr,
                                            1,
                                            writeSize,
                                            generatedFile );

                                    dataCurr += writeSize;
                                }

                                if( strncmp( dataCurr, TEMPLATE_BODY, ARRAY_COUNT( TEMPLATE_BODY ) - 1 ) == 0 )
                                {
                                    dataCurr += ARRAY_COUNT( TEMPLATE_BODY );
                                    skipWhiteSpace( &dataCurr );

                                    char* str = strstr( dataCurr, TEMPLATE_FOOTER );
                                    s32 writeSize = 0;
                                    if( str )
                                        writeSize = ( s32 ) ( str - dataCurr );
                                    else
                                        writeSize = templateSize - ( s32 ) ( dataCurr - templateData );

                                    for( s32 i = 0; i < tTypeCount; ++i )
                                    {
                                        str = strstr( dataCurr, TEMPLATE_TAG );
                                        char* dataBody = dataCurr;

                                        s32 tTypeLength = ( s32 ) strlen( tType[ i ] );

                                        while( str && str < dataCurr + writeSize )
                                        {
                                            fwrite( dataBody,
                                                    1,
                                                    str - dataBody,
                                                    generatedFile );

                                            fwrite( tType[ i ],
                                                    1,
                                                    tTypeLength,
                                                    generatedFile );

                                            dataBody += ( str - dataBody ) + ARRAY_COUNT( TEMPLATE_TAG ) - 1;
                                            str = strstr( dataBody, TEMPLATE_TAG );
                                        }

                                        if( dataBody < dataCurr + writeSize )
                                        {
                                            fwrite( dataBody,
                                                    1,
                                                    ( dataCurr + writeSize ) - dataBody,
                                                    generatedFile );
                                        }
                                    }

                                    dataCurr += writeSize;
                                }

                                if( strncmp( dataCurr, TEMPLATE_FOOTER, ARRAY_COUNT( TEMPLATE_FOOTER ) - 1 ) == 0 )
                                {
                                    dataCurr += ARRAY_COUNT( TEMPLATE_FOOTER );
                                    skipWhiteSpace( &dataCurr );

                                    s32 writeSize = templateSize - ( s32 ) ( dataCurr - templateData );
                                    fwrite( dataCurr,
                                            1,
                                            writeSize,
                                            generatedFile );
                                }
                            }
                            else
                            {
                                printf( "templatetypes not found at start of file\n" );
                            }

                            fclose( generatedFile );
                        }

                        FREE( generatedName );
                    }
                    else
                    {
                        printf( "failed to read whole file\n" );
                    }

                    fclose( templateFile );
                }

                FREE( templateData );
            }
            else
            {
                printf( "%s, stat failed: %d\n",
                        templateName,
                        statResult );
            }
        }

        ++input;
    }

    system( "pause" );
    return 0;
}
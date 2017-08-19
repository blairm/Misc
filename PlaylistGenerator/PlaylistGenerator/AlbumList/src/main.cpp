/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org>
*/

#include <windows.h>
#include <fstream>
#include <string>
#include <vector>

std::string MUSIC_FOLDER = "..\\..\\..\\Music\\";

int main( int argc, char** argv )
{
	WIN32_FIND_DATA artistFindData;
	HANDLE artistHandle = FindFirstFile( ( MUSIC_FOLDER + "*" ).c_str(), &artistFindData );
	if( artistHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			bool isDirectory = (artistFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			char* artistFolderName = artistFindData.cFileName;
			if( isDirectory && artistFolderName[ 0 ] != '.' )
			{
				printf( "%s\n", artistFolderName );

				std::vector<std::string> albumName;

				std::string albumTxtPath = MUSIC_FOLDER + artistFolderName + "\\Albums.txt";

				std::ifstream inStream( albumTxtPath );
				if( inStream.is_open() )
				{
					std::string line;
					while( std::getline( inStream, line ) )
						albumName.push_back( line );
				}

				WIN32_FIND_DATA albumFindData;
				HANDLE albumHandle = FindFirstFile( ( MUSIC_FOLDER + artistFolderName + "\\*" ).c_str(), &albumFindData );
				if( albumHandle != INVALID_HANDLE_VALUE )
				{
					std::vector< std::string > newAlbumName;

					do
					{
						bool isDirectory = ( albumFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) != 0;
						char* albumFolderName = albumFindData.cFileName;
						if( isDirectory && albumFolderName[ 0 ] != '.' )
						{
							printf( "\t%s\n", albumFolderName );

							std::string album = albumFolderName;

							bool found = false;
							for( int i = 0; i < albumName.size(); ++i )
							{
								if( album == albumName[ i ] )
								{
									found = true;
									break;
								}
							}

							if( !found )
							{
								printf( "\t\tadding\n", albumFolderName );
								newAlbumName.push_back( album );
							}
						}
					}
					while( FindNextFile( albumHandle, &albumFindData ) );

					FindClose( albumHandle );

					if( newAlbumName.size() > 0 )
					{
						std::ofstream outStream( albumTxtPath, std::ios::out | std::ios::app );
						if( outStream.is_open() )
						{
							for( int i = 0; i < newAlbumName.size(); ++i )
								outStream << newAlbumName[ i ] << std::endl;

							outStream.close();
						}
					}
				}
			}
		}
		while( FindNextFile( artistHandle, &artistFindData ) );

		FindClose( artistHandle );
	}

	system( "pause" );
	return 0;
}
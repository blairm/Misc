/*
Permission to use, copy, modify, and/or distribute this software for any purpose with or
without fee is hereby granted.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO
THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT
SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR
ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE
USE OR PERFORMANCE OF THIS SOFTWARE.
*/

#include <windows.h>
#include <fstream>
#include <string>
#include <vector>


#define ARRAY_COUNT( array ) sizeof( ( array ) ) / sizeof( ( array )[ 0 ] )


char tags[] = { '*', '-' };

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
					{
						for( int i = 0; i < ARRAY_COUNT( tags ); ++i )
						{
							if( line.back() == tags[ i ] )
								line.pop_back();
						}

						albumName.push_back( line );
					}
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
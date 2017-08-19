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

std::string TOPLEVEL_FOLDER = "..\\..\\..\\";
std::string MUSIC_FOLDER = TOPLEVEL_FOLDER + "Music\\";
std::string PLAYLIST_FOLDER = TOPLEVEL_FOLDER + "Playlists\\";
std::string PLAYLIST_TO_MUSIC_FOLDER = "..\\Music\\";

char toLower( char param )
{
	if( param >= 65 && param <= 90 )
		param += 32;
	else if( param == 'Ö' )
		param = 'ö';

	return param;
}

bool ignoreChar( char param, char charToIgnore )
{
	return param == charToIgnore;
}

int compareString( char* a, char* b )
{
	char testA = 0;
	char testB = 0;

	while( *a != 0 && *b != 0 )
	{
		if( ignoreChar( *a, '.' ) )
			++a;
		testA = toLower( *a );

		if( ignoreChar( *b, '.' ) )
			++b;
		testB = toLower( *b );

		if( testA < testB )
			return -1;
		else if( testA > testB )
			return 1;

		++a;
		++b;
	}

	testA = toLower( *a );
	testB = toLower( *b );

	if( testA == testB )
		return 0;
	else if( testA < testB )
		return -1;
	else
		return 1;
}

void mergeSort( std::vector<std::string>* vectorToSort, bool ascending )
{
	int arrayCount		= vectorToSort->size();
	int stride			= 2;
	int halfStride		= stride / 2;
	int writeIndex		= 0;
	
	std::vector< std::string > vectorBuffer( arrayCount );
	std::vector< std::string >* readVector	= &vectorBuffer;
	std::vector< std::string >* writeVector	= vectorToSort;
	
	while( halfStride < arrayCount )
	{
		//ping pong between buffers
		std::vector< std::string >* swapVector	= readVector;
		readVector								= writeVector;
		writeVector								= swapVector;

		writeIndex	= ++writeIndex & 1;

		for( int i = 0; i < arrayCount; i += stride )
		{
			int indexA = i;
			int indexB = i + halfStride;
			
			for( int j = 0; j < stride && i + j < arrayCount; ++j )
			{
				if( indexA >= i + halfStride )
				{
					( *writeVector )[ i + j ] = ( *readVector )[ indexB++ ];
				}
				else if( indexB >= i + stride || indexB >= arrayCount )
				{
					( *writeVector )[ i + j ] = ( *readVector )[ indexA++ ];
				}
				else
				{
					int result = compareString( &( ( *readVector )[ indexA ][ 0 ] ), &( *readVector )[ indexB ][ 0 ] );
					if( ( result < 0 && ascending ) || ( result > 0 && !ascending ) )
						( *writeVector )[ i + j ] = ( *readVector )[ indexA++ ];
					else
						( *writeVector )[ i + j ] = ( *readVector )[ indexB++ ];
				}
			}
		}

		halfStride	= stride;
		stride		*= 2;
	}

	if( writeIndex == 1 )
		vectorToSort->swap( vectorBuffer );
}

int main( int argc, char** argv )
{
	WIN32_FIND_DATA playlistFindData;
	HANDLE playlistHandle = FindFirstFile( ( PLAYLIST_FOLDER + "*" ).c_str(), &playlistFindData );
	if( playlistHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			DeleteFile( ( PLAYLIST_FOLDER + playlistFindData.cFileName ).c_str() );
		}
		while( FindNextFile( playlistHandle, &playlistFindData ) );

		FindClose( playlistHandle );
	}

	std::vector<std::string> artistList;

	WIN32_FIND_DATA artistFindData;
	HANDLE artistHandle = FindFirstFile( ( MUSIC_FOLDER + "*" ).c_str(), &artistFindData );
	if( artistHandle != INVALID_HANDLE_VALUE )
	{
		do
		{
			bool isDirectory = (artistFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
			char* artistFolderName = artistFindData.cFileName;
			if( isDirectory && artistFolderName[ 0 ] != '.' )
				artistList.push_back( artistFolderName );
		}
		while( FindNextFile( artistHandle, &artistFindData ) );

		FindClose( artistHandle );
	}

	mergeSort( &artistList, false );

	for( int i = 0; i < artistList.size(); ++i )
	{
		printf( "%s\n", artistList[ i ].c_str() );

		std::string albumTxtPath = MUSIC_FOLDER + artistList[ i ] + "\\Albums.txt";

		std::ifstream inStream( albumTxtPath );
		if( inStream.is_open() )
		{
			std::vector< std::string > songList;
			std::vector< std::string > songPathList;

			std::string albumName;
			while( std::getline( inStream, albumName ) )
			{
				WIN32_FIND_DATA songFindData;
				HANDLE songHandle = FindFirstFile( ( MUSIC_FOLDER + artistList[ i ] + "\\" + albumName + "\\*.mp3" ).c_str(), &songFindData );
				if( songHandle != INVALID_HANDLE_VALUE )
				{
					std::vector< std::string > albumSongList;

					do
					{
						std::string songName = songFindData.cFileName;
						std::size_t songStart = songName.find(" ") + 1;

						bool found = false;
						for( int j = 0; j < songList.size(); ++j )
						{
							if( songName.substr( songStart ) == songList[ j ].substr( songStart ) )
							{
								found = true;
								break;
							}
						}

						if(!found)
							albumSongList.push_back( songName );
					}
					while( FindNextFile( songHandle, &songFindData ) );

					FindClose( songHandle );

					mergeSort( &albumSongList, true );

					for( int j = 0; j < albumSongList.size(); ++j )
					{
						songList.push_back( albumSongList[ j ] );
						songPathList.push_back( PLAYLIST_TO_MUSIC_FOLDER + artistList[ i ] + "\\" + albumName + "\\" + albumSongList[ j ] );
					}
				}
			}

			if( songList.size() > 0 )
			{
				std::string playlistPath = PLAYLIST_FOLDER + artistList[ i ] + ".m3u";
				std::ofstream outStream( playlistPath, std::ios::out );
				outStream.rdbuf()->pubsetbuf( 0, 0 );
				if( outStream.is_open() )
				{
					outStream << "#EXTM3U" << std::endl;

					for( int i = 0; i < songList.size(); ++i )
					{
						outStream << "#EXTINF:0," << songList[ i ] << std::endl;
						outStream << songPathList[ i ] << std::endl;
						outStream << std::endl;
					}

					outStream.close();
				}
			}
		}
	}

	system( "pause" );
	return 0;
}
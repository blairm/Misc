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

#ifdef _MSC_VER
	typedef char				s8;
	typedef unsigned char		u8;
	#define S8_MAX	SCHAR_MAX
	#define S8_MIN	SCHAR_MIN
	#define U8_MAX	UCHAR_MAX

	typedef short				s16;
	typedef unsigned short		u16;
	#define S16_MAX	SHRT_MAX
	#define S16_MIN	SHRT_MIN
	#define U16_MAX	USHRT_MAX

	typedef int					s32;
	typedef unsigned int		u32;
	#define S32_MAX	INT_MAX
	#define S32_MIN	INT_MIN
	#define	U32_MAX	UINT_MAX

	typedef long long			s64;
	typedef unsigned long long	u64;
	#define S64_MAX	LLONG_MAX
	#define S64_MIN	LLONG_MIN
	#define U64_MAX	ULLONG_MAX
	
	typedef float				f32;
	#define F32_MAX	FLT_MAX
	#define F32_MIN	-FLT_MAX

	typedef double				f64;
	#define F64_MAX	DBL_MAX
	#define F64_MIN	-DBL_MAX
#else
	//add s32, u32 etc. typedefs and #defines for
	//other compilers here
	#error
#endif

#ifdef _MSC_VER
	#include <windows.h>

	struct TimedBlock
	{
		f64 counterFrequency;
		f64 startTime;
		f32* _timeMs;

		TimedBlock( f32* timeMs )
		{
			_timeMs = timeMs;

			LARGE_INTEGER wallClockFrequency;
			QueryPerformanceFrequency( &wallClockFrequency );
			counterFrequency = ( f64 ) wallClockFrequency.QuadPart;

			LARGE_INTEGER wallClock;
			QueryPerformanceCounter( &wallClock );
			startTime = ( f64 ) wallClock.QuadPart;
		}

		~TimedBlock()
		{
			LARGE_INTEGER wallClock;
			QueryPerformanceCounter( &wallClock );
			f64 totalTime = ( ( f64 ) wallClock.QuadPart - startTime ) / counterFrequency;
			*_timeMs = ( f32 ) totalTime * 1000.0f;
		}
	};
	#define TIMED_BLOCK( time ) TimedBlock timedBlock( ( time ) )
#else
	#define TIMED_BLOCK( time )
#endif

#define ARRAY_COUNT( array ) sizeof( ( array ) ) / sizeof( ( array )[ 0 ] )

#ifdef _DEBUG
	#define ASSERT( expression ) if( !( expression ) ) { *( int* ) 0 = 0; }
#else
	#define ASSERT( expression )
#endif


#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <time.h>


const s32	NODE_NAME_MAX_SIZE	= 32;
const s32	MAX_PLACE_COUNT		= 1000;

const bool	VALIDATE			= false;


struct KDNode2D
{
	s32			pos[ 2 ];
	void*		data;
	KDNode2D*	left;
	KDNode2D*	right;
};


void swap( KDNode2D* a, KDNode2D* b )
{
	KDNode2D temp;
	memcpy( &temp, a, sizeof( KDNode2D ) );
	memcpy( a, b, sizeof( KDNode2D ) );
	memcpy( b, &temp, sizeof( KDNode2D ) );
}

KDNode2D* buildKDTree2D( KDNode2D* node, s32 length, s8 currAxis )
{
	ASSERT( node );

	KDNode2D* median = 0;

	if( length > 0 )
	{
		median = node;

		if( length > 1 )
		{
			KDNode2D* start	= node;
			KDNode2D* end	= node + length;
			median			= start + ( length / 2 );

			while( start != end )
			{
				s32 pivotValue = median->pos[ currAxis ];
				
				swap( median, end - 1 );

				KDNode2D* store = start;
				for( KDNode2D* currNode = start; currNode < end; ++currNode )
				{
					if( currNode->pos[ currAxis ] < pivotValue )
					{
						if( currNode != store )
							swap( currNode, store );

						++store;
					}
				}

				swap( store, end - 1 );

				if( store->pos[ currAxis ] == median->pos[ currAxis ] )
				{
					median = store;
					break;
				}
				else if( store > median )
				{
					end = store;
				}
				else
				{
					start = store;
				}
			}
		}

		currAxis		= ++currAxis & 1;

		median->left	= buildKDTree2D( node, ( s32 ) ( median - node ), currAxis );
		median->right	= buildKDTree2D( median + 1, ( s32 ) ( node + length - median - 1 ), currAxis );
	}

	return median;
}

KDNode2D* getNearestNeighbour( KDNode2D* root, s32 pos[], s64& currDistSqr, s8 currAxis = 0, KDNode2D* nearest = 0 )
{
	ASSERT( root );

	s32 nextAxis		= ( currAxis + 1 ) & 1;

	s64 splitDist		= root->pos[ currAxis ] - pos[ currAxis ];
	s64 splitDistSqr	= splitDist * splitDist;
	s64 otherDistSqr	= root->pos[ nextAxis ] - pos[ nextAxis ];
	otherDistSqr		*= otherDistSqr;
	s64 distSqr			= splitDistSqr + otherDistSqr;

	if( distSqr < currDistSqr )
	{
		nearest		= root;
		currDistSqr	= distSqr;
	}

	KDNode2D* nearSubTree	= root->left;
	KDNode2D* farSubTree	= root->right;

	if( splitDist < 0 )
	{
		nearSubTree	= root->right;
		farSubTree	= root->left;
	}

	currAxis = nextAxis;

	if( nearSubTree )
		nearest = getNearestNeighbour( nearSubTree, pos, currDistSqr, currAxis, nearest );

	if( splitDistSqr < currDistSqr && farSubTree )
		nearest = getNearestNeighbour( farSubTree, pos, currDistSqr, currAxis, nearest );

	return nearest;
}

void validateNearestNeighbourResult( KDNode2D* nodeArray, s32 length, s32 pos[], KDNode2D* nearest )
{
	KDNode2D* nearestFound	= 0;
	s64 currDistSqr			= S64_MAX;

	for( KDNode2D* end = nodeArray + length; nodeArray != end; ++nodeArray )
	{
		s64 distX	= nodeArray->pos[ 0 ] - pos[ 0 ];
		distX		*= distX;
		s64 distY	= nodeArray->pos[ 1 ] - pos[ 1 ];
		distY		*= distY;

		if( distX + distY < currDistSqr )
		{
			nearestFound	= nodeArray;
			currDistSqr		= distX + distY;
		}
	}

	ASSERT( nearestFound == nearest );
}

s32 main( s32 argc, char** argv )
{
	s32 seed = ( s32 ) time( 0 );
	srand( seed );

	s32 mapPlaceCount				= MAX_PLACE_COUNT;
	KDNode2D* mapPlaceKDNodeArray	= new KDNode2D[ mapPlaceCount ];

	for( s32 i = 0; i < mapPlaceCount; ++i )
	{
		mapPlaceKDNodeArray[ i ].data		= new char[ NODE_NAME_MAX_SIZE ];
		snprintf( ( char* ) mapPlaceKDNodeArray[ i ].data, NODE_NAME_MAX_SIZE, "node %d", i );
		mapPlaceKDNodeArray[ i ].pos[ 0 ]	= rand() % 10000001;
		mapPlaceKDNodeArray[ i ].pos[ 1 ]	= rand() % 10000001;
	}

	KDNode2D* kdTreeRoot;
	KDNode2D* nearest;

	f32 setupTimeMs = 0;
	{
		TIMED_BLOCK( &setupTimeMs );
		kdTreeRoot = buildKDTree2D( mapPlaceKDNodeArray, mapPlaceCount, 0 );
	}
	printf( "setup time:%.4fms\n", setupTimeMs );

	s32 posToTest[] = { rand() % 10000001, rand() % 10000001 };

	f32 computeTimeMs = 0;
	{
		TIMED_BLOCK( &computeTimeMs );
		s64 distSqr = S64_MAX;
		nearest = getNearestNeighbour( kdTreeRoot, posToTest, distSqr );
		printf( "nearest neighbour: %s, distance: %f\n", ( char* ) nearest->data, sqrt( ( f64 ) distSqr ) );
	}
	printf( "compute time:%.4fms\n", computeTimeMs );

	if( VALIDATE )
		validateNearestNeighbourResult( mapPlaceKDNodeArray, mapPlaceCount, posToTest, nearest );

	system( "pause" );
	return 0;
}
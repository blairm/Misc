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

#define ARRAY_COUNT( array ) sizeof( ( array ) ) / sizeof( ( array )[ 0 ] )

#ifdef _DEBUG
	#define ASSERT( expression ) if( !( expression ) ) { *( int* ) 0 = 0; }
#else
	#define ASSERT( expression )
#endif


#include <stdio.h>
#include <windows.h>


const s32	ITEM_COUNT					= 10;
const s32	PASS_COUNT					= 5;

const bool	VALIDATE					= false;

const bool	BUBBLE_SORT_ENABLED			= true;
const bool	INSERTION_SORT_ENABLED		= true;
const bool	QUICKSORT_ENABLED			= true;
const bool	MERGE_SORT_ENABLED			= true;
const bool	MERGE_SORT_IN_PLACE_ENABLED	= true;
const bool	RADIX_8_SORT_ENABLED		= true;
const bool	RADIX_16_SORT_ENABLED		= true;


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


void bubbleSort( s32* arrayToSort,
					s32 length )
{
	if( length < 2 )
		return;

	bool swapped;
	s32* arrayIter;
	s32* arrayEndMinusOne	= arrayToSort + length - 1;
	s32 swap;

	do
	{
		swapped = false;

		for( arrayIter = arrayToSort;
				arrayIter != arrayEndMinusOne;
				++arrayIter )
		{
			if( *arrayIter > *( arrayIter + 1 ) )
			{
				swap				= *arrayIter;
				*arrayIter			= *( arrayIter + 1 );
				*(arrayIter + 1)	= swap;

				swapped = true;
			}
		}
	}
	while( swapped );
}

void insertionSort( s32* arrayToSort,
					s32 length )
{
	if( length < 2 )
		return;

	s32* arrayIter;
	s32* arrayEnd	= arrayToSort + length;

	for( arrayIter = arrayToSort + 1;
			arrayIter != arrayEnd;
			++arrayIter )
	{
		for( s32* insertionIter = arrayIter;
				insertionIter != arrayToSort;
				--insertionIter )
		{
			s32* insertionMinusOneIter	= insertionIter - 1;

			s32 valueToInsert			= *insertionIter;
			s32 valueToTest				= *insertionMinusOneIter;

			if( valueToInsert < valueToTest )
			{
				*insertionMinusOneIter	= valueToInsert;
				*insertionIter			= valueToTest;
			}
			else
			{
				break;
			}
		}
	}
}

void quickSort( s32* arrayToSort,
				s32 length )
{
	if( length < 2 )
		return;

	s32 pivot = arrayToSort[ length / 2 ];

	s32 start = 0;
	for( s32 end = length - 1; true; ++start, --end )
	{
		while( arrayToSort[ start ] < pivot )
			++start;

		while( arrayToSort[ end ] > pivot )
			--end;

		if( start >= end )
			break;

		s32 swap				= arrayToSort[ start ];
		arrayToSort[ start ]	= arrayToSort[ end ];
		arrayToSort[ end ]		= swap;
	}

	quickSort( arrayToSort, start );
	quickSort( arrayToSort + start, length - start );
}

void mergeSort( s32* arrayToSort,
				s32 length )
{
	s32 stride			= 2;
	s32 halfStride		= stride / 2;
	
	s32* arrayBuffer	= new s32[ length ];
	s32* readBuffer		= arrayBuffer;
	s32* writeBuffer	= arrayToSort;
	
	while( halfStride < length )
	{
		//ping pong between buffers
		s32* buffer	= readBuffer;
		readBuffer	= writeBuffer;
		writeBuffer	= buffer;

		for( s32 i = 0; i < length; i += stride )
		{
			s32 indexA = i;
			s32 indexB = i + halfStride;
			
			for( s32 j = 0; j < stride && i + j < length; ++j )
			{
				if( indexA >= i + halfStride )
				{
					writeBuffer[ i + j ] = readBuffer[ indexB++ ];
				}
				else if( indexB >= i + stride || indexB >= length )
				{
					writeBuffer[ i + j ] = readBuffer[ indexA++ ];
				}
				else
				{
					if( readBuffer[ indexA ] <= readBuffer[ indexB ] )
						writeBuffer[ i + j ] = readBuffer[ indexA++ ];
					else
						writeBuffer[ i + j ] = readBuffer[ indexB++ ];
				}
			}
		}

		halfStride	= stride;
		stride		*= 2;
	}

	if( writeBuffer == arrayBuffer )
		memcpy( arrayToSort,
				arrayBuffer,
				length * sizeof( s32 ) );

	delete [] arrayBuffer;
}

void mergeSortInPlace( s32* arrayToSort,
						s32 length )
{
	s32 stride		= 2;
	s32 halfStride	= stride / 2;

	while( halfStride < length )
	{
		for( s32 i = 0; i < length; i += stride )
		{
			s32 indexA = i;
			s32 indexB = i + halfStride;
			
			for( s32 j = 0; j < stride && indexA < indexB && indexB < i + stride && indexB < length; ++j )
			{
				if( arrayToSort[ indexA ] > arrayToSort[ indexB ] )
				{
					s32 valueToSwap = arrayToSort[ indexB ];
					memmove( &arrayToSort[ indexA + 1 ], &arrayToSort[ indexA ], ( indexB - indexA ) * sizeof( s32 ) );
					arrayToSort[ indexA ] = valueToSwap;

					++indexB;
				}

				++indexA;
			}
		}

		halfStride	= stride;
		stride		*= 2;
	}
}

void radixSort( s32* arrayToSort,
				s32 length,
				s8 radixBits )
{
	s32* arrayBuffer		= new s32[ length ];
	s32* readBuffer			= arrayToSort;
	s32* writeBuffer		= arrayBuffer;
	
	ASSERT( radixBits == 8 || radixBits == 16 );
	s32 bitCount			= sizeof( s32 ) * 8;
	s32 groupCount			= bitCount / radixBits;
	u32 bitMask				= ( u32 ) ( ( 1 << radixBits ) - 1 );

	s32 radixOffsetCount	= ( s32 ) ( bitMask + 1 );
	s32* radixOffset		= new s32[ radixOffsetCount ];

	for( s32 i = 0; i < groupCount; ++i )
	{
		memset( radixOffset, 0, radixOffsetCount * sizeof( s32 ) );

		for( s32 j = 0; j < length; ++j )
		{
			s32 index = readBuffer[ j ];

			//flip sign bits during last group so negative numbers appear before positive numbers
			if( i == groupCount - 1 )
				index ^= 1 << ( bitCount - 1 );

			index = ( index & bitMask ) >> ( i * radixBits );
			++radixOffset[ index ];
		}

		//convert counts to offsets
		s32 offset = 0;
		for( s32 j = 0; j < radixOffsetCount; ++j )
		{
			s32 thisOffset		= radixOffset[ j ];
			radixOffset[ j ]	= offset;
			offset				+= thisOffset;
		}

		for( s32 j = 0; j < length; ++j )
		{
			s32 value = readBuffer[ j ];
			s32 index = value;

			//flip sign bits during last group so negative numbers appear before positive numbers
			if( i == groupCount - 1 )
				index ^= 1 << ( bitCount - 1 );

			index = ( index & bitMask ) >> ( i * radixBits );
			writeBuffer[ radixOffset[ index ] ] = value;
			++radixOffset[ index ];
		}
		
		bitMask <<= radixBits;

		//swap buffers
		if( readBuffer == arrayToSort )
		{
			readBuffer	= arrayBuffer;
			writeBuffer	= arrayToSort;
		}
		else
		{
			readBuffer	= arrayToSort;
			writeBuffer	= arrayBuffer;
		}
	}

	delete [] radixOffset;

	delete [] arrayBuffer;
}

//doesn't change arrays
void validateSortResult( s32* originalArray,
							s32* sortedArray,
							s32 length )
{
	for( s32 i = 0; i < length - 1; ++i )
		ASSERT( sortedArray[ i ] <= sortedArray[ i + 1 ] );

	s32* originalArrayCopy	= new s32[ length ];
	memcpy( originalArrayCopy,
			originalArray,
			length * sizeof( s32 ) );

	s32 checkCount = length;
	for( s32 i = 0;
			i < length;
			++i )
	{
		s32 currCheckCount = checkCount;
		for( s32 j = 0;
				j < checkCount;
				++j )
		{
			if( sortedArray[ i ] == originalArrayCopy[ j ] )
			{
				originalArrayCopy[ j ] = originalArrayCopy[ checkCount - 1 ];
				--checkCount;
				break;
			}
		}

		ASSERT( checkCount == currCheckCount - 1 );
	}

	delete [] originalArrayCopy;

	return;
}

int main( int argc,
			char** argv )
{
	LARGE_INTEGER wallClock;
	QueryPerformanceCounter( &wallClock );
	srand( ( s32 ) wallClock.QuadPart );

	s32* originalAscendingItemArray		= new s32[ ITEM_COUNT ];
	s32* originalDescendingItemArray	= new s32[ ITEM_COUNT ];
	s32* originalRandomItemArray		= new s32[ ITEM_COUNT ];

	for( s32 i = 0;
			i < ITEM_COUNT;
			++i )
	{
		originalAscendingItemArray[ i ]		= i;
		originalDescendingItemArray[ i ]	= ITEM_COUNT - i - 1;
		originalRandomItemArray[ i ]		= rand();
	}

	s32* sortedAscendingItemArray		= new s32[ ITEM_COUNT ];
	s32* sortedDescendingItemArray		= new s32[ ITEM_COUNT ];
	s32* sortedRandomItemArray			= new s32[ ITEM_COUNT ];

	f32 bubbleDescendingTimeMs			= 0.0f;
	f32 bubbleRandomTimeMs				= 0.0f;
	f32 bubbleAscendingTimeMs			= 0.0f;
	f32 insertionAscendingTimeMs		= 0.0f;
	f32 insertionDescendingTimeMs		= 0.0f;
	f32 insertionRandomTimeMs			= 0.0f;
	f32 quickSortAscendingTimeMs		= 0.0f;
	f32 quickSortDescendingTimeMs		= 0.0f;
	f32 quickSortRandomTimeMs			= 0.0f;
	f32 mergeAscendingTimeMs			= 0.0f;
	f32 mergeDescendingTimeMs			= 0.0f;
	f32 mergeRandomTimeMs				= 0.0f;
	f32 mergeInPlaceAscendingTimeMs		= 0.0f;
	f32 mergeInPlaceDescendingTimeMs	= 0.0f;
	f32 mergeInPlaceRandomTimeMs		= 0.0f;
	f32 radix8AscendingTimeMs			= 0.0f;
	f32 radix8DescendingTimeMs			= 0.0f;
	f32 radix8RandomTimeMs				= 0.0f;
	f32 radix16AscendingTimeMs			= 0.0f;
	f32 radix16DescendingTimeMs			= 0.0f;
	f32 radix16RandomTimeMs				= 0.0f;

	f32 timeMs;

	for( s32 i = 0;
			i < PASS_COUNT;
			++i )
	{
		if( BUBBLE_SORT_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				bubbleSort( sortedAscendingItemArray,
							ITEM_COUNT );
			}
			bubbleAscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				bubbleSort( sortedDescendingItemArray,
							ITEM_COUNT );
			}
			bubbleDescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				bubbleSort( sortedRandomItemArray,
							ITEM_COUNT );
			}
			bubbleRandomTimeMs += timeMs;

			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}

		if( INSERTION_SORT_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				insertionSort( sortedAscendingItemArray,
							   ITEM_COUNT );
			}
			insertionAscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				insertionSort( sortedDescendingItemArray,
							   ITEM_COUNT );
			}
			insertionDescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				insertionSort( sortedRandomItemArray,
							   ITEM_COUNT );
			}
			insertionRandomTimeMs += timeMs;
			
			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}

		if( QUICKSORT_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				quickSort( sortedAscendingItemArray,
							ITEM_COUNT );
			}
			quickSortAscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				quickSort( sortedDescendingItemArray,
							ITEM_COUNT );
			}
			quickSortDescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				quickSort( sortedRandomItemArray,
							ITEM_COUNT );
			}
			quickSortRandomTimeMs += timeMs;

			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}

		if( MERGE_SORT_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				mergeSort( sortedAscendingItemArray,
							ITEM_COUNT );
			}
			mergeAscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				mergeSort( sortedDescendingItemArray,
							ITEM_COUNT );
			}
			mergeDescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				mergeSort( sortedRandomItemArray,
							ITEM_COUNT );
			}
			mergeRandomTimeMs += timeMs;

			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}

		if( MERGE_SORT_IN_PLACE_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				mergeSortInPlace( sortedAscendingItemArray,
									ITEM_COUNT );
			}
			mergeInPlaceAscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				mergeSortInPlace( sortedDescendingItemArray,
									ITEM_COUNT );
			}
			mergeInPlaceDescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				mergeSortInPlace( sortedRandomItemArray,
									ITEM_COUNT );
			}
			mergeInPlaceRandomTimeMs += timeMs;

			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}

		if( RADIX_8_SORT_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				radixSort( sortedAscendingItemArray,
							ITEM_COUNT,
							8 );
			}
			radix8AscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				radixSort( sortedDescendingItemArray,
							ITEM_COUNT,
							8 );
			}
			radix8DescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				radixSort( sortedRandomItemArray,
							ITEM_COUNT,
							8 );
			}
			radix8RandomTimeMs += timeMs;

			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}

		if( RADIX_16_SORT_ENABLED )
		{
			memcpy( sortedAscendingItemArray,
					originalAscendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedDescendingItemArray,
					originalDescendingItemArray,
					ITEM_COUNT * sizeof( s32 ) );
			memcpy( sortedRandomItemArray,
					originalRandomItemArray,
					ITEM_COUNT * sizeof( s32 ) );

			{
				TIMED_BLOCK( &timeMs );
				radixSort( sortedAscendingItemArray,
							ITEM_COUNT,
							16 );
			}
			radix16AscendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				radixSort( sortedDescendingItemArray,
							ITEM_COUNT,
							16 );
			}
			radix16DescendingTimeMs += timeMs;

			{
				TIMED_BLOCK( &timeMs );
				radixSort( sortedRandomItemArray,
							ITEM_COUNT,
							16 );
			}
			radix16RandomTimeMs += timeMs;

			if( VALIDATE )
			{
				validateSortResult( originalAscendingItemArray,
									sortedAscendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalDescendingItemArray,
									sortedDescendingItemArray,
									ITEM_COUNT );
				validateSortResult( originalRandomItemArray,
									sortedRandomItemArray,
									ITEM_COUNT );
			}
		}
	}

	f32 avgAscendingTimeMs;
	f32 avgDescendingTimeMs;
	f32 avgRandomTimeMs;

	if( BUBBLE_SORT_ENABLED )
	{
		avgAscendingTimeMs	= bubbleAscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= bubbleDescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= bubbleRandomTimeMs	/ ( f32 ) PASS_COUNT;

		printf( "bubble sort:%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	if( INSERTION_SORT_ENABLED )
	{
		avgAscendingTimeMs	= insertionAscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= insertionDescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= insertionRandomTimeMs		/ ( f32 ) PASS_COUNT;

		printf( "insertion sort:%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	if( QUICKSORT_ENABLED )
	{
		avgAscendingTimeMs	= quickSortAscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= quickSortDescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= quickSortRandomTimeMs		/ ( f32 ) PASS_COUNT;

		printf( "quicksort:%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	if( MERGE_SORT_ENABLED )
	{
		avgAscendingTimeMs	= mergeAscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= mergeDescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= mergeRandomTimeMs		/ ( f32 ) PASS_COUNT;

		printf( "merge sort:%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	if( MERGE_SORT_IN_PLACE_ENABLED )
	{
		avgAscendingTimeMs	= mergeInPlaceAscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= mergeInPlaceDescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= mergeInPlaceRandomTimeMs		/ ( f32 ) PASS_COUNT;

		printf( "merge sort in place:%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	if( RADIX_8_SORT_ENABLED )
	{
		avgAscendingTimeMs	= radix8AscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= radix8DescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= radix8RandomTimeMs	/ ( f32 ) PASS_COUNT;

		printf( "radix sort (8 bit):%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	if( RADIX_16_SORT_ENABLED )
	{
		avgAscendingTimeMs	= radix16AscendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgDescendingTimeMs	= radix16DescendingTimeMs	/ ( f32 ) PASS_COUNT;
		avgRandomTimeMs		= radix16RandomTimeMs		/ ( f32 ) PASS_COUNT;

		printf( "radix sort (16 bit):%.4fms, %.4fms, %.4fms\n",
				avgAscendingTimeMs,
				avgDescendingTimeMs,
				avgRandomTimeMs );
	}

	system( "pause" );
	return 0;
}
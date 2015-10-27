
#include "stdafx.h"
#include "main.h"
#include "renderbuffer.h"
#include "mainboard.h"


#define			CONVERT_5BIT8BIT( x )			(Byte_t)( ((float)(x)) / ((float)0x1F) * ((float)0xFF) )
#define			CONVERT_6BIT8BIT( x )			(Byte_t)( ((float)(x)) / ((float)0x3F) * ((float)0xFF) )



static const int RenderBufferWidth = 256;
static const int RenderBufferHeight = 240;


NtscFilter::NtscFilter()
	: mSetup( nes_ntsc_composite )
{
	mBurstPhase = 0;
	mNtscBuffer = new nes_ntsc_t;

	ApplySetupParams();
}


void NtscFilter::ApplySetupParams()
{
	nes_ntsc_init( mNtscBuffer, &mSetup );
}


NtscFilter::~NtscFilter()
{
	delete mNtscBuffer;
	mNtscBuffer = NULL;
}


void NtscFilter::Filter( const Byte_t* paletteBufferIn, Byte_t* outRgbBuffer )
{
	mBurstPhase ^= 1;
	if ( mSetup.merge_fields )
		mBurstPhase = 0;
	
	nes_ntsc_blit( mNtscBuffer, paletteBufferIn, RenderBufferWidth, mBurstPhase, RenderBufferWidth, RenderBufferHeight,
		outRgbBuffer, NES_NTSC_OUT_WIDTH( RenderBufferWidth )*4 );
}


void NtscFilter::GetBufferSize( int& x, int& y ) const
{
	x = NES_NTSC_OUT_WIDTH( 256 );
	y = 480;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


const Byte_t defaultPalette[] = 
{
0x80,0x80,0x80, 0x00,0x3D,0xA6, 0x00,0x12,0xB0, 0x44,0x00,0x96,
0xA1,0x00,0x5E, 0xC7,0x00,0x28, 0xBA,0x06,0x00, 0x8C,0x17,0x00,
0x5C,0x2F,0x00, 0x10,0x45,0x00, 0x05,0x4A,0x00, 0x00,0x47,0x2E,
0x00,0x41,0x66, 0x00,0x00,0x00, 0x05,0x05,0x05, 0x05,0x05,0x05,
0xC7,0xC7,0xC7, 0x00,0x77,0xFF, 0x21,0x55,0xFF, 0x82,0x37,0xFA,
0xEB,0x2F,0xB5, 0xFF,0x29,0x50, 0xFF,0x22,0x00, 0xD6,0x32,0x00,
0xC4,0x62,0x00, 0x35,0x80,0x00, 0x05,0x8F,0x00, 0x00,0x8A,0x55,
0x00,0x99,0xCC, 0x21,0x21,0x21, 0x09,0x09,0x09, 0x09,0x09,0x09,
0xFF,0xFF,0xFF, 0x0F,0xD7,0xFF, 0x69,0xA2,0xFF, 0xD4,0x80,0xFF,
0xFF,0x45,0xF3, 0xFF,0x61,0x8B, 0xFF,0x88,0x33, 0xFF,0x9C,0x12,
0xFA,0xBC,0x20, 0x9F,0xE3,0x0E, 0x2B,0xF0,0x35, 0x0C,0xF0,0xA4,
0x05,0xFB,0xFF, 0x5E,0x5E,0x5E, 0x0D,0x0D,0x0D, 0x0D,0x0D,0x0D,
0xFF,0xFF,0xFF, 0xA6,0xFC,0xFF, 0xB3,0xEC,0xFF, 0xDA,0xAB,0xEB,
0xFF,0xA8,0xF9, 0xFF,0xAB,0xB3, 0xFF,0xD2,0xB0, 0xFF,0xEF,0xA6,
0xFF,0xF7,0x9C, 0xD7,0xE8,0x95, 0xA6,0xED,0xAF, 0xA2,0xF2,0xDA,
0x99,0xFF,0xFC, 0xDD,0xDD,0xDD, 0x11,0x11,0x11, 0x11,0x11,0x11
};


RenderBuffer::RenderBuffer( )
{
	priorityBuffer = new boost::uint32_t[ RenderBufferWidth * RenderBufferHeight ];
	memset( priorityBuffer, 0, RenderBufferWidth * RenderBufferHeight * sizeof( boost::uint32_t ) );

	renderBuffer = new Byte_t[ RenderBufferWidth * RenderBufferHeight ];
	memset( renderBuffer, 0x0, RenderBufferWidth * RenderBufferHeight );

	rgbBuffer = new Byte_t[ NES_NTSC_OUT_WIDTH( 256 ) * 480 * 4 ];
	mTempRgbBuffer = new Byte_t[ NES_NTSC_OUT_WIDTH( 256 ) * 480 * 4 ];
	mNtscFilter = new NtscFilter;
}


RenderBuffer::~RenderBuffer()
{
	delete[] priorityBuffer;
	priorityBuffer = NULL;

	delete[] renderBuffer;
	renderBuffer = NULL;

	delete[] rgbBuffer;
	rgbBuffer = NULL;

	delete[] mTempRgbBuffer;
	mTempRgbBuffer = NULL;

	delete mNtscFilter;
	mNtscFilter = NULL;
}


void RenderBuffer::GetPaletteColour( Byte_t index, Byte_t& r, Byte_t& g, Byte_t& b ) const
{
	if ( index < 64 )
	{
		r = defaultPalette[ index * 3 + 0 ];
		g = defaultPalette[ index * 3 + 1 ];
		b = defaultPalette[ index * 3 + 2 ];
	}
	else
		r = g = b = 0;
}


void RenderBuffer::ClearBuffer( Byte_t backgroundPaletteIndex )
{ // called at the end of each frame
	memset( priorityBuffer, 0, RenderBufferHeight * RenderBufferWidth * sizeof( boost::uint32_t ) );
	memset( renderBuffer, backgroundPaletteIndex, RenderBufferHeight * RenderBufferWidth );
}


const boost::uint32_t* RenderBuffer::GetPriorityBuffer( ) const
{
	return priorityBuffer;
}


const Byte_t* RenderBuffer::GetPaletteBuffer( ) const
{
	return renderBuffer;
}


void RenderBuffer::DoubleOutputHeight()
{
	int output_pitch = NES_NTSC_OUT_WIDTH( 256 ) * 3;
	int y;
	for ( y = 480 / 2; --y >= 0; )
	{
		unsigned char const* in = rgbBuffer + y * output_pitch;
		unsigned char* out = rgbBuffer + y * 2 * output_pitch;
		int n;
		for ( n = NES_NTSC_OUT_WIDTH( 256 ); n; --n )
		{
			Byte_t prevR, prevG, prevB, nextR, nextG, nextB;

			prevR = in[ n * 3 + 0 ];
			prevG = in[ n * 3 + 1 ];
			prevB = in[ n * 3 + 2 ];
			nextR = in[ n * 3 + 0 + output_pitch ];
			nextG = in[ n * 3 + 1 + output_pitch ];
			nextB = in[ n * 3 + 2 + output_pitch ];

			Byte_t mixR, mixG, mixB;
			mixR = prevR + nextR + (( prevR ^ nextR ));
			mixG = prevG + nextG + (( prevG ^ nextG ));
			mixB = prevB + nextB + (( prevB ^ nextB ));

			out[ n * 3 + 0 ] = prevR;
			out[ n * 3 + 1 ] = prevG;
			out[ n * 3 + 2 ] = prevB;

			out[ n * 3 + 0 + output_pitch ] = prevR;
			out[ n * 3 + 1 + output_pitch ] = prevG;
			out[ n * 3 + 2 + output_pitch ] = prevB;
		}
	}
}


const Byte_t* RenderBuffer::GetRGBBuffer( bool upsideDown )
{
	if ( g_options->UseNtscFiltering )
	{
		int width = 0, height = 0;
		mNtscFilter->GetBufferSize( width, height );
		int stride = NES_NTSC_OUT_WIDTH( 256 ) * 4;

//		mNtscFilter->Filter( renderBuffer, mTempRgbBuffer );

		for ( int y=0; y<height/2; ++y )
		{
			int baseIndex = upsideDown ? ((height/2)-1-y)*width : y*width;

			for ( int x=0; x<width; ++x )
			{
				int index = ( baseIndex + x ) * 3;
				int tempIndex = y*stride + x * 4;

				// swap BGR -> RGB
				rgbBuffer[ index + 0 ] = mTempRgbBuffer[ tempIndex + 2 ];
				rgbBuffer[ index + 1 ] = mTempRgbBuffer[ tempIndex + 1 ];
				rgbBuffer[ index + 2 ] = mTempRgbBuffer[ tempIndex + 0 ];
			}
		}

		DoubleOutputHeight();
	}
	else
	{
		for ( int y=0; y<RenderBufferHeight; ++y )
		{
			int baseIndex = upsideDown ? (RenderBufferHeight-1-y)*RenderBufferWidth : y*RenderBufferWidth;

			for ( int x=0; x<RenderBufferWidth; ++x )
			{
				int index = ( baseIndex + x ) * 3;
				int paletteIndex = renderBuffer[ (y*RenderBufferWidth) + x ] * 3;

				for ( int p=0; p<3; ++p )
					rgbBuffer[ index + p ] = paletteIndex < ( 64 * 3 ) ? defaultPalette[ paletteIndex + p ] : 0;
			}
		}
	}

	return rgbBuffer;
}


bool RenderBuffer::RenderPixel( boost::uint16_t priority, boost::uint16_t spritenum, int x, int y, Byte_t paletteIndex )
{
	if ( x < 0 || y < 0 || x > RenderBufferWidth || y >= RenderBufferHeight )
		throw std::exception( ( boost::format( "Invalid renderpixel call X: %1% Y: %2%" ) % x % y ).str().c_str() );

	boost::uint32_t oldPriority = priorityBuffer[ x + (y*RenderBufferWidth) ];

	bool hitzero = false;
	bool dontrender = false;

	// check if a sprite was written to the pixel first
	if ( ( oldPriority & ( RENDER_SPRITE | RENDER_SPRITE_EXTRA ) ) > 0 )
	{
		// replacing sprite with background
		if ( ( priority & RENDER_BACKGROUND ) > 0 )
		{
			if ( ( oldPriority & ( RENDER_SPRITE_ZERO ) ) > 0 && x < (RenderBufferWidth-1) )
				hitzero = true;

			if ( ( oldPriority & RENDER_SPRITE_BEHIND ) == 0 )
				dontrender = true; // dont render background over sprite
		}
		else if ( ( priority & RENDER_BACKGROUND_TRANSPARENT ) > 0 )
			dontrender = true; // attempting to write transparent bg on sprites
		
		if ( ( priority & ( RENDER_SPRITE | RENDER_SPRITE_EXTRA ) ) > 0 )
		{ // replacing sprite with sprite
			// dont render new sprite over old sprite
			dontrender = true;
		}
	}

	priorityBuffer[ x + (y*RenderBufferWidth) ] |= priority | ( spritenum << 16 );

	if ( dontrender || ( oldPriority & RENDER_DEBUG_1 ) > 0 )
		return hitzero;

	if ( g_nesMainboard->GetModeConstants()->Mode() == TERRITORY_NTSC && ( y < 8 || y > 231 ) )
		return hitzero;

	renderBuffer[ x + (y*RenderBufferWidth) ] = paletteIndex;

	return hitzero;
}


void RenderBuffer::GetBufferSize( int& x, int& y ) const
{
	if ( mNtscFilter != NULL && g_options->UseNtscFiltering )
	{
		mNtscFilter->GetBufferSize( x, y );
	}
	else
	{
		x = 256;
		y = 240;
	}
}



#ifndef RENDERBUFFER_H
#define RENDERBUFFER_H

#include "main.h"
#include "irenderbuffer.h"
#include "ntsc/nes_ntsc.h"

// RenderBuffer class handles rendering pixels and putting them in a format OpenGL understands

enum RENDER_PRIORITY
{
	RENDER_TRANSPARENT = 0,
	RENDER_BACKGROUND_TRANSPARENT = 1,
	RENDER_BACKGROUND = 2,
	RENDER_SPRITE = 4,
	RENDER_SPRITE_BEHIND = 8,
	RENDER_SPRITE_ZERO = 16,
	RENDER_SPRITE_EXTRA = 32,
	RENDER_DEBUG_1 = 64,
	RENDER_DEBUG_2 = 128
};


class NtscFilter
{
	nes_ntsc_t* mNtscBuffer;
	nes_ntsc_setup_t mSetup;
	int mBurstPhase;

public:
	NtscFilter();
	virtual ~NtscFilter();

	void ApplySetupParams();

	void Filter( const Byte_t* paletteBufferIn, Byte_t* outRgbBuffer );

	void GetBufferSize( int& x, int& y ) const;

};


class RenderBuffer : public IRenderBuffer
{
private:
	boost::uint32_t* priorityBuffer;
	Byte_t* renderBuffer;
	Byte_t* rgbBuffer, *mTempRgbBuffer;
	NtscFilter *mNtscFilter;

	void DoubleOutputHeight();

public:
	RenderBuffer( );
	~RenderBuffer();

	virtual void ClearBuffer( Byte_t backgroundPaletteIndex );

	bool RenderPixel( boost::uint16_t priority, boost::uint16_t spritenum, int x, int y, Byte_t paletteIndex );

	const boost::uint32_t* GetPriorityBuffer( ) const;
	virtual const Byte_t* GetRGBBuffer( bool upsideDown );
	const Byte_t* GetPaletteBuffer( ) const;

	virtual void GetBufferSize( int& x, int& y ) const;

	void GetPaletteColour( Byte_t index, Byte_t& r, Byte_t& g, Byte_t& b ) const;
};


#endif


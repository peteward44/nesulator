

#ifndef MAIN_H
#define MAIN_H

#include <cstdio>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <cassert>
#include <string>
#include <iostream>

#include "boost/shared_ptr.hpp"
#include "boost/utility.hpp"
#include "boost/cstdint.hpp"
#include "boost/signals2.hpp"
#include <boost/bind/bind.hpp>

#include "log.h"
#include "wx/log.h"

#define LOG_PROCESSOR_INSTRUCTIONS
#define LOG_PPU_EVENTS
#define LOG_MAPPER_EVENTS
//#define NESTEST

#define RENDER_UPSIDEDOWN

//#ifdef _DEBUG
#define ENABLE_LOGGING
//#endif

#define RENDER_DEBUG_LAYER
#define RENDER_SPRITEHIT

#define MAKE_WORD( low, high )		( ((low) & 0xFF) | ( ((high) & 0xFF) << 8 ) )

#ifdef _WINDOWS
#define FORCE_INLINE __forceinline
#else
#define FORCE_INLINE inline
#endif

#define IS_INT_BETWEEN( i, low, high )		( (low) <= (i) && (i) < (high) )
#define IS_BIT_SET( b, offset )			( ((b) & ( 1 << ( offset ) )) > 0 )
#define SET_BIT( b, offset )			( (b) |= ( 1 << ( offset ) ) )
#define CLEAR_BIT( b, offset )			( (b) &= ~( 1 << ( offset ) ) )

typedef unsigned char Byte_t;
typedef unsigned char UInt8_t;
typedef unsigned short UInt16_t;
typedef unsigned int UInt32_t;

extern const wchar_t* APP_NAME;
extern const wchar_t* APP_VERSION_STRING;

const int PPU_TICKS_PER_SCANLINE = 341;
const int MASTER_CYCLES_PER_PPU = 5;
const int CPU_CYCLES_USED_FOR_SPRITE_DMA = 513;
const int SOUND_RATE = 44100;

/*
Although the SNES NTSC and PAL versions operate on different power supply frequencies, there is only a small difference between the clock frequency of NTSC and PAL units.
The CPU and PPU use the same clock. One CPU machine cycle corresponds to; 6 clock cycles (high speed), 8 clock cycles (middle speed), and 12 clock cycles (low speed).
The vertical display timing is the same in both units, except that the period for vertical blanking is longer for PAL. There is no difference in sound processing between the two units.

Aside from the following chart, NTSC display at 60fps, PAL displays at 50fps.

NTSC non-interlaced
===================
Clock Frequency:               21.47727 MHz
# of clock cycles for 1 dot:   1 (mode 1,2,3,4,7); 2 (mode 4,5)
# of clock cycles for 1 line:  1364 (341x4 = 1364)
# of scanlines (1st field):    262
# of scanlines (2nd field):    262
Horizontal display time:       256 dots (512 dots in mode 5,6)
Vertical display time:         224 lines

NTSC interlaced
===============
Clock Frequency:               Identical to NTSC non-interlaced
# of clock cycles for 1 dot:   Identical to NTSC non-interlaced
# of clock cycles for 1 line:  Identical to NTSC non-interlaced
# of scanlines (1st field):    Identical to NTSC non-interlaced
# of scanlines (2nd field):    263
Horizontal display time:       Identical to NTSC non-interlaced
Vertical display time:         239 lines

PAL non-interlaced
==================
Clock Frequency:               21.28137 MHz
# of clock cycles for 1 dot:   Identical to NTSC non-interlaced
# of clock cycles for 1 line:  Identical to NTSC non-interlaced
# of scanlines (1st field):    312
# of scanlines (2nd field):    312
Horizontal display time:       Identical to NTSC non-interlaced
Vertical display time:         Identical to NTSC non-interlaced

PAL interlaced
==============
Clock Frequency:               Identical to PAL non-interlaced
# of clock cycles for 1 dot:   Identical to PAL non-interlaced
# of clock cycles for 1 line:  Identical to PAL non-interlaced
# of scanlines (1st field):    Identical to PAL non-interlaced
# of scanlines (2nd field):    313
Horizontal display time:       Identical to PAL non-interlaced
Vertical display time:         Identical to NTSC interlaced

The SNES's CPU is a 65c816 based processor. While its clock speed is at about 21 MHz, it's effective speed is considerably lower, at 3.58 MHz for quick access
(i.e. hardware registers at $2100-$21FF in banks $00-$3F), 2.68 MHz for slow access (i.e. ROM and RAM) and 1.79 MHz for very slow access
(i.e. hardware registers at $4000-$41FFF in banks $00 through $3F). 

if you have the following instruction running in FastROM:
lda $12 ; assume accum 8-bit.

Fetch 'lda' would be 6 MCs
Fetch '$12' would be 6 MCs
Fetch accumulator value would be 8 MCs.

Quote:
In addition to the 6/8 master cycles, isn't there something which takes 12 master cycles?? writing to certain PPU registers or something?]

yeah, in addition to the ROM speed mapping, the snes apportions the following:
W-RAM : (8 MCs)
PPU-1 & 2: (6 MCs)
NES Joyport/IO Area ($4000 ~ $41ff) (12 MCs)
other areas (8 MCs)

Ah, my apologies for not making that clearer!! Yes, your right: if the Direct page register was $2100 (3.58MHz region), and the code was runnin g in FastROM,then
lda $00 ; [$00:2100]
6 + 6 + 6 (fetch) = 18 Machine Cycles

if the Direct page was $0000 (W-RAM -> 2.68MHz), then:
lda $00 ; [$00:0000]
6 + 6 + 8 (fetch) = 20 Machine Cycles


*/

const int SNES_MASTER_TICKS_PER_CPU_IO = 6;
const int SNES_MASTER_TICKS_PER_MEM_ACCESS_VFAST = 6;
const int SNES_MASTER_TICKS_PER_MEM_ACCESS_FAST = 8;
const int SNES_MASTER_TICKS_PER_MEM_ACCESS_SLOW = 12;

const int SNES_MASTER_TICKS_PER_SCANLINE = 1364;
const int SNES_MASTER_TICKS_PER_SCANLINE_EVERY_OTHER_FRAME_NONINTERLACE = 1360;

const int SNES_CPU_FRAME_PAUSE_MASTER_TICKS_FROM_START_SCANLINE = 536; // CPU pauses itself every scanline
const int SNES_CPU_FRAME_PAUSE_MASTER_TICKS_PERIOD = 40;
const int SNES_MASTER_CYCLES_PER_PPU = 4;

const int SOUND_VOLUME_MIN = 0;
const int SOUND_VOLUME_MAX = 100;

const bool SOUND_ENABLED_BY_DEFAULT = true;

// Defines the differences between PAL and NTSC modes

enum TERRITORY_MODE
{
	TERRITORY_NTSC,
	TERRITORY_PAL
};

extern unsigned long CalculateCRC32( unsigned char *data, unsigned long datalen );
extern unsigned long CalculateCRC32( const std::wstring& filename );


class HasState
{
public:
	virtual ~HasState() {}

	virtual void SaveState( std::ostream& ostr ) = 0;
	virtual void LoadState( std::istream& istr ) = 0;

};


template < int SIZE, Byte_t INITIAL_VALUE >
struct Page : public HasState
{
	const int Size_t;

	Byte_t* Data;

	FORCE_INLINE Page() : Size_t( SIZE )
	{
		Data = new Byte_t[ SIZE ];
		memset( Data, INITIAL_VALUE, SIZE );
	}

	FORCE_INLINE ~Page()
	{
		delete[] Data;
	}

	virtual void LoadState( std::istream& istr ) { istr.read( reinterpret_cast< char* >( Data ), SIZE ); }
	virtual void SaveState( std::ostream& ostr ) { ostr.write( reinterpret_cast< const char* >( Data ), SIZE ); }

};


template <typename type>
FORCE_INLINE void WriteType( std::ostream& ostr, const type& t)
{
	ostr.write( reinterpret_cast< const char* >( boost::addressof( t ) ), sizeof( type ) );
}


template <typename type>
FORCE_INLINE type ReadType( std::istream& istr )
{
	type t;
	istr.read( reinterpret_cast< char* >( boost::addressof( t ) ), sizeof( type ) );
	return t;
}



#endif // #ifndef MAIN_H

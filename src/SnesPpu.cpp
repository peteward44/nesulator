
#include "stdafx.h"
#include "main.h"
#include "SnesPpu.h"
#include <cassert>


UInt16_t SnesPpu::GetVramAddress() const
{
	UInt16_t addr = mVramAddress;
	switch( mVramAddressFormation )
	{
		// copied from bsnes
		//mere mortals dare not try and understand the below bit blender ...
		case 0: break;
		case 1: addr = (addr & 0xff00) | ((addr & 0x001f) << 3) | ((addr >> 5) & 7); break;
		case 2: addr = (addr & 0xfe00) | ((addr & 0x003f) << 3) | ((addr >> 6) & 7); break;
		case 3: addr = (addr & 0xfc00) | ((addr & 0x007f) << 3) | ((addr >> 7) & 7); break;
	}
	return (addr << 1);
}



void SnesPpu::WriteCgRam( UInt16_t address, Byte_t data )
{
}


bool SnesPpu::IsInVBlank() const
{
	return false; // TODO:
}


bool SnesPpu::IsInHBlank() const
{
	return false; // TODO:
}


bool SnesPpu::IsJoypadReady() const
{
	return false; // TODO:
}




SnesPpu::SnesPpu()
{
	mBrightness = 0xF;
	mScreenEnabled = true;

	mSpritesEnabled = true;
	mSpriteSize = 0;
	mSpriteNameSelection = 0;
	mSpriteBaseSelection = 0;
	mSpriteDataAddress = 0;
	mSpritePriorityActivated = false;
	mFastCycleModeActive = false;

	mBackgroundMode = 0;
	mBackground3Priority = false;

	mVramAddressIncrementMode = false;
	mVramAddressFormation = 0;
	mVramAddressIncrementSize = 1;
	mVramAddress = 0;
	mVramReadBuffer = 0;

	mCgAddress = 0;
	mCgLatchData = 0;

	g_snesMainboard->GetSynchroniser()->RegisterListener( SNES_SYNC_PPU, this );
}


SnesPpu::~SnesPpu()
{
}



void SnesPpu::WriteToVram( UInt16_t address, Byte_t data )
{
  //if(regs.display_disabled == true) {
  //  memory::vram[addr] = data;
  //} else {
  //  uint16 v = cpu.vcounter();
  //  uint16 h = cpu.hcounter();
  //  if(v == 0) {
  //    if(h <= 4) {
  //      memory::vram[addr] = data;
  //    } else if(h == 6) {
  //      memory::vram[addr] = cpu.regs.mdr;
  //    } else {
  //      //no write
  //    }
  //  } else if(v < (!overscan() ? 225 : 240)) {
  //    //no write
  //  } else if(v == (!overscan() ? 225 : 240)) {
  //    if(h <= 4) {
  //      //no write
  //    } else {
  //      memory::vram[addr] = data;
  //    }
  //  } else {
  //    memory::vram[addr] = data;
  //  }
  //}

//	assert( address >= 0 && address < mVram::Size_t );
//	mVram.Data[ address ] = data;
}


Byte_t SnesPpu::ReadVram( UInt16_t address )
{
  //uint8 data;

  //if(regs.display_disabled == true) {
  //  data = memory::vram[addr];
  //} else {
  //  uint16 v = cpu.vcounter();
  //  uint16 h = cpu.hcounter();
  //  uint16 ls = ((system.region() == System::Region::NTSC ? 525 : 625) >> 1) - 1;
  //  if(interlace() && !cpu.field()) ls++;

  //  if(v == ls && h == 1362) {
  //    data = 0x00;
  //  } else if(v < (!overscan() ? 224 : 239)) {
  //    data = 0x00;
  //  } else if(v == (!overscan() ? 224 : 239)) {
  //    if(h == 1362) {
  //      data = memory::vram[addr];
  //    } else {
  //      data = 0x00;
  //    }
  //  } else {
  //    data = memory::vram[addr];
  //  }
  //}

  //return data;

//	assert( address >= 0 && address < mVram::Size_t );
//	return mVram.Data[ address ];
	return 0;
}


UInt8_t SnesPpu::Read8( UInt8_t bankRegister, UInt16_t address )
{
	UInt8_t val = 0;
	
	PWLOG1( LOG_MISC, "PPU read %1$04X", address );

	return val;
}


//====================================================================================================================================================
//====================================================================================================================================================
//====================================================================================================================================================


void SnesPpu::Write8( UInt8_t bankRegister, UInt16_t address, UInt8_t value )
{
	PWLOG1( LOG_MISC, "PPU write %1$04X", address );

	switch ( address )
	{
/*
|----------------------------------------------------------------------------|
|rwd2?|Address|Title & Explanation                                           |
|----------------------------------------------------------------------------|
| w   |$2100  |Screen display register [INIDISP]                             |
|     |       |x000bbbb              x: 0 = Screen on.                       |
|     |       |                         1 = Screen off.                      |
|     |       |                   bbbb: Brightness ($0-$F).                  |
*/
	case 0x2100:
		mScreenEnabled = ( value & 0x80 ) > 0;
		mBrightness = ( value & 0xF );
	break;

/*
| w   |$2101  |OAM size register [OBSEL]                                     |
|     |       |sssnnbbb              s: 000 =  8x8  or 16x16.                |
|     |       |                         001 =  8x8  or 32x32.                |
|     |       |                         010 =  8x8  or 64x64.                |
|     |       |                         011 = 16x16 or 32x32.                |
|     |       |                         100 = 16x16 or 64x64.                |
|     |       |                         101 = 32x32 or 64x64.                |
|     |       |                      n: Name selection (upper 4k word addr). |
|     |       |                      b: Base selection (8k word seg. addr).  |
*/
	//NOTE: OAM accesses during active display are rerouted to 0x0218 ... this can be considered
//a hack. The actual address varies during rendering, as the S-PPU reads in data itself for
//processing. Unfortunately, we have yet to determine how this works. The algorithm cannot be
//reverse engineered using a scanline renderer such as this, and at this time, there does not
//exist a more accurate SNES PPU emulator to work from. The only known game to actually access
//OAM during active display is Uniracers. It expects accesses to map to offset 0x0218.
//It was decided by public consensus to map writes to this address to match Uniracers, primarily
//because it is the only game observed to do this, but also because mapping to this address does
//not contradict any of our findings, because we have no findings whatsoever on this behavior.
//Think of this what you will, I openly admit that this is a hack. But it is more accurate than
//writing to the 'expected' address set by $2102,$2103, and will catch problems in software that
//accidentally accesses OAM during active display by virtue of not returning the expected data.
	case 0x2101: // Sprites
		mSpriteSize = ( value & 0xE0 ) >> 5;
		mSpriteNameSelection = ( value & 0x18 ) >> 3;
		mSpriteBaseSelection = ( value & 0x07 );
	break;

/*
| w 2 |$2102  |OAM address register [OAMADDL/OAMADDH]                        |
|     |       |aaaaaaaa r000000m     a: OAM address.                         |
|     |       |                      r: OAM priority rotation.               |
|     |       |                      m: OAM address MSB.                     |

The internal OAM address is invalidated when scanlines are being
		rendered. This invalidation is deterministic, but we do not know how
		it is determined. Thus, the last value written to these registers is
		reloaded into the internal OAM address at the beginning of V-Blank if
		that occurs outside of a force-blank period. This is known as 'OAM
		reset'. 'OAM reset' also occurs on certain writes to $2100.
*/
	case 0x2102:
		//regs.oam_baseaddr    = (regs.oam_baseaddr & ~0xff) | (data << 0);
		//regs.oam_baseaddr   &= 0x01ff;
		//regs.oam_addr        = regs.oam_baseaddr << 1;
		//regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
		//mSpriteDataAddress &= 0xFF00;
		//mSpriteDataAddress |= value;
	break;
	case 0x2103:
		//regs.oam_priority    = !!(data & 0x80);
		//regs.oam_baseaddr    = (regs.oam_baseaddr &  0xff) | (data << 8);
		//regs.oam_baseaddr   &= 0x01ff;
		//regs.oam_addr        = regs.oam_baseaddr << 1;
		//regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
		mSpriteDataAddress &= 0xFF;
		mSpriteDataAddress |= ( ( value & 0x01 ) << 8 );
		mSpritePriorityActivated = ( value & 0x80 ) > 0;
		break;

/*
| wd  |$2104  |OAM data register [OAMDATA]                                   |
|     |       |???????? ????????                                             |
*/
	case 0x2104:
		//if(regs.oam_addr & 0x0200)
		//{
		//	oam_mmio_write(regs.oam_addr, data);
		//}
		//else if((regs.oam_addr & 1) == 0)
		//{
		//	regs.oam_latchdata = data;
		//}
		//else
		//{
		//	oam_mmio_write((regs.oam_addr & ~1) + 0, regs.oam_latchdata);
		//	oam_mmio_write((regs.oam_addr & ~1) + 1, data);
		//}

		//regs.oam_addr++;
		//regs.oam_addr &= 0x03ff;
		//regs.oam_firstsprite = (regs.oam_priority == false) ? 0 : (regs.oam_addr >> 2) & 127;
	break;

/*
| w   |$2105  |Screen mode register [BGMODE]                                 |
|     |       |abcdefff              a: BG4 tile size (0=8x8, 1=16x16).      |
|     |       |                      b: BG3 tile size (0=8x8, 1=16x16).      |
|     |       |                      c: BG2 tile size (0=8x8, 1=16x16).      |
|     |       |                      d: BG1 tile size (0=8x8, 1=16x16).      |
|     |       |                      e: Highest priority for BG3 in MODE 1.  |
|     |       |                      f: MODE definition.                     |
*/
	case 0x2105:
		mBackgroundInfo[0].mUse16x16Tiles = ( value & 0x10 ) > 0;
		mBackgroundInfo[1].mUse16x16Tiles = ( value & 0x20 ) > 0;
		mBackgroundInfo[2].mUse16x16Tiles = ( value & 0x40 ) > 0;
		mBackgroundInfo[3].mUse16x16Tiles = ( value & 0x80 ) > 0;
		mBackground3Priority = ( value & 0x08 ) > 0;
		mBackgroundMode = value & 7;
	break;

/*
| w   |$2106  |Screen pixelation register [MOSAIC]                           |
|     |       |xxxxabcd              x: Pixel size (0=Smallest, $F=Largest). |
|     |       |                      a: Affect BG4.                          |
|     |       |                      b: Affect BG3.                          |
|     |       |                      c: Affect BG2.                          |
|     |       |                      d: Affect BG1.                          |
*/
	case 0x2106:
		{
			const UInt8_t pixelSize = value & 0xF0;

			if ( ( value & 0x1 ) > 0 )
				mBackgroundInfo[ 0 ].mScreenPixelation = pixelSize;
			if ( ( value & 0x2 ) > 0 )
				mBackgroundInfo[ 1 ].mScreenPixelation = pixelSize;
			if ( ( value & 0x4 ) > 0 )
				mBackgroundInfo[ 2 ].mScreenPixelation = pixelSize;
			if ( ( value & 0x8 ) > 0 )
				mBackgroundInfo[ 3 ].mScreenPixelation = pixelSize;
		}
	break;

/*
| w   |$2107  |BG1 VRAM location register [BG1SC]                            |
|     |       |xxxxxxab              x: Base address                         |
|     |       |                     ab: SC size                              |
*/
	case 0x2107:
		mBackgroundInfo[ 0 ].mTileMapBaseVramAddress = value & 0xFC;
		mBackgroundInfo[ 0 ].mSCSize = static_cast< BackgroundTileSize >( value & 0x3 );
	break;

/*
| w   |$2108  |BG2 VRAM location register [BG2SC] -|                         |
*/
	case 0x2108:
		mBackgroundInfo[ 1 ].mTileMapBaseVramAddress = value & 0xFC;
		mBackgroundInfo[ 1 ].mSCSize = static_cast< BackgroundTileSize >( value & 0x3 );
	break;

/*
| w   |$2109  |BG3 VRAM location register [BG3SC]  |- Same as $2107.         |
*/
	case 0x2109:
		mBackgroundInfo[ 2 ].mTileMapBaseVramAddress = value & 0xFC;
		mBackgroundInfo[ 2 ].mSCSize = static_cast< BackgroundTileSize >( value & 0x3 );
	break;

/*
| w   |$210A  |BG4 VRAM location register [BG4SC] -|                         |
*/
	case 0x210A:
		mBackgroundInfo[ 3 ].mTileMapBaseVramAddress = value & 0xFC;
		mBackgroundInfo[ 3 ].mSCSize = static_cast< BackgroundTileSize >( value & 0x3 );
	break;

/*
| w   |$210B  |BG1 & BG2 VRAM location register [BG12NBA]                    |
|     |       |aaaabbbb              a: Base address for BG2.                |
|     |       |                      b: Base address for BG1.                |
*/
	case 0x210B:
		mBackgroundInfo[ 0 ].mCharacterDataBaseVramAddress = value & 0x0F;
		mBackgroundInfo[ 1 ].mCharacterDataBaseVramAddress = value & 0xF0;
	break;

/*
| w   |$210C  |BG3 & BG4 VRAM location register [BG34NBA]                    |
|     |       |aaaabbbb              a: Base address for BG4.                |
|     |       |                      b: Base address for BG3.                |
*/
	case 0x210C:
		mBackgroundInfo[ 2 ].mCharacterDataBaseVramAddress = value & 0x0F;
		mBackgroundInfo[ 3 ].mCharacterDataBaseVramAddress = value & 0xF0;
		break;

/*
| wd  |$210D  |BG1 horizontal scroll register [BG1HOFS]                      |
|     |       |mmmmmaaa aaaaaaaa              a: Horizontal offset.          |
|     |       |                               m: Only set with MODE 7.       |
|     |       |                                                              |
|     |       |This is an intruiging register. Like the types define, it has |
|     |       |to be written to twice: The first byte holds the first 8 bits,|
|     |       |and the second byte holds the last 3 bits. This makes a total |
|     |       |of 11 bits for information. This only proves true for MODes   |
|     |       |0 to 6. MODE 7 uses 13 bits instead of 11. As long as you're  |
|     |       |not in MODE 7, you can store $00 in the 2nd byte for a smooth |
|     |       |scrolling background.                                         |
*/
	case 0x210D:
	//regs.m7_hofs  = (value << 8) | regs.m7_latch;
	//  regs.m7_latch = value;

	//  regs.bg_hofs[BG1] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG1] >> 8) & 7);
	//  regs.bg_ofslatch  = value;
	break;

/*
| wd  |$210E  |BG1 vertical scroll register   [BG1VOFS] -|                   |
*/
	case 0x210E:
//		//BG1VOFS
//void bPPU::mmio_w210e(uint8 value) {
//	regs.m7_vofs  = (value << 8) | regs.m7_latch;
//	regs.m7_latch = value;
//
//	regs.bg_vofs[BG1] = (value << 8) | (regs.bg_ofslatch);
//	regs.bg_ofslatch  = value;
//}
	break;

/*
| wd  |$210F  |BG2 horizontal scroll register [BG2HOFS]  |                   |
*/
	case 0x210F:
//		  regs.bg_hofs[BG2] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG2] >> 8) & 7);
//  regs.bg_ofslatch  = value;
	break;

/*
| wd  |$2110  |BG3 vertical scroll register   [BG2VOFS]  |                   |
*/
	case 0x2110:
//		  regs.bg_vofs[BG2] = (value << 8) | (regs.bg_ofslatch);
 // regs.bg_ofslatch  = value;
	break;

/*
| wd  |$2111  |BG3 horizontal scroll register [BG3HOFS]  |- Same as $210D.   |
*/
	case 0x2111:
//		  regs.bg_hofs[BG3] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG3] >> 8) & 7);
//  regs.bg_ofslatch  = value;
	break;

/*
| wd  |$2112  |BG3 vertical scroll register   [BG3VOFS]  |                   |
*/
	case 0x2112:
//		  regs.bg_vofs[BG3] = (value << 8) | (regs.bg_ofslatch);
//  regs.bg_ofslatch  = value;
	break;

/*
| wd  |$2113  |BG4 horizontal scroll register [BG4HOFS]  |                   |
*/
	case 0x2113:
//		  regs.bg_hofs[BG4] = (value << 8) | (regs.bg_ofslatch & ~7) | ((regs.bg_hofs[BG4] >> 8) & 7);
 // regs.bg_ofslatch  = value;
	break;

/*
| wd  |$2114  |BG4 vertical scroll register   [BG4VOFS] -|                   |
*/
	case 0x2114:
//  regs.bg_vofs[BG4] = (value << 8) | (regs.bg_ofslatch);
//  regs.bg_ofslatch  = value;
	break;

/*
| w   |$2115  |Video port control [VMAIN]                                    |
|     |       |i000abcd              i: 0 = Addr-inc after writing to $2118  |
|     |       |                             or reading from $2139.           |
|     |       |                         1 = Addr-inc after writing to $2119  |
|     |       |                             or reading from $213A.           |
|     |       |                     ab: Full graphic (see table below).      |
|     |       |                     cd: SC increment (see table below).      |
|     |       |abcd|Result                                                   |
|     |       |----|---------------------------------------------------------|
|     |       |0100|Increment by 8 for 32 times (2-bit formation).           |
|     |       |1000|Increment by 8 for 64 times (4-bit formation).           |
|     |       |1100|Increment by 8 for 128 times (8-bit formation).          |
|     |       |0000|Address increments 1x1.                                  |
|     |       |0001|Address increments 32x32.                                |
|     |       |0010|Address increments 64x64.                                |
|     |       |0011|Address increments 128x128.                              |
|     |       |----|---------------------------------------------------------|
*/
	case 0x2115:
		mVramAddressIncrementMode = ( value & 0x80 ) > 0;
		mVramAddressFormation = ( value >> 2 ) & 3;
		switch ( value & 3 )
		{
		case 0: mVramAddressIncrementSize = 1; break;
		case 1: mVramAddressIncrementSize = 32; break;
		case 2: mVramAddressIncrementSize = 128; break; // Query: Shouldnt this be 64?
		case 3: mVramAddressIncrementSize = 128; break;
		}
	break;

/*
| w 2 |$2116  |Video port address [VMADDL/VMADDH]                            |
|     |       |???????? ????????                                             |
*/
	case 0x2116:
		{
		mVramAddress &= 0xFF00;
		mVramAddress |= value;
		// Do dummy read into read buffer
		const UInt16_t address = GetVramAddress();
		mVramReadBuffer = ReadVram( address );
		mVramReadBuffer |= ReadVram( address + 1 ) << 8;
		}
	break;
	case 0x2117:
		{
		mVramAddress &= 0x00FF;
		mVramAddress |= value << 8;
		const UInt16_t address = GetVramAddress();
		mVramReadBuffer = ReadVram( address );
		mVramReadBuffer |= ReadVram( address + 1 ) << 8;
		}
		break;

/*
| w 2 |$2118  |Video port data [VMDATAL/VMDATAH]                             |
|     |       |???????? ????????                                             |
|     |       |                                                              |
|     |       |According to bit 7 of $2115, the data can be stored as:       |
|     |       |                                                              |
|     |       |Bit 7|Register                   |Result                      |
|     |       |-----|---------------------------|----------------------------|
|     |       |  0  |Write to $2118 only.       |Lower 8-bits written then   |
|     |       |     |                           |address is increased.       |
|     |       |  0  |Write to $2119 then $2118. |Address increased when both |
|     |       |     |                           |are written to (in order).  |
|     |       |  1  |Write to $2119 only.       |Upper 8-bits written, then  |
|     |       |     |                           |address is increased.       |
|     |       |  1  |Write to $2118 then $2119. |Address increased when both |
|     |       |     |                           |are written to (in order).  |
|     |       |-----|---------------------------|----------------------------|
*/
	case 0x2118:
		{
			const UInt16_t address = GetVramAddress();
			WriteToVram( address, value );
	//  bg_tiledata_state[TILE_2BIT][(addr >> 4)] = 1;
	//  bg_tiledata_state[TILE_4BIT][(addr >> 5)] = 1;
	//  bg_tiledata_state[TILE_8BIT][(addr >> 6)] = 1;

			if ( !mVramAddressIncrementMode )
				mVramAddress += mVramAddressIncrementSize;
		}
	break;
	case 0x2119:
		{
			const UInt16_t address = GetVramAddress() + 1;
			WriteToVram( address, value );
  //bg_tiledata_state[TILE_2BIT][(addr >> 4)] = 1;
  //bg_tiledata_state[TILE_4BIT][(addr >> 5)] = 1;
  //bg_tiledata_state[TILE_8BIT][(addr >> 6)] = 1;

			if ( mVramAddressIncrementMode )
				mVramAddress += mVramAddressIncrementSize;
		}
		break;

/*
| w   |$211A  |MODE7 settings register [M7SEL]                               |
|     |       |ab0000yx             ab: (see table below).                   |
|     |       |                      y: Vertical screen flip (1=flip).       |
|     |       |                      x: Horizontal screen flip (1=flip).     |
|     |       |                                                              |
|     |       |ab|Result                                                     |
|     |       |--|-----------------------------------------------------------|
|     |       |00|Screen repetition if outside of screen area.               |
|     |       |10|Character 0x00 repetition if outside of screen area.       |
|     |       |11|Outside of screen area is back-drop screen in 1 colour.    |
|     |       |--|-----------------------------------------------------------|
*/
	case 0x211A:
		//  regs.mode7_repeat = (value >> 6) & 3;
  //regs.mode7_vflip  = !!(value & 0x02);
  //regs.mode7_hflip  = !!(value & 0x01);
	break;

/*
| w   |$211B  |COS (COSINE) rotate angle / X Expansion [M7A]                 |
*/
	case 0x211B:
		//regs.m7a      = (value << 8) | regs.m7_latch;
  //regs.m7_latch = value;
	break;

/*
| w   |$211C  |SIN (SIN)    rotate angle / X Expansion [M7B]                 |
*/
	case 0x211C:
		//  regs.m7b      = (value << 8) | regs.m7_latch;
  //regs.m7_latch = value;
	break;

/*
| w   |$211D  |SIN (SIN)    rotate angle / Y Expansion [M7C]                 |
*/
	case 0x211D:
		//  regs.m7c      = (value << 8) | regs.m7_latch;
  //regs.m7_latch = value;
	break;

/*
| w   |$211E  |COS (COSINE) rotate angle / Y Expansion [M7D]                 |
*/
	case 0x211E:
		//  regs.m7d      = (value << 8) | regs.m7_latch;
  //regs.m7_latch = value;
	break;

/*
| wd  |$211F  |Center position X (13-bit data only)    [M7X]                 |
*/
	case 0x211F:
		//  regs.m7x      = (value << 8) | regs.m7_latch;
  //regs.m7_latch = value;
	break;

/*
| wd  |$2120  |Center position Y (13-bit data only)    [M7Y]                 |
|     |       |                                                              |
|     |       |MODE 7 formulae for rotation/enlargement/reduction:           |
|     |       |                                                              |
|     |       |X2 = AB * X1-X0 + X0                                          |
|     |       |Y2 = CD * Y1-Y0 + Y0                                          |
|     |       |                                                              |
|     |       |A = COS(GAMMA)*(1/ALPHA)      B = SIN(GAMMA)*(1/ALPHA)        |
|     |       |C = SIN(GAMMA)*(1/BETA)       D = COS(GAMMA)*(1/BETA)         |
|     |       |                                                              |
|     |       |  GAMMA: Rotation angle.                                      |
|     |       |  ALPHA: Reduction rates for X (horizontal).                  |
|     |       |   BETA: Reduction rates for Y (vertical).                    |
|     |       |X0 & Y0: Center coordinate.                                   |
|     |       |X1 & Y1: Display coordinate.                                  |
|     |       |X2 & Y2: Coordinate before calculation.                       |
*/
	case 0x2120:
 // regs.m7y      = (value << 8) | regs.m7_latch;
 // regs.m7_latch = value;
	break;

/*
| w   |$2121  |Colour # (or pallete) selection register [CGADD]              |
|     |       |xxxxxxxx              x: Address (color #).                   |
*/
	case 0x2121:
		mCgAddress = value << 1;
	break;

/*
| wd  |$2122  |Colour data register [CGDATA]                                 |
|     |       |xxxxxxxx              x: Value of colour.                     |
|     |       |                                                              |
|     |       |SNES colour is 15 bit; 5 bits for red, green, and blue. The   |
|     |       |order isn't RGB though: It's BGR (RGB reversed!).             |
*/
	case 0x2122:
		{
			if ( ( mCgAddress & 1 ) > 0 )
			{
				mCgLatchData = value;
			}
			else
			{
				WriteCgRam( mCgAddress & 0x01FE, mCgLatchData );
				WriteCgRam( ( mCgAddress & 0x01FE ) + 1, value & 0x7F );
			}
			mCgAddress++;
			mCgAddress &= 0x01FF;
		}
	break;

/*
| w   |$2123  |Window mask settings register [W12SEL]                        |
|     |       |abcdefgh              a: Disable/enable BG2 Window 2.         |
|     |       |                      b: BG2 Window 2 I/O (0=IN).             |
|     |       |                      c: Disable/enable BG2 Window 1.         |
|     |       |                      d: BG2 Window 1 I/O (0=IN).             |
|     |       |                      e: Disable/enable BG1 Window 2.         |
|     |       |                      f: BG1 Window 2 I/O (0=IN).             |
|     |       |                      g: Disable/enable BG1 Window 1.         |
|     |       |                      h: BG1 Window 1 I/O (0=IN).             |
*/
	case 0x2123:
	 // regs.window2_enabled[BG2] = !!(value & 0x80);
  //regs.window2_invert [BG2] = !!(value & 0x40);
  //regs.window1_enabled[BG2] = !!(value & 0x20);
  //regs.window1_invert [BG2] = !!(value & 0x10);
  //regs.window2_enabled[BG1] = !!(value & 0x08);
  //regs.window2_invert [BG1] = !!(value & 0x04);
  //regs.window1_enabled[BG1] = !!(value & 0x02);
  //regs.window1_invert [BG1] = !!(value & 0x01);
  break;

/*
| w   |$2124  |Window mask settings register [W34SEL]                        |
|     |       |abcdefgh              a: Disable/enable BG4 Window 2.         |
|     |       |                      b: BG4 Window 2 I/O (0=IN).             |
|     |       |                      c: Disable/enable BG4 Window 1.         |
|     |       |                      d: BG4 Window 1 I/O (0=IN).             |
|     |       |                      e: Disable/enable BG3 Window 2.         |
|     |       |                      f: BG3 Window 2 I/O (0=IN).             |
|     |       |                      g: Disable/enable BG3 Window 1.         |
|     |       |                      h: BG3 Window 1 I/O (0=IN).             |
*/
	case 0x2124:
  //regs.window2_enabled[BG4] = !!(value & 0x80);
  //regs.window2_invert [BG4] = !!(value & 0x40);
  //regs.window1_enabled[BG4] = !!(value & 0x20);
  //regs.window1_invert [BG4] = !!(value & 0x10);
  //regs.window2_enabled[BG3] = !!(value & 0x08);
  //regs.window2_invert [BG3] = !!(value & 0x04);
  //regs.window1_enabled[BG3] = !!(value & 0x02);
  //regs.window1_invert [BG3] = !!(value & 0x01);
	break;

/*
| w   |$2125  |Window mask settings register [WOBJSEL]                       |
|     |       |abcdefgh              a: Disable/enable colour Window 2.      |
|     |       |                      b: Colour Window 2 I/O (0=IN).          |
|     |       |                      c: Disable/enable colour Window 1.      |
|     |       |                      d: Colour Window 1 I/O (0=IN).          |
|     |       |                      e: Disable/enable OBJ Window 2.         |
|     |       |                      f: OBJ Window 2 I/O (0=IN).             |
|     |       |                      g: Disable/enable OBJ Window 1.         |
|     |       |                      h: OBJ Window 1 I/O (0=IN).             |
*/
	case 0x2125:
  //regs.window2_enabled[COL] = !!(value & 0x80);
  //regs.window2_invert [COL] = !!(value & 0x40);
  //regs.window1_enabled[COL] = !!(value & 0x20);
  //regs.window1_invert [COL] = !!(value & 0x10);
  //regs.window2_enabled[OAM] = !!(value & 0x08);
  //regs.window2_invert [OAM] = !!(value & 0x04);
  //regs.window1_enabled[OAM] = !!(value & 0x02);
  //regs.window1_invert [OAM] = !!(value & 0x01);
	break;

/*
| w   |$2126  |Window 1 left position register [WH0]                         |
|     |       |aaaaaaaa              a: Position.                            |
*/
	case 0x2126:
	break;

/*
| w   |$2127  |Window 1 right position register [WH1] -|                     |
*/
	case 0x2127:
	break;

/*
| w   |$2128  |Window 2 left position register [WH2]   |- Same as $2126.     |
*/
	case 0x2128:
	break;

/*
| w   |$2129  |Window 2 right position register [WH3] -|                     |
|     |       |                                                              |
|     |       |I may have the Window numbers reversed; as in, $2126 may be   |
|     |       |for Window 2, not Window 1; $2127 may be for Window 2, not    |
|     |       |Window 1... and so on...                                      |
*/
	case 0x2129:
	break;

/*
| w   |$212A  |Mask logic settings for Window 1 & 2 per screen [WBGLOG]      |
|     |       |aabbccdd              a: BG4 parms -|                         |
|     |       |                      b: BG3 parms  |- See table in $212B.    |
|     |       |                      c: BG2 parms  |                         |
|     |       |                      d: BG1 parms -|                         |
*/
	case 0x212A:
  //regs.window_mask[BG4] = (value >> 6) & 3;
  //regs.window_mask[BG3] = (value >> 4) & 3;
  //regs.window_mask[BG2] = (value >> 2) & 3;
  //regs.window_mask[BG1] = (value     ) & 3;
	break;

/*
| w   |$212B  |Mask logic settings for Colour Windows & OBJ Windows [WOBJLOG]|
|     |       |0000aabb              a: Colour Window parms (see table below)|
|     |       |                      b: OBJ Window parms (see table below).  |
|     |       |                                                              |
|     |       |Hi-bit|Lo-bit|Logic                                           |
|     |       |------|------|------------------------------------------------|
|     |       |   0  |   0  |OR                                              |
|     |       |   0  |   1  |AND                                             |
|     |       |   1  |   0  |XOR                                             |
|     |       |   1  |   1  |XNOR                                            |
|     |       |------|------|------------------------------------------------|
*/
	case 0x212B:
  //regs.window_mask[COL] = (value >> 2) & 3;
  //regs.window_mask[OAM] = (value     ) & 3;
	break;

/*
| w   |$212C  |Main screen designation [TM]                                  |
|     |       |000abcde              a: OBJ/OAM disable/enable.              |
|     |       |                      b: Disable/enable BG4.                  |
|     |       |                      c: Disable/enable BG3.                  |
|     |       |                      d: Disable/enable BG2.                  |
|     |       |                      e: Disable/enable BG1.                  |
*/
	case 0x212C:
		mSpritesEnabled = ( value & 0x10 ) > 0;
		mBackgroundInfo[0].mEnabled = ( value & 0x1 ) > 0;
		mBackgroundInfo[1].mEnabled = ( value & 0x2 ) > 0;
		mBackgroundInfo[2].mEnabled = ( value & 0x4 ) > 0;
		mBackgroundInfo[3].mEnabled = ( value & 0x8 ) > 0;
	break;

/*
| w   |$212D  |Sub-screen designation [TD]                                   |
|     |       |*** Same as $212C, but for the sub-screens, not the main.     |
|     |       |                                                              |
|     |       |Remember: When screen addition/subtraction is enabled, the    |
|     |       |sub screen is added/subtracted against the main screen.       |
*/
	case 0x212D:
	break;

/*
| w   |$212E  |Window mask main screen designation register [TMW]            |
|     |       |*** Same as $212C, but for window-masks.                      |
*/
	case 0x212E:
	break;

/*
| w   |$212F  |Window mask sub screen designation register [TSW]             |
|     |       |*** Same as $212E, but for the sub screen.                    |
*/
	case 0x212F:
	break;

/*
| w   |$2130  |Fixed color addition or screen addition register [CGWSEL]     |
|     |       |abcd00ef             ab: Main (see table below).              |
|     |       |                     cd: Sub (see table below).               |
|     |       |                      e: 0 = Enable +/- for fixed colour.     |
|     |       |                         1 = Enable +/- for sub screen.       |
|     |       |                      f: Colour & char-data = direct color    |
|     |       |                         data (MODE 3, 4 & 7 only).           |
|     |       |                                                              |
|     |       |ab|Result                                                     |
|     |       |--|-----------------------------------------------------------|
|     |       |00|All the time.                                              |
|     |       |01|Inside window only.                                        |
|     |       |10|Outside window only.                                       |
|     |       |11|All the time.                                              |
|     |       |--|-----------------------------------------------------------|
*/
	case 0x2130:
	break;

/*
| w   |$2131  |Addition/subtraction for screens, BGs, & OBJs [CGADSUB]       |
|     |       |mrgsabcd              m: 0 = Enable + colour-data mode.       |
|     |       |                         1 = Enable - colour-data mode.       |
|     |       |                      r: See below for more info.             |
|     |       |                      g: Affect back-area.                    |
|     |       |                      s: Affect OBJs.                         |
|     |       |                      a: Affect BG4.                          |
|     |       |                      b: Affect BG3.                          |
|     |       |                      c: Affect BG2.                          |
|     |       |                      d: Affect BG1.                          |
|     |       |                                                              |
|     |       |*** 'r' is some sort-of "1/2 of colour data" on/off bit. When |
|     |       |    the colour constant +/- or screen +/- is performed, desig-|
|     |       |    nate whether the RGB result in the +/- area should be 1/2 |
|     |       |    or not; the back-area is not affected.                    |
*/
	case 0x2131:
	break;

/*
| w   |$2132  |Fixed colour data for fixed colour +/- [COLDATA]              |
|     |       |bgrdddddd             b: Set to change blue.                  |
|     |       |                      g: Set to change green.                 |
|     |       |                      r: Set to change red.                   |
|     |       |                      d: Set colour constant data for +/-.    |
*/
	case 0x2132:
	break;

/*
| w   |$2133  |Screen mode/video select register [SETINI]                    |
|     |       |sn00pvshi                                                     |
|     |       |                      s: Super-impose SFX graphics over ex-   |
|     |       |                         ternal video (usually 0).            |
|     |       |                      n: External mode (screen expand). When  |
|     |       |                         sing MODE 7, enable.                 |
|     |       |                      p: 0 = 256 resolution.                  |
|     |       |                         1 = 512 sub screen resolution.       |
|     |       |                      v: 0 = 224 vertical resolution.         |
|     |       |                         1 = 239 vertical resolution.         |
|     |       |                      s: See below for more info.             |
|     |       |                      i: 0 = No interlace.                    |
|     |       |                         1 = Interlaced display.              |
|     |       |                                                              |
|     |       |*** When in interlace mode, select either the 1-dot per line  |
|     |       |    mode or the 1-dot repeated every 2-lines mode. If '1' is  |
|     |       |    set in this bit, the OBJ seems to be reduced vertically   |
|     |       |    by 1/2.                                                   |
|     |       |                                                              |
|     |       |*** Interlaced mode is used in the SNES test cartridge. It    |
|     |       |    does flicker, but it gives a FULL 480 vertical resolution.|
*/
	case 0x2133:
	break;

/*
|r    |$2134  |Multiplication result register (low) [MPYL]                   |
*/
	case 0x2134:
	break;

/*
|r    |$2135  |Multiplication result register (middle) [MPYM]                |
*/
	case 0x2135:
	break;

/*
|r    |$2136  |Multiplication result register (high) [MPYH]                  |
|     |       |*** Result is 8 bits long for $2134, $2135, and $2136.        |
*/
	case 0x2136:
	break;

/*
|r    |$2137  |Software latch for horizontal/vertical counter [SLHV]         |
|     |       |aaaaaaaa                 a: Result.                           |
|     |       |                                                              |
|     |       |The counter value at the point when $2137 is read can be      |
|     |       |latched. Data read is meaningless.                            |
*/
	case 0x2137:
	break;
/*
|r    |$2138  |Read data from OAM {OAMDATAREAD}                              |
*/
	case 0x2138:
	break;

/*
|r  2 |$2139  |Read data from VRAM {VMDATALREAD/VMDATAHREAD}                 |
*/
	case 0x2139:
		/*
		uint16 addr = get_vram_address();
  regs.ppu1_mdr = regs.vram_readbuffer;
  if(regs.vram_incmode == 0) {
    addr &= 0xfffe;
    regs.vram_readbuffer  = vram_mmio_read(addr + 0);
    regs.vram_readbuffer |= vram_mmio_read(addr + 1) << 8;
    regs.vram_addr += regs.vram_incsize;
  }
  return regs.ppu1_mdr;
  */
	break;

/*
|r    |$213B  |Read data from CG-RAM (colour) {CGDATAREAD}                   |
*/
	case 0x213B:
	break;

/*
|r d  |$213C  |Horizontal scanline location [OPHCT]                          |
*/
	case 0x213C:
	break;

/*
|r d  |$213D  |Vertical scanline location  [OPVCT]                           |
|     |       |*** Registers $213C and $213D are 9-bits in length.           |
*/
	case 0x213D:
	break;

/*
|r    |$213E  |PPU status flag & version number [STAT77]                     |
|     |       |trm0vvvv              t: Time over (see below).               |
|     |       |                      r: Range over (see below).              |
|     |       |                      m: Master/slave mode select. Usually 0. |
|     |       |                      v: Version # ($5C77 (???)).             |
|     |       |                                                              |
|     |       |*** Range: When the quantity of the OBJ (size is non-relevant)|
|     |       |           becomes 33 pieces or more, '1' is set.             |
|     |       |     Time: When the quantity of the OBJ which is converted to |
|     |       |           8x8 is 35 pieces or more, '1' will be set.         |
*/
	case 0x213E:
	break;

/*
|r    |$213F  |PPU status flag & version number [STAT78]                     |
|     |       |fl0mvvvv              f: Field # scanned in int. mode (0=1st).|
|     |       |                      l: Set if external signal (light pen,   |
|     |       |                         etc.) is installed/applied.          |
|     |       |                      m: NTSC/PAL mode (0=NTSC, 1=PAL).       |
|     |       |                      v: Version # ($5C78 (???)).             |
*/
	case 0x213F:
	break;

/*
|rw   |$2140  |[APUI00] -|                                                   |
*/
	case 0x2140:
	break;

/*
|rw   |$2141  |[APUI01]  |- Audio registers. See sound.doc and sid-spc.src.  |
*/
	case 0x2141:
	break;

/*
|rw   |$2142  |[APUI02]  |                                                   |
*/
	case 0x2142:
	break;

/*
|rw   |$2143  |[APUI03] -|                                                   |
*/
	case 0x2143:
	break;

/*
| w   |$4200  |Counter enable [NMITIMEN]                                     |
|     |       |a0yx000b              a: NMI/VBlank interrupt.                |
|     |       |                      y: Vertical counter.                    |
|     |       |                      x: Horizontal counter.                  |
|     |       |                      b: Joypad read-enable.                  |
*/
	case 0x4200:
	break;

/*
| w   |$4201  |Programmable I/O port (out-port) [WRIO]                       |
*/
	case 0x4201:
	break;

/*
| w   |$4202  |Multiplicand 'A' [WRMPYA]                                     |
*/
	case 0x4202:
	break;

/*
| w   |$4203  |Multiplier 'B' [WRMPYB]                                       |
|     |       |*** Absolute multiplication used when using the two above reg-|
|     |       |    isters. Formulae is: 'A (8-bit) * B (8-bit) = C (16-bit)'.|
|     |       |    Result can be read from $4216.                            |
*/
	case 0x4203:
	break;

/*
| w 2 |$4204  |Dividend C [WRDIVL/WRDIVH]                                    |
*/
	case 0x4204:
	break;

/*
| w   |$4205  |Divisor B [WRDIVB]                                            |
|     |       |*** Absolute division used when using the two above registers.|
|     |       |    Formulae is 'C (16-bit) / B (8-bit) = A (16-bit)'.        |
|     |       |    Result can be read from $4214, and the remainder read from|
|     |       |    $4216.                                                    |
|     |       |*** Operation will start when $4205 is set, and will be com-  |
|     |       |    pleted after 16 machine cycles.                           |
*/
	case 0x4205:
	break;

/*
| w 2 |$4207  |Video horizontal IRQ beam position/pointer [HTIMEL/HTIMEH]    |
|     |       |0000000x xxxxxxxx     x: Beam position.                       |
|     |       |                                                              |
|     |       |Valid values for x range from 0 to 339, due to overscan. The  |
|     |       |timer is reset every scanline, so unless it's disabled, you'll|
|     |       |receive an interrupt every time the beam hits the value given.|
*/
	case 0x4207:
	break;

/*
| w 2 |$4209  |Video vertical IRQ beam position/pointer [VTIMEL/VTIMEH]      |
|     |       |0000000y yyyyyyyy     y: Beam position.                       |
|     |       |                                                              |
|     |       |Same as $4207, but valid values for y are 0 to 261 (based from|
|     |       |overscan at the top of the screen).                           |
*/
	case 0x4209:
	break;

/*
| w   |$420B  |DMA enable register [MDMAEN]                                  |
|     |       |abcdefgh              a: DMA channel #7.                      |
|     |       |                      b: DMA channel #6.                      |
|     |       |                      c: DMA channel #5.                      |
|     |       |                      d: DMA channel #4.                      |
|     |       |                      e: DMA channel #3.                      |
|     |       |                      f: DMA channel #2.                      |
|     |       |                      g: DMA channel #1.                      |
|     |       |                      h: DMA channel #0.                      |
*/
	case 0x420B:
	break;

/*
| w   |$420C  |HDMA enable register.                                         |
|     |       |*** Same as $420B, virtually.                                 |
*/
	case 0x420C:
	break;

/*
| w   |$420D  |Cycle speed register [MEMSEL]                                 |
|     |       |0000000x              x: 0 = Normal (2.68MHz).                |
|     |       |                         1 = Fast (3.58MHz).                  |
|     |       |                                                              |
|     |       |Note that using the fast mode requires 120ns or faster EPROMs.|
*/
	case 0x420D:
		mFastCycleModeActive = ( value & 0x1 ) > 0;
	break;

/*
|r    |$4210  |NMI register [RDNMI]                                          |
|     |       |x000vvvv              x: Disable/enable NMI.                  |
|     |       |                      v: Version # ($5A22 (???))              |
|     |       |                                                              |
|     |       |Bit 7 can be reset to 0 by reading this register.             |
*/
	case 0x4210:
		/*
		 uint8 r = (regs.mdr & 0x70);
  r |= (uint8)(rdnmi()) << 7;
  r |= (cpu_version & 0x0f);
  return r;
		*/
	break;

/*
|rw   |$4211  |Video IRQ register [TIMEUP]                                   |
|     |       |i0000000              i: 0 = IRQ is not enabled.              |
|     |       |                         1 = IRQ is enabled.                  |
|     |       |                                                              |
|     |       |This location MUST be read to clear a horizontal or vertical  |
|     |       |raster interrupt. It's all relative to $4200. If the horiz-   |
|     |       |ontal timer interrupt (bit 4, $4200) is set then the interrupt|
|     |       |will be generated according to the position in $4207. Same    |
|     |       |thing is for vertical timing (bit 5, $4200) but the position  |
|     |       |will be read from $4209, not $4207.                           |
*/
	case 0x4211:
	break;

/*
|rw   |$4212  |Status register [HVBJOY]                                      |
|     |       |xy00000a              x: 0 = Not in VBlank state.             |
|     |       |                         1 = In VBlank state.                 |
|     |       |                      y: 0 = Not in HBlank state.             |
|     |       |                         1 = In HBlank state.                 |
|     |       |                      a: 0 = Joypad not ready.                |
|     |       |                         1 = Joypad ready.                    |
*/
	case 0x4212:
		/*
		  uint8 r = (regs.mdr & 0x3e);
  if(status.auto_joypad_active) r |= 0x01;
  if(hcounter() <= 2 || hcounter() >= 1096) r |= 0x40;  //hblank
  if(vcounter() >= (ppu.overscan() == false ? 225 : 240)) r |= 0x80;  //vblank
  return r;
		*/
	break;

/*
|r    |$4213  |Programmable I/O port (in-port) [RDIO]                        |
*/
	case 0x4213:
	break;

/*
|r  2 |$4214  |Quotient of divide result [RDDIVL/RDDIVH]                     |
*/
	case 0x4214:
	break;

/*
|r  2 |$4216  |Multiplication or divide result [RDMPYL/RDMPYH]               |
*/
	case 0x4216:
	break;

/*
|r    |$4218  |Joypad #1 status register [JOY1L]                             |
|     |       |abcd0000              a: A button (1=pressed).                |
|     |       |                      b: X button (1=pressed).                |
|     |       |                      c: Top-Left (1=pressed).                |
|     |       |                      d: Top-Rght (1=pressed).                |
*/
	case 0x4218:
	break;

/*
|r    |$4219  |Joypad #1 status register [JOY1H]                             |
|     |       |abcdefgh              a: B button (1=pressed).                |
|     |       |                      b: Y button (1=pressed).                |
|     |       |                      c: Select   (1=pressed).                |
|     |       |                      d: Start    (1=pressed).                |
|     |       |                      e: Up       (1=pressed).                |
|     |       |                      f: Down     (1=pressed).                |
|     |       |                      g: Left     (1=pressed).                |
|     |       |                      h: Right    (1=pressed).                |
*/
	case 0x4219:
	break;

/*
|r    |$421A  |Joypad #2 status register [JOY2L] -|                          |
*/
	case 0x421A:
	break;

/*
|r    |$421B  |Joypad #2 status register [JOY2H]  |                          |
*/
	case 0x421B:
	break;

/*
|r    |$421C  |Joypad #3 status register [JOY3L]  |- Same as $4218 & $4219.  |
*/
	case 0x421C:
	break;

/*
|r    |$421D  |Joypad #3 status register [JOY3H]  |                          |
*/
	case 0x421D:
	break;

/*
|r    |$421E  |Joypad #4 status register [JOY4L]  |                          |
*/
	case 0x421E:
	break;

/*
|r    |$421F  |Joypad #4 status register [JOY4H] -|                          |
|     |       |*** Joypad registers can be read w/ a 16-bit accum/X/Y and    |
|     |       |    both the high and low bytes will received valid data.     |
*/
	case 0x421F:
	break;

	}

	
/*
|----------------------------------------------------------------------------|
|The following data is for DMA-transfers. 'x' represents the DMA channel #,  |
|which ranges from 0 to 7. So, the following would represent each section:   |
|DMA #0: $4300-$4305.                                                        |
|DMA #1: $4310-$4315.                                                        |
|....................                                                        |
|DMA #7: $4370-$4375.                                                        |
|----------------------------------------------------------------------------|
| w   |$43x0  |DMA Control register [DMAPX]                                  |
|     |       |vh0cbaaa              v: 0 = CPU memory -> PPU.               |
|     |       |                         1 = PPU -> CPU memory.               |
|     |       |                      h: For HDMA only:                       |
|     |       |                         0 = Absolute addressing.             |
|     |       |                         1 = Indirect addressing.             |
|     |       |                      c: 0 = Auto address inc/decrement.      |
|     |       |                         1 = Fixed address (for VRAM, etc.).  |
|     |       |                      b: 0 = Automatic increment.             |
|     |       |                         1 = Automatic decrement.             |
|     |       |                      a: Transfer type:
|     |       |                         000 = 1 address write twice: LH.     |
|     |       |                         001 = 2 addresses: LH.               |
|     |       |                         010 = 1 address write once.          |
|     |       |                         011 = 2 addresses write twice: LLHH  |
|     |       |                         100 = 4 addresses: LHLH              |
|     |       |                                                              |
|     |       |                                                              |
| w   |$43x1  |DMA Destination register [BBADX]                              |
|     |       |xxxxxxxx              x: Low-byte address.                    |
|     |       |                                                              |
|     |       |*** The upper-byte address is assumed to be $21, making your  |
|     |       |    access addresses $2100 to $21FF.                          |
|     |       |                                                              |
|     |       |                                                              |
| w 2 |$43x2  |Source address [A1TXL/A1TXH]                                  |
| w   |$43x4  |Source bank address [A1BX]                                    |
| w 2 |$43x5  |DMA transfer size & HDMA address register [DASXL/DASXH]       |
|     |       |*** When using DMA, $43x5 defines the # of bytes to be trans- |
|     |       |    ferred via DMA itself. When using HDMA, $43x5 defines the |
|     |       |    data address ($43x5 = low byte, $43x6 = hi byte).         |
|     |       |                                                              |
|     |       |                                                              |
| w   |$43xA  |Number of lines for HDMA transfer [NTRLX]                     |
|     |       |cxxxxxxx              c: Continue (0=yes, 1=no (???)).        |
|     |       |                      x: # of lines to transfer.              |
*/
}



void SnesPpu::SaveState( std::ostream& ostr )
{
}


void SnesPpu::LoadState( std::istream& istr )
{
}



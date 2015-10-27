
#include "stdafx.h"
#include "main.h"
#include "SnesCpuMemory.h"
#include "SnesMainboard.h"



SnesCpuMemory::SnesCpuMemory()
{
	mLowRam = new Byte_t[ 0x2000 ];
	memset( mLowRam, 0, 0x2000 );

	mExpandedRam0 = new Byte_t[ 0x2000 ];
	memset( mExpandedRam0, 0, 0x2000 );

	mExpandedRam1 = new Byte_t[ 0xE000 ];
	memset( mExpandedRam1, 0, 0xE000 );

	mExpandedRam2 = new Byte_t[ 0x10000 ];
	memset( mExpandedRam2, 0, 0x10000 );
}


SnesCpuMemory::~SnesCpuMemory()
{
	delete[] mLowRam;
	delete[] mExpandedRam0;
	delete[] mExpandedRam1;
	delete[] mExpandedRam2;
}


/*
 ----------------------------------------------------------------------------
|Here's a really basic memory map of the SNES's memory. Thanks to Geggin of  |
|Censor for supplying this. Reminder: this is a memory map in MODE 20.       |
|----------------------------------------------------------------------------|
|Bank   |Address       |Description                                          |
|-------|--------------|-----------------------------------------------------|
|$00-$3F|$0000-$1FFF   |Scratchpad RAM. Set D-reg here if you'd like (I do)  |
|       |$2000-$5FFF   |Reserved (PPU, DMA)                                  |
|       |$6000-$7FFF   |Expand (???)                                         |
|       |$8000-$FFFF   |ROM (for code, graphics, etc.)                       |
|$70    |$0000-$7FFF   |SRAM (BRAM) - Battery RAM                            |
|$7E    |$0000-$1FFF   |Scratchpad RAM (same as bank $00 to $3F)             |
|       |$2000-$FFFF   |RAM (for music, or whatever)                         |
|$7F    |$0000-$FFFF   |RAM (for whatever)                                   |
 ----------------------------------------------------------------------------
*/

UInt8_t SnesCpuMemory::Read8LoRom( UInt8_t bankRegister, UInt16_t address )
{
	UInt8_t val = 0;

	if ( IS_INT_BETWEEN( bankRegister, 0, 0x40 ) )
	{
		if ( IS_INT_BETWEEN( address, 0, 0x2000 ) ) // | LowRAM, shadowed from $7E          |   $7E   |
		{
			val = Read8LoRom( 0x7E, address );
		}
		else if ( IS_INT_BETWEEN( address, 0x2000, 0x3000 ) ) // | PPU1, APU                          | $00-$3F |
		{
			val = g_snesMainboard->GetPPU()->Read8( bankRegister, address );
		}
		else if ( IS_INT_BETWEEN( address, 0x3000, 0x4000 ) ) // | SFX, DSP, etc.                     | $00-$3F |
		{
		}
		else if ( IS_INT_BETWEEN( address, 0x4000, 0x4200 ) ) // | Controller                         | $00-$3F |
		{
		}
		else if ( IS_INT_BETWEEN( address, 0x4200, 0x6000 ) ) // | PPU2, DMA, etc.                    | $00-$3F |
		{
			val = g_snesMainboard->GetPPU()->Read8( bankRegister, address );
		}
		else if ( IS_INT_BETWEEN( address, 0x6000, 0x8000 ) ) // | Expanded Ram 0													  | $00-$3F |
		{
			val = mExpandedRam0[ address - 0x6000 ];
		}
		else if ( IS_INT_BETWEEN( address, 0x8000, 0x10000 ) ) // | (Mode 20 ROM)[LoRom]                      | ------- |
		{
			val = g_snesMainboard->GetSnesCartridge()->ReadRom8( bankRegister, address - 0x8000 );
		}
	}
	else if ( IS_INT_BETWEEN( bankRegister, 0x40, 0x70 ) )
	{
		assert( false );
	}
	else if ( bankRegister == 0x70 )
	{ // SRAM
		val = g_snesMainboard->GetSnesCartridge()->ReadSRam8( 0, address );
	}
	else if ( IS_INT_BETWEEN( bankRegister, 0x71, 0x7E ) )
	{
		assert( false );
	}
	else if ( bankRegister == 0x7E )
	{
		if ( IS_INT_BETWEEN( address, 0, 0x2000 ) ) // | LowRAM                             | $00-$3F |
		{
			val = mLowRam[ address ];
		}
		else if ( IS_INT_BETWEEN( address, 0x2000, 0x10000 ) ) // | Expanded RAM                       | ------- |
		{
			val = mExpandedRam1[ address - 0x2000 ];
		}
	}
	else if ( bankRegister == 0x7F )
	{ // Expanded RAM 2
		val = mExpandedRam2[ address ];
	}
	else if ( IS_INT_BETWEEN( bankRegister, 0x80, 0x100 ) )
	{ // mirror of 0 -> 7F
		val = Read8LoRom( bankRegister - 0x80, address );
	}

	return val;
}


UInt8_t SnesCpuMemory::Read8HiRom( UInt8_t bankRegister, UInt16_t address )
{
	UInt8_t val = 0;
	return val;
}


void SnesCpuMemory::Write8LoRom( UInt8_t bankRegister, UInt16_t address, UInt8_t value )
{
	if ( IS_INT_BETWEEN( bankRegister, 0, 0x40 ) )
	{
		if ( IS_INT_BETWEEN( address, 0, 0x2000 ) ) // | LowRAM, shadowed from $7E          |   $7E   |
		{
			Write8LoRom( 0x7E, address, value );
		}
		else if ( IS_INT_BETWEEN( address, 0x2000, 0x3000 ) ) // | PPU1, APU                          | $00-$3F |
		{
			switch ( address )
			{
				case 0x2180: /* |rw   |$2180  |Read/write WRAM register [WMDATA] */
				break;

				case 0x2181: /* |rw   |$2181  |WRAM data register (low byte) [WMADDL] */
				break;

				case 0x2182: /* |rw   |$2182  |WRAM data register (middle byte) [WMADDM] */
				break;

				case 0x2183: /* |rw   |$2183  |WRAM data register (high byte) [WMADDH] */
				break;
				
				default:
					g_snesMainboard->GetPPU()->Write8( bankRegister, address, value );
					break;
			}
		}
		else if ( IS_INT_BETWEEN( address, 0x3000, 0x4000 ) ) // | SFX, DSP, etc.                     | $00-$3F |
		{
		}
		else if ( IS_INT_BETWEEN( address, 0x4000, 0x4200 ) ) // | Controller                         | $00-$3F |
		{
		}
		else if ( IS_INT_BETWEEN( address, 0x4200, 0x6000 ) ) // | PPU2, DMA, etc.                    | $00-$3F |
		{
			g_snesMainboard->GetPPU()->Write8( bankRegister, address, value );
		}
		else if ( IS_INT_BETWEEN( address, 0x6000, 0x8000 ) ) // | Expanded Ram 0													  | $00-$3F |
		{
			mExpandedRam0[ address - 0x6000 ] = value;
		}
		else if ( IS_INT_BETWEEN( address, 0x8000, 0x10000 ) ) // | (Mode 20 ROM)                      | ------- |
		{
			g_snesMainboard->GetSnesCartridge()->WriteRom8( bankRegister, address - 0x8000, value );
		}
	}
	else if ( IS_INT_BETWEEN( bankRegister, 0x40, 0x70 ) )
	{
		assert( false );
	}
	else if ( bankRegister == 0x70 )
	{ // SRAM
		g_snesMainboard->GetSnesCartridge()->WriteSRam8( 0, address, value );
	}
	else if ( IS_INT_BETWEEN( bankRegister, 0x71, 0x7E ) )
	{
		assert( false );
	}
	else if ( bankRegister == 0x7E )
	{
		if ( IS_INT_BETWEEN( address, 0, 0x2000 ) ) // | LowRAM shadow                             | $00-$3F |
		{
			mLowRam[ address ] = value;
		}
		else if ( IS_INT_BETWEEN( address, 0x2000, 0x10000 ) ) // | Expanded RAM                       | ------- |
		{
			mExpandedRam1[ address - 0x2000 ] = value;
		}
	}
	else if ( bankRegister == 0x7F )
	{ // Expanded RAM
		mExpandedRam2[ address ] = value;
	}
	else if ( IS_INT_BETWEEN( bankRegister, 0x80, 0x100 ) )
	{ // mirror of 0 -> 7F
		Write8LoRom( bankRegister - 0x80, address, value );
	}
}


void SnesCpuMemory::Write8HiRom( UInt8_t bankRegister, UInt16_t address, UInt8_t value )
{
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

UInt8_t SnesCpuMemory::Read8( UInt8_t bankRegister, UInt16_t address )
{
	if ( g_snesMainboard->GetSnesCartridge()->IsHiRom() )
		return Read8HiRom( bankRegister, address );
	else
		return Read8LoRom( bankRegister, address );
}


UInt16_t SnesCpuMemory::Read16( UInt8_t bankRegister, UInt16_t address )
{
	UInt16_t num = 0; // TODO: check this is right order
	num |= Read8( bankRegister, address );
	num |= Read8( bankRegister, address + 1 ) << 8;
	return num;
}


UInt32_t SnesCpuMemory::Read24( UInt8_t bankRegister, UInt16_t address )
{
	UInt32_t num = 0; // TODO: check this is right order
	num |= Read8( bankRegister, address );
	num |= Read8( bankRegister, address + 1 ) << 8;
	num |= Read8( bankRegister, address + 2 ) << 16;
	return num;
}


UInt8_t SnesCpuMemory::Read8( UInt32_t address )
{
	return Read8( ( address >> 16 ) & 0xFF, address & 0xFFFF );
}


UInt16_t SnesCpuMemory::Read16( UInt32_t address )
{
	UInt16_t num = 0; // TODO: check this is right order
	num |= Read8( address );
	num |= Read8( address + 1 ) << 8;
	return num;
}


UInt32_t SnesCpuMemory::Read24( UInt32_t address )
{
	UInt32_t num = 0; // TODO: check this is right order
	num |= Read8( address );
	num |= Read8( address + 1 ) << 8;
	num |= Read8( address + 2 ) << 16;
	return num;
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



void SnesCpuMemory::Write8( UInt8_t bankRegister, UInt16_t address, UInt8_t value )
{
	if ( g_snesMainboard->GetSnesCartridge()->IsHiRom() )
		Write8HiRom( bankRegister, address, value );
	else
		Write8LoRom( bankRegister, address, value );
}


void SnesCpuMemory::Write16( UInt8_t bankRegister, UInt16_t address, UInt16_t value )
{
	Write8( bankRegister, address, value & 0xFF );
	Write8( bankRegister, address + 1, ( value << 8 ) & 0xFF );
}


void SnesCpuMemory::Write24( UInt8_t bankRegister, UInt16_t address, UInt32_t value )
{
	Write8( bankRegister, address, value & 0xFF );
	Write8( bankRegister, address + 1, ( value << 8 ) & 0xFF );
	Write8( bankRegister, address + 2, ( value << 16 ) & 0xFF );
}


void SnesCpuMemory::Write8( UInt32_t address, UInt8_t value )
{
	Write8( ( address >> 16 ) & 0xFF, address & 0xFFFF, value );
}


void SnesCpuMemory::Write16( UInt32_t address, UInt16_t value )
{
	Write8( address, value & 0xFF );
	Write8( address + 1, ( value << 8 ) & 0xFF );
}


void SnesCpuMemory::Write24( UInt32_t address, UInt32_t value )
{
	Write8( address, value & 0xFF );
	Write8( address + 1, ( value << 8 ) & 0xFF );
	Write8( address + 2, ( value << 16 ) & 0xFF );
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////



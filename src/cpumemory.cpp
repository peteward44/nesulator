
#include "stdafx.h"
#include "cpumemory.h"
#include "mainboard.h"
#include <sstream>



// TODO: Implement IO ports properly

CPUMemory::CPUMemory()
{
	resetConnection = g_nesMainboard->ResetEvent.connect( boost::bind( &CPUMemory::OnReset, this, _1 ) );
}


CPUMemory::~CPUMemory()
{
	resetConnection.disconnect();
}


void CPUMemory::OnReset( bool cold )
{
	if ( cold )
	{
		// these memory locations are set to various values on power-up
		memset( ramPage.Data, 0xFF, 0x800 );
		ramPage.Data[ 0x0008 ] = 0xF7;
		ramPage.Data[ 0x0009 ] = 0xEF;
		ramPage.Data[ 0x000a ] = 0xDF;
		ramPage.Data[ 0x000f ] = 0xBF;
	}
}


void CPUMemory::Write8( UInt16_t offset, Byte_t data )
{
	if ( offset < 0x2000 )
	{ // address is within RAM boundaries
		// account for 4x mirroring (we only actually keep one copy of a quarter of the addressable space)
		ramPage.Data[ offset % 0x800 ] = data;
	}
	else if ( IS_INT_BETWEEN( offset, 0x2000, 0x4000 ) )
	{ // PPU registers (mirrored)
		g_nesMainboard->GetPPU()->WriteToRegister( ( ( offset - 0x2000 ) % 0x08 ) + 0x2000, data );
	}
	else if ( IS_INT_BETWEEN( offset, 0x4000, 0x4018 ) )
	{
		switch ( offset )
		{
			case 0x4014: // sprite DMA access
				g_nesMainboard->GetPPU()->WriteToRegister( offset, data );
				break;
			case 0x4016: // input
			case 0x4017:
				g_nesMainboard->GetInputDeviceBus()->WriteToRegister( offset, data );
	//			break;
			default: // APU (write input 4016 + 4017 to APU as well)
				g_nesMainboard->GetAPU()->WriteToRegister( offset, data );
				break;
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0x4018, 0x6000 ) )
	{
		g_nesMainboard->GetMemoryMapper()->Write8EXRam( offset, data );
	}
	else if ( IS_INT_BETWEEN( offset, 0x6000, 0x8000 ) )
	{
		g_nesMainboard->GetMemoryMapper()->Write8SRam( offset, data );
	}
	else// if ( IS_INT_BETWEEN( offset, 0x8000, 0x10000 ) )
	{
		g_nesMainboard->GetMemoryMapper()->Write8PrgRom( offset, data );
	}
}


Byte_t CPUMemory::Read8( UInt16_t offset )
{
	if ( offset < 0x2000 )
	{ // address is within RAM boundaries
		// account for 4x mirroring
		return ramPage.Data[ offset % 0x800 ];
	}
	else if ( IS_INT_BETWEEN( offset, 0x2000, 0x4000 ) )
	{ // PPU registers are 0x08 long and are mirrored between 0x2000 -> 0x4000
		return g_nesMainboard->GetPPU()->ReadFromRegister( ( ( offset - 0x2000 ) % 0x08 ) + 0x2000 );
	}
	else if ( IS_INT_BETWEEN( offset, 0x4000, 0x4018 ) )
	{ // address is second set of IO ports
		Byte_t ret = 0;

		switch ( offset )
		{
			case 0x4015:
				ret = g_nesMainboard->GetAPU()->ReadFromRegister( offset );
			break;
			case 0x4016: // input
			case 0x4017:
				ret = g_nesMainboard->GetInputDeviceBus()->ReadFromRegister( offset );
				break;
			default:
			case 0x4014: // sprite DMA access
				ret = g_nesMainboard->GetPPU()->ReadFromRegister( offset );
				break;
		}
		
		return ret;
	}
	else if ( IS_INT_BETWEEN( offset, 0x4018, 0x6000 ) )
	{
		return g_nesMainboard->GetMemoryMapper()->Read8EXRam( offset );
	}
	else if ( IS_INT_BETWEEN( offset, 0x6000, 0x8000 ) )
	{
		return g_nesMainboard->GetMemoryMapper()->Read8SRam( offset );
	}
	else// if ( IS_INT_BETWEEN( offset, 0x8000, 0x10000 ) )
	{
		return g_nesMainboard->GetMemoryMapper()->Read8PrgRom( offset );
	}
}


UInt16_t CPUMemory::Read16( UInt16_t offset, bool zeropage )
{
	UInt16_t ret = Read8( offset );
	UInt16_t newoffset;
	if ( zeropage && (offset % 0x100) == 0xff)
		newoffset = offset - ( offset % 0x100 );
	else
		newoffset = offset+1;

	ret |= ( Read8( newoffset ) << 8 );
	return ret;
}


void CPUMemory::SaveState( std::ostream& ostr )
{
	ramPage.SaveState( ostr );
}


void CPUMemory::LoadState( std::istream& istr )
{
	ramPage.LoadState( istr );
}

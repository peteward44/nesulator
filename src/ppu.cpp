

#include "ppu.h"
#include "mainboard.h"
#include <cstdlib>
#include <cstring>
#include <stdexcept>

using namespace std;
using namespace boost::placeholders;


PictureProcessingUnit::PictureProcessingUnit( )
{
	mirroringMethod = PPU_MIRROR_HORIZONTAL;

	memset( nameTablePointers, 0, sizeof( NameTable_t* ) * 4 );

	control1 = (unsigned char *) &ppuControl1;
	control2 = (unsigned char *) &ppuControl2;
	status = (unsigned char *) &ppuStatus;
	mLastTransferredValue = 0;
	frameCount = 0;

	paletteTables[0] = new PaletteTable_t();
	paletteTables[1] = new PaletteTable_t();

	SetDefaultVariables( true );

	g_nesMainboard->GetSynchroniser()->RegisterListener( SYNC_PPU, this );
	resetConnection = g_nesMainboard->ResetEvent.connect( boost::bind( &PictureProcessingUnit::OnReset, this, _1 ) );
}


void PictureProcessingUnit::SetDefaultVariables( bool cold )
{
	if ( cold )
		*control1 = *control2 = *status = 0;
	else
		ppuControl1.vBlankNmi = false;

	ppuStatus.vBlank = true;

	masterTickCount = renderMTC = 0;
	mTriggerNmiNextClock = false;
	mTriggerNmiMTC = 0;

	isOddFrame = false;
	suppressNmi = suppressVblank = false;
	forceNmi = false;
	mDoSpriteTransferAfterNextCpuInstruction = false;

	fineX = 0;
	bufferedppuread = 0;
	ppuSecondAddressWrite = false;
	ppuReadAddress = ppuLatchAddress = 0;
	spriteaddress = 0;

	// background
	memset( renderScanline, 0, 34 * 8 * sizeof( Byte_t ) );
	memset( priorityBuffer, 0, 34 * 8 * sizeof( boost::uint16_t ) );

	// foreground
	ticksWhenToSetOverflow = -1;
}


PictureProcessingUnit::~PictureProcessingUnit()
{
	for ( int i=0; i<2; ++i )
		delete paletteTables[i];

	resetConnection.disconnect();
}


bool PictureProcessingUnit::IsRendering( int tickCount, bool includeHblank ) const
{
	if ( IsRenderingEnabled() )
	{
		int x, y;
		TicksToScreenCoordinates( tickCount, &x, &y );
		return ( includeHblank ? IS_INT_BETWEEN( x, 0, 256 ) : true ) && IS_INT_BETWEEN( y, -1, 241 );
	}
	else
		return false;
}


int PictureProcessingUnit::GetMasterTicksTillVBlankClearDue( int tickCount ) const
{
	return ( g_nesMainboard->GetModeConstants()->VBlankScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE - MASTER_CYCLES_PER_PPU * 0 )- tickCount;
}

int PictureProcessingUnit::GetMasterTicksTillClockSkip( int tickCount ) const
{
	return ( ( g_nesMainboard->GetModeConstants()->VBlankScanlines() + 0 ) * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE + ( MASTER_CYCLES_PER_PPU * ( 328 + 9 ) ) ) - tickCount;
}

int PictureProcessingUnit::GetMasterTicksTillFrameDone( int tickCount ) const
{
	return ( g_nesMainboard->GetModeConstants()->FrameScanlines() * MASTER_CYCLES_PER_PPU * PPU_TICKS_PER_SCANLINE ) - tickCount;
}

void PictureProcessingUnit::TicksToScreenCoordinates( int tickCount, int* x, int* y ) const
{
	assert( x && y );
	*x = ( ( ( tickCount / MASTER_CYCLES_PER_PPU ) ) % PPU_TICKS_PER_SCANLINE );
	*y = ( ( ( tickCount / MASTER_CYCLES_PER_PPU ) ) / PPU_TICKS_PER_SCANLINE ) - g_nesMainboard->GetModeConstants()->VBlankScanlines() - 1;
}


void PictureProcessingUnit::ChangeMirroringMethod( PPU_MIRRORING_METHOD method )
{
	mirroringMethod = method;
	std::wstring name;

	switch ( mirroringMethod )
	{
		default:
		case PPU_MIRROR_HORIZONTAL: // mirrors 3 & 4 point to the second nametable
			this->nameTablePointers[ 0 ] = this->nameTablePointers[ 1 ] = &nameTables[0];
			this->nameTablePointers[ 2 ] = this->nameTablePointers[ 3 ] = &nameTables[1];
			name = L"horizontal";
			break;
		case PPU_MIRROR_VERTICAL: // mirrors 2 & 4 point to the second nametable
			this->nameTablePointers[ 0 ] = this->nameTablePointers[ 2 ] = &nameTables[0];
			this->nameTablePointers[ 1 ] = this->nameTablePointers[ 3 ] = &nameTables[1];
			name = L"vertical";
			break;
		case PPU_MIRROR_FOURSCREEN: // no mirroring done, requires an extra 0x800 of memory kept on cart
			this->nameTablePointers[ 0 ] = &nameTables[0];
			this->nameTablePointers[ 1 ] = &nameTables[1];
			this->nameTablePointers[ 2 ] = &fsNameTables[ 0 ];
			this->nameTablePointers[ 3 ] = &fsNameTables[ 1 ];
			name = L"four screen";
			break;
		case PPU_MIRROR_SINGLESCREEN_NT0:
			for ( int i=0; i<4; ++i )
				this->nameTablePointers[ i ] = &nameTables[ 0 ];
			name = L"single screen #0";
			break;
		case PPU_MIRROR_SINGLESCREEN_NT1:
			for ( int i=0; i<4; ++i )
				this->nameTablePointers[ i ] = &nameTables[ 1 ];
			name = L"single screen #1";
			break;
		case PPU_MIRROR_CHRROM: // calls Write/Read8NameTable on the mapper
			memset( this->nameTablePointers, 0, sizeof( NameTable_t* ) * 4 );
			name = L"CHR ROM";
			break;
	}

	Log::Write( LOG_MAPPER, ( boost::wformat( L"PPU name table memory 0x2000 -> 0x3000 now uses %1% mirroring" ) % name ).str() );
}


void PictureProcessingUnit::UpdatePPUReadAddress( UInt16_t newAddress, bool invokedFromRegisterWrite )
{
	if ( invokedFromRegisterWrite )
	{
		if ( /*( ppuReadAddress & ( 1 << 12 ) ) == 0 &&*/ ( newAddress & ( 1 << 12 ) ) > 0 )
		{
			g_nesMainboard->GetMemoryMapper()->PPUA12Latch();
		}
	}

	ppuReadAddress = newAddress;
}


void PictureProcessingUnit::WriteToRegister( UInt16_t offset, Byte_t data )
{
	mLastTransferredValue = data;

	switch ( offset )
	{
	case 0x2000:
		{
			if ( !IS_BIT_SET( data, 7 ) )
			{
				// special case code for disabling NMI when disabled near vblank set
				g_nesMainboard->GetSynchroniser()->Synchronise( g_nesMainboard->GetSynchroniser()->GetCpuMTC() - MASTER_CYCLES_PER_PPU * 5 );

				// NMI should occur when disabled 2, 3 or 4 PPU clocks after VBL
				int vblankTicks = this->GetMasterTicksTillFrameDone( g_nesMainboard->GetSynchroniser()->GetCpuMTC() );

//				Log::Write( LOG_MISC, ( boost::format( "Vblank ticks left: %1%" ) % vblankTicks ).str() );

				if ( vblankTicks <= -MASTER_CYCLES_PER_PPU * 2 && vblankTicks >= -MASTER_CYCLES_PER_PPU * 4 )
				{
					forceNmi = true;
				}
				else if ( vblankTicks >= -MASTER_CYCLES_PER_PPU * 1 && vblankTicks <= MASTER_CYCLES_PER_PPU * 1 )
				{
					suppressNmi = true;
				}
			}
			else
			{
				// NMI should occur if enabled when VBL already set
				g_nesMainboard->GetSynchroniser()->Synchronise();

				if ( ppuStatus.vBlank && !ppuControl1.vBlankNmi )
				{ // there be a 1-PPU clock latency for this
					mTriggerNmiNextClock = true;
					mTriggerNmiMTC = g_nesMainboard->GetSynchroniser()->GetCpuMTC() + MASTER_CYCLES_PER_PPU * 1;
					g_nesMainboard->GetSynchroniser()->NewNextEvent( mTriggerNmiMTC );

				//	Log::Write( LOG_MISC, ( boost::format( "SET mTriggerNmiMTC: %i CPU MTC: %i" ) % mTriggerNmiMTC % g_nesMainboard->GetSynchroniser()->GetCpuMTC() ).str() );

					//g_nesMainboard->GetProcessor()->NonMaskableInterrupt( mTriggerNmiMTC );
				}
			}

			g_nesMainboard->GetSynchroniser()->Synchronise();

			// update nametable switch
			ppuLatchAddress &= 0xF3FF;
			ppuLatchAddress |= ((data & 3) << 10);

			const bool spriteScreenAddressChanged = ( (*control1) & 0x18 ) != ( data & 0x18 );

			*control1 = data;

			if ( spriteScreenAddressChanged )
				g_nesMainboard->GetMemoryMapper()->SpriteScreenEnabledUpdate( ppuControl1.spritePatternTableAddress, ppuControl1.screenPatternTableAddress );
		}
		break;
	case 0x2001:
		{
			g_nesMainboard->GetSynchroniser()->Synchronise();
			const bool renderingEnabledChanged = ( ( (*control2) & 0x18 ) > 0 ) != ( ( data & 0x18 ) > 0 );
			*control2 = data;
			Log::Write( LOG_MISC, ( boost::wformat( L"control2 changed to %1%" ) % int(data) ).str() );
			if ( renderingEnabledChanged )
				g_nesMainboard->GetMemoryMapper()->RenderingEnabledChanged( ( (*control2) & 0x18 ) > 0 );
		}
		break;
	case 0x2002: // no effect
		break;

		case 0x2003: // sprite memory address
			g_nesMainboard->GetSynchroniser()->Synchronise();
	//		if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
			{
				spriteaddress = data;
			}

			break;
		case 0x2004: // sprite memory data
			g_nesMainboard->GetSynchroniser()->Synchronise();
	//		if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
			{
				spriteMemory.Data[ spriteaddress ] = data;
				spriteaddress++;
			}

			break;
		case 0x2005: // PPU scrolling
			g_nesMainboard->GetSynchroniser()->Synchronise();

//			if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
			{
/*
I don't know anything about this game in particular, but bear in mind when examining
$2005 writes that changes to bits 0-2 take effect immediately, while changes to bits 3-7
(as well as bit 0 of $2000) don't have any effect until the start of the next H-Blank
(cycle 257 to be specific).
Thus, if $2005 were written in the middle of H-Blank, bits 0-2 will affect the next scanline,
but bits 3-7 won't take effect until the scanline afterwards.
Emulators that don't use cycle-accurate PPU rendering will not correctly handle this condition.
*/

				// bottom 3 bits are "fine" scroll value, top 5 are tile number
				// first write is horizontal value, second is vertical
				if ( !ppuSecondAddressWrite )
				{
					ppuLatchAddress &= 0xFFE0;
					ppuLatchAddress |= ((data & 0xF8) >> 3);
					fineX = (data & 0x07);
				}
				else
				{
					ppuLatchAddress &= 0xFC1F;
					ppuLatchAddress |= ((data & 0xF8) << 2);
					ppuLatchAddress &= 0x8FFF;
					ppuLatchAddress |= ((data & 0x07) << 12);
				}

				ppuSecondAddressWrite = !ppuSecondAddressWrite;
			}
			break;
		case 0x2006: // PPU memory address
			// first write is upper byte of address, second is lower
			g_nesMainboard->GetSynchroniser()->Synchronise();

		//	Log::Write( LOG_MISC, ( boost::format( "" ) ).str() );

//			if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
			{
				if ( !ppuSecondAddressWrite )
				{
					ppuControl1.nameTable = (data & 0x0C) >> 2;
					ppuLatchAddress &= 0x00FF;
					ppuLatchAddress |= ((data & 0x3F) << 8);
				}
				else
				{
					ppuLatchAddress &= 0xFF00;
					ppuLatchAddress |= data;
					
					//if ( IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), false ) &&
					//	( ppuReadAddress & ( 1 << 12 ) ) == 0 && ( ppuLatchAddress & ( 1 << 12 ) ) > 0 )
					//{
					//	g_nesMainboard->GetCartridge()->GetMemoryMapper()->PPUA12Latch();
					//}

					UpdatePPUReadAddress( ppuLatchAddress, true );
				}

				ppuSecondAddressWrite = !ppuSecondAddressWrite;
			}
		break;
		case 0x2007: // PPU memory data
			{
				g_nesMainboard->GetSynchroniser()->Synchronise();
				
				// TODO: should this be allowed during rendering?
				if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), false ) )
				{
					UInt16_t bufferedAddress = ppuReadAddress;

					// increment PPU address as according to bit 2 of 0x2000
					const UInt16_t newAddress = ppuReadAddress + ( ppuControl1.verticalWrite ? 32 : 1 );
					UpdatePPUReadAddress( newAddress, true );

					if ( !ppuStatus.vramWrite ) // check VRAM flag
					{
						Write8( bufferedAddress /*& 0x3FFF*/, data );
					}
				}
			}
		break;

		case 0x4014:
			g_nesMainboard->GetSynchroniser()->Synchronise();
			mDoSpriteTransferAfterNextCpuInstruction = true;
			mSpriteTransferArgument = data;
			break;
		default:
		break;
	}
}


void PictureProcessingUnit::HandleSpriteTransfer()
{
	/*
	SPR DMA should take 513 cycles if it starts on an even cycle, 514 if it starts on an odd cycle.

	Remember that SPR DMA only takes place when the instruction has finished executing, and not at the write cycle of the instruction! You can check this behavior by using a RMW instruction with $4014, it will only do one sprite dma at the end of the instruction.

	I m able to pass irq and dma, as well as both spr and dma tests using this implementation.
	*/
	if ( mDoSpriteTransferAfterNextCpuInstruction)
	{
		mDoSpriteTransferAfterNextCpuInstruction = false;
		// TODO: Optimise 
		const bool isOddFrame = (( g_nesMainboard->GetSynchroniser()->GetCpuMTC() / MASTER_CYCLES_PER_PPU ) % 2 ) > 0;
		g_nesMainboard->GetSynchroniser()->AdvanceCpuMTC( 1 * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() );
		for ( int i=0; i<0x100; ++i )
		{
			const Byte_t dmaData = g_nesMainboard->GetCPUMemory()->Read8( ( mSpriteTransferArgument * 0x100 ) + i );
			g_nesMainboard->GetSynchroniser()->AdvanceCpuMTC( 1 * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() );
			spriteMemory.Data[ spriteaddress /*(spriteaddress + i) % 0x100*/ ] = dmaData;
			spriteaddress = (spriteaddress + 1 ) % 0x100;
			mLastTransferredValue = dmaData;
			g_nesMainboard->GetSynchroniser()->AdvanceCpuMTC( 1 * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() );
		}
		// add extra cycle on odd frame
		if ( isOddFrame )
			g_nesMainboard->GetSynchroniser()->AdvanceCpuMTC( 1 * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() );

		// set CPU clock forward (DMA operation takes 513 ticks)
	//				g_nesMainboard->GetSynchroniser()->AdvanceCpuMTC( CPU_CYCLES_USED_FOR_SPRITE_DMA * g_nesMainboard->GetModeConstants()->MasterCyclesPerCPU() );
		g_nesMainboard->GetSynchroniser()->Synchronise();
	}
}


Byte_t PictureProcessingUnit::ReadFromRegister( UInt16_t offset )
{
	Byte_t ret = 0;

	switch ( offset )
	{
	//case 0x2000:
	//	g_nesMainboard->GetSynchroniser()->Synchronise();
	////	ret = *control1;
	//	ret = mLastTransferredValue;
	//	break;
	//case 0x2001:
	//	g_nesMainboard->GetSynchroniser()->Synchronise();
	////	ret = *control2;
	//	ret = mLastTransferredValue;
	//	break;
	case 0x2002:
	{
		g_nesMainboard->GetSynchroniser()->Synchronise( g_nesMainboard->GetSynchroniser()->GetCpuMTC() - MASTER_CYCLES_PER_PPU * 2 );
		// vblankTicks is the number of ticks till the end of the frame
		int vblankTicks = this->GetMasterTicksTillFrameDone( g_nesMainboard->GetSynchroniser()->GetCpuMTC() );

		bool suppress = false;

		// check that vblank flag isn't tried to be set 1 PPU clock before VBlank is due
		if ( vblankTicks == MASTER_CYCLES_PER_PPU )
		{
			suppressVblank = true;
			suppressNmi = true;
			suppress = true;
		}
		// Suppress NMI on the tick VBlank is read and the one after
		else if ( vblankTicks <= 0 && vblankTicks >= -MASTER_CYCLES_PER_PPU * 1 )
		{
			suppressNmi = true;
		}

		g_nesMainboard->GetSynchroniser()->Synchronise();

		ret = *status;
		ppuSecondAddressWrite = false; // reset latch on read to 0x2002

		if ( suppress )
			ret &= ~(1 << 7);
		else
			ppuStatus.vBlank = false;
	}
	break;

//	case 0x2003: // sprite memory address
//		g_nesMainboard->GetSynchroniser()->Synchronise();
//////		if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
////		{
////			ret = spriteaddress;
////		}
//		ret = mLastTransferredValue;
//		break;
	case 0x2004: // sprite memory data
		g_nesMainboard->GetSynchroniser()->Synchronise();
////		if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
//		{
			ret = spriteMemory.Data[ spriteaddress ];
//		}
//		ret = mLastTransferredValue;
		break;

	case 0x2007: // PPU memory data
	  // dont buffer reads from palette space
		{
			g_nesMainboard->GetSynchroniser()->Synchronise();
			UInt16_t bufferedaddress = ppuReadAddress;

			if ( !IsRendering( g_nesMainboard->GetSynchroniser()->GetCpuMTC(), true ) )
			{
				const UInt16_t newAddress = ppuReadAddress + (ppuControl1.verticalWrite ? 32 : 1);
				UpdatePPUReadAddress( newAddress, true );
			//		if ( ppuReadAddress > 0x3FFF )
			//			ppuReadAddress = 0;

				if ( IS_INT_BETWEEN( bufferedaddress, 0x3F00, 0x4000 ) )
				{
					ret = this->Read8( bufferedaddress );
					// reading from the palette: dont buffer return value, but update it with data from 0x2F00 -> 0x2FFF
					bufferedppuread = g_nesMainboard->GetPPU()->Read8( bufferedaddress - 0x1000 );
				}
				else
				{
					ret = bufferedppuread;
					bufferedppuread = this->Read8( bufferedaddress );
				}
			}
			else
			{
				ret = bufferedppuread;
				bufferedppuread = 0;
			}
		}

		break;

	//case 0x2005:
	//	Log::Write( LOG_ERROR, "Read from PPU register 0x2005 - Emulation may be inaccurate and problematic" );
	//	ret = mLastTransferredValue;
	//	throw std::runtime_error( "Read to 0x2005" );
	//case 0x2006:
	//	Log::Write( LOG_ERROR, "Read from PPU register 0x2006 - Emulation may be inaccurate and problematic" );
	//	ret = mLastTransferredValue;
	//	throw std::runtime_error( "Read to 0x2006" );
	default:
		ret = mLastTransferredValue;
		break;
	}

	mLastTransferredValue = ret;

	return ret;
}


/*
        Programmer Memory Map
      +---------+-------+-------+--------------------+
      | Address | Size  | Flags | Description        |
      +---------+-------+-------+--------------------+
      | $0000   | $1000 | C     | Pattern Table #0   |
      | $1000   | $1000 | C     | Pattern Table #1   |
      | $2000   | $3C0  |       | Name Table #0      |
      | $23C0   | $40   |  N    | Attribute Table #0 |
      | $2400   | $3C0  |  N    | Name Table #1      |
      | $27C0   | $40   |  N    | Attribute Table #1 |
      | $2800   | $3C0  |  N    | Name Table #2      |
      | $2BC0   | $40   |  N    | Attribute Table #2 |
      | $2C00   | $3C0  |  N    | Name Table #3      |
      | $2FC0   | $40   |  N    | Attribute Table #3 |
      | $3000   | $F00  |   R   |                    |
      | $3F00   | $10   |       | Image Palette #1   |
      | $3F10   | $10   |       | Sprite Palette #1  |
      | $3F20   | $E0   |    P  |                    |
      | $4000   | $C000 |     F |                    |
      +---------+-------+-------+--------------------+
                          C = Possibly CHR-ROM
                          N = Mirrored (see Subsection G)
                          P = Mirrored (see Subsection H)
                          R = Mirror of $2000-2EFF (VRAM)
                          F = Mirror of $0000-3FFF (VRAM)
*/

void PictureProcessingUnit::Write8( UInt16_t offset, Byte_t data )
{
	if ( offset < 0x2000 )
	{ // pattern tables
		g_nesMainboard->GetMemoryMapper()->Write8ChrRom( offset, data );
	}
	else if ( IS_INT_BETWEEN( offset, 0x2000, 0x3000 ) )
	{
		if ( mirroringMethod == PPU_MIRROR_CHRROM )
		{
			g_nesMainboard->GetMemoryMapper()->Write8NameTable( offset, data );
		}
		else
		{
			int nt = ( offset - 0x2000 ) / 0x400;
			assert( nameTablePointers[ nt ] != 0 );
			nameTablePointers[ nt ]->Data[ offset % 0x400 ] = data;
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0x3000, 0x3F00 ) )
	{ // Mirror of $2000-2EFF
		Write8( ( offset - 0x1000 ), data );
	}
	else if ( IS_INT_BETWEEN( offset, 0x3F00, 0x3F20 ) )
	{ // image and sprite palettes (both mirror each other)
		int paletteOffset = ( offset - 0x3F00 );

		if ( paletteOffset >= 0x10 )
		{
			paletteTables[1]->Data[ paletteOffset - 0x10 ] = data;
			if ( paletteOffset % 4 == 0 )
				paletteTables[0]->Data[ paletteOffset - 0x10 ] = data;
		}
		else
		{
			paletteTables[0]->Data[ paletteOffset ] = data;
			if ( paletteOffset % 4 == 0 )
				paletteTables[1]->Data[ paletteOffset ] = data;
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0x3F20, 0x4000 ) )
	{ // palette mirror
		Write8( ((offset - 0x3F20) % 0x20) + 0x3F00, data );
	}
	else // if ( offset >= 0x4000 /* && offset < 0x10000 */ )
	{ // Mirror of $0000-3FFF ( mirrored 3 times )
		Write8( offset % 0x4000, data );
	}
}


Byte_t PictureProcessingUnit::Read8( UInt16_t offset, bool dataOnly )
{
	if ( offset < 0x2000 )
	{ // pattern tables
		return g_nesMainboard->GetMemoryMapper()->Read8ChrRom( offset, dataOnly );
	}
	else if ( IS_INT_BETWEEN( offset, 0x2000, 0x3000 ) )
	{
		if ( mirroringMethod == PPU_MIRROR_CHRROM )
		{
			return g_nesMainboard->GetMemoryMapper()->Read8NameTable( offset, dataOnly );
		}
		else
		{
			int nt = ( offset - 0x2000 ) / 0x400;
			assert( nameTablePointers[ nt ] != 0 );
			return nameTablePointers[ nt ]->Data[ offset % 0x400 ];
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0x3000, 0x3F00 ) )
	{ // Mirror of $2000-2EFF (name tables)
		return Read8( ( offset - 0x1000 ), dataOnly );
	}
	else if ( IS_INT_BETWEEN( offset, 0x3F00, 0x3F20 ) )
	{	// palettes
		int paletteOffset = ( offset - 0x3F00 );

		if ( paletteOffset >= 0x10 )
		{
			return paletteTables[1]->Data[ paletteOffset - 0x10 ];
		}
		else
		{
			return paletteTables[0]->Data[ paletteOffset ];
		}
	}
	else if ( IS_INT_BETWEEN( offset, 0x3F20, 0x4000 ) )
	{ // palette mirror
		return Read8( ((offset - 0x3F00) % 0x20) + 0x3F00, dataOnly );
	}
	else //if ( offset >= 0x4000 )
	{ // Mirror of $0000-3FFF ( mirrored 3 times )
		return Read8( offset % 0x4000, dataOnly );
	}
}


void PictureProcessingUnit::Synchronise( int syncMTC )
{
	if ( masterTickCount >= syncMTC )
		return;

	while ( masterTickCount < syncMTC )
	{
		int nextPPUEvent = GetNextEvent( masterTickCount );
		int syncVal = min( nextPPUEvent, syncMTC );
		
		bool eventOccurred = false;
		int newMTC = 0;

		// Skip a PPU clock cycle if the background is enabled
		if ( GetMasterTicksTillClockSkip( 0 ) <= syncVal && GetMasterTicksTillClockSkip( 0 ) > masterTickCount )
		{
			RenderTo( GetMasterTicksTillClockSkip( 0 ) );
			newMTC = newMTC > 0 ? min( GetMasterTicksTillClockSkip( 0 ), newMTC ) : GetMasterTicksTillClockSkip( 0 );
			eventOccurred = true;

			if ( isOddFrame && ppuControl2.backgroundSwitch && g_nesMainboard->GetModeConstants()->Mode() == TERRITORY_NTSC )
			{
				masterTickCount += MASTER_CYCLES_PER_PPU;
				g_nesMainboard->GetSynchroniser()->AdvanceCpuMTC( MASTER_CYCLES_PER_PPU );
				g_nesMainboard->GetSynchroniser()->Synchronise();
			}
		}

		if ( GetMasterTicksTillVBlankClearDue( 0 ) <= syncVal && GetMasterTicksTillVBlankClearDue( 0 ) > masterTickCount )
		{
			RenderTo( GetMasterTicksTillVBlankClearDue( 0 ) );
			newMTC = newMTC > 0 ? min( GetMasterTicksTillVBlankClearDue( 0 ), newMTC ) : GetMasterTicksTillVBlankClearDue( 0 );
			eventOccurred = true;
			 
			// clear bits 5,6,7 in 0x2002
			*status &= 0x1F;

#ifdef LOG_PPU_EVENTS
			if ( Log::IsTypeEnabled( LOG_PPU ) )
			{
				Log::Write( LOG_PPU, L"VBlank cleared - rendering period started" );
			}
#endif
		}

		if ( mTriggerNmiNextClock )
		{
			if ( mTriggerNmiMTC <= syncVal && mTriggerNmiMTC > masterTickCount )
			{
				RenderTo( mTriggerNmiMTC );
				newMTC = newMTC > 0 ? min( mTriggerNmiMTC, newMTC ) : mTriggerNmiMTC;
				eventOccurred = true;

	//			Log::Write( LOG_MISC, ( boost::format( "mTriggerNmiMTC: %i syncVal: %i masterTickCount: %i" ) % mTriggerNmiMTC % syncVal % masterTickCount ).str() );

			//	if ( !suppressNmi && ( ppuControl1.vBlankNmi && ppuStatus.vBlank ) )
				if ( ( ppuControl1.vBlankNmi && ppuStatus.vBlank ) )
				{
					g_nesMainboard->GetProcessor()->NonMaskableInterrupt( mTriggerNmiMTC );
				}

				mTriggerNmiMTC = 0;
				mTriggerNmiNextClock = false;
			}
		}

		if ( eventOccurred )
		{
			masterTickCount = newMTC;
		}
		else
		{
			if ( masterTickCount < syncVal )
			{
				RenderTo( syncVal );
				masterTickCount = syncVal;
			}
		}
	}
}


void PictureProcessingUnit::OnEndFrame()
{
	// start vblank period
	if ( !suppressVblank )
	{
		ppuStatus.vBlank = true;
	}

	if ( forceNmi || (!suppressNmi && ppuControl1.vBlankNmi ) )
	{
		g_nesMainboard->GetProcessor()->NonMaskableInterrupt( GetMasterTicksTillFrameDone( 0 ) + MASTER_CYCLES_PER_PPU );
	}

	suppressNmi = false;
	suppressVblank = false;
	forceNmi = false;
	frameCount++;

#ifdef LOG_PPU_EVENTS
	if ( Log::IsTypeEnabled( LOG_PPU ) )
	{
		Log::Write( LOG_PPU, L"VBlank set - Frame finished" );
	}
#endif

	isOddFrame = !isOddFrame;
	masterTickCount = renderMTC = 0;

	ticksWhenToSetOverflow = -1;
}


int PictureProcessingUnit::GetNextEvent( int cpuMTC )
{
//	return cpuMTC + MASTER_CYCLES_PER_PPU;

	int nextScanline = ( ( cpuMTC / MASTER_CYCLES_PER_PPU  ) / PPU_TICKS_PER_SCANLINE ) + 1;
	int scanlineTicks = PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU * nextScanline;

	int vblankDue = GetMasterTicksTillVBlankClearDue( 0 );
	int frameEndDue = GetMasterTicksTillFrameDone( 0 );
	int clockSkipDue = GetMasterTicksTillClockSkip( 0 );
	
	std::deque< int > events;
	
	if ( mTriggerNmiNextClock && mTriggerNmiMTC > cpuMTC )
		events.push_back( mTriggerNmiMTC );
	if ( vblankDue > cpuMTC )
		events.push_back( vblankDue );
	if ( frameEndDue > cpuMTC )
		events.push_back( frameEndDue );
//	if ( scanlineTicks > cpuMTC )
//		events.push_back( scanlineTicks );
	if ( clockSkipDue > cpuMTC )
		events.push_back( clockSkipDue );

	std::sort( events.begin(), events.end(), std::less< int >() );

	return events.empty() ? scanlineTicks : events[0];
}


void PictureProcessingUnit::OnReset( bool cold )
{
	SetDefaultVariables( cold );
}


void PictureProcessingUnit::SaveState( std::ostream& ostr )
{
	for ( int i=0; i<2; ++i )
		nameTables[i].SaveState( ostr );

	for ( int i=0; i<2; ++i )
		fsNameTables[i].SaveState( ostr );

	for ( int i=0; i<2; ++i )
		paletteTables[i]->SaveState( ostr );

	spriteMemory.SaveState( ostr );

	WriteType< boost::uint32_t >( ostr, masterTickCount );
	WriteType< boost::uint32_t >( ostr, mirroringMethod );
	WriteType< boost::uint8_t >( ostr, isOddFrame ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, suppressNmi ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, suppressVblank ? 1 : 0 );
	WriteType< boost::uint8_t >( ostr, forceNmi ? 1 : 0 );

	WriteType< boost::uint8_t >( ostr, *control1 );
	WriteType< boost::uint8_t >( ostr, *control2 );
	WriteType< boost::uint8_t >( ostr, *status );

	WriteType< boost::uint8_t >( ostr, bufferedppuread );
	WriteType< boost::uint16_t >( ostr, ppuReadAddress );
	WriteType< boost::uint16_t >( ostr, ppuLatchAddress );

	WriteType< boost::uint8_t >( ostr, spriteaddress );
	WriteType< boost::uint8_t >( ostr, ppuSecondAddressWrite ? 1 : 0 );

	WriteType< boost::uint32_t >( ostr, fineX );
	WriteType< boost::int32_t >( ostr, renderMTC );
	WriteType< boost::int32_t >( ostr, ticksWhenToSetOverflow );

	// background stuff
	ostr.write( reinterpret_cast< char* >( renderScanline ),  34 * 8 * sizeof( Byte_t ) );
	ostr.write( reinterpret_cast< char* >( priorityBuffer ),  34 * 8 * sizeof( boost::uint16_t ) );

	// foreground
	WriteType< boost::int32_t >( ostr, nextScanlineSpritesCount );
	ostr.write( reinterpret_cast< char* >( nextScanlineSprites ), 64 * 4 * sizeof( Byte_t ) );
	ostr.write( reinterpret_cast< char* >( nextScanlineSpritePriority ), 64 * sizeof( Byte_t ) );

	WriteType< boost::int32_t >( ostr, mTriggerNmiMTC );
	WriteType< boost::uint8_t >( ostr, mTriggerNmiNextClock ? 1 : 0 );
}


void PictureProcessingUnit::LoadState( std::istream& istr )
{
	for ( int i=0; i<2; ++i )
		nameTables[i].LoadState( istr );

	for ( int i=0; i<2; ++i )
		fsNameTables[i].LoadState( istr );

	for ( int i=0; i<2; ++i )
		paletteTables[i]->LoadState( istr );

	spriteMemory.LoadState( istr );

	masterTickCount = ReadType< boost::uint32_t >( istr );
	mirroringMethod = (PPU_MIRRORING_METHOD)ReadType< boost::uint32_t >( istr );

	isOddFrame = ReadType< boost::uint8_t >( istr ) > 0;
	suppressNmi = ReadType< boost::uint8_t >( istr ) > 0;
	suppressVblank = ReadType< boost::uint8_t >( istr ) > 0;
	forceNmi = ReadType< boost::uint8_t >( istr ) > 0;

	*control1 = ReadType< boost::uint8_t >( istr );
	*control2 = ReadType< boost::uint8_t >( istr );
	*status = ReadType< boost::uint8_t >( istr );

	bufferedppuread = ReadType< boost::uint8_t >( istr );
	ppuReadAddress = ReadType< boost::uint16_t >( istr );
	ppuLatchAddress = ReadType< boost::uint16_t >( istr );

	spriteaddress = ReadType< boost::uint8_t >( istr );
	ppuSecondAddressWrite = ReadType< boost::uint8_t >( istr ) > 0;

	fineX = ReadType< boost::uint32_t >( istr );
	renderMTC = ReadType< boost::int32_t >( istr );
	ticksWhenToSetOverflow = ReadType< boost::int32_t >( istr );

	// dont forget to reset mirroring
	this->ChangeMirroringMethod( mirroringMethod );

	// background stuff
	istr.read( reinterpret_cast< char* >( renderScanline ), 34 * 8 * sizeof( Byte_t ) );
	istr.read( reinterpret_cast< char* >( priorityBuffer ), 34 * 8 * sizeof( boost::uint16_t ) );

	// foreground
	nextScanlineSpritesCount = ReadType< boost::int32_t >( istr );
	istr.read( reinterpret_cast< char* >( nextScanlineSprites ), 64 * 4 * sizeof( Byte_t ) );
	istr.read( reinterpret_cast< char* >( nextScanlineSpritePriority ), 64 * sizeof( Byte_t ) );

	mTriggerNmiMTC = ReadType< boost::int32_t >( istr );
	mTriggerNmiNextClock = ReadType< boost::uint8_t >( istr ) > 0;
}


Byte_t PictureProcessingUnit::GetBackgroundPaletteIndex() const
{
	return paletteTables[ 0 ]->Data[ 0 ];
}




#include "ppu.h"
#include "mainboard.h"
#include <cmath>


#include "wx/wx.h"


using namespace std;



void PictureProcessingUnit::RenderTo( int syncMTC )
{
	if ( renderMTC < syncMTC )
	{
//		PWLOG2( LOG_MISC, "RenderTo: renderMTC: %1% syncMTC: %2%", renderMTC, syncMTC );

		if ( IsRenderingEnabled() )
		{
			// tickLimit is the start of the rendering frame - only started being clocked when rendering
			int tickLimit = g_nesMainboard->GetModeConstants()->VBlankScanlines() * ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU );

			if ( syncMTC > tickLimit )
			{
				if ( renderMTC < tickLimit )
					renderMTC = tickLimit;

				int startmtc = renderMTC % ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ); // ticks from the start of the current scanline
				int startline = ( renderMTC / ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ) ) - g_nesMainboard->GetModeConstants()->VBlankScanlines() - 1; // starting scanline

				int endmtc = syncMTC % ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ); // ticks from the start of the last scanline
				int endline = ( syncMTC / ( PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU ) ) - g_nesMainboard->GetModeConstants()->VBlankScanlines() - 1; // ending scanline

//				PWLOG4( LOG_MISC, "startmtc: %1% startline: %2% endmtc: %3% endline: %4%", startmtc, startline, endmtc, endline );

				if ( startline == endline )
				{
					RenderPartialScanline( startline, startmtc, endmtc, syncMTC );
				}
				else
				{
					RenderPartialScanline( startline, startmtc, PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU, syncMTC );

					for ( int scanline = startline+1; scanline < endline; ++scanline )
					{
						RenderPartialScanline( scanline, 0, PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU, syncMTC );
					}

					RenderPartialScanline( endline, 0, endmtc, syncMTC );
				}
			}
		}

		renderMTC = syncMTC;
	}
}


void PictureProcessingUnit::ClearRenderScanlineWithBGColour()
{
	Byte_t bgIndex = GetBackgroundPaletteIndex();

	memset( renderScanline, bgIndex, sizeof( Byte_t ) * 34 * 8 );

	for ( int i=0; i<34*8; ++i )
	{
		priorityBuffer[i] = RENDER_BACKGROUND_TRANSPARENT;
	}
}


void PictureProcessingUnit::RenderPartialScanline( int scanline, int xstart, int xend, int tickLimit )
{
	if ( ! ( xstart < xend && scanline >= -1 && scanline < 240 ) )
		return;

//	PWLOG4( LOG_MISC, "RenderPartialScanline: scanline: %1% xstart: %2% xend: %3% tickLimit: %4%", scanline, xstart, xend, tickLimit );

	const int ScrollReloadTime = 304 * MASTER_CYCLES_PER_PPU; // pre-render SL only
	const int XReloadTime = 257 * MASTER_CYCLES_PER_PPU;
	const int SecondLastTileReloadTime = 324 * MASTER_CYCLES_PER_PPU;
	const int LastTileReloadTime = 332 * MASTER_CYCLES_PER_PPU;
	const int XIncrementCycle = 3 * MASTER_CYCLES_PER_PPU;
	const int YIncrementTime = 251 * MASTER_CYCLES_PER_PPU;

	EvaluateSprites( scanline, xstart, xend, tickLimit );

	if ( scanline == -1 )
	{
		if ( xend >= ScrollReloadTime && xstart < ScrollReloadTime )
		{ // reset ppu address on cycle 304 of pre-render scanline
			UpdatePPUReadAddress( this->ppuLatchAddress, false );
		}
	}
	else
	{
		for ( int tiles=0; tiles<32; ++tiles )
		{ // TODO: optimise
			int ticks = ( tiles * 8 * MASTER_CYCLES_PER_PPU ) + XIncrementCycle;
			if ( xend >= ticks && xstart < ticks )
			{
				Background_PrefetchTile( tiles + 2 );
				Background_IncrementXTile();
			}
		}

		// render last tile on screen, increment Y
		if ( xend >= YIncrementTime && xstart < YIncrementTime )
		{
			Background_IncrementYTile();
		}

		Background_RenderTileLine( scanline, xstart, xend );

		if ( xend >= XReloadTime && xstart < XReloadTime )
		{
			// x scroll is set to what is in the temp address
			UpdatePPUReadAddress( (ppuReadAddress & ~0x041F) | (ppuLatchAddress & 0x041F), false );
		}
	}

	// end of scanline prefetch (happens on all scanlines from -1 to 240
	if ( xend >= SecondLastTileReloadTime && xstart < SecondLastTileReloadTime )
	{
		ClearRenderScanlineWithBGColour();

		Background_PrefetchTile( 0 );
		Background_IncrementXTile();
	}

	if ( xend >= LastTileReloadTime && xstart < LastTileReloadTime )
	{
		Background_PrefetchTile( 1 );
		Background_IncrementXTile();
	}
}



Byte_t PictureProcessingUnit::ReadNameTable( UInt16_t address )
{
	return nameTablePointers[(address >> 10) & 3]->Data[(address & 0x3FF)];
}


void PictureProcessingUnit::Background_PrefetchTile( int tilenum )
{
//	Log::Write( LOG_MISC, ( boost::format( "TILENUM: %1%" ) % tilenum ).str() );

	int baseindex = tilenum * 8;

	int htile = (ppuReadAddress & 0x001F);
	int vtile = (ppuReadAddress & 0x03E0) >> 5;
	int finey = ((ppuReadAddress & 0x7000) >> 12);

	UInt16_t nameTableAddress = 0x2000 + (ppuReadAddress & 0x0FFF);
	Byte_t tileNumber = ReadNameTable( nameTableAddress );

	// (screen address) + (tilenumber * 16) + finey
	UInt16_t tileAddress = ( ppuControl1.screenPatternTableAddress ? 0x1000 : 0 ) + tileNumber * 16 + ((ppuReadAddress & 0x7000) >> 12);
	
	Byte_t attributeByte = ReadNameTable( 0x23C0 + (ppuReadAddress & 0x0C00) + ((vtile & 0xFFFC) << 1) + (htile >> 2) );

	Byte_t firstByte = Read8( tileAddress );
	Byte_t secondByte = Read8( tileAddress + 8 );

	for ( int x=0; x<8; ++x )
	{
		Byte_t patternMask = 0x80 >> x;

		int paletteIndex = ( firstByte & patternMask ) ? 1 : 0;
		paletteIndex |= ( secondByte & patternMask ) ? 2 : 0;

		if ( paletteIndex > 0 )
		{
			int modtilex = htile % 4;
			int modtiley = vtile % 4;

			if ( modtilex < 2 && modtiley < 2 )
				paletteIndex |= ( attributeByte & ( 3 << 0 ) ) << 2;
			else if ( modtilex >= 2 && modtiley < 2 )
				paletteIndex |= ( attributeByte & ( 3 << 2 ) );
			else if ( modtilex < 2 && modtiley >= 2 )
				paletteIndex |= ( attributeByte & ( 3 << 4 ) ) >> 2;
			else if ( modtilex >= 2 && modtiley >= 2 )
				paletteIndex |= ( attributeByte & ( 3 << 6 ) ) >> 4;

			priorityBuffer[ baseindex + x ] = RENDER_BACKGROUND;

			if ( paletteIndex % 4 == 0 )
				paletteIndex = 0;

			renderScanline[ baseindex + x ] = paletteTables[ 0 ]->Data[ paletteIndex ];
		}
		//else
		//	renderScanline[ baseindex + x ] = GetBackgroundPaletteIndex();
	}
}



void PictureProcessingUnit::Background_RenderTileLine( int scanline, int xstart, int xend )
{
	xstart /= MASTER_CYCLES_PER_PPU;
	xend /= MASTER_CYCLES_PER_PPU;

	for ( int i=xstart+1; i<=xend; ++i )
	{
		if ( i >= 0 && i <= ( 34 * 8 ) )
		{
			int canvasx = i - fineX;

			if ( ppuControl2.backgroundSwitch )
			{
				if ( !( !ppuControl2.backgroundClipping && canvasx < 8 ) && canvasx < 256 && canvasx >= 0 )
				{
					if ( g_nesMainboard->GetNesRenderBuffer()->RenderPixel( priorityBuffer[ i ], 0, canvasx, scanline, renderScanline[ i ] ) )
					{
						ppuStatus.spriteHit = true;

			//#ifdef LOG_PPU_EVENTS
			//			if ( Log::IsTypeEnabled( LOG_PPU ) )
			//			{
			//				Log::Write( LOG_PPU, ( boost::format( "Sprite Zero hit: TICKS: %6$X SL: %1% XS: %2% XE: %3% XX: %4% CANVASX: %5%" )
			//					% scanline % xstart % xend % i % canvasx % g_nesMainboard->GetSynchroniser()->GetCpuMTC() ).str() );
			//			}
			//#endif
					}
				}
			}
		}
	}
}


void PictureProcessingUnit::Background_IncrementXTile()
{
	if ((ppuReadAddress & 0x001F) == 0x001F)
	{   
		// switch name tables (bit 10) and reset tile x to 0
		const UInt16_t newAddress = ( ppuReadAddress ^ 0x0400 ) & 0xFFE0;
		UpdatePPUReadAddress( newAddress, false );
	}
	else
	{   
		// next tile
		UpdatePPUReadAddress( ppuReadAddress + 1, false );
	}
}


void PictureProcessingUnit::Background_IncrementYTile()
{
	UInt16_t newAddress = ppuReadAddress;

	if ((newAddress & 0x7000) == 0x7000)
	{
		// wrap when tile y offset = 7
		newAddress &= ~0x7000;

		if ((newAddress & 0x03E0) == 0x03A0)
		{
			// wrap tile y and switch name table bit 11, if tile y is 29
			newAddress ^= 0x0800; 
			newAddress &= 0xFC1F;
		}
		else if ((newAddress & 0x03E0) == 0x03E0)
		{
			// wrap tile y if it is 31
			newAddress &= 0xFC1F; 
		}
		else
		{
			// just increment tile y
			newAddress += 0x0020;
		}
	}
	else
	{
		// increment tile y offset
		newAddress += 0x1000; 
	}

	if ( newAddress != ppuReadAddress )
		UpdatePPUReadAddress( newAddress, false );
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void PictureProcessingUnit::RenderSprite( int scanline, int spritenum, bool extraSprite )
{
	Byte_t sy = spriteMemory.Data[ spritenum*4 ] + 1;
	Byte_t patternnum = spriteMemory.Data[ spritenum*4 + 1 ];
	Byte_t attribs = spriteMemory.Data[ spritenum*4 + 2 ];
	Byte_t sx = spriteMemory.Data[ spritenum*4 + 3 ];

	bool behindBackground = IS_BIT_SET( attribs, 5 );
	bool flipHorz = IS_BIT_SET( attribs, 6 );
	bool flipVert = IS_BIT_SET( attribs, 7 );

	UInt16_t ppuAddress;

	if ( !ppuControl1.spriteSize )
	{
		ppuAddress = ( patternnum * 16 ) + ( ( flipVert ? 7 - ( scanline - sy ) : scanline - sy ) & 0x7 ) + ( ppuControl1.spritePatternTableAddress ? 0x1000 : 0 );
	}
	else // big sprites - if sprite num is even, use 0x0 else use 0x1000
	{
		ppuAddress = ((patternnum & 0xFE) * 16) + ((patternnum & 0x01) * 0x1000);

		bool topsprite = IS_INT_BETWEEN( scanline, sy, sy + 8 );

		if ( !topsprite )
		{ // on flipped, put top sprite on bottom & vis versa
			ppuAddress += ( flipVert ? 0 : 16 );
			ppuAddress += ( ( flipVert ? 7 - ( scanline - sy - 8 ) : scanline - sy - 8 ) /*& 0x7*/ );
		}
		else
		{
			ppuAddress += ( flipVert ? 16 : 0 );
			ppuAddress += ( ( flipVert ? 7 - ( scanline - sy ) : scanline - sy ) /*& 0x7*/ );
		}
	}

	Byte_t firstByte = Read8( ppuAddress, extraSprite );
	Byte_t secondByte = Read8( ppuAddress + 8, extraSprite );

	for ( int x=0; x<8; ++x )
	{
		int mask = 0x80 >> ( flipHorz ? 7 - x : x );
		int absx = x + sx;

		// check sprite clipping
		if ( ( !ppuControl2.spriteClipping && absx < 8 ) || absx > 255 )
			continue;

		// get 2 lower bits from the pattern table for the colour index
		int paletteindex = ( firstByte & mask ) > 0 ? 1 : 0; // first bit
		paletteindex |= ( secondByte & mask ) > 0 ? 2 : 0; // second bit

		// add 2 upper bits
		if ( paletteindex > 0 )
		{
			paletteindex |= ((attribs & 3) << 2);

			boost::uint16_t priority = ( extraSprite ? RENDER_SPRITE_EXTRA : RENDER_SPRITE ) | ( behindBackground ? RENDER_SPRITE_BEHIND : 0 ) | ( spritenum == 0 ? RENDER_SPRITE_ZERO : 0 );
			g_nesMainboard->GetNesRenderBuffer()->RenderPixel( priority, spritenum + 1, absx, scanline, paletteTables[ 1 ]->Data[ paletteindex ] );

			//if ( spritenum == 0 )
			//	g_nesMainboard->GetRenderBuffer()->RenderPixel( RENDER_DEBUG_1, 0, absx, scanline, 0 );
		}

		//if ( spritenum == 0 )
		//{
		//	g_nesMainboard->GetRenderBuffer()->RenderPixel( RENDER_DEBUG_1, 0, absx, scanline, 0 );
		//}
	}
}


void PictureProcessingUnit::EvaluateSprites( int pscanline, int xstart, int xend, int tickLimit )
{
	if ( pscanline < -1 || pscanline > 239 )
		return;

	const int spriteEvaluationStart = 64 * MASTER_CYCLES_PER_PPU;

	int spriteHeight = ppuControl1.spriteSize ? 16 : 8;
	int nextScanlineSpritesCount = 0;
	int readFromY = 0;

	int scanline = pscanline + 1;

	if ( spriteEvaluationStart <= xend && spriteEvaluationStart > xstart )
	{
		for ( int spritenum=0; spritenum < 64; ++spritenum )
		{
			if ( !ppuStatus.spriteOverflow && ticksWhenToSetOverflow < 0 && nextScanlineSpritesCount >= 8 )
			{ // bug in ppu, must emulate for purposes of calculating correct overflow set time
				Byte_t overflowSpriteY = spriteMemory.Data[ spritenum * 4 + readFromY ];
				readFromY++;
				if ( readFromY >= 4 )
					readFromY = 0;

				if ( overflowSpriteY >= 0 && overflowSpriteY < 240 && IS_INT_BETWEEN( scanline, overflowSpriteY + 1, overflowSpriteY + spriteHeight + 1 ) )
				{
					// ( number of ticks since start of frame to the current scanline (-1 because this function predicts the scanline ahead, +1 for pre-render scanline
					// and last one of vblank) + ( 64 ppu ticks into scanline when evaluation begins )
					// + ( 2 ppu ticks per sprite evaluated ) + ( 6 ppu ticks per sprite found )
					ticksWhenToSetOverflow = ( (scanline - 1 + 1 + g_nesMainboard->GetModeConstants()->VBlankScanlines()) * PPU_TICKS_PER_SCANLINE * MASTER_CYCLES_PER_PPU )
						+ ( 64 * MASTER_CYCLES_PER_PPU )
						+ ( spritenum * 2 * MASTER_CYCLES_PER_PPU ) + ( 8 * 6 * MASTER_CYCLES_PER_PPU );

					//#ifdef LOG_PPU_EVENTS
					//					if ( Log::IsTypeEnabled( LOG_PPU ) )
					//						Log::Write( LOG_PPU, ( boost::format( "Overflow ticks set. SL: %1% Time: %2%" ) % scanline % ticksWhenToSetOverflow ).str() );
					//#endif
				}
			}

			Byte_t spritey = spriteMemory.Data[ spritenum * 4 ];

				//#ifdef LOG_PPU_EVENTS
				//				if ( Log::IsTypeEnabled( LOG_PPU ) )
				//					Log::Write( LOG_PPU, ( boost::format( "Evaluating sprite #%1% Y: %2%" ) % spritenum % (int)spritey ).str() );
				//#endif

			if ( spritey >= 0 && spritey < 240 && IS_INT_BETWEEN( scanline, spritey + 1, spritey + spriteHeight + 1 ) )
			{
				const bool moreThanEightSprites = nextScanlineSpritesCount >= 8;
				if ( ppuControl2.spritesVisible && scanline < 240 )
					RenderSprite( scanline, spritenum, moreThanEightSprites );
				nextScanlineSpritesCount++;

				//#ifdef LOG_PPU_EVENTS
				//				if ( Log::IsTypeEnabled( LOG_PPU ) )
				//					Log::Write( LOG_PPU, ( boost::format( "Sprite found. Num: %1% SL: %2%" ) % spritenum % scanline ).str() );
				//#endif
			}
		}
	}

	if ( ticksWhenToSetOverflow > 0 && ticksWhenToSetOverflow <= tickLimit && ticksWhenToSetOverflow > renderMTC )
	{
		ppuStatus.spriteOverflow = true;
		ticksWhenToSetOverflow = -1;
	}
}


//////////////////////////////////////////////////////


//void PictureProcessingUnit::WriteOutCHRRam(  )
//{
//	Byte_t* imagedata = (Byte_t*)malloc( 32 * 8 * 34 * 8 * 3 );
//
//	WriteOutPatternTable( 0, imagedata, 0, 0 );
//	WriteOutPatternTable( 1, imagedata, 16*8, 0 );
//
//	wxImage* image = new wxImage( 8 * 32, 8 * 34, true );
//	image->SetData( imagedata );
//	if ( !image->SaveFile( "c:\\test.bmp", wxBITMAP_TYPE_BMP ) )
//	{
//	}
//	image->Destroy();
//}
//
//
//void PictureProcessingUnit::WriteOutPatternTable( int table, Byte_t* imagedata, int offsetx, int offsety )
//{
//	UInt16_t tileNumber = 0;
//
//	for ( int v=0; v < 16; ++v )
//	{
//		for ( int h=0; h < 16; ++h )
//		{
//			//	UInt16_t nameTableAddress = 0x2000 + v * 32 + h;
//			//	Byte_t tileNumber = ReadNameTable( nameTableAddress );
//
//			// (screen address) + (tilenumber * 16) + finey
//			for ( int y=0; y<8; ++y )
//			{
//				UInt16_t tileAddress = ( table > 0 ? 0x1000 : 0 ) + tileNumber * 16 + y;
//
//				//	Byte_t attributeByte = Read8( 0x23C0 + (ppuReadAddress & 0x0C00) + ((vtile & 0xFFFC) << 1) + (htile >> 2), true );
//
//				for ( int x=0; x<8; ++x )
//				{
//					Byte_t patternMask = 0x80 >> x;
//
//					int paletteIndex = ( Read8( tileAddress /*+ y*/ ) & patternMask ) ? 1 : 0;
//					paletteIndex |= ( Read8( tileAddress /*+ y*/ + 8 ) & patternMask ) ? 2 : 0;
//
//					//	if ( paletteIndex > 0 )
//					{
//						Byte_t r, g, b;
//						//	g_nesMainboard->GetPalette()->GetColour( GetPaletteIndexAt( 0, paletteIndex ), r, g, b );
//						switch ( paletteIndex )
//						{
//						case 0:
//							r = g = b = 0;
//							break;
//						case 1:
//							r = 255;
//							g = b = 0;
//							break;
//						case 2:
//							g = 255;
//							r = b = 0;
//							break;
//						case 3:
//							b = 255;
//							g = r = 0;
//							break;
//						}
//
//						int index = ( v * 8 + y + offsety ) * ( 256*3 ) + ( h * 8 + x + offsetx ) * 3;
//
//						imagedata[index] = r;
//						imagedata[index] = g;
//						imagedata[index] = b;
//					}
//				}
//			}
//
//			tileNumber++;
//		}
//	}
//}

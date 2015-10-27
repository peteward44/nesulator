
#pragma once

#ifndef SNESPPU_H
#define SNESPPU_H

#include "snessynchroniser.h"



class SnesPpu : public SnesSynchronisable, public HasState, private boost::noncopyable
{
	typedef Page< 0x10000, 0 > Vram_t;
	typedef Page< 0x220, 0 > Oam_t;
	typedef Page< 0x200, 0 > CgRam_t;

	/* The main data area is the VRAM, a 64 KB memory space that can be accessed with registers $2115, $2116, $2118, and $2139. 
		This area is used for storing all the tiles used in your game, as well as the tile maps. */
	Vram_t mVram;  

	 /* The second data area, the OAM, is used to store properties of the sprites. It includes information about position, size, priority, etc.
		There can be 128 objects maximum, and the memory is 544 bytes: the first 512 bytes have four bytes of information per sprite,
		and the last 32 bytes have two more bits of information.  Two or more sprites can share the same set of tiles. */
	Oam_t mOam;

	/* The third area is the CGRAM, where the palette data is stored. It is 512 bytes: two bytes, and fifteen bits, for each of the 256 on-screen colors.
		There are five bits for each primary color (red, green, and blue). */
	CgRam_t mCgRam;

	bool mScreenEnabled;
	UInt8_t mBrightness; // value from 0->F

	bool mSpritesEnabled;
	UInt8_t mSpriteSize;
	UInt8_t mSpriteNameSelection;
	UInt8_t mSpriteBaseSelection;
	UInt16_t mSpriteDataAddress;
	UInt8_t mSpriteLatchData;
	bool mSpritePriorityActivated;

	UInt16_t mCgAddress;
	Byte_t mCgLatchData;

	void WriteCgRam( UInt16_t address, Byte_t data );

	UInt16_t GetVramAddress() const;
	void WriteToVram( UInt16_t address, Byte_t data );
	Byte_t ReadVram( UInt16_t address );

	bool IsInVBlank() const;
	bool IsInHBlank() const;
	bool IsJoypadReady() const;


	enum BackgroundTileSize
	{
		BGTILE_32x32 = 0,
		BGTILE_64x32 = 1,
		BGTILE_32x64 = 2,
		BGTILE_64x64 = 3
	};

	// This struct used for each of the backgrounds (BG1-4)
	struct BackgroundTileInfo
	{
		BackgroundTileInfo() : mEnabled( true ), mUse16x16Tiles( false ), mScreenPixelation( 0 ), mTileMapBaseVramAddress( 0 ), mCharacterDataBaseVramAddress( 0 ), mSCSize( BGTILE_32x32 ) {}

		bool mEnabled;
		bool mUse16x16Tiles;
		UInt8_t mScreenPixelation; // MOSAIC: 0 smallest, 0xF largest
		UInt8_t mTileMapBaseVramAddress;
		UInt8_t mCharacterDataBaseVramAddress;
		BackgroundTileSize mSCSize; // SC size: 00=32x32 01=64x32 10=32x64 11=64x64
	};

	BackgroundTileInfo mBackgroundInfo[4];
	bool mBackground3Priority;
	UInt8_t mBackgroundMode;

	bool mVramAddressIncrementMode;
	UInt8_t mVramAddressFormation;
	UInt8_t mVramAddressIncrementSize;

	UInt16_t mVramAddress;
	UInt8_t mVramReadBuffer;

	bool mFastCycleModeActive;

	void RenderTo( int syncMTC );
	void RenderPartialScanline( int scanline, int xstart, int xend, int tickLimit );

public:
	SnesPpu();
	~SnesPpu();

	UInt8_t Read8( UInt8_t bankRegister, UInt16_t address );
	void Write8( UInt8_t bankRegister, UInt16_t address, UInt8_t value );

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );

	virtual void Synchronise( int syncMTC );
	virtual int GetNextEvent( int cpuMTC );
	virtual void OnEndFrame();

};


#endif


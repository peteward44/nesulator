

#ifndef PICTUREPROCESSINGUNIT_H
#define PICTUREPROCESSINGUNIT_H


#include "main.h"
#include <queue>
#include "synchroniser.h"

// Picture Processing Unit: Does all graphics related processing

/*: VRAM memory table

    +---------+-------+-------+--------------------+
    | Address | Size  | Flags | Description        |
    +---------+-------+-------+--------------------+
    | $0000   | $1000 | C     | Pattern Table #0   |
    | $1000   | $1000 | C     | Pattern Table #1   |
    | $2000   | $3C0  |       | Name Table #0      |
    | $23C0   | $40   |       | Attribute Table #0 |
    | $2400   | $3C0  |  M    | Name Table #1      |
    | $27C0   | $40   |       | Attribute Table #1 |
    | $2800   | $3C0  |  M    | Name Table #2      |
    | $2BC0   | $40   |       | Attribute Table #2 |
    | $2C00   | $3C0  |  M    | Name Table #3      |
    | $2FC0   | $40   |       | Attribute Table #3 |
    | $3000   | $F00  |   U   |                    |
    | $3F00   | $10   |       | Image Palette #1   |
    | $3F10   | $10   |       | Sprite Palette #1  |
    | $3F20   | $E0   |    P  | Palette Mirror     |
    +---------+-------+-------+--------------------+
           Flag Legend: C = Possibly CHR-ROM
                        M = Mirrored (see Subsection G)
                        P = Mirrored (see Subsection H)
                        U = Unused

*/

typedef Page< 0x400, 0 > PatternTable_t;
typedef boost::shared_ptr< PatternTable_t > PatternTablePtr_t;
typedef Page< 0x400, 0 > NameTable_t;
typedef Page< 0x10, 0 > PaletteTable_t;


/*
        Name                       NT#0   NT#1   NT#2   NT#3   Flags
      +--------------------------+------+------+------+------+-------+
      | Horizontal               | $000 | $000 | $400 | $400 |       | 
      | Vertical                 | $000 | $400 | $000 | $400 |       | 
      | Four-screen              | $000 | $400 | $800 | $C00 | F     | 
      | Single-screen            |      |      |      |      |  S    | 
      | CHR-ROM mirroring        |      |      |      |      |   C   |
      +--------------------------+------+------+------+------+-------+
        F = Four-screen mirroring relies on an extra 2048 ($800) of RAM
            (kept on the cart), resulting in four (4) physical independent
            Name Tables.
        S = Single-screen games have mappers which allow you to select
            which PPU RAM area you want to use ($000, $400, $800, or
            $C00); all the NTs point to the same PPU RAM address.
        C = Mapper #68 (Afterburner 2) allows you to map CHR-ROM to the
            Name Table region of the NES's PPU RAM area. Naturally this
            makes the Name Table ROM-based, and one cannot write to it.
            However, this feature can be controlled via the mapper itself,
            allowing you to enable or disable this feature.
*/

enum PPU_MIRRORING_METHOD
{
	PPU_MIRROR_NONE,
	PPU_MIRROR_HORIZONTAL, // default
	PPU_MIRROR_VERTICAL,
	PPU_MIRROR_FOURSCREEN,
	PPU_MIRROR_SINGLESCREEN_NT0,
	PPU_MIRROR_SINGLESCREEN_NT1,
	PPU_MIRROR_CHRROM, // special case for mapper #68
};





class PictureProcessingUnit : public Synchronisable, public HasState
{
	Byte_t mLastTransferredValue;

public:
	typedef Page< 0x100, 0 > SpriteMemoryPage_t;

private:
	boost::signals::connection resetConnection;

	NameTable_t nameTables[ 2 ];
	NameTable_t fsNameTables[ 2 ]; // in case of four-screen mirroring, needs extra 0x800 RAM
	NameTable_t* nameTablePointers[ 4 ];
	PaletteTable_t* paletteTables[2];

	SpriteMemoryPage_t spriteMemory;

	int masterTickCount, renderMTC;
	unsigned int frameCount;
	PPU_MIRRORING_METHOD mirroringMethod;
	bool isOddFrame, suppressNmi, suppressVblank, forceNmi;
	bool mTriggerNmiNextClock;
	int mTriggerNmiMTC;

	void UpdatePPUReadAddress( UInt16_t newAddress, bool invokedFromRegisterWrite );

	/////////////////////////////////////
public:

	struct
	{
		unsigned char nameTable : 2;
		bool verticalWrite : 1;
		bool spritePatternTableAddress : 1;
		bool screenPatternTableAddress : 1;
		bool spriteSize : 1;
		bool hitSwitch : 1;
		bool vBlankNmi : 1;
	} ppuControl1;

	struct
	{
		unsigned char unknown : 1;
		bool backgroundClipping : 1;
		bool spriteClipping : 1;
		bool backgroundSwitch : 1;
		bool spritesVisible : 1;
		unsigned char unknown2 : 3;
	}  ppuControl2;

	struct
	{
		unsigned char unknown : 4;
		bool vramWrite : 1;
		bool spriteOverflow : 1;
		bool spriteHit : 1;
		bool vBlank : 1;
	} ppuStatus;


	Byte_t *control1, *control2, *status;

private:
	/////////////////////////////////////

	Byte_t bufferedppuread;
	UInt16_t ppuReadAddress, ppuLatchAddress;
	Byte_t spriteaddress;
	bool ppuSecondAddressWrite;
	int fineX;

	void RenderTo( int syncMTC );

	//////////////
	// background

	Byte_t renderScanline[ 34 * 8 ];
	boost::uint16_t priorityBuffer[ 34 * 8 ];

	void ClearRenderScanlineWithBGColour();

	void Background_PrefetchTile( int tilenum );
	void Background_RenderTileLine( int scanline, int xstart, int xend );

	void Background_IncrementXTile();
	void Background_IncrementYTile();

	Byte_t ReadNameTable( UInt16_t address );

	//////////////

	void SetDefaultVariables( bool cold );

	// foreground members
	int ticksWhenToSetOverflow;

	void EvaluateSprites( int scanline, int xstart, int xend, int tickLimit );
	void RenderSprite( int scanline, int spritenum, bool extraSprite );

	int nextScanlineSpritesCount;
	Byte_t nextScanlineSprites[ 64 * 4 ];
	Byte_t nextScanlineSpritePriority[ 64 ];

	bool mDoSpriteTransferAfterNextCpuInstruction;
	Byte_t mSpriteTransferArgument;

	////////////

	void RenderPartialScanline( int scanline, int xstart, int xend, int tickLimit );
	void OnReset( bool cold );

	void RenderPixel( int x, int y );

	int GetMasterTicksTillVBlankClearDue( int tickCount ) const;
	int GetMasterTicksTillClockSkip( int tickCount ) const;
	int GetMasterTicksTillFrameDone( int tickCount ) const;

	bool IsRendering( int tickCount, bool includeHblank ) const;

//	void WriteOutCHRRam( );
//	void WriteOutPatternTable( int table, Byte_t* imagedata, int offsetx, int offsety );

public:
	PictureProcessingUnit();
	~PictureProcessingUnit();
	
	FORCE_INLINE bool IsRenderingEnabled() const { return ( (*control2) & 0x18 ) > 0; }
		
	Byte_t GetC1() const { return *control1; }
	Byte_t GetC2() const { return *control2; }
	Byte_t GetS() const { return *status; }
	unsigned int GetFrameCount() const { return frameCount; }

	void HandleSpriteTransfer();

	void ChangeMirroringMethod( PPU_MIRRORING_METHOD method );
	Byte_t GetBackgroundPaletteIndex() const;
	void TicksToScreenCoordinates( int tickCount, int* x, int* y ) const;

	Byte_t ReadFromRegister( UInt16_t offset );
	void WriteToRegister( UInt16_t offset, Byte_t data );
	
	virtual void Write8( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8( UInt16_t offset, bool dataOnly = false );

	virtual void Synchronise( int syncMTC );
	virtual int GetNextEvent( int cpuMTC );

	virtual void OnEndFrame();

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );
};



#endif // #ifndef PICTUREPROCESSINGUNIT_H


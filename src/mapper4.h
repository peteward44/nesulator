
#pragma once


#include "mappers.h"

const int mmc3IrqPixel = 257;

class Mapper4 : public IMapper
{
private:
	Byte_t bankSwapByte;
	bool prgRamDisableWrite, chipEnable;

	int irqCounter, irqLatch;
	bool mReloadFlag;
	bool interruptsEnabled;
	Byte_t banks[8];

	int masterTickCount, lastA12Raise;

	bool mSpriteAddress, mScreenAddress;
	bool mRenderingEnabled;

	void SyncBanks();

	void DecrementIrqCounter( int tickCount, bool fromRegisterWrite );

	int CalculateNextA12Raise( int cpuMTC, int& upperLimit ) const;


public:
	Mapper4( PPU_MIRRORING_METHOD mirroringMethod );

	virtual void OnReset( bool cold );
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );

	virtual void Write8SRam( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8SRam( UInt16_t offset );

	virtual void Synchronise( int syncMTC );
	virtual int GetNextEvent( int cpuMTC );
	virtual void OnEndFrame();

	virtual void OnSaveState( std::ostream& ostr );
	virtual void OnLoadState( std::istream& istr );

	virtual std::string GetName() const { return "Nintendo MMC3"; }

	virtual void PPUA12Latch();
	virtual void RenderingEnabledChanged( bool enabled );
	virtual void SpriteScreenEnabledUpdate( bool spriteAddress, bool screenAddress );

	virtual bool HasKnownProblems() const { return true; } // not enough testing, IRQ timing problems, apparent on many games (see todo.txt)

};


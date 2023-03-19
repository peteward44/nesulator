

#ifndef MAPPER_H
#define MAPPER_H

#include "main.h"
#include "cpumemory.h"
#include "ppu.h"
#include <vector>
#include "synchroniser.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IMapper is the base class all mappers derive from. All cartridges have a mapper


typedef Page< 0x1FE8, 0 > CartExpansionRAM_t;
typedef Page< 0x2000, 0 > CartRAM_t;


class IMapper : public Synchronisable, public HasState
{
private:
	boost::signals2::connection resetConnection;

	std::vector< PRGPagePtr_t > prgpages;
	std::vector< PatternTablePtr_t > patternpages;

	PRGPagePtr_t prgPagePtrs[ 4 ]; // Program code (0x8000 -> 0xFFFF) 0x2000 x4
	int prgPagesIds[ 4 ];
	PatternTablePtr_t patternTables[ 8 ]; // 1k each
	int patternTablesIds[ 8 ];

	CartRAM_t sram; // Cartridge RAM (0x6000 -> 0x7FFF), (AKA SRAM) may or may not be battery-backed
	CartExpansionRAM_t expansionRam; // Cartridge ram (0x4018 - 0x5FFF)

	FORCE_INLINE void SetPrgPage( int id, int pos )
	{ prgPagePtrs[ pos ] = this->prgpages[ id ]; prgPagesIds[ pos ] = id; }

	FORCE_INLINE void SetChrPage( int id, int pos )
	{ patternTables[ pos ] = patternpages[ id ]; patternTablesIds[ pos ] = id; }


protected:
	PPU_MIRRORING_METHOD mirroringMethod;

	virtual void OnSaveState( std::ostream& ostr ) {}
	virtual void OnLoadState( std::istream& istr ) {}

	// Functions for switching PRG and CHR banks
	void Switch1kChrBank( int id, int pos );
	void Switch2kChrBank( int id, int pos );
	void Switch4kChrBank( int id, bool low );
	void Switch8kChrBank( int id );

	void Switch8kPrgBank( int id, int pos );
	void Switch16kPrgBank( int id, bool low );
	void Switch32kPrgBank( int id );

	void UseVRAM( int numbanks = 8 );

	FORCE_INLINE int Get1kChrBankCount() const { return (int)patternpages.size(); }
	FORCE_INLINE int Get2kChrBankCount() const { return (int)patternpages.size() / 2; }
	FORCE_INLINE int Get4kChrBankCount() const { return (int)patternpages.size() / 4; }
	FORCE_INLINE int Get8kChrBankCount() const { return (int)patternpages.size() / 8; }

	FORCE_INLINE int Get8kPrgBankCount() const { return (int)prgpages.size(); }
	FORCE_INLINE int Get16kPrgBankCount() const { return (int)prgpages.size() / 2; }
	FORCE_INLINE int Get32kPrgBankCount() const { return (int)prgpages.size() / 4; }

public:
	IMapper();
	virtual ~IMapper();

	virtual bool HasKnownProblems() const { return false; }

	// expansion ram: 0x4018 -> 0x5FFF
	virtual void Write8EXRam( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8EXRam( UInt16_t offset );

	// read and writes to 0x6000 -> 0x7FFF
	virtual void Write8SRam( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8SRam( UInt16_t offset );

	// 0x8000 -> 0xFFFF
	virtual void Write8PrgRom( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8PrgRom( UInt16_t offset );

	// VRAM 0x0000 -> 0x2000
	virtual void Write8ChrRom( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8ChrRom( UInt16_t offset, bool dataOnly = false );

	// VRAM 0x2000 -> 0x3000 (only called when CHRROM name table mirroring is set)
	virtual Byte_t Read8NameTable( UInt16_t offset, bool dataOnly = false ) { return 0; }
	virtual void Write8NameTable( UInt16_t offset, Byte_t data ) {}

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );

	virtual void OnReset( bool cold ) {}

	virtual std::wstring GetName() const { throw std::runtime_error( "Undefined GetName() method on mapper" ); }

	virtual void Synchronise( int syncMTC );
	virtual int GetNextEvent( int cpuMTC );
	virtual void OnEndFrame();

	// functions used by MMC3
	virtual void PPUA12Latch() {}
	virtual void RenderingEnabledChanged( bool enabled ) {}
	virtual void SpriteScreenEnabledUpdate( bool spriteAddress, bool screenAddress ) {}

	// TODO: find a better way of accessing PRG and CHR banks from cartridge class?
	FORCE_INLINE void AddPRGBank( PRGPagePtr_t ptr, bool writeable )
	{ prgpages.push_back( ptr ); }

	FORCE_INLINE void AddCHRBank( PatternTablePtr_t ptr, bool writeable )
	{ patternpages.push_back( ptr ); }
};


typedef boost::shared_ptr< IMapper > MemoryMapperPtr_t;


MemoryMapperPtr_t CreateMemoryMapper( int mapperid, PPU_MIRRORING_METHOD mirroringMethod );



#endif


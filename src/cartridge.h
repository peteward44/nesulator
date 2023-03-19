

#ifndef CARTRIDGE_H
#define CARTRIDGE_H

#include "main.h"
#include "mappers.h"
#include "icartridge.h"


class Cartridge : public ICartridge
{
private:
	MemoryMapperPtr_t memorymapper;
	int programPageCount, characterPageCount, mapperId;
	bool hasTrainer, sramEnabled;
	std::wstring filename, name;
	unsigned int crc32;

	// returns corrected mapper id
	static int CheckCRCForBrokenMapperId( unsigned int crc, int supposedMapper );

public:
	Cartridge( const std::wstring& nesfilename, bool onlyloadinfo );
	~Cartridge();

	FORCE_INLINE const std::wstring& GetFilename() const { return filename; }

	FORCE_INLINE int GetProgramPageCount() const { return programPageCount; }
	FORCE_INLINE int GetCharacterPageCount() const { return characterPageCount; }
	FORCE_INLINE int GetMapperId() const { return mapperId; }
	FORCE_INLINE bool HasTrainer() const { return hasTrainer; }
	FORCE_INLINE bool IsSRamEnabled() const { return sramEnabled; }

	virtual std::wstring GetName() const { return name; }
	virtual unsigned int GetCRC32() const { return crc32; }

	FORCE_INLINE MemoryMapperPtr_t GetMemoryMapper() { return memorymapper; }
};


typedef boost::shared_ptr< Cartridge > CartridgePtr_t;

CartridgePtr_t CreateCartridgeFromROM( const std::wstring& filename, bool onlyloadinfo );

#endif

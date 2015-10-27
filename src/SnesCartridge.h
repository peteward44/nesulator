
#ifndef SNESCARTRIDGE_H
#define SNESCARTRIDGE_H


#include "icartridge.h"


class SnesCartridge : public ICartridge
{
	std::string mTitle;
	bool mIsHiRom, mFastRom, mHasSmcHeader;

	int mRomSize, mSRamSize;

	Byte_t* mRomData;
	Byte_t* mSRamData;

	FORCE_INLINE UInt32_t GetMemorySize() const { return mIsHiRom ? 4194304 : 3145728; }

public:
	SnesCartridge( const std::string& snesfilename, bool onlyloadinfo );
	virtual ~SnesCartridge();

	FORCE_INLINE bool IsHiRom() const { return mIsHiRom; }

	virtual unsigned int GetCRC32() const { return 0; }
	virtual std::string GetName() const { return ""; }

	UInt8_t ReadRom8( UInt8_t bankId, UInt16_t address );
	UInt8_t ReadSRam8( UInt8_t bankId, UInt16_t address );

	void WriteRom8( UInt8_t bankId, UInt16_t address, UInt8_t value );
	void WriteSRam8( UInt8_t bankId, UInt16_t address, UInt8_t value );

};


typedef boost::shared_ptr< SnesCartridge > SnesCartidgePtr_t;


#endif



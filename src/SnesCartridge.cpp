
#include "stdafx.h"
#include "main.h"
#include "SnesCartridge.h"

#include <cstdio>
#include "boost/filesystem/operations.hpp"
#include <boost/algorithm/string.hpp>


const UInt32_t g_SRamDataSize = 0x10000;

const int TitleSize = 21;

/*
COUNTRY CODE  : 1 BYTE

  Hex    Country                 Video Mode
  ------------------------------------------
   00    Japan                   (NTSC)
   01    USA                     (NTSC)
   02    Europe, Oceania, Asia    (PAL)
   03    Sweden                   (PAL)
   04    Finland                  (PAL)
   05    Denmark                  (PAL)
   06    France                   (PAL)
   07    Holland                  (PAL)
   08    Spain                    (PAL)
   09    Germany, Austria, Switz  (PAL)
   10    Italy                    (PAL)
   11    Hong Kong, China         (PAL)
   12    Indonesia                (PAL)
   13    Korea                    (PAL)

  * Country Codes are from SU.INI, could someone verify these?

LICENSE       : 1 BYTE
     0 <Invalid License Code>
     1 Nintendo
     5 Zamuse
     8 Capcom
     9 HOT B
    10 Jaleco
    11 STORM (Sales Curve) (1)
    15 Mebio Software
    18 Gremlin Graphics
    21 COBRA Team
    22 Human/Field
    24 Hudson Soft
    26 Yanoman
    28 Tecmo (1)
    30 Forum
    31 Park Place Productions / VIRGIN
    33 Tokai Engeneering (SUNSOFT?)
    34 POW
    35 Loriciel / Micro World
    38 Enix
    40 Kemco (1)
    41 Seta Co.,Ltd.
    45 Visit Co.,Ltd.
    53 HECT
    61 Loriciel
    64 Seika Corp.
    65 UBI Soft
    71 Spectrum Holobyte
    73 Irem
    75 Raya Systems/Sculptured Software
    76 Renovation Pruducts
    77 Malibu Games (T*HQ Inc.) / Black Pearl
    79 U.S. Gold
    80 Absolute Entertainment
    81 Acclaim
    82 Activision
    83 American Sammy
    84 GameTek
    85 Hi Tech
    86 LJN Toys
    90 Mindscape
    93 Technos Japan Corp. (Tradewest)
    95 American Softworks Corp.
    96 Titus
    97 Virgin Games
    98 Maxis
   103 Ocean
   105 Electronic Arts
   107 Laser Beam
   110 Elite
   111 Electro Brain
   112 Infogrames
   113 Interplay
   114 LucasArts
   115 Sculptured Soft
   117 STORM (Sales Curve) (2)
   120 THQ Software
   121 Accolade Inc.
   122 Triffix Entertainment
   124 Microprose
   127 Kemco (2)
   130 Namcot/Namco Ltd. (1)
   132 Koei/Koei! (second license?)
   134 Tokuma Shoten Intermedia
   136 DATAM-Polystar
   139 Bullet-Proof Software
   140 Vic Tokai
   143 I'Max
   145 CHUN Soft
   146 Video System Co., Ltd.
   147 BEC
   151 Kaneco
   153 Pack in Video
   154 Nichibutsu
   155 TECMO (2)
   156 Imagineer Co.
   160 Wolf Team
   164 Konami
   165 K.Amusement
   167 Takara
   169 Technos Jap. ????
   170 JVC
   172 Toei Animation
   173 Toho
   175 Namcot/Namco Ltd. (2)
   177 ASCII Co. Activison
   178 BanDai America
   180 Enix
   182 Halken
   186 Culture Brain
   187 Sunsoft
   188 Toshiba EMI/System Vision
   189 Sony (Japan) / Imagesoft
   191 Sammy
   192 Taito
   194 Kemco (3) ????
   195 Square
   196 NHK
   197 Data East
   198 Tonkin House
   200 KOEI
   202 Konami USA
   205 Meldac/KAZe
   206 PONY CANYON
   207 Sotsu Agency
   209 Sofel
   210 Quest Corp.
   211 Sigma
   214 Naxat
   216 Capcom Co., Ltd. (2)
   217 Banpresto
   219 Hiro
   221 NCS
   222 Human Entertainment
   223 Ringler Studios
   224 K.K. DCE / Jaleco
   226 Sotsu Agency
   228 T&ESoft
   229 EPOCH Co.,Ltd.
   231 Athena
   232 Asmik
   233 Natsume
   234 King/A Wave
   235 Atlus
   236 Sony Music
   238 Psygnosis / igs
   243 Beam Software
   244 Tec Magik
   255 Hudson Soft
*/


//struct headerdata {
//	boolean hirom; // Apparent HiROM
//	word hadd; // Set to 512
//	char name [22];
//	byte romsize; // Megabits
//	byte sramsize; // Kilobits
//	boolean fastrom;
//	byte country, license, version;
//	word resetvector, nmivector;
//};


// TODO: multi file support


// This is 'suspect' has SMC header as this is not 100% foolproof test
bool SuspectRomHasSMCHeader( FILE* file )
{
	fseek( file, 0, SEEK_END );
	const size_t fileSize = ftell( file );

	const size_t fileSizeMod = fileSize % 1024;
	return fileSizeMod == 512;
}


bool TestChecksum( FILE* file, int offset )
{
	UInt16_t checksum = 0, compliment = 0;

	fseek( file, offset + 0x1C, SEEK_SET );
	fread( &compliment, 2, 1, file );
	fread( &checksum, 2, 1, file );

	return ( compliment ^ checksum ) == 0xFFFF;
}


bool TestTitleCharacters( FILE* file, int offset )
{
	fseek( file, offset, SEEK_SET );
	char title[ TitleSize + 1 ];
	fread( title, TitleSize, 1, file );
	title[ TitleSize ] = 0;
	std::string tstr( title );

	int asciiCharCount = 0;
	for ( int i=0; i<TitleSize; ++i )
	{
		const char c = title[ i ];
		if ( ( c >= 0x20 && c <= 0x7E ) || c == 0x00 )
			asciiCharCount++;
	}
	return asciiCharCount >= TitleSize-1;
}


bool TestHiLoRomByte( FILE* file, int offset )
{
	fseek( file, offset + 0x15, SEEK_SET );

	const Byte_t romLayout = fgetc( file );
	const Byte_t cartridgeType = fgetc( file );
	/*
	$ffd5 => ROM layout, typically $20 for LoROM, or $21 for HiROM. Add $10 for FastROM.
	$ffd6 => Cartridge type, typically $00 for ROM only, or $02 for ROM with save-RAM. 
	*/
	// bit 6 should always be set
	const bool romLayoutValid = ( romLayout & 0x20 ) > 0; // romLayout == 0x20 || romLayout == 0x21 || romLayout == 0x30 || romLayout == 0x31;
	// megaman 2 has weird cartridge type - cant be used for valid file
	const bool cartridgeTypeValid = true;// cartridgeType == 0x00 || cartridgeType == 0x02;

	return romLayoutValid && cartridgeTypeValid;
}


UInt32_t GetSNESHeaderPosition( FILE* file, bool& isHiRom, bool& hasSmcHeader, bool& hasSnesHeader )
{
	const UInt32_t loRomPosition = 0x7fc0;
	const UInt32_t hiRomPosition = 0xffc0;
	const UInt32_t loRomPosition_wh = 0x81c0;
	const UInt32_t hiRomPosition_wh = 0x101c0;

	const bool suspectHasSmcHeader = SuspectRomHasSMCHeader( file );

	// List of offsets within the file to test to see if the snes header exists, in order of likelihood (reduces chance of false positive detection occurring)
	const UInt32_t addressesToTest[] = {
		suspectHasSmcHeader ? loRomPosition_wh : loRomPosition,
		suspectHasSmcHeader ? hiRomPosition_wh : hiRomPosition,
		suspectHasSmcHeader ? loRomPosition : loRomPosition_wh,
		suspectHasSmcHeader ? hiRomPosition : hiRomPosition_wh,
	};
	const int addressesToTestCount = sizeof( addressesToTest ) / sizeof( UInt32_t );

	// Test checksum first
	for ( int addressIndex=0; addressIndex< addressesToTestCount; ++addressIndex )
	{
		const UInt32_t address = addressesToTest[ addressIndex ];
		if ( TestChecksum( file, address ) && TestHiLoRomByte( file, address ) )
		{
			isHiRom = (addressIndex % 2) != 0; // Lo roms always even index in addressesToTest array
			// "with header" addresses are first in the list if "suspectHasSmcHeader" is true, otherwise the other way round
			hasSmcHeader = suspectHasSmcHeader ? addressIndex < 2 : addressIndex >= 2;
			hasSnesHeader = true;
			return address;
		}
	}

	// if checksum tests failed, check the title for readable characters (for roms with invalid checksums)
	for ( int addressIndex=0; addressIndex< addressesToTestCount; ++addressIndex )
	{
		const UInt32_t address = addressesToTest[ addressIndex ];
		if ( TestTitleCharacters( file, address ) && TestHiLoRomByte( file, address ) )
		{
			isHiRom = (addressIndex % 2) != 0; // Lo roms always even index in addressesToTest array
			// "with header" addresses are first in the list if "suspectHasSmcHeader" is true, otherwise the other way round
			hasSmcHeader = suspectHasSmcHeader ? addressIndex < 2 : addressIndex >= 2;
			hasSnesHeader = true;
			return address;
		}
	}

//	throw std::runtime_error( "Inconsistent SNES header information for rom, could not determine rom type" );
	// Assume no snes header
	hasSnesHeader = false;
	hasSmcHeader = suspectHasSmcHeader;
	isHiRom = false;
	return 0x200;
}


SnesCartridge::SnesCartridge( const std::string& snesfilename, bool onlyloadinfo )
{
	mRomData = NULL;
	mIsHiRom = mFastRom = mHasSmcHeader = false;
	mRomSize = mSRamSize = 0;

	mSRamData = new Byte_t[ g_SRamDataSize ];
	memset( mSRamData, 0, g_SRamDataSize );

	FILE* file = 0;

	try
	{
		Log::Write( LOG_MISC, ( boost::format( "Loading SNES ROM file: %1%" ) % snesfilename ).str() );

		// TODO: split file support (extensions *.a* *.1* *.A*)

		fopen_s( &file, snesfilename.c_str(), "rb" );
		if ( file == 0 )
			throw std::runtime_error( "File not found" );

		bool hasSnesHeader = false;
		const UInt32_t snesHeaderPosition = GetSNESHeaderPosition( file, mIsHiRom, mHasSmcHeader, hasSnesHeader );

		if ( mIsHiRom )
			throw std::runtime_error( "Hi rom not supported yet" );

		if ( hasSnesHeader )
		{
			fseek( file, snesHeaderPosition, SEEK_SET );

			char title[ TitleSize + 1 ];
			fread( title, TitleSize, 1, file );
			title[ TitleSize ] = '\0';
			mTitle = title;
			boost::trim( mTitle );

			Byte_t romMakeup = 0;
			fread( &romMakeup, 1, 1, file );

			mFastRom = ( romMakeup & 0x30 ) > 0;

			Byte_t romType = 0;
			fread( &romType, 1, 1, file );

			Byte_t romSize = 0;
			fread( &romSize, 1, 1, file );

			mRomSize = 1 << ( romSize - 7 );
			if ( mRomSize == 0 )
				mRomSize = 1;

			Byte_t sramSize = 0;
			fread( &sramSize, 1, 1, file );

			mSRamSize = 1 << ( sramSize + 3 );

			Byte_t countryCode = 0;
			fread( &countryCode, 1, 1, file );

			Byte_t license = 0;
			fread( &license, 1, 1, file );

			Byte_t version = 0;
			fread( &version, 1, 1, file );

			Byte_t checksumComplement[2];
			fread( checksumComplement, 1, 2, file );

			Byte_t checksum[2];
			fread( checksum, 1, 2, file );
		}

		mRomData = new Byte_t[ GetMemorySize() ]; // 4MB for hi roms, 3MB for lo roms

		fseek( file, mHasSmcHeader ? 0x200 : 0x00, SEEK_SET );

		for ( int pos = 0; !feof( file ); pos += 32768 )
		{ // read in 32k chunks
			size_t x = fread( mRomData + pos, 1, 32768, file );
			if ( x < 32768 || feof( file ) )
				break;
		}

		fclose(file);
		file = 0;

		Log::Write( LOG_MISC, ( boost::format( "Successfully loaded SNES ROM file: %1%" ) % mTitle ).str() );
		Log::Write( LOG_MISC, ( boost::format( "[SMC header: %1%][ROM memory format: %2%][Fast ROM: %3%]" )
			% ( mHasSmcHeader ? "Yes" : "No" ) % ( mIsHiRom ? "Hi" : "Lo" ) % ( mFastRom ? "Yes" : "No" ) ).str() );
	}
	catch ( std::exception& )
	{
		if ( file != 0 )
			fclose(file);
		file = 0;

		delete[] mRomData;
		mRomData = NULL;
		delete[] mSRamData;
		mSRamData = NULL;

		throw;
	}
}


SnesCartridge::~SnesCartridge()
{
	delete[] mRomData;
	delete[] mSRamData;
}


UInt8_t SnesCartridge::ReadRom8( UInt8_t bankId, UInt16_t address )
{
	UInt32_t index = bankId * 0x8000 + address;
	assert( index >= 0 && index < GetMemorySize() );
	return mRomData[ index ];
}


UInt8_t SnesCartridge::ReadSRam8( UInt8_t bankId, UInt16_t address )
{
	assert( bankId == 0 );
	assert( address >= 0 && address < g_SRamDataSize );
	return mSRamData[ address ];
}


void SnesCartridge::WriteRom8( UInt8_t bankId, UInt16_t address, UInt8_t value )
{
	UInt32_t index = bankId * 0x8000 + address;
	assert( index >= 0 && index < GetMemorySize() );
	mRomData[ index ] = value;
}


void SnesCartridge::WriteSRam8( UInt8_t bankId, UInt16_t address, UInt8_t value )
{
	assert( bankId == 0 );
	assert( address >= 0 && address < g_SRamDataSize );
	mSRamData[ address ] = value;
}


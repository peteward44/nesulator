
#include "stdafx.h"
#include "cartridge.h"
#include "mainboard.h"
#include "wx/log.h"

#include <cstdio>
#include "boost/filesystem/operations.hpp"


CartridgePtr_t CreateCartridgeFromROM( const std::string& filename, bool onlyloadinfo )
{
	return CartridgePtr_t( new Cartridge( filename, onlyloadinfo ) );
}


Cartridge::Cartridge( const std::string& nesfilename, bool onlyloadinfo )
{
	programPageCount = characterPageCount = mapperId = 0;
	hasTrainer = sramEnabled = false;
	crc32 = 0;

	FILE* file = 0;

	try
	{
		Log::Write( LOG_MISC, ( boost::format( "Loading ROM file: %1%" ) % nesfilename.c_str() ).str() );

		fopen_s( &file, nesfilename.c_str(), "rb" );
		if (file == 0)
			throw std::runtime_error( "File not found" );

		// check header
		const Byte_t correctHeader[] = { 'N', 'E', 'S', 0x1A };
		Byte_t header[4];
		fread( header, 1, 4, file );
		if ( memcmp(correctHeader, header, 4) != 0 )
		{
			throw std::runtime_error( "Not an iNES file" );
		}

		Byte_t numProgramPages, numCharacterPages;

		fread( &numProgramPages, 1, sizeof(Byte_t), file );
		fread( &numCharacterPages, 1, sizeof(Byte_t), file );

		programPageCount = numProgramPages;
		characterPageCount = numCharacterPages;

		Byte_t controlByte1, controlByte2;

		fread( &controlByte1, 1, sizeof(Byte_t), file );
		fread( &controlByte2, 1, sizeof(Byte_t), file );

		fseek( file, 8, SEEK_CUR ); // bunch of zeroes

		bool horizontalMirroring = (controlByte1 & 0x01) == 0;
		sramEnabled = (controlByte1 & (0x01 << 1)) > 0;
		hasTrainer = (controlByte1 & (0x01 << 2)) > 0;
		bool fourScreenRamLayout = (controlByte1 & (0x01 << 3)) > 0;

		PPU_MIRRORING_METHOD mirroringMethod;

		if ( fourScreenRamLayout )
			mirroringMethod = PPU_MIRROR_FOURSCREEN;
		else if ( !horizontalMirroring )
			mirroringMethod = PPU_MIRROR_VERTICAL;
		else
			mirroringMethod = PPU_MIRROR_HORIZONTAL;

		mapperId = 0;
		mapperId |= (controlByte1 & 0xF0) >> 4;
		mapperId |= (controlByte2 & 0xF0);

		fclose( file );
		file = 0;

		this->filename = nesfilename;
		boost::filesystem::path filepath( this->filename, boost::filesystem::native );
		this->name = filepath.leaf().generic_string();

		if ( !onlyloadinfo )
		{
			// take CRC of file and compare it against known broken roms
			crc32 = CalculateCRC32( nesfilename );

			int newMapperId = CheckCRCForBrokenMapperId( crc32, mapperId );
			if ( newMapperId != mapperId )
			{
				Log::Write( LOG_MISC, ( boost::format( "Using mapper #%1% instead of reported #%2%" ) % newMapperId % mapperId ).str() );
				mapperId = newMapperId;
			}

			memorymapper = CreateMemoryMapper( mapperId, mirroringMethod );

			Log::Write( LOG_MISC, ( boost::format( "ROM uses mapper %1% (#%2%)" ) % memorymapper->GetName() % mapperId ).str() );

			if ( memorymapper->HasKnownProblems() )
			{
				Log::Write( LOG_ERROR, "This ROM mapper has known compatibility issues with some games. You have been warned" );
			}

			fopen_s( &file, nesfilename.c_str(), "rb" );
			if (file == 0)
				throw std::runtime_error( "File not found" );
			fseek( file, 16, SEEK_SET );

			// trainer (if exists) comes now
			// TODO: Trainer loading & support - low priority
			if ( hasTrainer )
			{
				fseek( file, 512, SEEK_CUR );
			}

			// start reading in program codes
			for ( int i=0; i<programPageCount * 2; ++i ) // read in 8k chunks, programPageCount is 16k chunks
			{
				PRGPagePtr_t prgpage = PRGPagePtr_t( new PRGPage_t() );
				fread( prgpage->Data, 1, 0x2000, file );
				memorymapper->AddPRGBank( prgpage, false );
			}

			// read in character maps
			for (int i=0; i<characterPageCount*8; ++i) // 1kb per pattern table, numCharacterPages is the 8kb count
			{
				PatternTablePtr_t patterntable = PatternTablePtr_t( new PatternTable_t() );
				fread( patterntable->Data, 1, 0x400, file );
				memorymapper->AddCHRBank( patterntable, true ); // TODO: check if writeable - depends on mapper
			}
			
			fclose(file);
			file = 0;
		}
	}
	catch ( std::exception& e )
	{
		if ( file != 0 )
			fclose( file );
		Log::Write( LOG_ERROR, ( boost::format( "Exception caught when attempting to load NES rom: %1% : %2%" ) % nesfilename % e.what() ).str() );
		throw;
	}
}


// static
int Cartridge::CheckCRCForBrokenMapperId( unsigned int crc, int supposedMapper )
{
	switch ( crc )
	{
	case 0x0cf82792: // USA - pipe dream, reported as mapper 0 but is mapper 3
		return 3;
	case 0x32dc646f: // USA - deadly towers, claims mapper 7, is actually 34
		return 34;
	case 0xB6C34D32: // USA - castle of deceit, reports 7, is 11
		return 11;
	case 0x3C7B330E: // USA - deathbots, reports 7, is 79
		return 79;
	default:
		return supposedMapper;
	}
}



Cartridge::~Cartridge()
{
}


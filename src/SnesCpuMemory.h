
#ifndef SNES_CPU_MEMORY_H
#define SNES_CPU_MEMORY_H


class SnesCpuMemory : private boost::noncopyable
{
	Byte_t* mLowRam;
	Byte_t* mExpandedRam0;
	Byte_t* mExpandedRam1;
	Byte_t* mExpandedRam2;

	UInt8_t Read8LoRom( UInt8_t bankRegister, UInt16_t address );
	UInt8_t Read8HiRom( UInt8_t bankRegister, UInt16_t address );

	void Write8LoRom( UInt8_t bankRegister, UInt16_t address, UInt8_t value );
	void Write8HiRom( UInt8_t bankRegister, UInt16_t address, UInt8_t value );

public:
	SnesCpuMemory();
	~SnesCpuMemory();

	UInt8_t Read8( UInt8_t bankRegister, UInt16_t address );
	UInt16_t Read16( UInt8_t bankRegister, UInt16_t address );
	UInt32_t Read24( UInt8_t bankRegister, UInt16_t address );

	UInt8_t Read8( UInt32_t address );
	UInt16_t Read16( UInt32_t address );
	UInt32_t Read24( UInt32_t address );

	void Write8( UInt8_t bankRegister, UInt16_t address, UInt8_t value );
	void Write16( UInt8_t bankRegister, UInt16_t address, UInt16_t value );
	void Write24( UInt8_t bankRegister, UInt16_t address, UInt32_t value );

	void Write8( UInt32_t address, UInt8_t value );
	void Write16( UInt32_t address, UInt16_t value );
	void Write24( UInt32_t address, UInt32_t value );

};


#endif


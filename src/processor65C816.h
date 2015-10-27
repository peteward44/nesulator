
#ifndef SNESCPU_H
#define SNESCPU_H

#include "SnesCpuMemory.h"
#include "multisizeint.h"



class Processor65C816
{

private:
	FORCE_INLINE void OperationADC( MultiSizeInt src );
	FORCE_INLINE void OperationAND( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationASL( MultiSizeInt src );

	FORCE_INLINE bool OperationBCC( MultiSizeInt src, bool incrementPc = true );
	FORCE_INLINE bool OperationBCS( MultiSizeInt src, bool incrementPc = true );
	FORCE_INLINE bool OperationBEQ( MultiSizeInt src, bool incrementPc = true );

	FORCE_INLINE void OperationBIT( MultiSizeInt src );
	FORCE_INLINE bool OperationBMI( MultiSizeInt src, bool incrementPc = true );
	FORCE_INLINE bool OperationBNE( MultiSizeInt src, bool incrementPc = true );
	FORCE_INLINE bool OperationBPL( MultiSizeInt src, bool incrementPc = true );
	void OperationBRK( SnesCpuMemory* cpuMemory );

	FORCE_INLINE bool OperationBRA( MultiSizeInt src, bool incrementPc = true ); // NEW
	FORCE_INLINE bool OperationBRL( MultiSizeInt src, bool incrementPc = true ); // NEW

	FORCE_INLINE bool OperationBVC( MultiSizeInt src, bool incrementPc = true );
	FORCE_INLINE bool OperationBVS( MultiSizeInt src, bool incrementPc = true );

	FORCE_INLINE void OperationCLC();
	FORCE_INLINE void OperationCLD();
	FORCE_INLINE void OperationCLI();
	FORCE_INLINE void OperationCLV();

	FORCE_INLINE void OperationCMP( MultiSizeInt src );
	FORCE_INLINE void OperationCPX( MultiSizeInt src );
	FORCE_INLINE void OperationCPY( MultiSizeInt src );

	void OperationCOP( SnesCpuMemory* cpuMemory ); // NEW

	FORCE_INLINE MultiSizeInt OperationDEC( MultiSizeInt src );
	FORCE_INLINE void OperationDEX();
	FORCE_INLINE void OperationDEY();

	FORCE_INLINE void OperationEOR( MultiSizeInt src );
	
	FORCE_INLINE MultiSizeInt OperationINC( MultiSizeInt src );
	FORCE_INLINE void OperationINX();
	FORCE_INLINE void OperationINY();

	FORCE_INLINE void OperationJML( SnesCpuMemory* cpuMemory, MultiSizeInt src ); // NEW
	FORCE_INLINE void OperationJMP( MultiSizeInt src );
	FORCE_INLINE void OperationJMP24( MultiSizeInt src ); // NEW
	FORCE_INLINE void OperationJSL( SnesCpuMemory* cpuMemory, MultiSizeInt src ); // NEW
	FORCE_INLINE void OperationJSR( SnesCpuMemory* cpuMemory, MultiSizeInt src );

	FORCE_INLINE void OperationLDA( MultiSizeInt src );
	FORCE_INLINE void OperationLDX( MultiSizeInt src );
	FORCE_INLINE void OperationLDY( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationLSR( MultiSizeInt src );

	FORCE_INLINE void OperationMVN( SnesCpuMemory* cpuMemory, MultiSizeInt operand ); // NEW
	FORCE_INLINE void OperationMVP( SnesCpuMemory* cpuMemory, MultiSizeInt operand ); // NEW

	FORCE_INLINE void OperationORA( MultiSizeInt src );

	FORCE_INLINE void OperationPEA( SnesCpuMemory* cpuMemory, MultiSizeInt immediateData ); // NEW
	FORCE_INLINE void OperationPEI( SnesCpuMemory* cpuMemory, MultiSizeInt dataAtPointer ); // NEW
	FORCE_INLINE void OperationPER( SnesCpuMemory* cpuMemory, MultiSizeInt immediateData ); // NEW
	FORCE_INLINE void OperationPHA( SnesCpuMemory* cpuMemory );
	FORCE_INLINE void OperationPHB( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPHD( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPHK( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPHP( SnesCpuMemory* cpuMemory );
	FORCE_INLINE void OperationPHX( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPHY( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPLA( SnesCpuMemory* cpuMemory );
	FORCE_INLINE void OperationPLB( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPLD( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPLP( SnesCpuMemory* cpuMemory );
	FORCE_INLINE void OperationPLX( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationPLY( SnesCpuMemory* cpuMemory ); // NEW

	FORCE_INLINE void OperationREP( MultiSizeInt newBits ); // NEW

	FORCE_INLINE MultiSizeInt OperationROL( MultiSizeInt src );
	FORCE_INLINE MultiSizeInt OperationROR( MultiSizeInt src );

	FORCE_INLINE void OperationRTI( SnesCpuMemory* cpuMemory );
	FORCE_INLINE void OperationRTL( SnesCpuMemory* cpuMemory ); // NEW
	FORCE_INLINE void OperationRTS( SnesCpuMemory* cpuMemory );

	FORCE_INLINE void OperationSBC( MultiSizeInt src, bool useCarryFlag = true );

	FORCE_INLINE void OperationSEC();
	FORCE_INLINE void OperationSED();
	FORCE_INLINE void OperationSEI();
	FORCE_INLINE void OperationSEP( MultiSizeInt src ); // NEW

	FORCE_INLINE void OperationSTP(); // NEW

	FORCE_INLINE MultiSizeInt OperationSTA( );
	FORCE_INLINE MultiSizeInt OperationSTX( );
	FORCE_INLINE MultiSizeInt OperationSTY( );
	FORCE_INLINE MultiSizeInt OperationSTZ();

	FORCE_INLINE void OperationTAX();
	FORCE_INLINE void OperationTAY();

	FORCE_INLINE void OperationTCD(); // NEW
	FORCE_INLINE void OperationTCS(); // NEW
	FORCE_INLINE void OperationTDC(); // NEW
	FORCE_INLINE MultiSizeInt OperationTRB( MultiSizeInt data ); // NEW
	FORCE_INLINE MultiSizeInt OperationTSB( MultiSizeInt data ); // NEW
	FORCE_INLINE void OperationTSC(); // NEW

	FORCE_INLINE void OperationTSX();
	FORCE_INLINE void OperationTXA();
	FORCE_INLINE void OperationTXS();
	FORCE_INLINE void OperationTXY(); // NEW
	FORCE_INLINE void OperationTYA();
	FORCE_INLINE void OperationTYX(); // NEW

	FORCE_INLINE void OperationWAI(); // NEW
	FORCE_INLINE void OperationXBA(); // NEW
	FORCE_INLINE void OperationXCE(); // NEW


	FORCE_INLINE UInt16_t PopStack16( SnesCpuMemory* cpuMemory );
	FORCE_INLINE UInt8_t PopStack8( SnesCpuMemory* cpuMemory );
	FORCE_INLINE void PushStack16( SnesCpuMemory* cpuMemory, UInt16_t val );
	FORCE_INLINE void PushStack8( SnesCpuMemory* cpuMemory, UInt8_t val );

	FORCE_INLINE UInt16_t TopStack16( SnesCpuMemory* cpuMemory );
	FORCE_INLINE UInt8_t TopStack8( SnesCpuMemory* cpuMemory );

	FORCE_INLINE void IncrementStackPointer();
	FORCE_INLINE void DecrementStackPointer();

	FORCE_INLINE bool Use8bitAccumulator() const;
	FORCE_INLINE bool Use8bitIndexReg() const;
	FORCE_INLINE UInt16_t GetStackAddress() const;

public:
	/*
===================================
3.01 Processor Status Register (P)
===================================

Bits  7   6   5   4   3   2   1   0
                                /---\
                                I e --- Emulation 0 = Native Mode
    /---l---l---l---l---l---l---+---I
    I n I v I m I x I d I i I z I c I
    \-l-I-l-I-l-I-l-I-l-I-l-I-l-I-l-/
      I   I   I   I   I   I   I   \-------- Carry 1 = Carry
      I   I   I   I   I   I   \------------- Zero 1 = Result Zero
      I   I   I   I   I   \---------- IRQ Disable 1 = Disabled
      I   I   I   I   \------------- Decimal Mode 1 = Decimal, 0 = Binary
      I   I   I   \-------- Index Register Select 1 = 8-bit, 0 = 16-bit
      I   I   \-------- Memory/Accumulator Select 1 = 8-bit, 0 = 16 bit
      I   \----------------------------- Overflow 1 = Overflow
      \--------------------------------- Negative 1 = Negative

Emulation Mode Processor Status Register (P)
--------------------------------------------

Bits  7   6   5   4   3   2   1   0
                                /---\
                                I e --- Emulation 1 = 6502 Emulation Mode
    /---l---l---l---l---l---l---+---I
    I n I v I   I b I d I i I z I c I
    \-l-I-l-I---I-l-I-l-I-l-I-l-I-l-/
      I   I       I   I   I   I   \-------- Carry 1 = Carry
      I   I       I   I   I   \------------- Zero 1 = Result Zero
      I   I       I   I   \---------- IRQ Disable 1 = Disabled
      I   I       I   \------------- Decimal Mode 1 = Decimal, 0 = Binary
      I   I       \------------ Break Instruction 1 = BRK caused IRQ
      I   I
      I   \----------------------------- Overflow 1 = Overflow
      \--------------------------------- Negative 1 = Negative

              65816 Emulation Mode Programming Model.
	*/
	struct CPUStatus
	{
		bool Carry : 1;
		bool Zero : 1;
		bool Interrupt : 1;
		bool Decimal : 1;
		bool IndexRegister8bit : 1;
		bool Accumulator8bit : 1;
		bool Overflow : 1;
		bool Sign : 1;
	};

private:

	CPUStatus statusRegister;
	Byte_t* mStatusRegisterPtr;

	bool mIrqPending, mNmiPending;
	bool mStoppedUntilReset;
	bool mEmulationMode, mWaitingForInterrupt;

	UInt16_t programCounter, mStackPointer;
	UInt16_t directPagePointer;
	UInt8_t mDataBankRegister, mProgramBankRegister;

	MultiSizeInt mAccumulator;
	MultiSizeInt mXRegister;
	MultiSizeInt mYRegister;

	bool mResetPending;
	bool mIrqLineLow;

	bool mValidProgramAddressLine, mValidDataAddressLine;
	int mSubCycleMasterTicks;

	FORCE_INLINE void SetSignAndZero( UInt16_t src );
	FORCE_INLINE void SetSignAndZero8( Byte_t src );

	FORCE_INLINE int CalculateRelativeDifference( MultiSizeInt src );
	FORCE_INLINE int CalculateRelativeDifference16( MultiSizeInt b );

	int MasterTicksToAccessMemory( UInt32_t memoryAddress ) const;

	void ClearSubCycle();
	void ProcessSubCycle( bool vda, bool vpa, int cpuCycleCount = 1 ); // NOTE: *Not* master tick count, cpu cycles

	void SetDefaultVariables();

public:
	Processor65C816();

	int ExecuteInstructionFast( SnesCpuMemory* cpuMemory );
	int HandlePendingInterrupts( SnesCpuMemory* cpuMemory );

	void Reset( bool cold );

	FORCE_INLINE void ResetInterrupt( ); // RESET interrupt
	FORCE_INLINE void NMIInterrupt();
	FORCE_INLINE void HoldIRQLineLow( bool low ); // IRQ interrupt

	static void OutputAllInstructions();

};


#include "processor65C816.inl"


#endif


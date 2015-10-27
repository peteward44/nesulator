

#ifndef PROCESSOR65C816_INL
#define PROCESSOR65C816_INL


bool Processor65C816::Use8bitAccumulator() const
{
	return mEmulationMode | statusRegister.Accumulator8bit;
}


bool Processor65C816::Use8bitIndexReg() const
{
	return mEmulationMode | statusRegister.IndexRegister8bit;
}


void Processor65C816::ResetInterrupt( )
{
	mResetPending = true;
}


void Processor65C816::HoldIRQLineLow( bool low )
{
	mIrqLineLow = low;
}


void Processor65C816::NMIInterrupt()
{
	mNmiPending = true;
}


void Processor65C816::SetSignAndZero( UInt16_t src )
{
	statusRegister.Zero = (src & 0xFFFF) == 0;	// This is not valid in decimal mode
	statusRegister.Sign = (src & 0x8000) > 0;
}


void Processor65C816::SetSignAndZero8( Byte_t src )
{
	statusRegister.Zero = (src & 0xFF) == 0;	// This is not valid in decimal mode
	statusRegister.Sign = (src & 0x80) > 0;
}


int Processor65C816::CalculateRelativeDifference( MultiSizeInt b )
{
	const Byte_t rb = b.Data.Part8.b1;
	Byte_t t = ~rb;
	t++;
	return ( (rb & (1 << 7)) > 0 ) ? -(((int)t) + 0) : rb;
}


int Processor65C816::CalculateRelativeDifference16( MultiSizeInt b )
{
	const UInt16_t rb = b.Data.Part16.w1;
	UInt16_t t = ~rb;
	t++;
	return ( (rb & (1 << 15)) > 0 ) ? -(((int)t) + 0) : rb;
}


void Processor65C816::IncrementStackPointer()
{
	mStackPointer--;
}


void Processor65C816::DecrementStackPointer()
{
	mStackPointer++;
}


UInt16_t Processor65C816::GetStackAddress() const
{
	return mEmulationMode ? ( 0x0100 | ( mStackPointer & 0xFF ) ) : mStackPointer;
}


UInt16_t Processor65C816::PopStack16( SnesCpuMemory* cpuMemory )
{ // low byte first
	UInt16_t ret = 0;
	ret |= ( PopStack8( cpuMemory ) & 0xFF );
	ret |= ( PopStack8( cpuMemory ) & 0xFF ) << 8;
	return ret;
}


UInt8_t Processor65C816::PopStack8( SnesCpuMemory* cpuMemory )
{
	DecrementStackPointer();
	UInt8_t ret = cpuMemory->Read8( 0, GetStackAddress() );
//	PWLOG0( LOG_CPU, ( boost::format( "PushStack [%1$04X][%2$02X]" ) % GetStackAddress() % (int)ret ).str() );
	return ret;
}


void Processor65C816::PushStack16( SnesCpuMemory* cpuMemory, UInt16_t val )
{ // high byte first
	PushStack8( cpuMemory, ( val & 0xFF00 ) >> 8 );
	PushStack8( cpuMemory, val & 0xFF );
}


void Processor65C816::PushStack8( SnesCpuMemory* cpuMemory, UInt8_t val )
{
	cpuMemory->Write8( 0, GetStackAddress(), val );
	IncrementStackPointer();
//	PWLOG0( LOG_CPU, ( boost::format( "PushStack [%1$04X][%2$02X]" ) % GetStackAddress() % (int)val ).str() );
}


UInt16_t Processor65C816::TopStack16( SnesCpuMemory* cpuMemory )
{
	UInt16_t ret = 0;
	ret |= ( TopStack8( cpuMemory ) & 0xFF );
	ret |= ( TopStack8( cpuMemory ) & 0xFF ) << 8;
	return ret;
}


UInt8_t Processor65C816::TopStack8( SnesCpuMemory* cpuMemory )
{
	return cpuMemory->Read8( GetStackAddress() );
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


void Processor65C816::OperationADC( MultiSizeInt src ) // UInt16_t
{
	if ( Use8bitAccumulator() )
	{
		unsigned int temp = src.Data.Part8.b1 + mAccumulator.Data.Part8.b1 + (statusRegister.Carry ? 1 : 0);

		statusRegister.Overflow = (!((mAccumulator.Data.Part8.b1 ^ src.Data.Part8.b1) & 0x80) && ((mAccumulator.Data.Part8.b1 ^ temp) & 0x80));
		statusRegister.Carry = temp > 0xff;
		SetSignAndZero8( temp & 0xFF );
	
		mAccumulator.Data.Part8.b1 = (Byte_t)temp;
	}
	else
	{
		unsigned int temp = src.Data.Part16.w1 + mAccumulator.Data.Part16.w1 + (statusRegister.Carry ? 1 : 0);

		statusRegister.Overflow = !( ( mAccumulator.Data.Part16.w1 ^ src.Data.Part16.w1 ) & 0x8000 ) && ( ( mAccumulator.Data.Part16.w1 ^ temp ) & 0x8000 );
		statusRegister.Carry = temp > 0xFFFF;
		SetSignAndZero( temp & 0xFFFF );

		mAccumulator.Data.Part16.w1 = temp & 0xFFFF;
	}
}


void Processor65C816::OperationAND( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		mAccumulator.Data.Part8.b1 &= src.Data.Part8.b1;
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}
	else
	{
		mAccumulator.Data.Part16.w1 &= src.Data.Part16.w1;
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
}


MultiSizeInt Processor65C816::OperationASL( MultiSizeInt src )
{
	MultiSizeInt ret;
	if ( Use8bitAccumulator() )
	{
		statusRegister.Carry = ( ( src.Data.Part8.b1 & 0x80 ) > 0 );
		ret.Data.Part8.b1 = src.Data.Part8.b1 << 1;
		SetSignAndZero8( ret.Data.Part8.b1 );
	}
	else
	{
		statusRegister.Carry = ( ( src.Data.Part16.w1 & 0x8000 ) > 0 );
		ret.Data.Part16.w1 = src.Data.Part16.w1 << 1;
		SetSignAndZero( ret.Data.Part16.w1 );
	}
	return ret;
}


bool Processor65C816::OperationBCC( MultiSizeInt src, bool incrementPc )
{
	if (!statusRegister.Carry)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


bool Processor65C816::OperationBCS( MultiSizeInt src, bool incrementPc )
{
	if (statusRegister.Carry)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


bool Processor65C816::OperationBEQ( MultiSizeInt src, bool incrementPc )
{
	if (statusRegister.Zero)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


void Processor65C816::OperationBIT( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		statusRegister.Zero = ( mAccumulator.Data.Part8.b1 & src.Data.Part8.b1 ) == 0;
		statusRegister.Overflow = ( src.Data.Part8.b1 & 0x20 ) > 0;	// Copy bit 6 to OVERFLOW flag.
		statusRegister.Sign = ( src.Data.Part8.b1 & 0x40 ) > 0;
	}
	else
	{
		statusRegister.Zero = ( mAccumulator.Data.Part16.w1 & src.Data.Part16.w1 ) == 0;
		statusRegister.Overflow = ( src.Data.Part16.w1 & 0x4000 ) > 0;
		statusRegister.Sign = ( src.Data.Part16.w1 & 0x8000 ) > 0;
	}
}


bool Processor65C816::OperationBMI( MultiSizeInt src, bool incrementPc )
{
	if (statusRegister.Sign)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


bool Processor65C816::OperationBNE( MultiSizeInt src, bool incrementPc )
{
	if (!statusRegister.Zero)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


bool Processor65C816::OperationBPL( MultiSizeInt src, bool incrementPc )
{
	if (!statusRegister.Sign)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


bool Processor65C816::OperationBRA( MultiSizeInt src, bool incrementPc ) // NEW
{
	if ( incrementPc )
		programCounter += CalculateRelativeDifference(src);
	return true;
}


bool Processor65C816::OperationBRL( MultiSizeInt src, bool incrementPc ) // NEW
{
	if ( incrementPc ) // TODO: check if this effects program bank register
		programCounter += CalculateRelativeDifference16( src );
	return true;
}


bool Processor65C816::OperationBVC( MultiSizeInt src, bool incrementPc )
{
	if (!statusRegister.Overflow)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


bool Processor65C816::OperationBVS( MultiSizeInt src, bool incrementPc )
{
	if (statusRegister.Overflow)
	{
		if ( incrementPc )
			programCounter += CalculateRelativeDifference(src);
		return true;
	}
	else
		return false;
}


void Processor65C816::OperationCLC()
{
	statusRegister.Carry = false;
}


void Processor65C816::OperationCLD()
{
	statusRegister.Decimal = false;
}


void Processor65C816::OperationCLI()
{
	statusRegister.Interrupt = false;
}


void Processor65C816::OperationCLV()
{
	statusRegister.Overflow = false;
}


void Processor65C816::OperationCMP( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		const Byte_t t = mAccumulator.Data.Part8.b1 - src.Data.Part8.b1;
		statusRegister.Carry = mAccumulator.Data.Part8.b1 > src.Data.Part8.b1;
		SetSignAndZero8( t );
	}
	else
	{
		const UInt16_t t = mAccumulator.Data.Part16.w1 - src.Data.Part16.w1;
		statusRegister.Carry = mAccumulator.Data.Part16.w1 > src.Data.Part16.w1;
		SetSignAndZero( t );
	}
}


void Processor65C816::OperationCPX( MultiSizeInt src )
{
	if ( Use8bitIndexReg() )
	{
		unsigned int t = mXRegister.Data.Part8.b1 - src.Data.Part8.b1;
		statusRegister.Carry = mXRegister.Data.Part8.b1 > src.Data.Part8.b1;
		SetSignAndZero8( t & 0xFF );
	}
	else
	{
		unsigned int t = mXRegister.Data.Part16.w1 - src.Data.Part16.w1;
		statusRegister.Carry = mXRegister.Data.Part16.w1 > src.Data.Part16.w1;
		SetSignAndZero( t & 0xFFFF );
	}
}


void Processor65C816::OperationCPY( MultiSizeInt src )
{
	if ( Use8bitIndexReg() )
	{
		unsigned int t = mYRegister.Data.Part8.b1 - src.Data.Part8.b1;
		statusRegister.Carry = mYRegister.Data.Part8.b1 > src.Data.Part8.b1;
		SetSignAndZero8( t & 0xFF );
	}
	else
	{
		unsigned int t = mYRegister.Data.Part16.w1 - src.Data.Part16.w1;
		statusRegister.Carry = mYRegister.Data.Part16.w1 > src.Data.Part16.w1;
		SetSignAndZero( t & 0xFFFF );
	}
}


MultiSizeInt Processor65C816::OperationDEC( MultiSizeInt src )
{
	MultiSizeInt ret;
	if ( Use8bitAccumulator() )
	{
		ret.Data.Part8.b1 = ( src.Data.Part8.b1 - 1 ) & 0xFF;
		SetSignAndZero8( ret.Data.Part8.b1 );
	}
	else
	{
		ret.Data.Part16.w1 = ( src.Data.Part16.w1 - 1 ) & 0xFFFF;
		SetSignAndZero( ret.Data.Part16.w1 );
	}
	return ret;
}


void Processor65C816::OperationDEX()
{
	if ( Use8bitIndexReg() )
	{
		mXRegister.Data.Part8.b1--;
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
	else
	{
		mXRegister.Data.Part16.w1--;
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationDEY()
{
	if ( Use8bitIndexReg() )
	{
		mYRegister.Data.Part8.b1--;
		SetSignAndZero8( mYRegister.Data.Part8.b1 );
	}
	else
	{
		mYRegister.Data.Part16.w1--;
		SetSignAndZero( mYRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationEOR( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		mAccumulator.Data.Part8.b1 ^= src.Data.Part8.b1;
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}
	else
	{
		mAccumulator.Data.Part16.w1 ^= src.Data.Part16.w1;
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
}


MultiSizeInt Processor65C816::OperationINC( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		src.Data.Part8.b1++;
		SetSignAndZero8( src.Data.Part8.b1 );
	}
	else
	{
		src.Data.Part16.w1++;
		SetSignAndZero( src.Data.Part16.w1 );
	}
	return src;
}


void Processor65C816::OperationINX()
{
	if ( Use8bitIndexReg() )
	{
		mXRegister.Data.Part8.b1++;
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
	else
	{
		mXRegister.Data.Part16.w1++;
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationINY()
{
	if ( Use8bitIndexReg() )
	{
		mYRegister.Data.Part8.b1++;
		SetSignAndZero8( mYRegister.Data.Part8.b1 );
	}
	else
	{
		mYRegister.Data.Part16.w1++;
		SetSignAndZero( mYRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationJML( SnesCpuMemory* cpuMemory, MultiSizeInt src ) // NEW
{
	/*
	The JMP (a) and JML (a) instructions use the direct Bank for indirect
addressing, while JMP (a,x) and JSR (a,x) use the Program Bank for indirect
address tables.
	*/
	PushStack8( cpuMemory, mProgramBankRegister );
	PushStack16( cpuMemory, programCounter - 1 );

	programCounter = src.Data.Part16.w1;
	mProgramBankRegister = src.Data.Part8.b3;
}


void Processor65C816::OperationJMP( MultiSizeInt src )
{
	programCounter = src.Data.Part16.w1;
}


void Processor65C816::OperationJMP24( MultiSizeInt src )
{
	programCounter = src.Data.Part16.w1;
	mProgramBankRegister = src.Data.Part8.b3;
}


void Processor65C816::OperationJSL( SnesCpuMemory* cpuMemory, MultiSizeInt src ) // NEW
{
	PushStack8( cpuMemory, mProgramBankRegister );
	PushStack16( cpuMemory, programCounter - 1 ); // TODO: need to decrement program bank register if going below zero
	programCounter = src.Data.Part16.w1;
//	if ( src.Data.Part32 > 0xFFFF ) // only change program bank if address is greater than 0xFFFF else keep on same bank but roll around (6502 behaviour)
	mProgramBankRegister = src.Data.Part8.b3;
}


void Processor65C816::OperationJSR( SnesCpuMemory* cpuMemory, MultiSizeInt src )
{ // TODO: check should we decrement PC before push?
	PushStack16( cpuMemory, programCounter - 1 );
	programCounter = src.Data.Part16.w1;
}


void Processor65C816::OperationLDA( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		mAccumulator.Data.Part8.b1 = src.Data.Part8.b1;
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}
	else
	{
		mAccumulator.Data.Part16.w1 = src.Data.Part16.w1;
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
}


void Processor65C816::OperationLDX( MultiSizeInt src )
{
	if ( Use8bitIndexReg() )
	{
		mXRegister.Data.Part8.b1 = src.Data.Part8.b1;
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
	else
	{
		mXRegister.Data.Part16.w1 = src.Data.Part16.w1;
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationLDY( MultiSizeInt src )
{
	if ( Use8bitIndexReg() )
	{
		mYRegister.Data.Part8.b1 = src.Data.Part8.b1;
		SetSignAndZero8( mYRegister.Data.Part8.b1 );
	}
	else
	{
		mYRegister.Data.Part16.w1 = src.Data.Part16.w1;
		SetSignAndZero( mYRegister.Data.Part16.w1 );
	}
}


MultiSizeInt Processor65C816::OperationLSR( MultiSizeInt src )
{
	MultiSizeInt ret;
	statusRegister.Carry = ((src.Data.Part8.b1 & 0x01) > 0);
	if ( Use8bitAccumulator() )
	{
		ret.Data.Part8.b1 = src.Data.Part8.b1 >> 1;
		SetSignAndZero8( ret.Data.Part8.b1 );
	}
	else
	{
		ret.Data.Part16.w1 = src.Data.Part16.w1 >> 1;
		SetSignAndZero( ret.Data.Part16.w1 );
	}
	return ret;
}


void Processor65C816::OperationMVN( SnesCpuMemory* cpuMemory, MultiSizeInt operand ) // NEW
{
	const UInt8_t destDataBank = operand.Data.Part8.b1;
	const UInt8_t sourceDataBank = operand.Data.Part8.b2;

	//mDataBankRegister = destDataBank; // TODO: check this

	// TODO: optimise this to make one big memory transfer but as this is rarely used not sure if this is necessary
	while ( mAccumulator.Data.Part16.w1 != 0xFFFF )
	{
		Byte_t b = cpuMemory->Read8( sourceDataBank, mXRegister.Data.Part16.w1 );
		cpuMemory->Write8( destDataBank, mYRegister.Data.Part16.w1, b );

		if ( Use8bitIndexReg() )
		{
			mXRegister.Data.Part8.b1++;
			mYRegister.Data.Part8.b1++;
		}
		else
		{
			mXRegister.Data.Part16.w1++;
			mYRegister.Data.Part16.w1++;
		}
		mAccumulator.Data.Part16.w1--;
	}
}


void Processor65C816::OperationMVP( SnesCpuMemory* cpuMemory, MultiSizeInt operand ) // NEW
{
	const UInt8_t destDataBank = operand.Data.Part8.b1;
	const UInt8_t sourceDataBank = operand.Data.Part8.b2;

	//mDataBankRegister = destDataBank; // TODO: check this

	// TODO: optimise this to make one big memory transfer but as this is rarely used not sure if this is necessary
	while ( mAccumulator.Data.Part16.w1 != 0xFFFF )
	{
		Byte_t b = cpuMemory->Read8( sourceDataBank, mXRegister.Data.Part16.w1 );
		cpuMemory->Write8( destDataBank, mYRegister.Data.Part16.w1, b );
		if ( Use8bitIndexReg() )
		{
			mXRegister.Data.Part8.b1--;
			mYRegister.Data.Part8.b1--;
		}
		else
		{
			mXRegister.Data.Part16.w1--;
			mYRegister.Data.Part16.w1--;
		}
		mAccumulator.Data.Part16.w1--;
	}
}


void Processor65C816::OperationORA( MultiSizeInt src )
{
	if ( Use8bitAccumulator() )
	{
		mAccumulator.Data.Part8.b1 |= src.Data.Part8.b1;
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}
	else
	{
		mAccumulator.Data.Part16.w1 |= src.Data.Part16.w1; 
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
}


void Processor65C816::OperationPEA( SnesCpuMemory* cpuMemory, MultiSizeInt immediateData ) // NEW
{
	PushStack16( cpuMemory, immediateData.Data.Part16.w1 );
}


void Processor65C816::OperationPEI( SnesCpuMemory* cpuMemory, MultiSizeInt dataAtPointer ) // NEW
{
	PushStack16( cpuMemory, dataAtPointer.Data.Part16.w1 );
}


void Processor65C816::OperationPER( SnesCpuMemory* cpuMemory, MultiSizeInt immediateData ) // NEW
{
	UInt16_t temp = programCounter + 3 + immediateData.Data.Part16.w1; // Add 3 (instruction size of PER) so value that is pushed points to the next instruction + immediateData
	PushStack16( cpuMemory, temp );
}


void Processor65C816::OperationPHA( SnesCpuMemory* cpuMemory )
{
	if ( !Use8bitAccumulator() )
		PushStack16( cpuMemory, mAccumulator.Data.Part16.w1 );
	else
		PushStack8( cpuMemory, mAccumulator.Data.Part8.b1 );
}


void Processor65C816::OperationPHB( SnesCpuMemory* cpuMemory ) // NEW
{
	PushStack8( cpuMemory, mDataBankRegister );
}


void Processor65C816::OperationPHD( SnesCpuMemory* cpuMemory ) // NEW
{
	PushStack16( cpuMemory, directPagePointer );
}


void Processor65C816::OperationPHK( SnesCpuMemory* cpuMemory ) // NEW
{
	PushStack8( cpuMemory, mProgramBankRegister );
}


void Processor65C816::OperationPHP( SnesCpuMemory* cpuMemory )
{
	PushStack8( cpuMemory, *mStatusRegisterPtr );
}


void Processor65C816::OperationPHX( SnesCpuMemory* cpuMemory ) // NEW
{
	if ( !Use8bitIndexReg() )
		PushStack16( cpuMemory, mXRegister.Data.Part16.w1 );
	else
		PushStack8( cpuMemory, mXRegister.Data.Part8.b1 );
}


void Processor65C816::OperationPHY( SnesCpuMemory* cpuMemory ) // NEW
{
	if ( !Use8bitIndexReg() )
		PushStack16( cpuMemory, mYRegister.Data.Part16.w1 );
	else
		PushStack8( cpuMemory, mYRegister.Data.Part8.b1 );
}


void Processor65C816::OperationPLA( SnesCpuMemory* cpuMemory )
{
	if ( !Use8bitAccumulator() )
	{
		mAccumulator.Data.Part16.w1 = PopStack16( cpuMemory );
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
	else
	{
		mAccumulator.Data.Part8.b1 = PopStack8( cpuMemory );
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}

}


void Processor65C816::OperationPLB( SnesCpuMemory* cpuMemory ) // NEW
{
	mDataBankRegister = PopStack8( cpuMemory );
}


void Processor65C816::OperationPLD( SnesCpuMemory* cpuMemory ) // NEW
{
	directPagePointer = PopStack16( cpuMemory );
}


void Processor65C816::OperationPLP( SnesCpuMemory* cpuMemory )
{
	*mStatusRegisterPtr = PopStack8( cpuMemory );
}


void Processor65C816::OperationPLX( SnesCpuMemory* cpuMemory ) // NEW
{
	if ( !Use8bitIndexReg() )
	{
		mXRegister.Data.Part16.w1 = PopStack16( cpuMemory );
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
	else
	{
		mXRegister.Data.Part8.b1 = PopStack8( cpuMemory );
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
}


void Processor65C816::OperationPLY( SnesCpuMemory* cpuMemory ) // NEW
{
	if ( !Use8bitIndexReg() )
	{
		mYRegister.Data.Part16.w1 = PopStack16( cpuMemory );
		SetSignAndZero( mYRegister.Data.Part16.w1 );
	}
	else
	{
		mYRegister.Data.Part8.b1 = PopStack8( cpuMemory );
		SetSignAndZero8( mYRegister.Data.Part8.b1 );
	}
}


void Processor65C816::OperationREP( MultiSizeInt newBits ) // NEW
{
	*mStatusRegisterPtr &= (~newBits.Data.Part8.b1);
}



MultiSizeInt Processor65C816::OperationROL( MultiSizeInt src )
{
	MultiSizeInt ret;
	unsigned int t = src.Data.Part32;
	t <<= 1;

	if ( statusRegister.Carry )
		t |= 0x1;

	if ( Use8bitAccumulator() )
	{
		statusRegister.Carry = ( t > 0xFF );
		ret.Data.Part8.b1 = t & 0xFF;
		SetSignAndZero8( ret.Data.Part8.b1 );
	}
	else
	{
		statusRegister.Carry = ( t > 0xFFFF );
		ret.Data.Part16.w1 = t & 0xFFFF;
		SetSignAndZero( ret.Data.Part16.w1 );
	}
	return ret;
}


MultiSizeInt Processor65C816::OperationROR( MultiSizeInt src )
{
	MultiSizeInt ret;
	if ( Use8bitAccumulator() )
	{
		unsigned int t = src.Data.Part8.b1;
		if ( statusRegister.Carry )
			t |= 0x100;
		statusRegister.Carry = ( t & 0x1 );

		t >>= 1;
		ret.Data.Part8.b1 = t & 0xFF;
		SetSignAndZero8( ret.Data.Part8.b1 );
	}
	else
	{
		unsigned int t = src.Data.Part16.w1;
		if ( statusRegister.Carry )
			t |= 0x10000;
		statusRegister.Carry = ( t & 0x1 );

		t >>= 1;
		ret.Data.Part16.w1 = t & 0xFFFF;
		SetSignAndZero( ret.Data.Part16.w1 );
	}
	return ret;
}



void Processor65C816::OperationRTI( SnesCpuMemory* cpuMemory )
{
	*mStatusRegisterPtr = PopStack8( cpuMemory );
	programCounter = PopStack16( cpuMemory );
	if ( mEmulationMode )
		mProgramBankRegister = PopStack8( cpuMemory );
}


void Processor65C816::OperationRTL( SnesCpuMemory* cpuMemory ) // NEW
{
	programCounter = PopStack16( cpuMemory );
	programCounter++;
	mProgramBankRegister = PopStack8( cpuMemory );
}


void Processor65C816::OperationRTS( SnesCpuMemory* cpuMemory )
{
	programCounter = PopStack16( cpuMemory );
	programCounter++;
}



void Processor65C816::OperationSBC( MultiSizeInt src, bool useCarryFlag )
{
	if ( Use8bitAccumulator() )
	{
		unsigned int temp = mAccumulator.Data.Part8.b1 - src.Data.Part8.b1 - (( useCarryFlag && statusRegister.Carry) ? 0 : 1);
		SetSignAndZero8( temp & 0xFF );
		statusRegister.Overflow = (((src.Data.Part8.b1 ^ ( temp & 0xFF )) & 0x80) && ((mAccumulator.Data.Part8.b1 ^ src.Data.Part8.b1) & 0x80));
		statusRegister.Carry = temp < 0x100;
		mAccumulator.Data.Part8.b1 = temp & 0xFF;
	}
	else
	{
		unsigned int temp = mAccumulator.Data.Part16.w1 - src.Data.Part16.w1 - (( useCarryFlag && statusRegister.Carry) ? 0 : 1);
		SetSignAndZero( temp );
		statusRegister.Overflow = (((mAccumulator.Data.Part16.w1 ^ temp) & 0x8000) && ((mAccumulator.Data.Part16.w1 ^ src.Data.Part16.w1) & 0x8000));
		statusRegister.Carry = temp < 0x10000;
		mAccumulator.Data.Part16.w1 = temp & 0xFFFF;
	}
}



void Processor65C816::OperationSEC()
{
	statusRegister.Carry = true;
}


void Processor65C816::OperationSED()
{
	statusRegister.Decimal = true;
}


void Processor65C816::OperationSEI()
{
	statusRegister.Interrupt = true;
}


void Processor65C816::OperationSEP( MultiSizeInt src ) // NEW
{
	/*
	* The REP and SEP instructions cannot modify the M and X bits when in the
	Emulation mode. In this mode the M and X bits will always be high (logic 1).
	*/
	if ( mEmulationMode ) // in emu mode, flags 4+5 are always 1
		*mStatusRegisterPtr |= ( src.Data.Part8.b1 | 0x18 );
	else
		*mStatusRegisterPtr |= src.Data.Part8.b1;
}


void Processor65C816::OperationSTP() // NEW
{
	mStoppedUntilReset = true;
}


MultiSizeInt Processor65C816::OperationSTA( )
{
	return mAccumulator;
}


MultiSizeInt Processor65C816::OperationSTX( )
{
	return mXRegister;
}


MultiSizeInt Processor65C816::OperationSTY( )
{
	return mYRegister;
}


MultiSizeInt Processor65C816::OperationSTZ()
{
	MultiSizeInt ret;
	return ret;
}


void Processor65C816::OperationTAX()
{
	if ( Use8bitIndexReg() )
	{
		mXRegister.Data.Part8.b1 = mAccumulator.Data.Part8.b1;
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
	else
	{
		mXRegister.Data.Part16.w1 = mAccumulator.Data.Part16.w1; // use all 16 bits of accumulator regardless of m status bit
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationTAY()
{
	if ( Use8bitIndexReg() )
	{
		mYRegister.Data.Part8.b1 = mAccumulator.Data.Part8.b1;
		SetSignAndZero8( mYRegister.Data.Part8.b1 );
	}
	else
	{
		mYRegister.Data.Part16.w1 = mAccumulator.Data.Part16.w1; // use all 16 bits of accumulator regardless of m status bit
		SetSignAndZero( mYRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationTCD() // NEW
{
	directPagePointer = mAccumulator.Data.Part16.w1;// always uses 16 bits regardless of m
}


void Processor65C816::OperationTCS() // NEW
{
	if ( mEmulationMode )
	{
		mStackPointer &= 0xFF00;
		mStackPointer |= mAccumulator.Data.Part8.b1; // TODO: Should the top 8 bits of the stack pointer be cleared in this instance?
	}
	else
		mStackPointer = mAccumulator.Data.Part16.w1;
}


void Processor65C816::OperationTDC() // NEW
{
	mAccumulator.Data.Part16.w1 = directPagePointer; // always uses 16 bits regardless of m
}


MultiSizeInt Processor65C816::OperationTRB( MultiSizeInt data ) // NEW
{
	if ( Use8bitAccumulator() )
	{
		data.Data.Part8.b1 &= ~mAccumulator.Data.Part8.b1;
		statusRegister.Zero = data.Data.Part8.b1 == 0;
		return data;
	}
	else
	{
		data.Data.Part16.w1 &= ~mAccumulator.Data.Part16.w1;
		statusRegister.Zero = data.Data.Part16.w1 == 0;
		return data;
	}
}


MultiSizeInt Processor65C816::OperationTSB( MultiSizeInt data ) // NEW
{
	if ( Use8bitAccumulator() )
	{
		data.Data.Part8.b1 |= mAccumulator.Data.Part8.b1;
		statusRegister.Zero = data.Data.Part8.b1 == 0;
		return data;
	}
	else
	{
		data.Data.Part16.w1 |= mAccumulator.Data.Part16.w1;
		statusRegister.Zero = data.Data.Part16.w1 == 0;
		return data;
	}
}


void Processor65C816::OperationTSC() // NEW
{ // TODO: some confusion on what is stored here in emulation mode
	mAccumulator.Data.Part16.w1 = GetStackAddress(); // always uses 16 bits
	SetSignAndZero( mAccumulator.Data.Part16.w1 );
}



void Processor65C816::OperationTSX()
{
	if ( Use8bitIndexReg() )
	{
		mXRegister.Data.Part8.b1 = mStackPointer & 0xFF;
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
	else
	{
		mXRegister.Data.Part16.w1 = mStackPointer;
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationTXA()
{
	if ( Use8bitAccumulator() )
	{
		mAccumulator.Data.Part8.b1 = mXRegister.Data.Part8.b1;
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}
	else
	{
		mAccumulator.Data.Part16.w1 = mXRegister.Data.Part16.w1;
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
}


void Processor65C816::OperationTXS()
{
	mStackPointer = mXRegister.Data.Part16.w1;
}


void Processor65C816::OperationTXY() // NEW
{
	if ( Use8bitIndexReg() )
	{
		mYRegister.Data.Part8.b1 = mXRegister.Data.Part8.b1;
		SetSignAndZero8( mYRegister.Data.Part8.b1 );
	}
	else
	{
		mYRegister.Data.Part16.w1 = mXRegister.Data.Part16.w1;
		SetSignAndZero( mYRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationTYA()
{
	if ( Use8bitAccumulator() )
	{
		mAccumulator.Data.Part8.b1 = mYRegister.Data.Part8.b1;
		SetSignAndZero8( mAccumulator.Data.Part8.b1 );
	}
	else
	{
		mAccumulator.Data.Part16.w1 = mYRegister.Data.Part16.w1;
		SetSignAndZero( mAccumulator.Data.Part16.w1 );
	}
}


void Processor65C816::OperationTYX() // NEW
{
	if ( Use8bitIndexReg() )
	{
		mXRegister.Data.Part8.b1 = mYRegister.Data.Part8.b1;
		SetSignAndZero8( mXRegister.Data.Part8.b1 );
	}
	else
	{
		mXRegister.Data.Part16.w1 = mYRegister.Data.Part16.w1;
		SetSignAndZero( mXRegister.Data.Part16.w1 );
	}
}


void Processor65C816::OperationWAI() // NEW
{
	if ( statusRegister.Interrupt && mIrqPending )
	{ // carry on
		mWaitingForInterrupt = false;
	}
	else
	{
		mWaitingForInterrupt = true;
	}
}


void Processor65C816::OperationXBA() // NEW
{
	std::swap( mAccumulator.Data.Part8.b1, mAccumulator.Data.Part8.b2 );
	SetSignAndZero8( mAccumulator.Data.Part8.b1 );
}


void Processor65C816::OperationXCE() // NEW
{
	bool temp = statusRegister.Carry;
	statusRegister.Carry = mEmulationMode;
	mEmulationMode = temp;
	if ( mEmulationMode )
		*mStatusRegisterPtr |= 0x18;
}




#endif


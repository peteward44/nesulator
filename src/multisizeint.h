
#pragma once

#include "main.h"


struct MultiSizeInt
{
	MultiSizeInt( UInt32_t i = 0 )
	{
		Data.Part32 = i;
	}

	union
	{
		struct
		{
			UInt8_t b1;
			UInt8_t b2;
			UInt8_t b3;
			UInt8_t b4;
		} Part8;
		struct
		{
			UInt16_t w1;
			UInt16_t w2;
		} Part16;
		UInt32_t Part32;
	} Data;

};



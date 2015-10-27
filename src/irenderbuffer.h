
#ifndef IRENDERBUFFER_H
#define IRENDERBUFFER_H

#include "main.h"


class IRenderBuffer
{
public:
	virtual ~IRenderBuffer() {}

	virtual const Byte_t* GetRGBBuffer( bool upsideDown ) = 0 {}
	virtual void ClearBuffer( Byte_t backgroundPaletteIndex ) = 0 {}

	virtual void GetBufferSize( int& x, int& y ) const = 0 {}

};



#endif


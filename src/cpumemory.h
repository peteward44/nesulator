

#ifndef CPUMEMORY_H
#define CPUMEMORY_H

#include "main.h"
#include "input.h"
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cstring>

typedef Page< 0x2000, 0 > PRGPage_t;
typedef boost::shared_ptr< PRGPage_t > PRGPagePtr_t;
typedef Page< 0x800, 0x0 > RAMPage_t;

// CPUMemory - main memory bus

/*
    +---------+-------+-------+-----------------------+
    | Address | Size  | Flags | Description           |
    +---------+-------+-------+-----------------------+
    | $0000   | $800  |       | RAM                   |
    | $0800   | $800  | M     | RAM                   |
    | $1000   | $800  | M     | RAM                   |
    | $1800   | $800  | M     | RAM                   |
    | $2000   | 8     |       | Registers             |
    | $2008   | $1FF8 |  R    | Registers             |
    | $4000   | $17   |       | Registers             |
    | $4017   | $FE9  |   U   |                       |
    | $5000   | $1000 |       | Expansion Modules     |
    | $6000   | $2000 |       | SRAM                  |
    | $8000   | $4000 |       | PRG-ROM               |
    | $C000   | $4000 |       | PRG-ROM               |
    +---------+-------+-------+-----------------------+
           Flag Legend: M = Mirror of $0000
                        R = Mirror of $2000-$2008 every 8 bytes
                            (e.g. $2008=$2000, $2018=$2000, etc.)
                        U = Unknown
*/

class CPUMemory : public HasState
{
private:
	boost::signals::connection resetConnection;
	RAMPage_t ramPage; // RAM (0x0000 -> 0x4000) 0x800 x4 *all mirrored*

public:
	CPUMemory();
	~CPUMemory();

	void OnReset( bool cold );

	virtual void Write8( UInt16_t offset, Byte_t data );
	virtual Byte_t Read8( UInt16_t offset );
	UInt16_t Read16( UInt16_t offset, bool zeropage );

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );
};



#endif

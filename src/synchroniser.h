
#ifndef SYNCHRONISER_H
#define SYNCHRONISER_H

#include "main.h"

// Synchroniser class is responsible for ensuring all sub components on the NES
// stay abreast of each other

enum SYNC_ID
{
	SYNC_PPU = 0,
	SYNC_APU,
	SYNC_MAPPER,
	SYNC_COUNT
};


class Synchronisable
{
public:
	virtual ~Synchronisable() {}

	virtual void Synchronise( int syncMTC ) = 0;
	virtual int GetNextEvent( int cpuMTC ) = 0;
	virtual void OnEndFrame() = 0;
};


class Synchroniser : public HasState
{
private:
	boost::signals::connection resetConnection;

	Synchronisable* objects[ SYNC_COUNT ];

	int cpuMtc, currentNextEvent;
	bool mIsSynchronising, mBufferedSync;

	int GetNextEvent( int frameTicks );

public:
	Synchroniser();
	~Synchroniser();

	void AdvanceCpuMTC( int advance );
	int GetCpuMTC() const;

	void OnReset( bool cold );

	void RunCycle();

	void Synchronise( int ticks = -1 );
	void RegisterListener( int id, Synchronisable* syncObject );

	void NewNextEvent( int nextEvent ); // called by Synchronisable objects if it needs to synchronise sooner than it requested by GetNextEvent()

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );
};


#endif

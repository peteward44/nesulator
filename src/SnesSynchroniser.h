
#ifndef SNESSYNCHRONISER_H
#define SNESSYNCHRONISER_H


enum SNES_SYNC_ID
{
	SNES_SYNC_PPU = 0,
	SNES_SYNC_APU,
	SNES_SYNC_COUNT
};



class SnesSynchronisable
{
public:
	virtual ~SnesSynchronisable() {}

	virtual void Synchronise( int syncMTC ) = 0;
	virtual int GetNextEvent( int cpuMTC ) = 0;
	virtual void OnEndFrame() = 0;
};



class SnesSynchroniser : public HasState
{
private:
	boost::signals::connection resetConnection;

	SnesSynchronisable* objects[ SNES_SYNC_COUNT ];

	int cpuMtc, currentNextEvent;
	bool mIsSynchronising, mBufferedSync;

	int GetNextEvent( int frameTicks );

public:
	SnesSynchroniser();
	~SnesSynchroniser();

	void AdvanceCpuMTC( int advance );
	int GetCpuMTC() const;

	void OnReset( bool cold );

	void RunCycle();

	void Synchronise( int ticks = -1 );
	void RegisterListener( int id, SnesSynchronisable* syncObject );

	void NewNextEvent( int nextEvent ); // called by Synchronisable objects if it needs to synchronise sooner than it requested by GetNextEvent()

	virtual void SaveState( std::ostream& ostr );
	virtual void LoadState( std::istream& istr );
};


#endif


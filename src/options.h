
#ifndef OPTIONS_H
#define OPTIONS_H

#include <string>
#include "input.h"
#include "boost/signal.hpp"


class JoypadKeyMap
{
private:
	int keys[ JS_BUTTON_COUNT ];

public:
	JoypadKeyMap();

	JoypadKeyMap( const JoypadKeyMap& jkm );
	JoypadKeyMap& operator = ( const JoypadKeyMap& jkm );

	void SetKey( int key, JOYSTICK_BUTTON button );

	JOYSTICK_BUTTON KeyCodeToButton( int key ) const;
	int ButtonToKeyCode( JOYSTICK_BUTTON button ) const;

};



class Options
{
private:
	JoypadKeyMap keymaps[ 2 ];
	INPUT_DEVICE inputDevices[2];

	typedef boost::signal0< void > OptionsSyncEvent_t;
	OptionsSyncEvent_t mSyncEvent;

public:
	Options();

	template < class FUNCTOR >
	void AttachToSyncEvent( FUNCTOR functor )
	{
		mSyncEvent.connect( functor );
	}

	void InvokeSyncEvent()
	{
		mSyncEvent();
	}

	FORCE_INLINE JoypadKeyMap& GetJoypadKeyMap( int port )
	{ assert( port >= 0 && port < 2 ); return keymaps[ port ]; }

	FORCE_INLINE INPUT_DEVICE GetDeviceType( int port )
	{ assert( port >= 0 && port < 2 ); return inputDevices[ port ]; }

	FORCE_INLINE void SetDeviceType( int port, INPUT_DEVICE device )
	{ assert( port >= 0 && port < 2 ); inputDevices[ port ] = device; }

	void LoadFromFile( const std::string& filename );
	void SaveToFile( const std::string& filename );

	bool AllowMoreThanEightSprites;
	bool SoundEnabled;
	int SoundVolume;
	int WindowWidth, WindowHeight;

	int ApplicationSpeed;

	bool UseNtscFiltering;

	std::string SaveStatesDirectory, ScreenshotsDirectory;
};


extern Options* g_options;




#endif



// Simple sound queue for synchronous sound handling in SDL

// Copyright (C) 2005 Shay Green. MIT license.

#ifndef SOUND_QUEUE_H
#define SOUND_QUEUE_H
#define BUILD_WITH_SDL_SOUND

#include "boost/cstdint.hpp"
#ifdef BUILD_WITH_SDL_SOUND
#define SDL_MAIN_HANDLED
#include "SDL.h"
#endif

// Simple SDL sound wrapper that has a synchronous interface
class Sound_Queue {
public:
	Sound_Queue();
	~Sound_Queue();
	
	// Initialize with specified sample rate and channel count.
	// Returns NULL on success, otherwise error string.
	const char* init( long sample_rate, int chan_count = 1 );
	
	// Number of samples in buffer waiting to be played
	int sample_count() const;
	
	// Write samples to buffer and block until enough space is available
	typedef short sample_t;
	void write( const sample_t*, int count );
	
private:
	enum { buf_size = 2048 };
	enum { buf_count = 3 };
	sample_t* volatile bufs;
#ifdef BUILD_WITH_SDL_SOUND
	SDL_sem* volatile free_sem;
#endif
	int volatile read_buf;
	int write_buf;
	int write_pos;
	bool sound_open;
	
	sample_t* buf( int index );
	void fill_buffer( boost::uint8_t*, int );
	static void fill_buffer_( void*, boost::uint8_t*, int );
};

#endif


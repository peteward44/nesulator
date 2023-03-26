
// Nes_Snd_Emu 0.1.7. http://www.slack.net/~ant/


#include "Sound_Queue.h"

#include <assert.h>
#include <string.h>

#ifdef BUILD_WITH_SDL_SOUND
#pragma comment(lib, "SDL2.lib")
//#pragma comment(lib, "SDL2main.lib")
#pragma comment(lib, "SDL2_sound.lib")
#endif

/* Copyright (C) 2005 by Shay Green. Permission is hereby granted, free of
charge, to any person obtaining a copy of this software module and associated
documentation files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use, copy, modify,
merge, publish, distribute, sublicense, and/or sell copies of the Software, and
to permit persons to whom the Software is furnished to do so, subject to the
following conditions: The above copyright notice and this permission notice
shall be included in all copies or substantial portions of the Software. THE
SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

// Return current SDL_GetError() string, or str if SDL didn't have a string
static const char* sdl_error( const char* str )
{
#ifdef BUILD_WITH_SDL_SOUND
	const char* sdl_str = SDL_GetError();
	if ( sdl_str && *sdl_str )
		str = sdl_str;
	return str;
#endif
	return "";
}

Sound_Queue::Sound_Queue()
{
	bufs = NULL;
#ifdef BUILD_WITH_SDL_SOUND
	free_sem = NULL;
#endif
	write_buf = 0;
	write_pos = 0;
	read_buf = 0;
	sound_open = false;
}

Sound_Queue::~Sound_Queue()
{
#ifdef BUILD_WITH_SDL_SOUND
	if ( sound_open )
	{
		SDL_PauseAudio( true );
		SDL_CloseAudio();
	}
	
	if ( free_sem )
		SDL_DestroySemaphore( free_sem );
	
	delete [] bufs;
#endif
}

int Sound_Queue::sample_count() const
{
#ifdef BUILD_WITH_SDL_SOUND
	int buf_free = SDL_SemValue( free_sem ) * buf_size + (buf_size - write_pos);
	return buf_size * buf_count - buf_free;
#endif
	return 0;
}

const char* Sound_Queue::init( long sample_rate, int chan_count )
{
#ifdef BUILD_WITH_SDL_SOUND
	assert( !bufs ); // can only be initialized once
	
	bufs = new sample_t [(long) buf_size * buf_count];
	if ( !bufs )
		return "Out of memory";
	
	free_sem = SDL_CreateSemaphore( buf_count - 1 );
	if ( !free_sem )
		return sdl_error( "Couldn't create semaphore" );
	
	SDL_AudioSpec as;
	as.freq = sample_rate;
	as.format = AUDIO_S16SYS;
	as.channels = chan_count;
	as.silence = 0;
	as.samples = buf_size;
	as.size = 0;
	as.callback = fill_buffer_;
	as.userdata = this;
	if ( SDL_OpenAudio( &as, NULL ) < 0 )
		return sdl_error( "Couldn't open SDL audio" );
	SDL_PauseAudio( false );
	sound_open = true;
#endif
	return NULL;
}

inline Sound_Queue::sample_t* Sound_Queue::buf( int index )
{
#ifdef BUILD_WITH_SDL_SOUND
	assert( (unsigned) index < buf_count );
	return bufs + (long) index * buf_size;
#endif
	return 0;
}

void Sound_Queue::write( const sample_t* in, int count )
{
#ifdef BUILD_WITH_SDL_SOUND
	while ( count )
	{
		int n = buf_size - write_pos;
		if ( n > count )
			n = count;
		
		memcpy( buf( write_buf ) + write_pos, in, n * sizeof (sample_t) );
		in += n;
		write_pos += n;
		count -= n;
		
		if ( write_pos >= buf_size )
		{
			write_pos = 0;
			write_buf = (write_buf + 1) % buf_count;
			SDL_SemWait( free_sem );
		}
	}
#endif
}

void Sound_Queue::fill_buffer(boost::uint8_t* out, int count )
{
#ifdef BUILD_WITH_SDL_SOUND
	if ( SDL_SemValue( free_sem ) < buf_count - 1 )
	{
		memcpy( out, buf( read_buf ), count );
		read_buf = (read_buf + 1) % buf_count;
		SDL_SemPost( free_sem );
	}
	else
	{
		memset( out, 0, count );
	}
#endif
}

void Sound_Queue::fill_buffer_( void* user_data, boost::uint8_t* out, int count )
{
#ifdef BUILD_WITH_SDL_SOUND
	((Sound_Queue*) user_data)->fill_buffer( out, count );
#endif
}


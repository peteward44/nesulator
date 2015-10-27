

#ifndef APU_H
#define APU_H


#ifdef USE_DIRECTSOUND

#include "apu_dx.h"

#define APU		APU_DX

#else

#include "apu_sdl.h"

#define APU		APU_SDL

#endif



#endif


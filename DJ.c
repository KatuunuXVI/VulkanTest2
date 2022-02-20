#include "DJ.h"
#include "SDL2/include/SDL.h"
#include "SDL2/include/SDL_mixer.h"
#include <stdio.h>

/**Members*/
static Mix_Music* song;
static double loopPos;
static int musicVolume;

void initAudioPlayer() {
    if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 1024 ) < 0 ) {
        printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
    }
    musicVolume = Mix_VolumeMusic(-1);
}

void loopMusic() {
    Mix_PlayMusic( song, 1 );
    Mix_SetMusicPosition(loopPos);
}

void playMusic(const char* source, double loopFrom) {
    if((song = Mix_LoadMUS(source)) == 0) {
        fprintf(stderr,"Audio File Not Found\n");
        exit(1);
    }
    loopPos = loopFrom;
    Mix_HookMusicFinished(loopMusic);
    Mix_PlayMusic( song, 1 );
}

void stopMusic(int fadeTime) {
    Mix_HookMusicFinished(0);
    if(fadeTime) {
        Mix_FadeOutMusic(fadeTime);
    } else {
        Mix_HaltMusic();
    }
}
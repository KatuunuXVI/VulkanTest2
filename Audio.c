
#include "Audio.h"
#include <al.h>
#include <alc.h>
#include <stdio.h>


#ifndef NDEBUG
#define alCall(func) func; checkALErrors(__FILE__, __LINE__, function, __VA_ARGS__)
#define alcCall(func) func; checkALCErrors(__FILE__, __LINE__, function, __VA_ARGS__)
#else
#define alCall(func) func
#define alcCall(func) func
#endif

bool checkALErrors(const char* filename, uint line) {
    ALenum error = alGetError();
    if(error != AL_NO_ERROR)
    {
        fprintf(stderr, "Error: %s: %d\n", filename, line);
        switch(error) {
            case AL_INVALID_NAME:
                fprintf(stderr,"AL_INVALID_NAME: a bad name (ID) was passed to an OpenAL function");
                break;
            case AL_INVALID_ENUM:
                fprintf(stderr,"AL_INVALID_ENUM: an invalid enum value was passed to an OpenAL function");
                break;
            case AL_INVALID_VALUE:
                fprintf(stderr,"AL_INVALID_VALUE: an invalid value was passed to an OpenAL function");
                break;
            case AL_INVALID_OPERATION:
                fprintf(stderr,"AL_INVALID_OPERATION: the requested operation is not valid");
                break;
            case AL_OUT_OF_MEMORY:
                fprintf(stderr,"AL_OUT_OF_MEMORY: the requested operation resulted in OpenAL running out of memory");
                break;
            default:
                fprintf(stderr,"UNKNOWN AL ERROR: %d\n", error);
        }
        fprintf(stderr,"\n");
        return FALSE;
    }
    return TRUE;
}

bool checkALCErrors(const char* filename, uint line, ALCdevice* device) {
    ALCenum error = alcGetError(device);
    if(error != ALC_NO_ERROR)
    {
        fprintf(stderr,"ERROR: ");
        switch(error)
        {
            case ALC_INVALID_VALUE:
                fprintf(stderr,"ALC_INVALID_VALUE: an invalid value was passed to an OpenAL function\n");
                break;
            case ALC_INVALID_DEVICE:
                fprintf(stderr,"ALC_INVALID_DEVICE: a bad device was passed to an OpenAL function\n");
                break;
            case ALC_INVALID_CONTEXT:
                fprintf(stderr,"ALC_INVALID_CONTEXT: a bad context was passed to an OpenAL function\n");
                break;
            case ALC_INVALID_ENUM:
                fprintf(stderr,"ALC_INVALID_ENUM: an unknown enum value was passed to an OpenAL function\n");
                break;
            case ALC_OUT_OF_MEMORY:
                fprintf(stderr,"ALC_OUT_OF_MEMORY: an unknown enum value was passed to an OpenAL function\n");
                break;
            default:
                fprintf(stderr,"UNKNOWN ALC ERROR: \n");
        }
        return FALSE;
    }
    return TRUE;
}

bool getAvailableDevices(const char* vector, ALCdevice* device) {
    const ALCchar* devices;
    //if(!alcCall())
}

//void openAudio
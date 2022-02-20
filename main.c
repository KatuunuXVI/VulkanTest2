#include <stdio.h>
#include <stdlib.h>
#include "Display.h"
#include "Audio.h"
#include <vorbis/vorbisfile.h>
#include <vorbis/vorbisenc.h>
#include <vorbis/codec.h>
#include "DJ.h"
typedef struct fShader {
    int size;
    char* instructions;
} fShader;



int main() {
    //printf("%d\n", DEB);
    //StreamingAudioData aData;
    //oggToStream("gc.ogg", &aData);
    //playStream(&aData);
    //return 0;
    /**Recompile Shaders*/
    system("compileShaders.bat");
    openDisplay();
    runDisplay();
    closeDisplay();
    return 0;
}

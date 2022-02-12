#include <stdio.h>
#include <stdlib.h>
#include "Display.h"


typedef struct fShader {
    int size;
    char* instructions;
} fShader;



int main() {


    /**Recompile Shaders*/
    system("compileShaders.bat");
    openDisplay();
    runDisplay();
    closeDisplay();
    return 0;
}

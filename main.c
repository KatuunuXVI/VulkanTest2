#include <stdio.h>
#include "Display.h"


typedef struct fShader {
    int size;
    char* instructions;
} fShader;



int main() {

    openDisplay();
    runDisplay();
    closeDisplay();
    return 0;
}

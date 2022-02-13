#ifndef VULKANCLEANUP_EXTOOLS_H
#define VULKANCLEANUP_EXTOOLS_H
#define TRUE 1
#define FALSE 0
typedef unsigned int uint;
typedef unsigned char bool;
typedef unsigned char bool;

typedef struct cPoint {
    char x;
    char y;
} cPoint;

typedef struct ucPoint {
    unsigned char x;
    unsigned char y;
} ucPoint;

typedef struct sPoint {
    short x;
    short y;
} sPoint;

typedef struct usPoint {
    unsigned short x;
    unsigned short y;
} usPoint;

typedef struct iPoint {
    int x;
    int y;
} iPoint;

typedef struct uiPoint {
    unsigned int x;
    unsigned int y;
} uiPoint;

typedef struct lPoint {
    long x;
    long y;
} lPoint;

typedef struct ulPoint {
    unsigned long x;
    unsigned long y;
} ulPoint;

void require(bool condition, const char* message);

void printIntArray(int* arr, int size);

void printShortArray(short* arr, int size);


#endif //VULKANCLEANUP_EXTOOLS_H

#include <stdio.h>
#include <math.h>

#define RIDGE_DETECTION
#define EDGE_DETECTION
#define SHARPEN
#define BOX_BLUR
#define GAUSSIAN_BLUR3X3
#define GAUSSIAN_BLUR5X5
#define UNSHARP_MASKING

const double ridgeDetection[3][3] =
{
    {0, -1, 0},
    {-1, 4, -1},
    {0, -1, 0}
};

const double edgeDetection[3][3] =
{
    {-1, -1, -1},
    {-1, 8, -1},
    {-1, -1, -1}
};

const double sharpen[3][3] =
{
    {0, -1, 0},
    {-1, 5, -1},
    {0, -1, 0}
};

const double boxBlur[3][3] =
{
    {1/9, 1/9, 1/9},
    {1/9, 1/9, 1/9},
    {1/9, 1/9, 1/9}
};

const double gaussianBlur3x3[3][3] =
{
    {1/16, 2/16, 1/16},
    {2/16, 4/16, 2/16},
    {1/16, 2/16, 1/16}
};

const double gaussianBlur5x5[5][5] =
{
    {1/256, 4/256, 6/256, 4/256, 1/256},
    {4/256, 16/256, 24/256, 16/256, 4/256},
    {6/256, 24/256, 36/256, 24/256, 6/256},
    {4/256, 16/256, 24/256, 16/256, 4/256},
    {1/256, 4/256, 6/256, 4/256, 1/256},
};

const double unsharpMasking[5][5] =
{
    {-1/256, -4/256, -6/256, -4/256, -1/256},
    {-4/256, -16/256, -24/256, -16/256, -4/256},
    {-6/256, -24/256, 476/256, -24/256, -6/256},
    {-4/256, -16/256, -24/256, -16/256, -4/256},
    {-1/256, -4/256, -6/256, -4/256, -1/256},
};

void imageProcessing(double image[][], int option){
    if (option <=5 && option >=0){

    } else if(option ==6){

    }else{
        printf("du är dum i huvudet");
    }
};
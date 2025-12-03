#include "kernel.h"
static void applyKernel(int w, int h, volatile char input[w][h], volatile char output[w][h], const double *k, int radius);
static void inverted(int w, int h, volatile char input[w][h], volatile char output[w][h]);
void imageProcessing(int w, int h, volatile char input[w][h], volatile char output[w][h], int option);
extern char getColor(char pixel, char color);
extern void print(const char*);

const double ridgeDetection[3][3] =
{
    {0.0, -1.0, 0.0},
    {-1.0, 4.0, -1.0},
    {0.0, -1.0, 0.0}
};

const double edgeDetection[3][3] =
{
    {-1.0, -1.0, -1.0},
    {-1.0, 8.0, -1.0},
    {-1.0, -1.0, -1.0}
};

const double sharpen[3][3] =
{
    {0.0, -1.0, 0.0},
    {-1.0, 5.0, -1.0},
    {0.0, -1.0, 0.0}
};

const double boxBlur[3][3] =
{
    {1.0/9.0, 1.0/9.0, 1.0/9.0},
    {1.0/9.0, 1.0/9.0, 1.0/9.0},
    {1.0/9.0, 1.0/9.0, 1.0/9.0}
};

const double gaussianBlur3x3[3][3] =
{
    {1.0/16.0, 2.0/16.0, 1.0/16.0},
    {2.0/16.0, 4.0/16.0, 2.0/16.0},
    {1.0/16.0, 2.0/16.0, 1.0/16.0}
};

const double gaussianBlur5x5[5][5] =
{
    {1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
    {4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
    {6.0/256.0, 24.0/256.0, 36.0/256.0, 24.0/256.0, 6.0/256.0},
    {4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
    {1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
};

const double unsharpMasking[5][5] =
{
    {-1.0/256.0, -4.0/256.0, -6.0/256.0, -4.0/256.0, -1.0/256.0},
    {-4.0/256.0, -16.0/256.0, -24.0/256.0, -16.0/256.0, -4.0/256.0},
    {-6.0/256.0, -24.0/256.0, 476.0/256.0, -24.0/256.0, -6.0/256.0},
    {-4.0/256.0, -16.0/256.0, -24.0/256.0, -16.0/256.0, -4.0/256.0},
    {-1.0/256.0, -4.0/256.0, -6.0/256.0, -4.0/256.0, -1.0/256.0},
};

void imageProcessing(int w, int h, volatile char input[w][h], volatile char output[w][h], int option){
    print("Processing... ");
    switch (option) {
        case RIDGE_DETECTION:
            print("RIDGE_DETECTION ");
            applyKernel(w, h, input, output, &ridgeDetection[0][0], 1);
            break;
        case EDGE_DETECTION:
            print("EDGE_DETECTION ");
            applyKernel(w, h, input, output, &edgeDetection[0][0], 1);
            break;
        case SHARPEN:
            print("SHARPEN ");
            applyKernel(w, h, input, output, &sharpen[0][0], 1);
            break;
        case BOX_BLUR:
            print("BOX_BLUR ");
            applyKernel(w, h, input, output, &boxBlur[0][0], 1);
            break;
        case GAUSSIAN_BLUR3X3:
            print("GAUSSIAN_BLUR3X3 ");
            applyKernel(w, h, input, output, &gaussianBlur3x3[0][0], 1);
            break;
        case GAUSSIAN_BLUR5X5:
            print("GAUSSIAN_BLUR5X5 ");
            applyKernel(w, h, input, output, &gaussianBlur5x5[0][0], 2);
            break;
        case UNSHARP_MASKING:
            print("UNSHARP_MASKING ");
            applyKernel(w, h, input, output, &unsharpMasking[0][0], 2);
            break;
        case INVERTED:
            print("INVERTED ");
            inverted(w, h, input, output);
            break;
        default:
            print("Default ");
            for (int x = 0; x < w; ++x){
                for(int y = 0; y < h; ++y){
                    output[x][y] = input[x][y];
                }
            } 
            break;
    }
    print("Done!\n");
}

static int clamp_int(int v, int max){
    if (v < 0) return 0;
    if (v > max) return max;
    return v;
}

static void applyKernel(int w, int h, volatile char input[w][h], volatile char output[w][h], const double *k, int radius){
    int size = 2 * radius + 1;

    for (int x = 0; x < w; ++x){
        for (int y = 0; y < h; ++y){
            double R = 0.0, G = 0.0, B = 0.0;

            for (int dx = -radius; dx <= radius; ++dx){
                for (int dy = -radius; dy <= radius; ++dy){
                    int kx = dx + radius;
                    int ky = dy + radius;
                    double c = k[kx * size + ky];

                    int sx = clamp_int(x+dx, w-1);
                    int sy = clamp_int(y+dy, h-1);

                    char p = input[sx][sy];
                    R += c * getColor(p, 0);
                    G += c * getColor(p, 1);
                    B += c * getColor(p, 2);
                }
            }
            output[x][y] = (((unsigned char) R & 224) | ((unsigned char) G & 28) | ((unsigned char) B & 3));
        }
    }
}

static void inverted(int w, int h, volatile char input[w][h], volatile char output[w][h]){
    for (int x = 0; x < w; ++x){
        for (int y = 0; y < h; ++y){
            char p = input[x][y];
            double R = 224.0 - getColor(p, 0);
            double G = 28.0 - getColor(p, 1);
            double B = 3.0 - getColor(p, 2);

            output[x][y] = (((unsigned char) R & 224) | ((unsigned char) G & 28) | ((unsigned char) B & 3));
        }
    }
}

/*
* kernel.c
* Authors: Simon kåhre, Leo Ehrenberg
* Date: 07-12-2025
*/

#include "kernel.h"
static void applyKernel(int w, int h, volatile char input[w][h], volatile char output[w][h], const double *k, int radius);
static void inverted(int w, int h, volatile char input[w][h], volatile char output[w][h]);
void imageProcessing(int w, int h, volatile char input[w][h], volatile char output[w][h], int option);
extern char getColor(char pixel, char color);
extern void print(const char*);

// Highlights ridge structures
const double ridgeDetection[3][3] =
{
    {0.0, -1.0, 0.0},
    {-1.0, 4.0, -1.0},
    {0.0, -1.0, 0.0}
};

// Strong response where intensity changes sharply
const double edgeDetection[3][3] =
{
    {-1.0, -1.0, -1.0},
    {-1.0, 8.0, -1.0},
    {-1.0, -1.0, -1.0}
};

// Enhances edges while keeping overall structure
const double sharpen[3][3] =
{
    {0.0, -1.0, 0.0},
    {-1.0, 5.0, -1.0},
    {0.0, -1.0, 0.0}
};

// Each pixel becomes the avrage of its 3x3 neighborhood
const double boxBlur[3][3] =
{
    {1.0/9.0, 1.0/9.0, 1.0/9.0},
    {1.0/9.0, 1.0/9.0, 1.0/9.0},
    {1.0/9.0, 1.0/9.0, 1.0/9.0}
};

// Smoother blur that weights center pixels higher than corners
const double gaussianBlur3x3[3][3] =
{
    {1.0/16.0, 2.0/16.0, 1.0/16.0},
    {2.0/16.0, 4.0/16.0, 2.0/16.0},
    {1.0/16.0, 2.0/16.0, 1.0/16.0}
};

// Larger and smoother blur
const double gaussianBlur5x5[5][5] =
{
    {1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
    {4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
    {6.0/256.0, 24.0/256.0, 36.0/256.0, 24.0/256.0, 6.0/256.0},
    {4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
    {1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
};

// Combines a strong positive center with negative surroundings,
// effectiveky sharpening the image by substracting a blurred version
const double unsharpMasking[5][5] =
{
    {-1.0/256.0, -4.0/256.0, -6.0/256.0, -4.0/256.0, -1.0/256.0},
    {-4.0/256.0, -16.0/256.0, -24.0/256.0, -16.0/256.0, -4.0/256.0},
    {-6.0/256.0, -24.0/256.0, 476.0/256.0, -24.0/256.0, -6.0/256.0},
    {-4.0/256.0, -16.0/256.0, -24.0/256.0, -16.0/256.0, -4.0/256.0},
    {-1.0/256.0, -4.0/256.0, -6.0/256.0, -4.0/256.0, -1.0/256.0},
};

/**
 * Selects the appropriate kernel/filter based on option and applies
 * it to the input image and returns it as output.
 */
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
            // Default: no processing, copy input directly to output
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

/**
 * Clamps an integer value v into the range [0, max].
 * 
 * used for boundry handling when sampling pixels outside the image.
 * this copies the value of the closest edge (clamp to edge).
 */
static int clamp_int(int v, int max){
    if (v < 0) return 0;
    if (v > max) return max;
    return v;
}

/**
 * Generic NxN convolution kernel applicator for 3:3:2 RGB imgages.
 * 
 * w, h -> image dimensions
 * input -> source image with pixel values RRR GGG BB
 * output -> processed image (same format as input)
 * k -> pointer to the first element of a kernel
 * 
 * Border handling:
 * Pixels near the edge (x+dx, y+dy) are clamped to [0, w-1]/[0, h-1]
 * 
 */
static void applyKernel(int w, int h, volatile char input[w][h], volatile char output[w][h], const double *k, int radius){
    int size = 2 * radius + 1; // Kernel width/height (3 or 5)

    // Loop over every pixel in the image, including borders
    for (int x = 0; x < w; ++x){
        for (int y = 0; y < h; ++y){
            double R = 0.0, G = 0.0, B = 0.0;

            // Convolution window: iterate over all neighbors within radius
            for (int dx = -radius; dx <= radius; ++dx){
                for (int dy = -radius; dy <= radius; ++dy){
                    // Kernel coordinates (0, ..., size-1)
                    int kx = dx + radius;
                    int ky = dy + radius;
                    double c = k[kx * size + ky];

                    // Sample coordinates from image with clamp to edge
                    int sx = clamp_int(x+dx, w-1);
                    int sy = clamp_int(y+dy, h-1);
                    char p = input[sx][sy];

                    // Accumulate per-color values
                    R += c * getColor(p, 0); // RED
                    G += c * getColor(p, 1); // GREEN
                    B += c * getColor(p, 2); // BLUE
                }
            }
            // Re-pack accumulated RGB values back into 3:3:2 format
            // Bit mask (224,28, 3), only the relevant bits are kept.
            output[x][y] = (((unsigned char) R & 224) | ((unsigned char) G & 28) | ((unsigned char) B & 3));
        }
    }
}
/**
 * Inverted filter
 * 
 * outputs a simple color inversion effect on the packed 3:3:2 pixel:
 * 
 * inverted color = maxvalue - inputvalue
 * flips each color within its limited bit range.
 */
static void inverted(int w, int h, volatile char input[w][h], volatile char output[w][h]){
    for (int x = 0; x < w; ++x){
        for (int y = 0; y < h; ++y){
            char p = input[x][y];
            
            // Compute inverted color values in their respective ranges
            double R = 224.0 - getColor(p, 0);
            double G = 28.0 - getColor(p, 1);
            double B = 3.0 - getColor(p, 2);

            // Re-pack RGB values into 8-bit 3:3:2 pixel format
            output[x][y] = (((unsigned char) R & 224) | ((unsigned char) G & 28) | ((unsigned char) B & 3));
        }
    }
}

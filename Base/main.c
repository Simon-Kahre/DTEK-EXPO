/*
* main.c
* Authors: Simon kåhre, Leo Ehrenberg
* Date: 07-12-2025
*/

#include "kernel.h"
extern void imageProcessing(int w, int h,const char input[h][w], char output[h][w], int option);
extern void moveImage(volatile char *X,volatile int *Y, int sw, int w, int h, char I[h][w]);
extern void updateTransform(int Switches);
extern const char Image[];
extern void updateImage(int w, int h, volatile char* VGA, const char image[w][h]);

/**
 * Takes out a single color from a 8-bit VGA pixel
 * 
 * Pixel RGB format 3:3:2 bits: RRR GGG BB
 * color = 0 -> Red     (bits 5-7)
 * color = 1 -> Green   (bits 2-4)
 * color = 2 -> Blue    (bits 0-1)
 * 
 * returns the value of a color
 */
char getColor(char pixel, char color)
{
    char value = 0;
    switch (color)
    {
        case 0: //RED (1110 0000)
            value = pixel & 224;
            break;
        case 1: //GREEN (0001 1100)
            value = pixel & 28;
            break;
        case 2://BLUE (0000 0011)
            value = pixel & 3;
            break;
        default:
            break;
    }
    return value;
}

/**
 * Reads the state of the 10 switches from memory
 * each bit represent the state of a single switch
 * returns: the lowest 10 bits
 */
int get_sw()
{
  int *volatile ptr;
  ptr = (int*) 0x04000010;
  int activeSwitches = *ptr;
  activeSwitches = activeSwitches & 1023;
  
  return activeSwitches;
}

/**
 * Helper to check if a character is whitespace
 * used for PPM header
 */
int is_whitespace(char c) {
    return (c==' ' || c=='\n' || c=='\r' || c=='\t');
}

/**
 * Helper to check if a character is a decimal digit
 * used for PPM header
 */
int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

/**
 * Skips over the ASCII header of a P6 PPM image and returns
 * a pointer to the start of the raw RGB pixel data
 * 
 * Expected format:
 * P6
 * <width> <height>
 * <maxval>
 * <binary RGB data..>
 * 
 * width, height and maxval are written to the provided pointers.
 * If error, returns 0.
 */
char* skipPPMHeader(const char *ppm, int *width, int *height, int *maxval)
{
    int i = 0;
    int value, found;

    // Verify P6
    if (ppm[i++] != 'P' || ppm[i++] != '6')
    {
        return 0;
    }

    // Skip whitespace after p6
    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    // Skip comment lines
    while (ppm[i] == '#')
    {
        while (ppm[i] != '\n')
        {
            i++;
        }

        while (is_whitespace(ppm[i]))
        {
            i++;
        }
    }

    // Check width
    value = 0; found = 0;
    while (is_digit(ppm[i]))
    {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;
    *width = value;

    // Skip whitespace after width
    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    // Check height
    value = 0; found = 0;
    while (is_digit(ppm[i]))
    {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;
    *height = value;

    // Skip whitespace after height
    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    // Check maxval
    value = 0; found = 0;
    while (is_digit(ppm[i]))
    {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;
    *maxval = value;

    // Skip whitespace after maxval and return pointer to start of raw RGB data
    while (is_whitespace(ppm[i]))
    {
        i++;
    }
    return (char*)&ppm[i];
}
    
int main()
{   
    // Address for VGA frame buffer (320x240 packed 8-bit pixels)
    volatile char *VGA = (volatile char*) 0x08000000;

    int w,h,mv;

    // Get height, widht, maxvalue and pointer to start of raw RGB data from Image
    volatile char *rawImage = skipPPMHeader(Image, &w, &h, &mv);

    // Local image buffer in VGA 3:3:2 format
    char imageMatrix[w][h];

    // Convert raw PPM Image to 3:3:2 format
    for (int i = 0; i < w; i++)
    {
        for(int j = 0; j < h; j++)
        {   
            // Read original 8-bit RGB pixels from PPM data
            unsigned char r = rawImage[(j * w + i) * 3];
            unsigned char g = rawImage[(j * w + i) * 3+1];
            unsigned char b = rawImage[(j * w + i) * 3+2];

            // if maxval == 1, expand binary channel to full 0/255 range
            if (mv == 1) 
            {
                r = r ? 255 : 0;
                g = g ? 255 : 0;
                b = b ? 255 : 0;
            }
            // Convert values into 3:3:2 format
            imageMatrix[i][j] =(r & 0xE0) + ((g & 0xE0) >> 3) + ((b & 0xC0) >> 6);
        }
    }

    // Buffer for the processed image
    char processed[w][h];
    
    // Initial processing: option 10 -> default (no kernels)
    imageProcessing(w, h, imageMatrix, processed, 10);
    
    // Draw the initial processed image to the VGA frame buffer
    for (int i = 0; i < 320; i++)
    {
        for(int j = 0; j < h; j++)
        {
            // Re-pack by extracting RGB
            VGA[i+(j*320)] = (getColor(processed[i][j], 0) | getColor(processed[i][j], 1) | getColor(processed[i][j], 2));
        }
    }

    // Keep track of previous switch status to avoid re-process
    int pastSwStatus = 0;

    // VGA control registers: address for vertical panning
    volatile int *VGA_CTRL = (volatile int*) 0x04000100;

    // Main loop: get switch status, update filters and move image
    while (1)
    {
        int activeSw = get_sw();

        // If switch0 is active: filter selection mode
        if(activeSw & 1)
        {
            // Mask out bit 0, keep bit 1-9 for filter choice
            activeSw = activeSw & 1021;

            // Only run filter if switches has changed
            if(activeSw != pastSwStatus)
            {   
                // Each branch checks one filter bit,
                // only triggers when that bit flips from 0 -> 1
                if(activeSw & 512)
                {          
                    if(!(pastSwStatus & 512))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, RIDGE_DETECTION);
                    }
                }
                else if(activeSw & 256)
                {
                    if(!(pastSwStatus & 256))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, EDGE_DETECTION);
                    }
                }
                else if(activeSw & 128)
                {
                    if(!(pastSwStatus & 128))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, SHARPEN);
                    }
                }
                else if(activeSw & 64)
                {
                    if(!(pastSwStatus & 64))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, BOX_BLUR);
                    }
                }
                else if(activeSw & 32)
                {
                    if(!(pastSwStatus & 32))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, GAUSSIAN_BLUR3X3);
                    }
                }
                else if(activeSw & 16)
                {
                    if(!(pastSwStatus & 16))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, GAUSSIAN_BLUR5X5);
                    }
                }
                else if(activeSw & 8)
                {
                    if(!(pastSwStatus & 8))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, UNSHARP_MASKING);
                    }
                }
                else if(activeSw & 4)
                {
                    if(!(pastSwStatus & 4))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, INVERTED);
                    }
                }
                else
                {   
                    // No filter bits active
                    // Fall back to default (no processing)
                    imageProcessing(w, h, imageMatrix, processed, 10);
                }
                // Store current status for next iteration
                pastSwStatus = activeSw;

                // Re render the entire processed image to VGA after filter change
                updateImage(w, h, VGA, processed);
            }
        }
        else
        {
            // If Switch0 is OFF: transform mode
            updateTransform(activeSw);
        }

        // Always move the visible window over the image
        moveImage(VGA, VGA_CTRL, activeSw, w, h, processed);
    }
}

// Interupt handler, currently not used since we dont use interrupts
void handle_interrupt()
{

}
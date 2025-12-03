extern void imageProcessing(int w, int h,const char input[h][w], char output[h][w], int option);
extern void moveImage(volatile char *X,volatile int *Y, int sw, int w, int h, char I[h][w]);
extern void updateTransform(int Switches);
extern const char Image[];
extern void updateImage(int w, int h, volatile char* VGA, const char image[w][h]);

char getColor(char pixel, char color)
{
    char value = 0;
    switch (color)
    {
        case 0: //RED
            value = pixel & 224;
            break;
        case 1: //GREEN
            value = pixel & 28;
            break;
        case 2://BLUE
            value = pixel & 3;
            break;
        default:
            break;
    }
    return value;
}

int get_sw()
{
  int *volatile ptr;
  ptr = (int*) 0x04000010;
  int activeSwitches = *ptr;
  activeSwitches = activeSwitches & 1023;
  
  return activeSwitches;
}

int is_whitespace(char c) {
    return (c==' ' || c=='\n' || c=='\r' || c=='\t');
}

int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

char* skipPPMHeader(const char *ppm, int *width, int *height, int *maxval)
{
    int i = 0;

    if (ppm[i++] != 'P' || ppm[i++] != '6')
    {
        return 0;
    }

    while (is_whitespace(ppm[i]))
    {
        i++;
    }

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

    int value, found;

    value = 0; found = 0;
    while (is_digit(ppm[i]))
    {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }

    if (!found) return 0;

    *width = value;

    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    value = 0; found = 0;
    while (is_digit(ppm[i]))
    {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;

    *height = value;

    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    value = 0; found = 0;
    while (is_digit(ppm[i]))
    {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }

    if (!found) return 0;

    *maxval = value;

    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    return (char*)&ppm[i];
}
    
int main()
{
    volatile char *VGA = (volatile char*) 0x08000000;

    int w,h,mv;

    volatile char *rawImage = skipPPMHeader(Image, &w, &h, &mv);

    char imageMatrix[w][h];

    for (int i = 0; i < w; i++)
    {
        for(int j = 0; j < h; j++)
        {
            unsigned char r = rawImage[(j * w + i) * 3];
            unsigned char g = rawImage[(j * w + i) * 3+1];
            unsigned char b = rawImage[(j * w + i) * 3+2];

            if (mv == 1) 
            {
                r = r ? 255 : 0;
                g = g ? 255 : 0;
                b = b ? 255 : 0;
            }

            imageMatrix[i][j] =(r & 0xE0) + ((g & 0xE0) >> 3) + ((b & 0xC0) >> 6);
        }
    }
    char processed[w][h];
    imageProcessing(w, h, imageMatrix, processed, 10);
    
    for (int i = 0; i < 320; i++)
    {
        for(int j = 0; j < h; j++)
        {
            VGA[i+(j*320)] = (getColor(processed[i][j], 0) | getColor(processed[i][j], 1) | getColor(processed[i][j], 2));
        }
    }

    int pastSwStatus = 0;

    volatile int *VGA_CTRL = (volatile int*) 0x04000100;
    while (1)
    {
        int activeSw = get_sw();
        if(activeSw & 1)
        {
            activeSw = activeSw & 1021;
            if(activeSw != pastSwStatus)
            {
                if(activeSw & 512)
                {          
                    if(!(pastSwStatus & 512))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 0);
                    }
                }
                else if(activeSw & 256)
                {
                    if(!(pastSwStatus & 256))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 1);
                    }
                }
                else if(activeSw & 128)
                {
                    if(!(pastSwStatus & 128))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 2);
                    }
                }
                else if(activeSw & 64)
                {
                    if(!(pastSwStatus & 64))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 3);
                    }
                }
                else if(activeSw & 32)
                {
                    if(!(pastSwStatus & 32))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 4);
                    }
                }
                else if(activeSw & 16)
                {
                    if(!(pastSwStatus & 16))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 5);
                    }
                }
                else if(activeSw & 8)
                {
                    if(!(pastSwStatus & 8))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 6);
                    }
                }
                else if(activeSw & 4)
                {
                    if(!(pastSwStatus & 4))
                    {          
                        imageProcessing(w, h, imageMatrix, processed, 7);
                    }
                }
                else
                {
                    imageProcessing(w, h, imageMatrix, processed, 10);
                }
                pastSwStatus = activeSw;
                updateImage(w, h, VGA, processed);
            }
        }
        else
        {
            updateTransform(activeSw);
        }
        moveImage(VGA, VGA_CTRL, activeSw, w, h, processed);
    }
}

void handle_interrupt()
{

}
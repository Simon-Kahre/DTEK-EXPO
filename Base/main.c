typedef struct {
    char r;
    char g;
    char b;
} Pixel;

extern void moveImage(volatile char *X,volatile int *Y, int sw, int w, int h, volatile Pixel I[w][h]);
extern void updateTransform(int Switches);
extern void print(const char*);
extern void print_dec(unsigned int);
extern const char Image[];






// Aquires status of all the switches on the board
int get_sw()
{
  int *volatile ptr;
  ptr = (int*) 0x04000010; // Sets address for the pointer
  int activeSwitches = *ptr; // Aquires value from address which ptr points to
  activeSwitches = activeSwitches & 1023; // Makes sure only the 10 LSBs are used
  
  return activeSwitches;
}

int is_whitespace(char c) {
    return (c==' ' || c=='\n' || c=='\r' || c=='\t');
}

int is_digit(char c) {
    return (c >= '0' && c <= '9');
}

// Safe PPM header parser with bounds checking
char* skipPPMHeader(const char *ppm, int *width, int *height, int *maxval)
{
    int i = 0;

    // Magic number
    if (ppm[i++] != 'P' || ppm[i++] != '6')
    {
        return 0;
    }

    // Skip whitespace
    while (is_whitespace(ppm[i]))
    {
        i++;
    }

    // Skip comments
    while (ppm[i] == '#')
    {
        while (ppm[i] != '\n')
        {
            i++;  // skip line
        }

        while (is_whitespace(ppm[i]))
        {
            i++;
        }
    }

    // Read integer helper
    int value, found;

    // WIDTH
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

    // HEIGHT
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

    // MAXVAL
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

    return (char*)&ppm[i];   // start of pixel data
}


    
int main()
{
    volatile char *VGA = (volatile char*) 0x08000000;

    int w,h,mv;

    volatile char *rawImage = skipPPMHeader(Image, &w, &h, &mv);

    volatile Pixel imageMatrix[w][h];

    for (int i = 0; i < w; i++)
    {
        for(int j = 0; j < h; j++)
        {

            imageMatrix[i][j].r = rawImage[(j * w + i) * 3];
            imageMatrix[i][j].g = rawImage[(j * w + i) * 3+1];
            imageMatrix[i][j].b = rawImage[(j * w + i) * 3+2];

            char r = imageMatrix[i][j].r;
            char g = imageMatrix[i][j].g;
            char b = imageMatrix[i][j].b;

            if (mv == 1) { // expand 0/1 to full 8-bit
                r = r ? 255 : 0;
                g = g ? 255 : 0;
                b = b ? 255 : 0;
            }

            imageMatrix[i][j].r =(r & 0xE0);
            imageMatrix[i][j].g =((g & 0xE0) >> 3);
            imageMatrix[i][j].b =((b & 0xC0) >> 6);
        }
    }

    for (int i = 0; i < 320; i++)
    {
        for(int j = 0; j < h; j++)
        {
            VGA[i+(j*320)] = (imageMatrix[i][j].r | imageMatrix[i][j].g | imageMatrix[i][j].b);
        }
        
    }

    volatile int *VGA_CTRL = (volatile int*) 0x04000100;
    while (1)
    {

        int activeSw = get_sw();
        updateTransform(activeSw);
        moveImage(VGA, VGA_CTRL, activeSw, w, h, imageMatrix);
        
    }
}

void handle_interrupt()
{

}

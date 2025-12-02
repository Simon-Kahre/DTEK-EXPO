typedef struct __attribute__((packed)) {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Pixel;

extern void imageProcessing(int w, int h,const Pixel src[h][w], Pixel dst[h][w], int option);
extern void moveImage(volatile char *X,volatile int *Y, int sw, int w, int h, Pixel I[h][w]);
extern void updateTransform(int Switches);
extern void print(const char*);
extern void print_dec(unsigned int);
extern const char Image[];



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
    print_dec(sizeof(Pixel));
    volatile char *VGA = (volatile char*) 0x08000000;

    int w,h,mv;

    volatile char *rawImage = skipPPMHeader(Image, &w, &h, &mv);

    char imageMatrix[h][w];
    //Pixel processed[h][w];

    for (int i = 0; i < w; i++)
    {
        for(int j = 0; j < h; j++)
        {

            /*imageMatrix[j][i].r = rawImage[(j * w + i) * 3];
            imageMatrix[j][i].g = rawImage[(j * w + i) * 3+1];
            imageMatrix[j][i].b = rawImage[(j * w + i) * 3+2];

            unsigned char r = imageMatrix[j][i].r;
            unsigned char g = imageMatrix[j][i].g;
            unsigned char b = imageMatrix[j][i].b;*/

            unsigned char r = rawImage[(j * w + i) * 3];
            unsigned char g = rawImage[(j * w + i) * 3+1];
            unsigned char b = rawImage[(j * w + i) * 3+2];

            if (mv == 1) 
            { // expand 0/1 to full 8-bit
                r = r ? 255 : 0;
                g = g ? 255 : 0;
                b = b ? 255 : 0;
            }


            /*imageMatrix[j][i].r =(r & 0xE0);
            imageMatrix[j][i].g =((g & 0xE0) >> 3);
            imageMatrix[j][i].b =((b & 0xC0) >> 6);*/

            imageMatrix[j][i] =(r & 0xE0) + ((g & 0xE0) >> 3) + ((b & 0xC0) >> 6);
        }
    }
    char option = 10;

    //imageProcessing(w, h, imageMatrix, processed, option);

    /*for (int i = 0; i < w; i++){
        for (int j = 0; j < h; j++){
            imageMatrix[i][j] = processed[i][j];
        }
    }*/
    
    for (int i = 0; i < 320; i++)
    {
        for(int j = 0; j < h; j++)
        {
            VGA[i+(j*320)] = (getColor(imageMatrix[j][i], 0) | getColor(imageMatrix[j][i], 1) | getColor(imageMatrix[j][i], 2));
        }
        
    }

    volatile int *VGA_CTRL = (volatile int*) 0x04000100;
    while (1)
    {

        int activeSw = get_sw();
        //updateTransform(activeSw);
        //moveImage(VGA, VGA_CTRL, activeSw, w, h, imageMatrix);
        
    }
}

void handle_interrupt()
{

}
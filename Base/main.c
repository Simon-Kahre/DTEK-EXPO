typedef unsigned char uint8_t;

extern void moveImage(volatile char *X,volatile int *Y, int sw);
extern void updateTransform(int Switches);
extern void print(const char*);
extern void print_dec(unsigned int);
extern const uint8_t Image[];

typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} Pixel;

volatile Pixel imageMatrix[2000][2000];


// Aquires status of all the switches on the board
int get_sw()
{
  int *volatile ptr;
  ptr = (int*) 0x04000010; // Sets address for the pointer
  int activeSwitches = *ptr; // Aquires value from address which ptr points to
  activeSwitches = activeSwitches & 1023; // Makes sure only the 10 LSBs are used
  
  return activeSwitches;
}



int is_whitespace(uint8_t c) {
    return (c==' ' || c=='\n' || c=='\r' || c=='\t');
}

int is_digit(uint8_t c) {
    return (c >= '0' && c <= '9');
}

// Safe PPM header parser with bounds checking
uint8_t* skipPPMHeader(const uint8_t *ppm,
                       int *width, int *height, int *maxval)
{
    int i = 0;
    int size = 2000000000;

    if (size < 2) return 0;

    // Magic number
    if (ppm[i++] != 'P' || ppm[i++] != '6')
        return 0;

    // Skip whitespace
    while (i < size && is_whitespace(ppm[i])) i++;
    if (i >= size) return 0;

    // Skip comments
    while (i < size && ppm[i] == '#') {
        while (i < size && ppm[i] != '\n') i++;  // skip line
        if (i < size) i++;                       // skip newline
        while (i < size && is_whitespace(ppm[i])) i++;
    }
    if (i >= size) return 0;

    // Read integer helper
    int value, found;

    // WIDTH
    value = 0; found = 0;
    while (i < size && is_digit(ppm[i])) {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;
    *width = value;
    while (i < size && is_whitespace(ppm[i])) i++;

    // HEIGHT
    value = 0; found = 0;
    while (i < size && is_digit(ppm[i])) {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;
    *height = value;
    while (i < size && is_whitespace(ppm[i])) i++;

    // MAXVAL
    value = 0; found = 0;
    while (i < size && is_digit(ppm[i])) {
        value = value * 10 + (ppm[i++] - '0');
        found = 1;
    }
    if (!found) return 0;
    *maxval = value;
    while (i < size && is_whitespace(ppm[i])) i++;

    if (i >= size) return 0;

    return (uint8_t*)&ppm[i];   // start of pixel data
}
volatile Pixel imageMatrix[2000][2000];


int main()
{
    print("Header bytes:\n");
    for (int i = 0; i < 32; i++)
    {
        print_dec(Image[i]);
        print(" ");
    }
    print("\n");
    
    volatile char *VGA = (volatile char*) 0x08000000;
    

    int w,h,mv;
    int s = 0;

    volatile uint8_t *rawImage = skipPPMHeader(Image, &w, &h, &mv);
    s=s;

    for (int i=0;i<w;i++)
    {
        for(int j = 0; j < h; j++)
        {
            imageMatrix[i][j].r = rawImage[(j * w + i) * 3];
            imageMatrix[i][j].g = rawImage[(j * w + i) * 3+1];
            imageMatrix[i][j].b = rawImage[(j * w + i) * 3+2];
        }
        print_dec(w);
        print_dec(h);
        print(" ");
    }

    for (int i = 0; i < 320; i++)
    {
        for(int j = 0; j < h; j++)
        {

            uint8_t r = imageMatrix[i][j].r;
            uint8_t g = imageMatrix[i][j].g;
            uint8_t b = imageMatrix[i][j].b;

            /*uint8_t r = rawImage[i*3 + 0];
            uint8_t g = rawImage[i*3 + 1];
            uint8_t b = rawImage[i*3 + 2];*/

            if (mv == 1) { // expand 0/1 to full 8-bit
                r = r ? 255 : 0;
                g = g ? 255 : 0;
                b = b ? 255 : 0;
            }

            VGA[i+(j*320)] = ((r & 0xE0) | ((g & 0xE0) >> 3) | ((b & 0xC0) >> 6));
        }
        
    }

    volatile int *VGA_CTRL = (volatile int*) 0x04000100;
    while (1)
    {

        int activeSw = get_sw();
        updateTransform(activeSw);
        moveImage(VGA, VGA_CTRL, activeSw);
        
    }
}

void handle_interrupt()
{

}

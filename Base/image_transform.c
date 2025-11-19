typedef struct {
    char r;
    char g;
    char b;
} Pixel;

extern void print(const char*);
extern void print_dec(unsigned int);

int speedX = 0;
int offSetX = 0;

int speedY = 0;
int offSetY = 0;

int rotationSpeed = 0;

void set_leds(int led_mask)
{
  led_mask = led_mask & 1023; // Makes sure only the 10 LSBs are used
  int *volatile ptr;
  ptr = (int*) 0x04000000; // Sets address for the pointer

  *ptr = led_mask; // Sets value in address which ptr points to
}

void updateHorizontalSpeed(unsigned int moveRight, unsigned int moveLeft)
{
    if(moveRight)
    {
        moveRight = 1;
    }
    if(moveLeft)
    {
        moveLeft = 1;
    }
    speedX = moveRight - moveLeft;
}

void updateVerticalSpeed(unsigned int moveUp, unsigned int moveDown, int activeSw)
{
    if(moveUp)
    {
        moveUp = 1;
    }
    if(moveDown)
    {
        moveDown = 1;
    }
    speedY = moveUp - moveDown;
}

void updateRotation(unsigned int rotateRight, unsigned int rotateLeft)
{
    rotationSpeed = rotateRight - rotateLeft;
}

void moveImage(char *VGA, volatile int *VGA_CTRL, int activeSw, int w, int h, volatile Pixel image[w][h])
{
    offSetX = offSetX + speedX;
    offSetY = offSetY + speedY;
    if(offSetX < 0)
    {
        offSetX = 0;
    }
    else if(offSetX > (w-320))
    {
        offSetX = (w-320);
    }
    
    if(speedX != 0)
    {
        for(int i = 0; i < 320; i++)
        {
            for(int j = 0; j < h; j++)
            {
                VGA[i+(j*320)] = (image[i+offSetX][j].r | image[i+offSetX][j].g | image[i+offSetX][j].b);
            }
        }
    }
    
    if(offSetY < 0)
    {
        offSetY = 0;
    }
    else if (offSetY > (h - 240))
    {
        offSetY = (h - 240);
    }

    *(VGA_CTRL+1) = (unsigned int) (VGA+offSetY * 320);
    *(VGA_CTRL+0) = 0;

    
    for (int i = 0; i < 1000000; i++)
    {
        asm volatile ("nop");
    }

    
}

void updateTransform(int activeSw)
{
    updateHorizontalSpeed(activeSw & 256, activeSw & 512);
    updateVerticalSpeed(activeSw & 128, activeSw & 64, activeSw);
    
}
int speedX;
int offSetX = 0;

int speedY;
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

void moveImage(volatile char *VGA, volatile int *VGA_CTRL, int activeSw)
{
    offSetX = offSetX + speedX;
    offSetY = offSetY + speedY;
    if(offSetX < 0)
    {
        offSetX = 0;
    }
    if(offSetY < 0)
    {
        offSetY = 0;
    }

    *(VGA_CTRL+1) = (unsigned int) (VGA+offSetY * 320 + offSetX -(offSetX/10));
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
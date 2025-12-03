extern void print(const char*);
extern char getColor(char p, char c);

int offSetX = 0;

int offSetY = 0;

float angleTheta = 0;
float rotationChange[2] = {1,0};
int imageHasChanged = 0;

float scalingFactor = 1;

float cos(float angle)
{
    float numerator = angle * angle;
    float total = 1;
    float divisor = 2;
    int sign = -1;

    for(int i = 1; i <= 11; i++)
    {
        float sum = (numerator/divisor);
        total += sign * sum;

        numerator *= angle * angle;
        divisor *= (2*i + 2);
        divisor *= (2*i + 1);

        sign = -sign;
    }
    return total;
}

float sin(float angle)
{
    float numerator = angle * angle * angle;
    float total = angle;
    float divisor = 6;
    int sign = -1;

    for(int i = 1; i <= 11; i++)
    {
        float sum = (numerator/divisor);
        total += sign * sum;

        numerator *= angle * angle;
        divisor *= (2*i + 2);
        divisor *= (2*i + 3);

        sign = -sign;
    }
    return total;
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
    offSetX += (moveRight - moveLeft)*2;

    if(moveLeft != 0 || moveRight != 0)
    {
        imageHasChanged = 1;
    }
}

void updateVerticalSpeed(unsigned int moveUp, unsigned int moveDown)
{
    if(moveUp)
    {
        moveUp = 1;
    }
    if(moveDown)
    {
        moveDown = 1;
    }
    offSetY += (moveUp - moveDown);
}

void updateRotation(int rotateRight, int rotateLeft)
{
    if(rotateLeft)
    {
        rotateLeft = 1;
    }
    if(rotateRight)
    {
        rotateRight = 1;
    }
    
    

    if(rotateLeft != 0 || rotateRight != 0)
    {
        angleTheta += (rotateLeft - rotateRight) * 0.03;

    if(angleTheta < 0)
    {
        angleTheta = 6.28 - angleTheta;
    }
    else if(angleTheta > 6.28)
    {
        angleTheta = 0;
    }

        imageHasChanged = 1;

        rotationChange[0] = cos(angleTheta);
        rotationChange[1] = sin(angleTheta);
    }
}

void updateScaling(int zoomIn, int zoomOut)
{
    if(zoomIn)
    {
        zoomIn = 1;
    }
    if(zoomOut)
    {
        zoomOut = 1;
    }

    scalingFactor += (zoomIn - zoomOut) * 0.05;
    if(zoomIn != 0 || zoomOut != 0)
    {
        imageHasChanged = 1;
    }
}

void updateImage(int w, int h, volatile char* VGA, const char image[w][h])
{
    for(int i = 0; i < 320; i += 1)
        {
            for(int j = 0; j < h; j += 1)
            {
                
                int x = (int)(((w/2)+offSetX)*scalingFactor) + scalingFactor * rotationChange[0] * (i - (w/2)) - (scalingFactor * rotationChange[1] * (j - (h/2)));
                int y = (int)((h/2)*scalingFactor) + scalingFactor * rotationChange[0] * (j - (h/2)) + scalingFactor * rotationChange[1] * (i - (w/2));

                if(j < (h/scalingFactor))
                {
                    VGA[(i+((j)*320))] = (getColor(image[x][y], 0) | getColor(image[x][y], 1) | getColor(image[x][y], 2));
                }
                else
                {
                    VGA[(i+((j)*320))] = (0 | 0 | 0);
                }
            }
        }
}

void moveImage(volatile char *VGA, volatile int *VGA_CTRL, int activeSw, int w, int h,const char image[w][h])
{
    if(offSetX < 0)
    {
        offSetX = 0;
    }
    else if(offSetX > (w-320)/scalingFactor)
    {
        offSetX = (w-320)/scalingFactor;
    }
    
    if(imageHasChanged)
    {
        imageHasChanged = 0;

       updateImage(w, h, VGA, image);
    }
    
    if(offSetY < 0)
    {
        offSetY = 0;
    }
    else if (offSetY > (h/scalingFactor - 240))
    {
        offSetY = (h/scalingFactor - 240);
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
    updateVerticalSpeed(activeSw & 128, activeSw & 64);
    updateRotation(activeSw & 32, activeSw & 16);
    updateScaling(activeSw & 8, activeSw & 4);
}
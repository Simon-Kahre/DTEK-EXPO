extern char getColor(char p, char c);

// Saved offset in on the x- and y-axis for the image
int offSetX = 0;
int offSetY = 0;

// The angle in which the image has rotated
float angleTheta = 0;

// Saved cos and sin values for angleTheta for faster calculations
float rotationChange[2] = {1,0};

// Variable to check if the image has been changed
int imageHasChanged = 0;

// The zoom/scaling factor for the image
float scalingFactor = 1;

// A function which calculates cos of an angle using Taylor's Polynomial
float cos(float angle)
{
    // Starting values for the polynomial
    float numerator = angle * angle;
    float total = 1;
    float divisor = 2;

    // Value which indicated whether something should be added or subtracted
    int sign = -1;

    // Loop to approximate the final value of cos
    for(int i = 1; i <= 11; i++)
    {
        float sum = (numerator/divisor);
        total += sign * sum;

        // Update the numerator and denominator for the next iteration
        numerator *= angle * angle;
        divisor *= (2*i + 2);
        divisor *= (2*i + 1);

        // Flipping the sign
        sign = -sign;
    }
    return total;
}

// A function which calculates sin of an angle using Taylor's Polynomial
float sin(float angle)
{
    // Starting values for the polynomial
    float numerator = angle * angle * angle;
    float total = angle;
    float divisor = 6;

    // Value which indicated whether something should be added or subtracted
    int sign = -1;

    // Loop to approximate the final value of sin
    for(int i = 1; i <= 11; i++)
    {
        float sum = (numerator/divisor);
        total += sign * sum;

        // Update the numerator and denominator for the next iteration
        numerator *= angle * angle;
        divisor *= (2*i + 2);
        divisor *= (2*i + 3);

        // Flipping the sign
        sign = -sign;
    }
    return total;
}

// Updates the movemnt in the x-axis (name is kept from older version)
void updateHorizontalSpeed(unsigned int moveRight, unsigned int moveLeft)
{
    // Normalizes values of moveRight and moveLeft
    if(moveRight)
    {
        moveRight = 1;
    }
    if(moveLeft)
    {
        moveLeft = 1;
    }

    // Updates offset
    offSetX += (moveRight - moveLeft)*2;

    // Changes that the image has changed if any of the switches are active
    if(moveLeft != 0 || moveRight != 0)
    {
        imageHasChanged = 1;
    }
}

// Updates the movemnt in the y-axis (name is kept from older version)
void updateVerticalSpeed(unsigned int moveUp, unsigned int moveDown)
{
    // Normalizes values of moveUp and moveDown
    if(moveUp)
    {
        moveUp = 1;
    }
    if(moveDown)
    {
        moveDown = 1;
    }

    // Updates offset
    offSetY += (moveUp - moveDown);
}

// Function for updating the rotation of the image
void updateRotation(int rotateRight, int rotateLeft)
{
    // Normalizes the parameters
    if(rotateLeft)
    {
        rotateLeft = 1;
    }
    if(rotateRight)
    {
        rotateRight = 1;
    }

    // Updates the angle if any of the switches are active
    if(rotateLeft != 0 || rotateRight != 0)
    {
        // Update the angle
        angleTheta += (rotateLeft - rotateRight) * 0.03;

        // Limits the angle between 0 and 6.28 (2*pi)
        if(angleTheta < 0)
        {
            angleTheta = 6.28 - angleTheta;
        }
        else if(angleTheta > 6.28)
        {
            angleTheta = 0;
        }

        // Updates that the image has changed
        imageHasChanged = 1;

        // Calculated the new cos and sin values
        rotationChange[0] = cos(angleTheta);
        rotationChange[1] = sin(angleTheta);
    }
}

// Function for updating the zoom factor
void updateScaling(int zoomIn, int zoomOut)
{
    // Normalizes the parameters
    if(zoomIn)
    {
        zoomIn = 1;
    }
    if(zoomOut)
    {
        zoomOut = 1;
    }

    // Updates the zoom factor
    scalingFactor += (zoomIn - zoomOut) * 0.05;

    // Checks whether any of the switches are active
    if(zoomIn != 0 || zoomOut != 0)
    {
        imageHasChanged = 1;
    }
}

// The functions which updates what is being displayed
void updateImage(int w, int h, volatile char* VGA, const char image[w][h])
{
    // Looping through it change each pixel on the screen
    for(int i = 0; i < 320; i += 1)
    {
        for(int j = 0; j < h; j += 1)
        {
            /*
            * These two functions work with some help from matrix multiplication.

            * It starts by getting the offset from the center with (w/h+offSetX) 
            * and scales it by the scalingFactor.
            * 
            * Next we add onto it the rotation change using the mention matrix
            * multiplication. This is also multiplied by the scalingFactor to
            * keep the scaling proportional. It's also multiplied with the offset
            * of the specified pixel from the center to make sure the rotation
            * is based of the center.
            */
            int x = (int)(((w/2)+offSetX)*scalingFactor) + scalingFactor * rotationChange[0] * (i - (w/2)) - (scalingFactor * rotationChange[1] * (j - (h/2)));

            /*
            * It starts by getting the new center by scaling the center (h/2)
            * by the scalingFactor.
            * 
            * Next we add onto it the rotation change using the mention matrix
            * multiplication. This is also multiplied by the scalingFactor to
            * keep the scaling proportional. It's also multiplied with the offset
            * of the specified pixel from the center to make sure the rotation
            * is based of the center.
            */
            int y = (int)((h/2)*scalingFactor) + scalingFactor * rotationChange[0] * (j - (h/2)) + scalingFactor * rotationChange[1] * (i - (w/2));

            // This checks whether it tries to update any pixels outside the max possible height for the image based on how zoomed in it is
            if(j < (h/scalingFactor))
            {
                // Sets the correct colors if the j is within the max height
                VGA[(i+((j)*320))] = (getColor(image[x][y], 0) | getColor(image[x][y], 1) | getColor(image[x][y], 2));
            }
            else
            {
                // Sets the rest of the pixels to black
                VGA[(i+((j)*320))] = (0 | 0 | 0);
            }
        }
    }
}

// The function which moves the image into the correct position
void moveImage(volatile char *VGA, volatile int *VGA_CTRL, int activeSw, int w, int h,const char image[w][h])
{
    // Limits the offset on the x-axis so that the image can't move off screen
    if(offSetX < 0)
    {
        offSetX = 0;
    }
    else if(offSetX > (w-320)/scalingFactor)
    {
        offSetX = (w-320)/scalingFactor;
    }
  
    // Updates the image if anything has been changed
    if(imageHasChanged)
    {
        imageHasChanged = 0;

       updateImage(w, h, VGA, image);
    }
    
    // Limits the offset on the y-axis so that the image can't move off screen
    if(offSetY < 0)
    {
        offSetY = 0;
    }
    else if (offSetY > (h/scalingFactor - 240))
    {
        offSetY = (h/scalingFactor - 240);
    }

    // Send the image onto the buffer to be displayed
    *(VGA_CTRL+1) = (unsigned int) (VGA+offSetY * 320);
    *(VGA_CTRL+0) = 0;

    // Small delay
    for (int i = 0; i < 1000000; i++)
    {
        asm volatile ("nop");
    }
}

// Main update functions which branches of into the different update functions
void updateTransform(int activeSw)
{
    updateHorizontalSpeed(activeSw & 256, activeSw & 512);
    updateVerticalSpeed(activeSw & 128, activeSw & 64);
    updateRotation(activeSw & 32, activeSw & 16);
    updateScaling(activeSw & 8, activeSw & 4);
}
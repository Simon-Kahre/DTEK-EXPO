#ifndef KERNEL_H
#define KERNEL_H

/**
 * Enumeration of all available image processing filters.
 * Used in both kernel.c and main.c as option.
 * declares every name a value 0,1,2,3...
 */
enum {
    RIDGE_DETECTION = 0,
    EDGE_DETECTION,
    SHARPEN,
    BOX_BLUR,
    GAUSSIAN_BLUR3X3,
    GAUSSIAN_BLUR5X5,
    UNSHARP_MASKING,
    INVERTED
};

#endif
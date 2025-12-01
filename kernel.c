typedef struct {
    char r;
    char g;
    char b;
} Pixel;

enum {
    RIDGE_DETECTION = 0,
    EDGE_DETECTION,
    SHARPEN,
    BOX_BLUR,
    GAUSSIAN_BLUR3X3,
    GAUSSIAN_BLUR5X5,
    UNSHARP_MASKING
};

static void applyKernel(int w, int h, Pixel src[w][h], Pixel dst[w][h], const double *k, int radius);
void imageProcessing(int w, int h, Pixel src[w][h], Pixel dst[w][h], int option);

const double ridgeDetection[3][3] =
{
    {0.0, -1.0, 0.0},
    {-1.0, 4.0, -1.0},
    {0.0, -1.0, 0.0}
};

const double edgeDetection[3][3] =
{
    {-1.0, -1.0, -1.0},
    {-1.0, 8.0, -1.0},
    {-1.0, -1.0, -1.0}
};

const double sharpen[3][3] =
{
    {0.0, -1.0, 0.0},
    {-1.0, 5.0, -1.0},
    {0.0, -1.0, 0.0}
};

const double boxBlur[3][3] =
{
    {1.0/9.0, 1.0/9.0, 1.0/9.0},
    {1.0/9.0, 1.0/9.0, 1.0/9.0},
    {1.0/9.0, 1.0/9.0, 1.0/9.0}
};

const double gaussianBlur3x3[3][3] =
{
    {1.0/16.0, 2.0/16.0, 1.0/16.0},
    {2.0/16.0, 4.0/16.0, 2.0/16.0},
    {1.0/16.0, 2.0/16.0, 1.0/16.0}
};

const double gaussianBlur5x5[5][5] =
{
    {1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
    {4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
    {6.0/256.0, 24.0/256.0, 36.0/256.0, 24.0/256.0, 6.0/256.0},
    {4.0/256.0, 16.0/256.0, 24.0/256.0, 16.0/256.0, 4.0/256.0},
    {1.0/256.0, 4.0/256.0, 6.0/256.0, 4.0/256.0, 1.0/256.0},
};

const double unsharpMasking[5][5] =
{
    {-1.0/256.0, -4.0/256.0, -6.0/256.0, -4.0/256.0, -1.0/256.0},
    {-4.0/256.0, -16.0/256.0, -24.0/256.0, -16.0/256.0, -4.0/256.0},
    {-6.0/256.0, -24.0/256.0, 476.0/256.0, -24.0/256.0, -6.0/256.0},
    {-4.0/256.0, -16.0/256.0, -24.0/256.0, -16.0/256.0, -4.0/256.0},
    {-1.0/256.0, -4.0/256.0, -6.0/256.0, -4.0/256.0, -1.0/256.0},
};

void imageProcessing(int w, int h, Pixel src[w][h], Pixel dst[w][h], int option){
    switch (option) {
        case RIDGE_DETECTION:
            applyKernel(w, h, src, dst, &ridgeDetection[0][0], 1);
            break;
        case EDGE_DETECTION:
            applyKernel(w, h, src, dst, &edgeDetection[0][0], 1);
            break;
        case SHARPEN:
            applyKernel(w, h, src, dst, &sharpen[0][0], 1);
            break;
        case BOX_BLUR:
            applyKernel(w, h, src, dst, &boxBlur[0][0], 1);
            break;
        case GAUSSIAN_BLUR3X3:
            applyKernel(w, h, src, dst, &gaussianBlur3x3[0][0], 1);
            break;
        case GAUSSIAN_BLUR5X5:
            applyKernel(w, h, src, dst, &gaussianBlur5x5[0][0], 2);
            break;
        case UNSHARP_MASKING:
            applyKernel(w, h, src, dst, &unsharpMasking[0][0], 2);
            break;
        default:
            for (int x = 0; x < w; ++x){
                for(int y = 0; y < h; ++y){
                    dst[x][y] = src[x][y];
                }
            } 
            break;
    }
}

static unsigned char clamp255(double v){
    if (v < 0.0) v = 0.0;
    if( v > 255.0) v = 255.0;
    return (unsigned char) v;
}

static void applyKernel(int w, int h, Pixel src[w][h], Pixel dst[w][h], const double *k, int radius){
    int size = 2 * radius + 1;

    for (int x = radius; x < w -radius; ++x){
        for (int y = radius; y < h-radius; ++y){
            double R = 0.0, G = 0.0, B = 0.0;

            for (int dx = -radius; dx <= radius; ++dx){
                for (int dy = -radius; dy <= radius; ++dy){
                    int kx = dx + radius;
                    int ky = dy + radius;
                    double c = k[kx * size + ky];

                    Pixel p = src[x + dx][y + dy];
                    R += c * (unsigned char)p.r;
                    G += c * (unsigned char)p.g;
                    B += c * (unsigned char)p.b;
                }
            }

            dst[x][y].r = clamp255(R);
            dst[x][y].g = clamp255(G);
            dst[x][y].b = clamp255(B);
        }
    }
    for (int x = 0; x<w; ++x){
        for (int y = 0; y < radius; ++y){
            dst[x][y] = src[x][y];
            dst[x][h-1-y] = src[x][h-1-y];
        }
    }
    for (int y = 0; y<h; ++y){
        for(int x = 0; x < radius; ++x){
            dst[x][y] = src[x][y];
            dst[w-1-x][y] = src[w-1-x][y];
        }
    }
}

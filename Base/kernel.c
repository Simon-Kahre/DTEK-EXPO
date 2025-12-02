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

static void applyKernel(int w, int h, volatile Pixel input[w][h], volatile Pixel output[w][h], const double *k, int radius);
void imageProcessing(int w, int h, volatile Pixel input[w][h], volatile Pixel output[w][h], int option);

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

void imageProcessing(int w, int h, volatile Pixel input[w][h], volatile Pixel output[w][h], int option){
    switch (option) {
        case RIDGE_DETECTION:
            applyKernel(w, h, input, output, &ridgeDetection[0][0], 1);
            break;
        case EDGE_DETECTION:
            applyKernel(w, h, input, output, &edgeDetection[0][0], 1);
            break;
        case SHARPEN:
            applyKernel(w, h, input, output, &sharpen[0][0], 1);
            break;
        case BOX_BLUR:
            applyKernel(w, h, input, output, &boxBlur[0][0], 1);
            break;
        case GAUSSIAN_BLUR3X3:
            applyKernel(w, h, input, output, &gaussianBlur3x3[0][0], 1);
            break;
        case GAUSSIAN_BLUR5X5:
            applyKernel(w, h, input, output, &gaussianBlur5x5[0][0], 2);
            break;
        case UNSHARP_MASKING:
            applyKernel(w, h, input, output, &unsharpMasking[0][0], 2);
            break;
        default:
            for (int x = 0; x < w; ++x){
                for(int y = 0; y < h; ++y){
                    output[x][y] = input[x][y];
                }
            } 
            break;
    }
}

static unsigned char clamp255(double v){
    if (v < 0.0) v = 0.0;
    if (v > 255.0) v = 255.0;
    return (unsigned char) v;
}

static int clamp_int(int v, int max){
    if (v < 0) return 0;
    if (v > max) return max;
    return v;
}

static void applyKernel(int w, int h, volatile Pixel input[w][h], volatile Pixel output[w][h], const double *k, int radius){
    int size = 2 * radius + 1;

    for (int x = 0; x < w; ++x){
        for (int y = 0; y < h; ++y){
            double R = 0.0, G = 0.0, B = 0.0;

            for (int dx = -radius; dx <= radius; ++dx){
                for (int dy = -radius; dy <= radius; ++dy){
                    int kx = dx + radius;
                    int ky = dy + radius;
                    double c = k[kx * size + ky];

                    int sx = clamp_int(x+dx, w-1);
                    int sy = clamp_int(y+dy, h-1);

                    Pixel p = input[sx][sy];
                    R += c * (unsigned char)p.r;
                    G += c * (unsigned char)p.g;
                    B += c * (unsigned char)p.b;
                }
            }

            output[x][y].r = clamp255(R);
            output[x][y].g = clamp255(G);
            output[x][y].b = clamp255(B);
        }
    }
}

//const float ROI_X = -1.525,
//            ROI_Y = 0;
#include <TxLib.h>
#include <math.h>

#define HEIGHT  600
#define WIDTH   800
#define X_MIN   -2.2f
#define X_MAX    1.0f
#define Y_MIN   -1.2f
#define Y_MAX    1.2f

#define MAX_ITERATION 256
#define L_MAX 2.0f

COLORREF GetColor (int iteration);

int main ()
{
    txCreateWindow (WIDTH, HEIGHT);
    txTextCursor (false);  // отключаю курсор внутри окна чтобы не мешал
    txBegin ();

    RGBQUAD* videoBuf = txVideoMemory ();
    if (!videoBuf)
    {
        return -1;
    }

    float x_range = X_MAX - X_MIN;   // 1.0 - (-2.2) = 3.2
    float y_range = Y_MAX - Y_MIN;   // 1.2 - (-1.2) = 2.4

    for (int y = 0; y < HEIGHT; y++)
    {
        float pos0_im = Y_MAX - (float)y / HEIGHT * y_range;
        
        for (int x = 0; x < WIDTH; x++)
        {
            float pos0_re = X_MIN + (float)x / WIDTH * x_range;

            float pos_re = 0.0;
            float pos_im = 0.0;
            int iteration = 0;

            while (iteration < MAX_ITERATION)
            {
                if (pos_im * pos_im + pos_re * pos_re > L_MAX * L_MAX)
                {
                    break;
                }

                float pos_re_new = pos_re * pos_re - pos_im * pos_im + pos0_re;
                float pos_im_new = 2.0 * pos_re * pos_im + pos0_im;
                pos_re = pos_re_new;
                pos_im = pos_im_new;
                iteration ++;
            }

            COLORREF color = GetColor (iteration);

            RGBQUAD* pixel = &videoBuf[y * WIDTH + x];

            pixel->rgbRed   = GetRValue (color);
            pixel->rgbGreen = GetGValue (color);
            pixel->rgbBlue  = GetBValue (color);
            pixel->rgbReserved = 0;
        }
    }

    txEnd ();
    return 0;
}


COLORREF GetColor (int iteration)
{
    if (iteration == MAX_ITERATION)
    {
        return RGB (0, 0, 0); // черный внутри множества
    }
    else
    {
        int bright = iteration % 8;
        int gray = 70 + bright * 20;
        return RGB (gray, gray, gray);
    }
}
#include <TXLib.h>
#include <math.h>

#define WIDTH ((float)  800)
#define HEIGHT ((float) 600)

int main ()
{
    txCreateWindow (WIDTH, HEIGHT);
    Win32::_fpreset ();
    txBegin ();
    RGBQUAD* graphBuf = txVideoMemory ();

    const float r2Max = 4.0f;
    const int   nMax  = 256;
    double dx = 1.0f / WIDTH, dy = 1.0f / HEIGHT;
    float xC = 0.f, yC = 0.f, scale = 4.f;
    
    //DWORD lastTime = GetTickCount ();
    float fps = 0;
    //int frameCount = 0;

    for (;;)
    {
        if (GetAsyncKeyState (VK_ESCAPE)) break;
        if (txGetAsyncKeyState (VK_RIGHT))    xC    += dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState (VK_LEFT))     xC    -= dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState (VK_DOWN))     yC    += dy * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState (VK_UP))       yC    -= dy * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState ('W'))         scale -= dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState ('S'))         scale += dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
              
        for (int y = 0; y < HEIGHT; y++)
        {
            float y0 = (((float) y - 300.f) * dy + yC) *scale;

            for (int x = 0; x < WIDTH; x += 8) 
            {
                float x0 = (((float)x - 400.f) * dx + xC) *scale;
                float xStep = dx*scale;

                float x0Arr[8] = { x0, x0 + xStep, x0 + 2*xStep, x0 + 3*xStep, x0 + 4*xStep, x0 + 5*xStep, x0 + 6*xStep, x0 + 7*xStep};
                float y0Arr[8] = { y0, y0, y0, y0, y0, y0, y0, y0};

                float xArr[8] = {};  for (int i = 0; i < 8; i++) xArr[i] = x0Arr[i];
                float yArr[8] = {};  for (int i = 0; i < 8; i++) yArr[i] = y0Arr[i];

                int N[8] = {};
                for (int n = 0; n < nMax; n++) 
                {
                    float x2[8] = {};  for (int i = 0; i < 8; i++) x2[i] = xArr[i] * xArr[i];
                    float y2[8] = {};  for (int i = 0; i < 8; i++) y2[i] = yArr[i] * yArr[i];
                    float xy[8] = {};  for (int i = 0; i < 8; i++) xy[i] = xArr[i] * yArr[i];

                    float r2[8] = {};  for (int i = 0; i < 8; i++) r2[i] = x2[i] + y2[i];

                    int cmp[8] = {};
                    for (int i = 0; i < 8; i++) if (r2[i] <= r2Max) cmp[i] = 1;

                    int mask = 0;
                    for (int i = 0; i < 8; i++) mask |= (cmp[i] << i);
                    if (!mask) break;

                    for (int i = 0; i < 8; i++) xArr[i] = x2[i] - y2[i] + x0Arr[i];
                    for (int i = 0; i < 8; i++) yArr[i] = xy[i] + xy[i] + y0Arr[i];

                    for (int i = 0; i < 8; i++) N[i] += cmp[i];
                }

                BYTE gray[8] = {};
                for (int i = 0; i < 8; ++i)
                    gray[i] = (BYTE)(N[i] << 4);

                RGBQUAD color[8] = {};
                for (int i = 0; i < 8; ++i) 
                {
                    color[i].rgbBlue = gray[i];
                    color[i].rgbGreen = gray[i];
                    color[i].rgbRed = gray[i];
                    color[i].rgbReserved = 0;
                }

                int offset = (HEIGHT - 1 - y) * WIDTH + x;
                for (int i = 0; i < 8; ++i)
                    graphBuf[offset + i] = color[i];
            }
        }
        
        static int frameCount = 0;
        static DWORD lastFpsTime = 0;
        frameCount++;
        DWORD now = GetTickCount();
        if (now - lastFpsTime >= 1000) 
        {
            fps = frameCount * 1000.0f / (now - lastFpsTime);
            frameCount = 0;
            lastFpsTime = now;
        }

        char fpsBuf[10] = {};
        sprintf (fpsBuf, "%.1f", fps);

        txSetColor (TX_WHITE);
        txSetFillColor (TX_BLACK);
        txSelectFont ("Consolas", 24);
        txTextOut (10, 10, fpsBuf);
        txEnd ();
    }
    return 0;
}
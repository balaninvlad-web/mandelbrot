#include <TXLib.h>
#include <math.h>
#include <immintrin.h>
#include <windows.h>   

#define WIDTH  800
#define HEIGHT 600

//#ifdef BENCHMARK_MODE

int main ()
{
    txCreateWindow (WIDTH, HEIGHT);
    Win32::_fpreset ();
    txBegin ();
    RGBQUAD* graphBuf = txVideoMemory ();

    __m512 r2Max = _mm512_set1_ps (4.0f);
    __m512 arr16add = _mm512_set_ps (15, 14, 13, 12, 11, 10, 9, 8, 7, 6, 5, 4 ,3, 2, 1, 0);
    const int nMax  = 256;
    double dx = 1.0f / WIDTH, dy = 1.0f / HEIGHT;
    float xC = 0.f, yC = 0.f, scale = 4.f;
    
    //DWORD lastTime = GetTickCount ();
    float fps = 0.0f;
    //int frameCount = 0;

    for (;;)
    {
        if (GetAsyncKeyState   (VK_ESCAPE)) break;
        if (txGetAsyncKeyState (VK_RIGHT))    xC    += dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState (VK_LEFT))     xC    -= dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState (VK_DOWN))     yC    += dy * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState (VK_UP))       yC    -= dy * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState ('W'))         scale -= dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        if (txGetAsyncKeyState ('S'))         scale += dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
              
        for (int y = 0; y < HEIGHT; y++)
        {
            float y0 = (((float)y - 300.f) * dy + yC) *scale;

            for (int x = 0; x < WIDTH; x += 16) 
            {
                float x0 = (((float)x - 400.f) * dx + xC) *scale;
                float xStep = dx*scale;

                __m512 y0Arr = _mm512_set1_ps (y0);
                __m512 x0Arr = _mm512_set1_ps (x0);
                __m512 xStepArr = _mm512_set1_ps (xStep);
                __m512 arr16addVec = _mm512_mul_ps (arr16add, xStepArr);
                __m512 arr4sixteenAdd = _mm512_set1_ps (16.0f * xStep);
                __m512 C_real = _mm512_add_ps (x0Arr, arr16addVec);
                __m512 C_imag = y0Arr;

                //float x0Arr[8] = { x0, x0 + xStep, x0 + 2*xStep, x0 + 3*xStep, x0 + 4*xStep, x0 + 5*xStep, x0 + 6*xStep, x0 + 7*xStep};
                //float y0Arr[8] = { y0, y0, y0, y0, y0, y0, y0, y0};

                __m512 xArr = C_real;
                __m512 yArr = C_imag;
                //float xArr[8] = {};  for (int i = 0; i < 8; i++) xArr[i] = x0Arr[i];
                //float yArr[8] = {};  for (int i = 0; i < 8; i++) yArr[i] = y0Arr[i];

                __m512i N = _mm512_setzero_si512 ();
                //int N[8] = {};
                __m512 const2 = _mm512_set1_ps (2.0f);

                for (int n = 0; n < nMax; n++) 
                {
                    __m512 x2Arr = _mm512_mul_ps (xArr, xArr);
                    __m512 y2Arr = _mm512_mul_ps (yArr, yArr);
                    __m512 xyArr = _mm512_mul_ps (xArr, yArr);

                    //float x2[8] = {};  for (int i = 0; i < 8; i++) x2[i] = xArr[i] * xArr[i];
                    //float y2[8] = {};  for (int i = 0; i < 8; i++) y2[i] = yArr[i] * yArr[i];
                    //float xy[8] = {};  for (int i = 0; i < 8; i++) xy[i] = xArr[i] * yArr[i];

                    __m512 r2Arr = _mm512_add_ps (x2Arr, y2Arr);
                    //float r2[8] = {};  for (int i = 0; i < 8; i++) r2[i] = x2[i] + y2[i];
                    
                    __mmask16 mask = _mm512_cmp_ps_mask (r2Arr, r2Max, _CMP_LE_OQ);
                    if (!mask) break;

                    //for (int i = 0; i < 8; i++) if (r2[i] <= r2Max) cmp[i] = 1;

                    //int mask = 0;
                    //for (int i = 0; i < 8; i++) mask |= (cmp[i] << i);
                    //if (!mask) break;
                    __m512 NewXArr = _mm512_add_ps (_mm512_sub_ps (x2Arr, y2Arr), x0Arr);
                    __m512 NewYArr = _mm512_fmadd_ps (const2, xyArr, y0Arr);    
                    
                    xArr = _mm512_mask_mov_ps (xArr, mask, NewXArr);
                    yArr = _mm512_mask_mov_ps (yArr, mask, NewYArr);
                    //for (int i = 0; i < 8; i++) xArr[i] = x2[i] - y2[i] + x0Arr[i];
                    //for (int i = 0; i < 8; i++) yArr[i] = xy[i] + xy[i] + y0Arr[i];

                    N = _mm512_mask_add_epi32 (N, mask, N, _mm512_set1_epi32 (1));
                    //for (int i = 0; i < 8; i++) N[i] += cmp[i];
                }
                __m512i gray =  _mm512_slli_epi32 (N, 5);
                gray = _mm512_min_epi32 (gray, _mm512_set1_epi32 (255));
                //BYTE gray[8] = {};
                //for (int i = 0; i < 8; ++i)
                //    gray[i] = (BYTE)(N[i] << 4);

                alignas(64) int tmp[16] = {};
                _mm512_store_si512 ((__m512i*) tmp, gray);

                RGBQUAD* curRow = graphBuf + (HEIGHT - 1 - y) * WIDTH;
                for (int i = 0; i < 16; i++)
                {
                    int val = tmp[i];
                    if (val > 255) val = 255;
                    curRow[x + i] = (RGBQUAD) {(BYTE) val, (BYTE) val, (BYTE) val, 0};
                }


                //RGBQUAD color[8] = {};
                //for (int i = 0; i < 8; ++i) 
                //{
                //    color[i].rgbBlue = gray[i];
                //    color[i].rgbGreen = gray[i];
                //    color[i].rgbRed = gray[i];
                //    color[i].rgbReserved = 0;
                //}

                //int offset = (HEIGHT - 1 - y) * WIDTH + x;
                //for (int i = 0; i < 8; ++i)
                //    graphBuf[offset + i] = color[i];
                x0Arr = _mm512_add_ps (x0Arr, arr4sixteenAdd);
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
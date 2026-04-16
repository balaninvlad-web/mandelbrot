#include <TXLib.h>
#include <math.h>
#include <immintrin.h>
#include <windows.h>   

#define WIDTH  800
#define HEIGHT 600

int main ()
{
    txCreateWindow (WIDTH, HEIGHT);
    Win32::_fpreset ();
    txBegin ();
    RGBQUAD* graphBuf = txVideoMemory ();

    __m256 r2Max = _mm256_set1_ps (4.0f);
    __m256 arr4add = _mm256_set_ps (7, 6, 5, 4 ,3, 2, 1, 0);
    const int nMax  = 256;
    double dx = 1.0f / WIDTH, dy = 1.0f / HEIGHT;
    float xC = 0.f, yC = 0.f, scale = 4.f;
    
    //DWORD lastTime = GetTickCount ();
    float fps = 0.0f;
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
            float y0 = (((float)y - 300.f) * dy + yC) *scale;

            for (int x = 0; x < WIDTH; x += 8) 
            {
                float x0 = (((float)x - 400.f) * dx + xC) *scale;
                float xStep = dx*scale;

                __m256 y0Arr = _mm256_set1_ps (y0);
                __m256 x0Arr = _mm256_set1_ps (x0);
                __m256 xStepArr = _mm256_set1_ps (xStep);
                __m256 arr4addVec = _mm256_mul_ps (arr4add, xStepArr);
                __m256 arr4eightAdd = _mm256_set1_ps (8.0f * xStep);
                x0Arr = _mm256_add_ps (x0Arr, arr4addVec);

                //float x0Arr[8] = { x0, x0 + xStep, x0 + 2*xStep, x0 + 3*xStep, x0 + 4*xStep, x0 + 5*xStep, x0 + 6*xStep, x0 + 7*xStep};
                //float y0Arr[8] = { y0, y0, y0, y0, y0, y0, y0, y0};

                __m256 xArr = x0Arr;
                __m256 yArr = y0Arr;
                //float xArr[8] = {};  for (int i = 0; i < 8; i++) xArr[i] = x0Arr[i];
                //float yArr[8] = {};  for (int i = 0; i < 8; i++) yArr[i] = y0Arr[i];

                __m256i N = _mm256_setzero_si256 ();
                //int N[8] = {};
                __m256 const2 = _mm256_set1_ps (2.0f);

                for (int n = 0; n < nMax; n++) 
                {
                    __m256 x2Arr = _mm256_mul_ps (xArr, xArr);
                    __m256 y2Arr = _mm256_mul_ps (yArr, yArr);
                    __m256 xyArr = _mm256_mul_ps (xArr, yArr);

                    //float x2[8] = {};  for (int i = 0; i < 8; i++) x2[i] = xArr[i] * xArr[i];
                    //float y2[8] = {};  for (int i = 0; i < 8; i++) y2[i] = yArr[i] * yArr[i];
                    //float xy[8] = {};  for (int i = 0; i < 8; i++) xy[i] = xArr[i] * yArr[i];

                    __m256 r2Arr = _mm256_add_ps (x2Arr, y2Arr);
                    //float r2[8] = {};  for (int i = 0; i < 8; i++) r2[i] = x2[i] + y2[i];
                    
                    __m256 cmp_f = _mm256_cmp_ps (r2Arr, r2Max, _CMP_LE_OQ);
                    //int cmp[8] = {};
            
                    int mask = _mm256_movemask_ps (cmp_f);
                    if (!mask) break;

                    //for (int i = 0; i < 8; i++) if (r2[i] <= r2Max) cmp[i] = 1;

                    //int mask = 0;
                    //for (int i = 0; i < 8; i++) mask |= (cmp[i] << i);
                    //if (!mask) break;
                    __m256 NewXArr = _mm256_add_ps (_mm256_sub_ps (x2Arr, y2Arr), x0Arr);
                    __m256 NewYArr =  _mm256_fmadd_ps (const2, xyArr, y0Arr);    
                    
                    xArr = _mm256_blendv_ps (xArr, NewXArr, cmp_f);
                    yArr = _mm256_blendv_ps (yArr, NewYArr, cmp_f);
                    //for (int i = 0; i < 8; i++) xArr[i] = x2[i] - y2[i] + x0Arr[i];
                    //for (int i = 0; i < 8; i++) yArr[i] = xy[i] + xy[i] + y0Arr[i];

                    __m256i cmp_i = _mm256_castps_si256 (cmp_f);
                    N = _mm256_sub_epi32(N, cmp_i);
                    //for (int i = 0; i < 8; i++) N[i] += cmp[i];
                }

                __m256i byteMask = _mm256_set1_epi32 (0xFF);
                __m256i gray =  _mm256_slli_epi32 (N, 5);
                gray = _mm256_and_si256 (gray, byteMask);
                //BYTE gray[8] = {};
                //for (int i = 0; i < 8; ++i)
                //    gray[i] = (BYTE)(N[i] << 4);

                int tmp[8] = {};
                _mm256_storeu_si256 ((__m256i*) tmp, gray);
                RGBQUAD* curRow = graphBuf + (HEIGHT - 1 - y) * WIDTH;
                for (int i = 0; i < 8; i++)
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
                x0Arr = _mm256_add_ps (x0Arr, arr4eightAdd);
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
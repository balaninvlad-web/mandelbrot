#ifndef BENCHMARK_MODE
    #define SIMPLE_MODE(code) code
    #define BENCHMARK(code)
#else
    #define SIMPLE_MODE(code)
    #define BENCHMARK(code) code
#endif

#ifndef BENCHMARK_MODE
    #include <TXLib.h>
#endif

#include <math.h>
#include <windows.h>

#ifdef BENCHMARK_MODE
    #include <stdio.h>
    #include <stdlib.h>
    #include <x86intrin.h>   // __rdtsc
#endif

#define WIDTH   800
#define HEIGHT  600

int main ()
{
    BENCHMARK (const int   NUM_OF_TESTS = 10;)
    BENCHMARK (const int   WARMUP_TESTS = 3;)
    BENCHMARK (const int   INNER_FRAMES = 5;)

    const float r2Max = 4.0f;
    const int   nMax  = 256;
    double dx = 1.0f / WIDTH, dy = 1.0f / HEIGHT;
    float xC = 0.f, yC = 0.f, scale = 4.f;

    SIMPLE_MODE
    (
        float fps = 0.f;
        DWORD lastTime = GetTickCount ();
        txCreateWindow (WIDTH, HEIGHT);
        RGBQUAD* graphBuf = txVideoMemory ();
        Win32::_fpreset ();
        txBegin ();
    )
    
    BENCHMARK
    (
        RGBQUAD* offBuf = (RGBQUAD*)calloc(WIDTH * HEIGHT, sizeof(RGBQUAD));
        if (!offBuf) return 1;
        unsigned long long cycles[NUM_OF_TESTS] = {};
    )

    //DWORD lastTime = GetTickCount ();
    //float fps = 0;
    //int frameCount = 0;

    for (SIMPLE_MODE ( ; ; )
         BENCHMARK   (int curTest = 0; curTest < NUM_OF_TESTS; curTest++))
    {
        SIMPLE_MODE
        (
            if (GetAsyncKeyState (VK_ESCAPE)) break;
            if (txGetAsyncKeyState (VK_RIGHT))    xC    += dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
            if (txGetAsyncKeyState (VK_LEFT))     xC    -= dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
            if (txGetAsyncKeyState (VK_DOWN))     yC    += dy * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
            if (txGetAsyncKeyState (VK_UP))       yC    -= dy * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
            if (txGetAsyncKeyState ('W'))         scale -= dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
            if (txGetAsyncKeyState ('S'))         scale += dx * (txGetAsyncKeyState (VK_SHIFT) ? 100.0f : 1.0f);
        )  
        BENCHMARK
        (
            unsigned long long start = __rdtsc();
            for (int f = 0; f < INNER_FRAMES; f++)
        )   
        {
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

                    SIMPLE_MODE (RGBQUAD* curRow = graphBuf + y * WIDTH;)
                    BENCHMARK   (RGBQUAD* curRow = offBuf   + y * WIDTH;)

                    BYTE gray[8] = {};
                    for (int i = 0; i < 8; ++i)
                    {
                        gray[i] = (BYTE)(N[i] << 4);
                        curRow[x + i] = (RGBQUAD){gray[i], gray[i], gray[i], 0};
                    }
                }
            }
        }

        BENCHMARK
        (
            unsigned long long end = __rdtsc();
            cycles[curTest] = (end - start) / INNER_FRAMES;
        )

        SIMPLE_MODE
        (
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
        )
    }

    BENCHMARK
    (
        // Сохранение результатов
        FILE* f = fopen ("bench_quad.csv", "w");
        if (f) 
        {
            for (int i = 0; i < NUM_OF_TESTS; i++)
                fprintf (f, "%llu\n", cycles[i]);
            fclose (f);
        }
        free (offBuf);
    )

    return 0;
}
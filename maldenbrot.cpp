#ifndef BENCHMARK_MODE
    #define SIMPLE_MODE(...) __VA_ARGS__
    #define BENCHMARK(...)
#else
    #define SIMPLE_MODE(...)
    #define BENCHMARK(...) __VA_ARGS__
#endif

#ifndef BENCHMARK_MODE
    #include <TXLib.h>
#endif

#include <math.h>
#include <windows.h>

#ifdef BENCHMARK_MODE
    #include <stdio.h>
    #include <stdlib.h>
    #include <x86intrin.h>
#endif

#define WIDTH   800
#define HEIGHT  600

// Границы области
float X_MIN = -2.2f;
float X_MAX =  1.0f;
float Y_MIN = -1.2f;
float Y_MAX =  1.2f;

void DrawFractal ();

// Функция рисования фрактала
void DrawFractal (RGBQUAD* buf, float x_min, float x_max, float y_min, float y_max)
{
    float dx = (x_max - x_min) / WIDTH;
    float dy = (y_max - y_min) / HEIGHT;

    for (int y = 0; y < HEIGHT; y++) 
    {
        float c_im = y_max - (float)y * dy;
        for (int x = 0; x < WIDTH; x++) 
        {
            float c_re = x_min + (float)x * dx;

            float z_re = 0.f, z_im = 0.f;
            int iter = 0;

            while (iter < 256 && z_re*z_re + z_im*z_im < 4.f) 
            {
                float new_re = z_re*z_re - z_im*z_im + c_re;
                z_im = 2.f * z_re * z_im + c_im;
                z_re = new_re;
                iter++;
            }

            int color = (iter == 256) ? 0 : 50 + iter * 2;
            if (color > 255) color = 255;
            RGBQUAD* p = &buf [y * WIDTH + x];
            p->rgbRed = p->rgbGreen = p->rgbBlue = (unsigned char) color;
            p->rgbReserved = 0;
        }
    }
}

int main ()
{
    BENCHMARK
    (
        const int NUM_OF_TESTS  = 10;
        const int WARMUP_TESTS  = 3;
        const int INNER_FRAMES  = 400;
    )

    SIMPLE_MODE
    (
        int frameCount = 0;
        int fps = 0;
        DWORD lastTime = GetTickCount ();
        txCreateWindow (WIDTH, HEIGHT);
        RGBQUAD* graphBuf = txVideoMemory ();
        Win32::_fpreset ();
    )

    BENCHMARK
    (
        RGBQUAD* offBuf = (RGBQUAD*)calloc(WIDTH * HEIGHT, sizeof(RGBQUAD));
        if (!offBuf) return 1;
        unsigned long long cycles[NUM_OF_TESTS] = {};
    )

    for (SIMPLE_MODE ( ; ; )
         BENCHMARK (int curTest = 0; curTest < NUM_OF_TESTS; curTest++))
    {
        SIMPLE_MODE 
        (
            if (txGetAsyncKeyState (VK_ESCAPE)) break;
            if (txGetAsyncKeyState (VK_LEFT))  { X_MIN -= 0.1f; X_MAX -= 0.1f; }
            if (txGetAsyncKeyState (VK_RIGHT)) { X_MIN += 0.1f; X_MAX += 0.1f; }
            if (txGetAsyncKeyState (VK_DOWN))  { Y_MIN += 0.1f; Y_MAX += 0.1f; }
            if (txGetAsyncKeyState (VK_UP))    { Y_MIN -= 0.1f; Y_MAX -= 0.1f; }
            if (txGetAsyncKeyState (VK_UP) && txGetAsyncKeyState (VK_SHIFT))   
                                               { float cx = (X_MIN+X_MAX)/2, cy = (Y_MIN+Y_MAX)/2;
                                                 X_MIN = cx + (X_MIN-cx)*0.9f; X_MAX = cx + (X_MAX-cx)*0.9f;
                                                 Y_MIN = cy + (Y_MIN-cy)*0.9f; Y_MAX = cy + (Y_MAX-cy)*0.9f; }
            if (txGetAsyncKeyState (VK_DOWN) && txGetAsyncKeyState (VK_SHIFT)) 
                                               { float cx = (X_MIN+X_MAX)/2, cy = (Y_MIN+Y_MAX)/2;
                                                 X_MIN = cx + (X_MIN-cx)*1.1f; X_MAX = cx + (X_MAX-cx)*1.1f;
                                                 Y_MIN = cy + (Y_MIN-cy)*1.1f; Y_MAX = cy + (Y_MAX-cy)*1.1f; }
            if (txGetAsyncKeyState ('R'))      { X_MIN = -2.2f; X_MAX = 1.0f; Y_MIN = -1.2f; Y_MAX = 1.2f; }
        )

        BENCHMARK
        (
            unsigned long long start = __rdtsc ();
            for (int f = 0; f < INNER_FRAMES; f++)
        )

        {
            SIMPLE_MODE( DrawFractal (graphBuf, X_MIN, X_MAX, Y_MIN, Y_MAX); )
            BENCHMARK  ( DrawFractal (offBuf,   X_MIN, X_MAX, Y_MIN, Y_MAX); )
        }

        BENCHMARK
        (
            unsigned long long end = __rdtsc ();
            cycles[curTest] = (end - start) / INNER_FRAMES;
        )
         
        SIMPLE_MODE
        (
            frameCount++;
            DWORD now = GetTickCount ();
            if (now - lastTime >= 1000)
            {
                fps = frameCount * 1000 / (now - lastTime);
                frameCount = 0;
                lastTime = now;
            }

            // вывод FPS
            char buf[32] = {};
            sprintf (buf, "%.1d", fps);
            txTextOut (0, 0, buf);
            //txSleep (50);
        )
    }

    BENCHMARK
    (
        // Сохранение результатов
        FILE* f = fopen ("bench_simple-O0.csv", "w");
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
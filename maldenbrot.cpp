#include <TXLib.h>
#include <windows.h>

#define WIDTH   800
#define HEIGHT  600

// Границы области
float X_MIN = -2.2f;
float X_MAX =  1.0f;
float Y_MIN = -1.2f;
float Y_MAX =  1.2f;

void DrawFractal ();

// Функция рисования фрактала
void DrawFractal ()
{
    txBegin ();
    RGBQUAD* buf = txVideoMemory ();

    float dx = (X_MAX - X_MIN) / WIDTH;
    float dy = (Y_MAX - Y_MIN) / HEIGHT;

    for (int y = 0; y < HEIGHT; y++)
    {
        float c_im = Y_MAX - y * dy;
        for (int x = 0; x < WIDTH; x++)
        {
            float c_re = X_MIN + x * dx;
            
            float z_re = 0, z_im = 0;
            int iter = 0;
            
            while (iter < 100 && z_re*z_re + z_im*z_im < 4)
            {
                float new_re = z_re*z_re - z_im*z_im + c_re;
                z_im = 2*z_re*z_im + c_im;
                z_re = new_re;
                iter++;
            }
            
            // Цвет: оттенки серого
            int color = iter == 100 ? 0 : 50 + iter * 2;
            RGBQUAD* p = &buf[y*WIDTH + x];
            p->rgbRed = p->rgbGreen = p->rgbBlue = color;
        }
    }
    txEnd ();
}

int main ()
{
    txCreateWindow (WIDTH, HEIGHT);

    int frameCount = 0;
    float fps = 0;
    DWORD lastTime = GetTickCount ();
    
    while (!txGetAsyncKeyState (VK_ESCAPE))
    {
        // Управление
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
        
        DrawFractal ();
                                            
        frameCount++;
        DWORD now = GetTickCount ();
        if (now - lastTime >= 1000)
        {
            fps = (float) frameCount * 1000.0f / (now - lastTime);
            frameCount = 0;
            lastTime = now;
        }

        // вывод FPS
        char buf[32];
        sprintf(buf, "FPS: %.1f", fps);
        //txSetColor(RGB(255,255,255));
        //txSelectFont("Arial", 20);
        txTextOut(0, 0, buf);
        //txSleep (50);
    }
    
    return 0;
}
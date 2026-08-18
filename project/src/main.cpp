//External includes
#include "SDL.h"
#include "SDL_surface.h"
#undef main

//Standard includes
#include <iostream>

//Project includes
#include "Timer.h"
#include "Renderer.h"
#if defined(_DEBUG)
#include "LeakDetector.h"
#endif

using namespace dae;

void ShutDown(SDL_Window* pWindow)
{
    SDL_DestroyWindow(pWindow);
    SDL_Quit();
}

int main(int argc, char* args[])
{
    //Unreferenced parameters
    (void)argc;
    (void)args;

    // Leak detection
    #if defined(_DEBUG)
    LeakDetector detector { };
    #endif

    //Create window + surfaces
    SDL_Init(SDL_INIT_VIDEO);

    const uint32_t width = 640;
    const uint32_t height = 480;

    SDL_Window* pWindow = SDL_CreateWindow(
        "Dual Rasterizer",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        width, height, 0);

    if (!pWindow)
        return 1;

    //Initialize "framework"
    const auto pTimer = new Timer();
    const auto pRenderer = new Renderer(pWindow);

    //Start loop
    pTimer->Start();

    // Start Benchmark
    // TODO pTimer->StartBenchmark();

    // Print controls
    std::cout << "CONTROLS:\n";
    std::cout << "[F1]  - toggle rendering backend (software / DirectX11)\n";
    std::cout << "[F2]  - toggle mesh rotation\n";
    std::cout << "[F3]  - toggle transparent mesh rendering\n";
    std::cout << "[F4]  - toggle sampling mode (Linear / Anisotropic / Point)\n";
    std::cout << "[F5]  - toggle render mode (observed area / diffuse only / specular only / combined)\n";
    std::cout << "[F6]  - toggle normal mapping\n";
    std::cout << "[F7]  - toggle draw depth\n";
    std::cout << "[F10] - toggle uniform clear color\n";
    std::cout << "[F11] - toggle FPS printing\n";

    float printTimer = 0.f;
    bool printFps = false;
    bool isLooping = true;
    bool takeScreenshot = false;
    while (isLooping)
    {
        //--------- Get input events ---------
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            switch (e.type)
            {
                case SDL_QUIT:
                    isLooping = false;
                    break;
                case SDL_KEYDOWN:
                    pRenderer->ToggleRenderMode(e.key.keysym.scancode);
                    pRenderer->ToggleDrawDepth(e.key.keysym.scancode);
                    pRenderer->ToggleNormalMapping(e.key.keysym.scancode);
                    pRenderer->ToggleMeshRotation(e.key.keysym.scancode);
                    pRenderer->ToggleRenderingBackend(e.key.keysym.scancode);
                    pRenderer->ToggleSamplingMode(e.key.keysym.scancode);
                    pRenderer->ToggleMeshRendering(e.key.keysym.scancode);
                    pRenderer->ToggleUniformClearColor(e.key.keysym.scancode);
                    if (e.key.keysym.scancode == SDL_SCANCODE_F11)
                    {
                        printFps = !printFps;
                        std::cout << "[Uniform] Toggled FPS printing to: " <<
                            (printFps ? "ON" : "OFF") << "\n";
                    }
                    break;
                case SDL_KEYUP:
                    if (e.key.keysym.scancode == SDL_SCANCODE_X)
                        takeScreenshot = true;
                    break;
            }
        }

        //--------- Update ---------
        pRenderer->Update(pTimer);

        //--------- Render ---------
        pRenderer->Render();

        //--------- Timer ---------
        pTimer->Update();
        printTimer += pTimer->GetElapsed();
        if (printTimer >= 1.f && printFps)
        {
            printTimer = 0.f;
            std::cout << "dFPS: " << pTimer->GetdFPS() << std::endl;
        }

        //Save screenshot after full render
        if (takeScreenshot)
        {
            if (!pRenderer->SaveBufferToImage())
                std::cout << "Screenshot saved!" << std::endl;
            else
                std::cout << "Something went wrong. Screenshot not saved!" << std::endl;

            takeScreenshot = false;
        }
    }
    pTimer->Stop();

    //Shutdown "framework"
    delete pRenderer;
    delete pTimer;

    ShutDown(pWindow);
    return 0;
}
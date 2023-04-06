#include <cstdio>

#include <Windows.h>

#include <Psapi.h>
#include <Shlwapi.h>
#include <d3d11.h>
#include <dwmapi.h>
#include <tchar.h>

#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

#include "Cheat.h"
#include "Util.h"

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// global render state
static ID3D11Device *g_pd3dDevice = NULL;
static ID3D11DeviceContext *g_pd3dDeviceContext = NULL;
static IDXGISwapChain *g_pSwapChain = NULL;
static ID3D11RenderTargetView *g_mainRenderTargetView = NULL;

void MainThread() {
#ifdef _DEBUG
    g_log = std::make_unique<Logger>("C:\\temp\\baseq.log");
#else
    g_log = std::make_unique<Logger>(nullptr);
#endif
    g_log->info(L"Started");

    // assume there is only one task manager window which belongs to us
    // probably not the best assumption to make
    auto tmgr = ::FindWindowW(L"TaskManagerWindow", L"Task Manager");
    if (!tmgr) return;

    // create the window class for the new window
    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    // current module
    wc.hInstance = ::GetModuleHandleW(NULL);
    wc.lpszClassName = L"baseq";
    wc.lpfnWndProc = WndProc;
    ::RegisterClassExW(&wc);

    HWND overlay = ::CreateWindowExW(
            WS_EX_TOPMOST | WS_EX_TRANSPARENT | WS_EX_LAYERED,
            wc.lpszClassName,
            L"baseq",
            WS_POPUP,
            0, 0, 0, 0,
            NULL, NULL, wc.hInstance, NULL);
    ::SetLayeredWindowAttributes(overlay, RGB(0, 0, 0), 0xff, LWA_ALPHA);
    ::ShowWindow(overlay, SW_SHOW);
    ::UpdateWindow(overlay);
    g_c = std::make_unique<Cheat>(overlay);

    if (!CreateDeviceD3D(overlay)) {
        CleanupDeviceD3D();
        ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
        return;
    }

    // hide task manager and show our overlay
    ::ShowWindow(tmgr, SW_HIDE);
    ::UpdateWindow(tmgr);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui_ImplWin32_EnableDpiAwareness();
    ImGui::StyleColorsDark();

    ImGui_ImplWin32_Init(overlay);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    auto &io = ImGui::GetIO();
    ImFontConfig cfg{};
    cfg.OversampleH = 3;
    cfg.OversampleV = 1;
    cfg.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 24, &cfg);

    while (true) {
        // listen for when we should stop
        MSG msg;
        while (::PeekMessageW(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg);
            ::DispatchMessageW(&msg);
            if (msg.message == WM_QUIT)
                g_c->shutdown = true;
        }
        if (g_c->shutdown) {
            g_log->info(L"Quitting");
            break;
        }

        // update state relating to the game process/window here
        g_c->update();

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        g_c->update_overlay();

        ImGui::Render();

        const float clear_color_with_alpha[4] = {0.f, 0.f, 0.f, 0.f};
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    // manual cleanup since raii doesn't really work for globals
    // we need to keep the pointer valid for destruction, therefore cannot use reset
    g_c.get_deleter()(g_c.get());
    g_c.release();
    g_log.reset();

    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
}

// --- boilerplate ---

DWORD WINAPI MainThreadWrapper(HMODULE module) {
    MainThread();
    FreeLibraryAndExitThread(module, 0);
    return 0;
}

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD reason,
                      LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        auto handle = CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE) &MainThreadWrapper, hModule, NULL, NULL);
        if (handle) {
            CloseHandle(handle);
        }
    }
    return TRUE;
}

bool CreateDeviceD3D(HWND hWnd) {
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = {
            D3D_FEATURE_LEVEL_11_0,
            D3D_FEATURE_LEVEL_10_0,
    };
    HRESULT res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED)// Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_WARP, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D() {
    CleanupRenderTarget();
    if (g_pSwapChain) {
        g_pSwapChain->Release();
        g_pSwapChain = NULL;
    }
    if (g_pd3dDeviceContext) {
        g_pd3dDeviceContext->Release();
        g_pd3dDeviceContext = NULL;
    }
    if (g_pd3dDevice) {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }
}

void CreateRenderTarget() {
    ID3D11Texture2D *pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget() {
    if (g_mainRenderTargetView) {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = NULL;
    }
}

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;
    /* if (ImGui::GetCurrentContext() != nullptr) {
        auto &io = ImGui::GetIO();
        if (io.WantCaptureMouse && (msg == WM_LBUTTONDOWN || msg == WM_LBUTTONUP || msg == WM_RBUTTONDOWN || msg == WM_RBUTTONUP || msg == WM_MBUTTONDOWN || msg == WM_MBUTTONUP || msg == WM_MOUSEWHEEL || msg == WM_MOUSEMOVE)) {
            return true;
        }
    } */

    switch (msg) {
        case WM_SIZE:
            if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED) {
                CleanupRenderTarget();
                g_pSwapChain->ResizeBuffers(0, (UINT) LOWORD(lParam), (UINT) HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
                CreateRenderTarget();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wParam & 0xfff0) == SC_KEYMENU)// Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}

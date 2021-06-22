#include <d3d9.h>

#include "imgui_impl_win32.h"

static LPDIRECT3D9              g_d3d = nullptr;
static LPDIRECT3DDEVICE9        g_d3d_device = nullptr;
static D3DPRESENT_PARAMETERS    g_d3d_parameters = {};

long __stdcall wnd_proc(HWND hwnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {

}

bool create_device_d3d(HWND hWnd)
{
    if ((g_d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) {
        return false;
    }

    // Create the D3DDevice
    ZeroMemory(&g_d3d_parameters, sizeof g_d3d_parameters);
    g_d3d_parameters.Windowed = TRUE;
    g_d3d_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3d_parameters.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3d_parameters.EnableAutoDepthStencil = TRUE;
    g_d3d_parameters.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3d_parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &g_d3d_parameters, &g_d3d_device) < 0) {
        return false;
    }

    return true;
}

int main() {
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"deadcell-gui-test", nullptr };

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, L"deadcell-gui-test", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);
}
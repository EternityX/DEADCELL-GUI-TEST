#include <thirdparty/imgui/imgui.h>
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

#include <d3d9.h>

#include <gui_instance.h>
#include <iostream>

#include <controls/button.h>
#include <wrapped/layout.h>

static LPDIRECT3D9 g_d3d = nullptr;
static LPDIRECT3DDEVICE9 g_d3d_device = nullptr;
static D3DPRESENT_PARAMETERS g_d3d_parameters = {};

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void reset_device() {
    ImGui_ImplDX9_InvalidateDeviceObjects();
    HRESULT hr = g_d3d_device->Reset(&g_d3d_parameters);

    if (hr == D3DERR_INVALIDCALL) {
        IM_ASSERT(0);
    }

    ImGui_ImplDX9_CreateDeviceObjects();
}

long __stdcall wnd_proc(HWND hwnd, unsigned int msg, WPARAM wparam, LPARAM lparam) {
    if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam))
        return true;

    switch (msg) {
        case WM_SIZE:
            if (g_d3d_device != NULL && wparam != SIZE_MINIMIZED) {
                g_d3d_parameters.BackBufferWidth = LOWORD(lparam);
                g_d3d_parameters.BackBufferHeight = HIWORD(lparam);
                reset_device();
            }
            return 0;
        case WM_SYSCOMMAND:
            if ((wparam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
                return 0;
            break;
        case WM_DESTROY:
            ::PostQuitMessage(0);
            return 0;
    }
    return ::DefWindowProc(hwnd, msg, wparam, lparam);
}

bool create_device_d3d(HWND hWnd) {
    if ((g_d3d = Direct3DCreate9(D3D_SDK_VERSION)) == nullptr) {
        return false;
    }

    // Create the D3DDevice
    ZeroMemory(&g_d3d_parameters, sizeof g_d3d_parameters);
    g_d3d_parameters.Windowed = TRUE;
    g_d3d_parameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
    g_d3d_parameters.BackBufferFormat = D3DFMT_UNKNOWN;
    // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    g_d3d_parameters.EnableAutoDepthStencil = TRUE;
    g_d3d_parameters.AutoDepthStencilFormat = D3DFMT_D16;
    g_d3d_parameters.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync
    //g_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate

    if (g_d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING,
                            &g_d3d_parameters, &g_d3d_device) < 0) {
        return false;
    }

    return true;
}

void cleanup_device_d3d() {
    if (g_d3d_device) {
        g_d3d_device->Release();
        g_d3d_device = nullptr;
    }
    if (g_d3d) {
        g_d3d->Release();
        g_d3d = nullptr;
    }
}

auto window1 = std::make_shared<deadcell::gui::window>("window_id1");
auto window2 = std::make_shared<deadcell::gui::window>("window_id2");
auto window3 = std::make_shared<deadcell::gui::window>("window_id3");

auto button = std::make_shared<deadcell::gui::button>("button_id1");
auto button2 = std::make_shared<deadcell::gui::button>("button_id2");

void draw_test() {

}

int main() {
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX), CS_CLASSDC, wnd_proc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr,
        L"gui", nullptr
    };

    RegisterClassEx(&wc);

    HWND hwnd = CreateWindow(wc.lpszClassName, L"deadcell-gui-test", WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL,
                             NULL, wc.hInstance, NULL);

    if (!create_device_d3d(hwnd)) {
        cleanup_device_d3d();
        UnregisterClass(wc.lpszClassName, wc.hInstance);
        return 1;
    }

    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX9_Init(g_d3d_device);

    const ImVec4 clear_color = ImVec4(0.29f, 0.31f, 0.36f, 1.00f);

    auto inst = std::make_shared<deadcell::gui::gui_instance>();

    window1->set_position_size({ 0, 0 }, { 250, 250 });
    window2->set_position_size({ 200, 200 }, { 250, 250 });
    window3->set_position_size({ 400, 400 }, { 250, 250 });

    //inst->wm()->add_window(window1);
    inst->wm()->add_window(window2);
    inst->wm()->add_window(window3);

    inst->wm()->move_to_front(window1, true);

    
    window2->set_resizeable(false);
    window3->set_titlebar_height(24.0f);

    window1->add_child(button);
    window1->add_child(button2);

    auto str = window1->build_class_tree();
    std::cout << str << "\n";

    bool done = false;
    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            if (msg.message == WM_QUIT) {
                done = true;
            }
        }
        if (done) {
            break;
        }

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        ImGui::GetBackgroundDrawList()->ChannelsSplit(8);
        ImGui::GetForegroundDrawList()->ChannelsSplit(8);

        ImGui::GetForegroundDrawList()->ChannelsSetCurrent(0);
        ImGui::GetBackgroundDrawList()->ChannelsSetCurrent(0);

        deadcell::gui::drawing::set_draw_list(ImGui::GetForegroundDrawList());

        draw_test();

        inst->wm()->new_frame();

        //ImGui::GetForegroundDrawList()->ChannelsMerge();
        ImGui::EndFrame();

        g_d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

        const D3DCOLOR clear_col_dx = D3DCOLOR_RGBA(static_cast<int>(clear_color.x * clear_color.w * 255.0f),
                                                    static_cast<int>(clear_color.y * clear_color.w * 255.0f),
                                                    static_cast<int>(clear_color.z * clear_color.w * 255.0f),
                                                    static_cast<int>(clear_color.w * 255.0f));

        g_d3d_device->Clear(0, nullptr, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
        if (g_d3d_device->BeginScene() >= 0) {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            g_d3d_device->EndScene();
        }
        const HRESULT result = g_d3d_device->Present(nullptr, nullptr, nullptr, nullptr);

        // Handle loss of D3D9 device
        if (result == D3DERR_DEVICELOST && g_d3d_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET) {
            reset_device();
        }
    }

    ImGui_ImplDX9_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    cleanup_device_d3d();
    DestroyWindow(hwnd);
    UnregisterClass(wc.lpszClassName, wc.hInstance);

    return 0;
}

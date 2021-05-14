#define LEMI_LOADER_VERSION "4"

#include <Windows.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include <d3d9.h>
#include <tchar.h>
#include <thread>

#include "download.h"

#include "imgui/my_style.h"

#include "globals.h"
#include "ui.h"

#include "fonts.h"
#include "../resource.h"

#include "fonts/ubuntu_font.h"

static LPDIRECT3D9              d3d = NULL;
static LPDIRECT3DDEVICE9        d3d_device = NULL;
static D3DPRESENT_PARAMETERS    d3dpp = {};

bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void ResetDevice();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{

	WNDCLASSEX wc = {
		sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0l, 0l, GetModuleHandle(0),
		LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON1)), 0, 0, 0, "LemiProjectLoader", 0
	};

	RegisterClassEx(&wc);
	auto hwnd = CreateWindow(wc.lpszClassName, "LemiProject Loader", WS_OVERLAPPED | WS_MINIMIZEBOX | WS_SYSMENU, 100, 100, 650, 400, 0, 0, wc.hInstance, 0);
	
	
	if (!CreateDeviceD3D(hwnd))
	{
		CleanupDeviceD3D();
		UnregisterClass(wc.lpszClassName, wc.hInstance);
		return 1;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT);
	UpdateWindow(hwnd);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	auto& io = ImGui::GetIO();

	fonts::ubuntu = io.Fonts->AddFontFromMemoryCompressedTTF(ubuntu_font_compressed_data, ubuntu_font_compressed_size, 28.f, nullptr,
		io.Fonts->GetGlyphRangesCyrillic());
	fonts::ubuntu_big = io.Fonts->AddFontFromMemoryCompressedTTF(ubuntu_font_compressed_data, ubuntu_font_compressed_size, 48.f, nullptr,
		io.Fonts->GetGlyphRangesCyrillic());
	fonts::ubuntu_small = io.Fonts->AddFontFromMemoryCompressedTTF(ubuntu_font_compressed_data, ubuntu_font_compressed_size, 18.f, nullptr,
		io.Fonts->GetGlyphRangesCyrillic());
	
	io.IniFilename = nullptr;

	imgui_styles::std_dark_theme();

	ImGui_ImplWin32_Init(hwnd);
	ImGui_ImplDX9_Init(d3d_device);

	hack_list::init_hacks();

#ifndef DEBUG
	std::thread([&]()
		{
			std::string version;
			download::download_to_memory("https://raw.githubusercontent.com/Sanceilaks/LemiProject/main/loader_version", version, nullptr);
			if (!version._Starts_with(LEMI_LOADER_VERSION))
			{
				auto res = MessageBox(hwnd, "New version available", "Update", MB_OK);
				ShellExecute(hwnd, 0, "https://sanceilaks.github.io/LemiProject/", 0, 0, SW_HIDE);
			}
		}).detach();
#endif

	auto done = false;
	while (!done)
	{
		MSG msg;
		while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			if (msg.message == WM_QUIT)
				done = true;
		}
		if (done) break;

		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ui::render_ui();

		ImGui::Render();
		d3d_device->SetRenderState(D3DRS_ZENABLE, FALSE);
		d3d_device->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
		d3d_device->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
		d3d_device->Clear(0, 0, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_RGBA(0, 0, 0, 1), 1.f, 0);
		if (d3d_device->BeginScene() >= 0)
		{
			ImGui::Render();
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			d3d_device->EndScene();
		}
		auto result = d3d_device->Present(0, 0, 0, 0);

		if (result == D3DERR_DEVICELOST && d3d_device->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			ResetDevice();
	}
	
	ImGui_ImplDX9_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	CleanupDeviceD3D();
	DestroyWindow(hwnd);
	UnregisterClass(wc.lpszClassName, wc.hInstance);
	return 0;
}

bool CreateDeviceD3D(HWND hWnd)
{
	if ((d3d = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		return false;

	ZeroMemory(&d3dpp, sizeof(d3dpp));
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.FullScreen_RefreshRateInHz = D3DPRESENT_RATE_DEFAULT;
	if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3d_device) < 0)
		return false;

	return true;
}

void CleanupDeviceD3D()
{
	if (d3d_device)
		d3d_device->Release(), d3d_device = nullptr;
	if (d3d)
		d3d->Release(), d3d = nullptr;
}

void ResetDevice()
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	auto h = d3d_device->Reset(&d3dpp);
	if (h == D3DERR_INVALIDCALL)
		IM_ASSERT(0);
	ImGui_ImplDX9_CreateDeviceObjects();
}


extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (d3d_device != NULL && wParam != SIZE_MINIMIZED)
		{
			d3dpp.BackBufferWidth = LOWORD(lParam);
			d3dpp.BackBufferHeight = HIWORD(lParam);
			ResetDevice();
		}
		return 0;
	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}




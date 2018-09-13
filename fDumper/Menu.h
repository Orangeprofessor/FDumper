#pragma once

#include "pch.h"
#include <libda3m0n/Misc/Thunk.hpp>
#include <string>
#include <map>
#include <d3d9.h>

#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

class CImWindowBase;
class CImDialogBase;

class CImWindowBase
{
public:
	typedef INT_PTR(CImWindowBase::*fnWndProc)(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	typedef std::map<UINT, std::pair<CImWindowBase*, fnWndProc>> mapWndProc;

	CImWindowBase()
		: m_hwnd(NULL)
		, m_subThunk(&CImWindowBase::SubProc, this) { }

	virtual inline WNDPROC oldProc() const { return m_oldProc; }

	virtual void Subclass(UINT message, fnWndProc handler, CImWindowBase* instance = nullptr)
	{
		// Remove old handler
		if (handler == nullptr)
		{
			if (m_subMessages.count(message))
			{
				m_subMessages.erase(message);

				// Remove subproc
				if (m_subMessages.empty())
				{
					SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_oldProc));
					m_oldProc = nullptr;
				}

				return;
			}
		}
		else
		{
			m_subMessages[message] = std::make_pair(instance ? instance : this, handler);
			if (!m_oldProc)
				m_oldProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(m_hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(m_subThunk.GetThunk())));
		}
	}

private:
	LRESULT SubProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_subMessages.count(message))
			return (this->m_subMessages[message].first->*m_subMessages[message].second)(hwnd, message, wParam, lParam);

		return CallWindowProc(m_oldProc, hwnd, message, wParam, lParam);
	}

protected:
	HWND m_hwnd = NULL;
	UINT m_id = 0;
	WNDPROC m_oldProc = nullptr;
	mapWndProc m_subMessages;
	Win32Thunk<WNDPROC, CImWindowBase> m_subThunk;
};

class CImDialogBase : public CImWindowBase
{
public:
	typedef INT_PTR(CImDialogBase::*fnDlgProc)(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	typedef std::map<UINT, fnDlgProc> mapMsgProc;

	virtual void ImDraw() = 0;

	CImDialogBase()
	{
		m_messages[WM_CREATE] = &CImDialogBase::OnInit;
		m_messages[WM_SIZE] = &CImDialogBase::OnSize;
		m_messages[WM_DESTROY] = &CImDialogBase::OnDestroy;
	}

	virtual INT_PTR Run()
	{
		MSG msg = { 0 };

		Win32Thunk<WNDPROC, CImDialogBase> wndProc(&CImDialogBase::WndProc, this);

		WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, wndProc.GetThunk(), 0L, 0L,
			GetModuleHandle(NULL), NULL, NULL, NULL, NULL, L"fdumper", NULL };
		RegisterClassEx(&wc);

		m_hwnd = CreateWindow(L"fdumper", L"FDumper v0.2", WS_OVERLAPPEDWINDOW,
			100, 100, 560, 430, NULL, NULL, wc.hInstance, NULL);

		if ((m_pD3D = Direct3DCreate9(D3D_SDK_VERSION)) == NULL)
		{
			UnregisterClass(L"fdumper", wc.hInstance);
			throw std::exception("One day i'll implement actual exception handling for D3D errors");
			return 0;
		}

		memset(&m_d3dpp, 0, sizeof(m_d3dpp));
		m_d3dpp.Windowed = TRUE;
		m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
		m_d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
		m_d3dpp.EnableAutoDepthStencil = TRUE;
		m_d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
		m_d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE; // Present with vsync

		// Create Device
		if (m_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, m_hwnd,
			D3DCREATE_HARDWARE_VERTEXPROCESSING, &m_d3dpp, &m_pD3Ddevice) < 0)
		{
			m_pD3D->Release();
			UnregisterClass(L"fdumper", wc.hInstance);
			throw std::exception("One day i'll implement actual exception handling for D3D errors");
			return 0;
		}

		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		ImGui_ImplWin32_Init(m_hwnd);
		ImGui_ImplDX9_Init(m_pD3Ddevice);

		ImGui::StyleColorsClassic();
		ImGuiStyle& style = ImGui::GetStyle();

		style.Alpha = 1.f;
		style.WindowPadding = ImVec2(8, 8);
		style.WindowRounding = 0;
		style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
		style.FramePadding = ImVec2(4, 4);
		style.FrameRounding = 0.f;
		style.ItemSpacing = ImVec2(8, 4);
		style.TouchExtraPadding = ImVec2(0, 0);
		style.IndentSpacing = 21.0f;
		style.ColumnsMinSpacing = 3.0f;
		style.ScrollbarSize = 12.0f;
		style.ScrollbarRounding = 0.0f;
		style.GrabMinSize = 5.0f;
		style.GrabRounding = 0.0f;
		style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
		style.DisplayWindowPadding = ImVec2(22, 22);
		style.DisplaySafeAreaPadding = ImVec2(4, 4);
		style.AntiAliasedLines = true;
		style.CurveTessellationTol = 1.25f;

		io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\segoeui.ttf", 17);
		io.Fonts->AddFontDefault();

		ImVec4 clearcolor(0.45f, 0.55f, 0.60f, 1.00f);

		ShowWindow(m_hwnd, SW_SHOWDEFAULT);
		UpdateWindow(m_hwnd);
		while (msg.message != WM_QUIT)
		{
			if (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
				continue;
			}

			ImGui_ImplDX9_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();

			ImDraw();

			ImGui::EndFrame();
			m_pD3Ddevice->SetRenderState(D3DRS_ZENABLE, false);
			m_pD3Ddevice->SetRenderState(D3DRS_ALPHABLENDENABLE, false);
			m_pD3Ddevice->SetRenderState(D3DRS_SCISSORTESTENABLE, false);
			D3DCOLOR clear_col_dx = D3DCOLOR_RGBA((int)(clearcolor.x*255.0f), (int)(clearcolor.y*255.0f), (int)(clearcolor.z*255.0f), (int)(clearcolor.w*255.0f));
			m_pD3Ddevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, clear_col_dx, 1.0f, 0);
			if (m_pD3Ddevice->BeginScene() >= 0)
			{
				ImGui::Render();
				ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
				m_pD3Ddevice->EndScene();
			}
			HRESULT result = m_pD3Ddevice->Present(NULL, NULL, NULL, NULL);

			// Handle loss of D3D9 device
			if (result == D3DERR_DEVICELOST && m_pD3Ddevice->TestCooperativeLevel() == D3DERR_DEVICENOTRESET)
			{
				ImGui_ImplDX9_InvalidateDeviceObjects();
				m_pD3Ddevice->Reset(&m_d3dpp);
				ImGui_ImplDX9_CreateDeviceObjects();
			}
		}

		ImGui_ImplDX9_Shutdown();
		ImGui_ImplWin32_Shutdown();
		ImGui::DestroyContext();

		if (m_pD3Ddevice) m_pD3Ddevice->Release();
		if (m_pD3D) m_pD3D->Release();
		DestroyWindow(m_hwnd);
		UnregisterClass(L"fdumper", wc.hInstance);

		return TRUE;
	}

protected:

	LRESULT WndProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (ImGui_ImplWin32_WndProcHandler(hDlg, message, wParam, lParam))
			return TRUE;

		if (m_messages.count(message))
			return (this->*m_messages[message])(hDlg, message, wParam, lParam);

		return DefWindowProc(hDlg, message, wParam, lParam);
	}

	virtual LRESULT OnInit(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		m_hwnd = hDlg;
		return TRUE;
	}

	virtual LRESULT OnSize(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (m_pD3Ddevice != NULL && wParam != SIZE_MINIMIZED)
		{
			ImGui_ImplDX9_InvalidateDeviceObjects();
			m_d3dpp.BackBufferWidth = LOWORD(lParam);
			m_d3dpp.BackBufferHeight = HIWORD(lParam);
			HRESULT hr = m_pD3Ddevice->Reset(&m_d3dpp);
			if (hr == D3DERR_INVALIDCALL)
				IM_ASSERT(0);
			ImGui_ImplDX9_CreateDeviceObjects();
		}

		return FALSE;
	}

	virtual LRESULT OnDestroy(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
	{
		PostQuitMessage(0);
		return FALSE;
	}

protected:
	mapMsgProc m_messages;			// Message handlers	
	D3DPRESENT_PARAMETERS m_d3dpp;  // Params for D3D
	LPDIRECT3D9 m_pD3D;			    // D3D ptr
	LPDIRECT3DDEVICE9 m_pD3Ddevice; // D3D device ptr
};


class CImMenu : public CImDialogBase
{
public:
	CImMenu();
	~CImMenu();

	virtual void ImDraw()override;
private:
	int site;
	int dlType;
};



#include "pch.h"
#include "D3D12Renderer.h"
#include "D3D12Mesh.h"
#include "GeometryGenerator.h"

#include "../../Gen/LinkedList.h"

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

/*
=================
Main entry point
=================
*/

int main(int argc, char* argv[])
{
	// Memory leak profiler.
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// Register the window class.
	const wchar_t CLASS_NAME[] = L"Windows Class";
	const wchar_t WINDOW_NAME[] = L"XFree Engine Demo_v.1.0";

	WNDCLASS wc = { };

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = ::GetModuleHandle(nullptr);
	wc.lpszClassName = CLASS_NAME;
	::RegisterClass(&wc);

	// Create the window.
	HWND hwnd = ::CreateWindowEx(0, CLASS_NAME, WINDOW_NAME, WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, wc.hInstance, NULL);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, SW_SHOW);

	D3D12Renderer* renderer = new D3D12Renderer;
	if (!renderer->Init(hwnd))
		return -1;

	xlist* list = nullptr;
	xlist_init(&list);

	D3D12Mesh* mesh = renderer->CreateMesh(GeometryGenerator::MakeBox(0.25f));
	mesh->UpdateWorldMatrix(Matrix::CreateRotationY(DirectX::XM_PIDIV4) * Matrix::CreateTranslation(Vector3(0.0f, 0.2f, 0.0f)));
	xlist_insert(list, mesh);

	mesh = renderer->CreateMesh(GeometryGenerator::MakeBox(0.25f));
	mesh->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(-0.5f, 0.0f, 0.0f)));
	xlist_insert(list, mesh);

	mesh = renderer->CreateMesh(GeometryGenerator::MakeBox(0.25f));
	mesh->UpdateWorldMatrix(Matrix::CreateTranslation(Vector3(0.5f, 0.0f, 0.0f)));
	xlist_insert(list, mesh);

	MSG msg = { };
	while (true)
	{
		if (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
		}
		else
		{
			renderer->Update();

			while (true)
			{
				void* iter = xlist_iter(list);
				if (iter == nullptr)
					break;
				((D3D12Mesh*)(xlist_get_data(iter)))->Update();
			}

			renderer->BeginRender();

			while (true)
			{
				void* iter = xlist_iter(list);
				if (iter == nullptr)
					break;
				renderer->RenderMesh((D3D12Mesh*)xlist_get_data(iter));
			}
			
			renderer->EndRender();
			renderer->Present();
		}
	}

	while (true)
	{
		void* iter = xlist_iter(list);
		if (iter == nullptr)
			break;
		renderer->DestroyMesh((D3D12Mesh*)xlist_get_data(iter));
	}

	xlist_clean(list);

	renderer->Clean();
	delete renderer;
	renderer = nullptr;

	::CloseWindow(hwnd);
	hwnd = nullptr;

	return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
	case WM_SIZE:
	{
		int width = LOWORD(lParam);  // Macro to get the low-order word.
		int height = HIWORD(lParam); // Macro to get the high-order word.
	}
	break;
	case WM_DESTROY:
	{
		::PostQuitMessage(-1);
	}
	break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}



#include "pch.h"
#include "D3D12Renderer.h"
#include "D3D12Mesh.h"

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

	// Define the geometry for a triangle.
	Vertex triangleVertices[] =
	{
		{ Vector3(0.0f, 0.25f, 0.9f), Vector4(1.0f, 0.0f, 0.0f, 1.0f) },
		{ Vector3{ 0.25f, -0.25f, 0.9f }, Vector4{ 0.0f, 1.0f, 0.0f, 1.0f } },
		{ Vector3{ -0.25f, -0.25f, 0.9f }, Vector4{ 0.0f, 0.0f, 1.0f, 1.0f } }
	};

	const uint32 vertexBufferSize = sizeof(triangleVertices);

	uint32 triangleIndices[] = { 0, 1, 2 };

	const uint32 indexBufferSize = sizeof(triangleIndices);

	MeshData triangle;
	triangle.vertices = triangleVertices;
	triangle.indices = triangleIndices;
	triangle.verticesSize = sizeof(triangleVertices);
	triangle.indicesSize = sizeof(triangleIndices);
	triangle.verticesCount = triangle.verticesSize / sizeof(Vertex);
	triangle.indicesCount = triangle.indicesSize / sizeof(Index);

	D3D12Mesh* mesh = renderer->CreateMesh(triangle);

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
			mesh->Update();

			renderer->BeginRender();

			renderer->RenderMesh(mesh);
			
			renderer->EndRender();
			renderer->Present();
		}
	}

	renderer->DestroyMesh(mesh);

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



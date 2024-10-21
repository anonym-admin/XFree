#pragma once

#include "../Common/Types.h"

#include <stdio.h>
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifdef _DEBUG
	#define new new( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
	#define new new
#endif
#include <Windows.h>

/*
==============
Renderer
==============
*/
#pragma warning(disable : 6001)

#include <d3d12.h>
#include <dxgi1_6.h>
#include "d3dx12.h"
#include <dxgidebug.h>
#include <d3dcompiler.h>

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "d3dcompiler.lib")

// TODO
#pragma comment(lib, "../Binary/Debug/x64/Gen.lib")

#include "D3D12GpuBuffer.h"
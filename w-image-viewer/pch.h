#pragma once

#define _CRT_SECURE_NO_WARNINGS

//windows
#include "targetver.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <ShlObj.h>
#include <wrl/client.h>
#include <shellapi.h>

#include <Shlwapi.h>
#pragma comment(lib, "Shlwapi.lib")

//directx
#include <d3d11_4.h>
#pragma comment(lib, "d3d11.lib")

#include <dxgi1_6.h>


//imgui
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

//oiio
#define OIIO_STATIC_DEFINE
#include <OpenImageIO/imageio.h>
#ifdef NDEBUG
	#pragma comment(lib, "OpenImageIO.lib")
	#pragma comment(lib, "OpenImageIO.lib")
#else
	#pragma comment(lib, "OpenImageIO_d.lib")
	#pragma comment(lib, "OpenImageIO_Util_d.lib")
#endif


//lcms
#include <lcms2.h>

//std
#include <cassert>
#include <filesystem>
#include <unordered_map>
#include <string>
#include <fstream>
#include <memory>
#include <array>
#include <vector>
#include <exception>
#include <numbers>
#include <utility>
#include <ranges>

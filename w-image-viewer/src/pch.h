#pragma once

// windows
#include "resources/targetver.h"
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <ShlObj.h>
#include <shellapi.h>
#include <Shlwapi.h>

// directx
#include <d3d11_4.h>
#include <dxgi1_6.h>


// imgui
#define IMGUI_USER_CONFIG "include\wiv_imconfig.h"
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

// oiio
#define OIIO_STATIC_DEFINE
#include <OpenImageIO/imageio.h>
#include <OpenImageIO/filesystem.h>

// libraw
#include <libraw/libraw.h>

// lcms
#include <lcms2.h>

#include "include\ComPtr.h"

// std
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

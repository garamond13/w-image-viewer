#pragma once

#include "pch.h"

// Non blocking.
// Ignores return value.
inline void wiv_message(std::wstring text, std::wstring caption = L"WIV - Message", UINT type = 0)
{
	std::thread t([=]() noexcept { MessageBoxW(nullptr, text.c_str(), caption.c_str(), type); });
	t.detach();
}

inline int wiv_message_blocking(std::wstring text, std::wstring caption = L"WIV - Message", UINT type = 0) noexcept
{
	return MessageBoxW(nullptr, text.c_str(), caption.c_str(), type);
}
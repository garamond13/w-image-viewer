#pragma once

#include "pch.h"
#include "image.h"

class File_manager
{
public:
    bool file_open(const wchar_t* path);
    void file_next();
    void file_previous();
    bool drag_and_drop(HDROP hdrop);
    void delete_file();
    std::filesystem::path file_current;
    Image image;
};

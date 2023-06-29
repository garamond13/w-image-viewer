#include "pch.h"
#include "file_manager.h"
#include "config.h"
#include "helpers.h"

bool File_manager::file_open(std::wstring_view path)
{
    if (image.set_image_input(path)) {
        file_current = path;
        return true;
    }
    return false;
}

void File_manager::file_next()
{
    //iterate directory
    std::vector<std::filesystem::path> files;
    for (const auto& file : std::filesystem::directory_iterator(file_current.parent_path())) {
        if (!file.is_directory() && PathMatchSpecExW(file.path().c_str(), WIV_SUPPORTED_FILE_TYPES, PMSF_MULTIPLE) == S_OK) {
            if (file.path() > file_current)
                files.push_back(file.path());
        }
    }

    if (files.empty())
        return;
    
    //get next valid file
    std::sort(files.begin(), files.end());
    for (const auto& file : files) {
        if (image.set_image_input(file.c_str())) {
            file_current = file;
            break;
        }
    }
}

void File_manager::file_previous()
{
    //iterate directory
    std::vector<std::filesystem::path> files;
    for (const auto& file : std::filesystem::directory_iterator(file_current.parent_path())) {
        if (!file.is_directory() && PathMatchSpecExW(file.path().c_str(), WIV_SUPPORTED_FILE_TYPES, PMSF_MULTIPLE) == S_OK) {
            if (file.path() < file_current)
                files.push_back(file.path());
        }
    }

    if (files.empty())
        return;
    
    //get previous valid file
    std::sort(files.begin(), files.end());
    for (const auto& file : std::ranges::views::reverse(files)) {
        if (image.set_image_input(file.c_str())) {
            file_current = file;
            break;
        }
    }
}

bool File_manager::drag_and_drop(HDROP hdrop)
{
    wchar_t path[MAX_PATH];
    wiv_assert(DragQueryFileW(hdrop, 0, path, MAX_PATH), != 0);
    DragFinish(hdrop);
    if (image.set_image_input(path)) {
        file_current = path;
        return true;
    }
    return false;
}

#include "pch.h"
#include "file_manager.h"
#include "config.h"
#include "include\helpers.h"
#include "include\supported_extensions.h"
#include "include\global.h"
#include "window.h"
#include "include\ensure.h"

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
    // Iterate directory for next files.
    std::vector<std::filesystem::path> files;
    for (const auto& file : std::filesystem::directory_iterator(file_current.parent_path())) {
        if (!file.is_directory() && PathMatchSpecExW(file.path().c_str(), WIV_SUPPORTED_EXTENSIONS, PMSF_MULTIPLE) == S_OK) {
            if (file.path() > file_current) {
                files.push_back(file.path());
            }
        }
    }

    if (files.empty()) {
        if (g_config.cycle_files.val) {
            for (const auto& file : std::filesystem::directory_iterator(file_current.parent_path())) {
                if (!file.is_directory() && PathMatchSpecExW(file.path().c_str(), WIV_SUPPORTED_EXTENSIONS, PMSF_MULTIPLE) == S_OK) {
                    files.push_back(file.path());
                }
            }
        }
        else {
            return;
        }
    }

    // Get next valid file.
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
    // Iterate directory for previous files.
    std::vector<std::filesystem::path> files;
    for (const auto& file : std::filesystem::directory_iterator(file_current.parent_path())) {
        if (!file.is_directory() && PathMatchSpecExW(file.path().c_str(), WIV_SUPPORTED_EXTENSIONS, PMSF_MULTIPLE) == S_OK) {
            if (file.path() < file_current) {
                files.push_back(file.path());
            }
        }
    }

    if (files.empty()) {
        if (g_config.cycle_files.val) {
            for (const auto& file : std::filesystem::directory_iterator(file_current.parent_path())) {
                if (!file.is_directory() && PathMatchSpecExW(file.path().c_str(), WIV_SUPPORTED_EXTENSIONS, PMSF_MULTIPLE) == S_OK) {
                    files.push_back(file.path());
                }
            }
        }
        else {
            return;
        }
    }
    
    // Get previous valid file.
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
    ensure(DragQueryFileW(hdrop, 0, path, MAX_PATH), != 0);
    DragFinish(hdrop);
    if (image.set_image_input(path)) {
        file_current = path;
        return true;
    }
    return false;
}

// Sends file to the recycle bin!
void File_manager::delete_file()
{
    if (!image.close())
        return;

    // SHFILEOPSTRUCTW::pFrom must be double null terminated.
    wchar_t path[MAX_PATH + 1];
    wcscpy(path, file_current.c_str());
    path[std::char_traits<wchar_t>::length(path) + 1] = '\0';

    // Configured to send file to the recycle bin.
    SHFILEOPSTRUCTW fileopenstruct = {};
    fileopenstruct.wFunc = FO_DELETE;
    fileopenstruct.pFrom = path;
    fileopenstruct.fFlags = FOF_ALLOWUNDO;

    if (SHFileOperationW(&fileopenstruct) == 0) {
        const auto deleted_file = file_current;
        file_next();
        if (file_current == deleted_file) // Do we have a next file?
            file_previous();
        if (file_current == deleted_file) { // At this point do we have a next or previous file?
            file_current.clear();
            ensure(PostMessageW(g_hwnd, WIV_WM_RESET_RESOURCES, 0, 0), != 0);
        }
        else
            ensure(PostMessageW(g_hwnd, WIV_WM_OPEN_FILE, 0, 0), != 0);
    }
}

## Compilation

### Dependencies

- Dear ImGui
- Little CMS
- LibRaw
- OpenImageIO

### Building from source on Windows

To install dependencies you can use [vcpkg](https://github.com/microsoft/vcpkg).

You can install Dear ImGui with:
`vcpkg install imgui[dx11-binding,win32-binding]:x64-windows-static --clean-after-build`
You can install Little CMS with:
`vcpkg install lcms:x64-windows-static --clean-after-build`
You can install LibRaw with:
`vcpkg install libraw[dng-lossy,openmp]:x64-windows-static --clean-after-build`
You can install OpenImageIO with:
`vcpkg install openimageio[gif,libheif,libraw,openjpeg,webp]:x64-windows-static --clean-after-build`

For building you can use the Visual Studio.
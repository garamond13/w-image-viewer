# User manual

## Controls

LMB double click - toggle fullscreen  
ALT + ENTER - toggle fullscreen  
LMB hold and drag - pan image  
RMB - open context menu  
MW scroll up or down - zoom in or out image  
CTRL + NUM+ - zoom in  
CTRL + NUM- - zoom out  
CTRL + 1 - zoom 100%  
CTRL + 0 - zoom to fit  
CTRL + O - open file  
LEFT ARROW - previous image  
RIGHT ARROW - next image  
CTRL + LEFT ARROW - rotate CCW  
CTRL + RIGHT ARROW - rotate CW  
ESC - exit fullscreen or quit app  

## Settings

You can access the settings window by right clicking anywhere to open the context menu, then chose `Settings...`.

- Currently inactive options will be dimmed out.
- You can save settings to config file by clicking `Write changes` at bottom of the settings window under `Changes` subsection. Config file `config.txt` should be automatically created in the same directory W Image Viewer .exe file is.
- There are no hard limits on kernel sizes (controlled by radius options). Very large kernels (radius > 12) may cause huge performance hit.
- Some processes require lineariztion (blur, unsharp, sigmoidize, downscale). It may be not possible to do linearization, in such cases some processes may be slightly incorrect or skipped entirely.

### Genearl

`Default window dimensions:`  
Window client area width and height in pixels.

`Enable window auto dimensions`  
Window dimensions will be set to open image dimensions. If the image dimensions are larger that screen resolution, window dimensions will be set to screen resolution * 0.9 while maintaining the same aspect ratio as the opened image.

`Background color`

`Window name`  
`Default name` sets the window name (title) to W Image Viewer, `Filename` sets window name to filename - W Image Viewer, and `Full filename` sets window name to file-path - W Image Viewer.

`Internal format`  
Sets the internal texture format. There shouldn't be a significant difference in visual quality between two options. `RGBA16F` may be significantly faster.

`Read only thumbnail in RAW image`  
Reading thumbnail should be significantly faster than processing RAW image itself. Option if enabled reads only thumbnail if it exists, if doesn't or if the option is disabled RAW image will be processed.

### Scale

`Profiles:`  
**Adding profile:** The left input field is for the lower bound of the range, and right input field is for the upper bound of the range. The upper bound shell be entered first since range will be clamped to prevent lower bound being larger than the upper bound. After that you have to click on add profile to add it to the list.

**Removing profile:** To remove a profile, select profile and click `Remove profile`.

**Making changes:** Changes you make in `Scale section` will be applied to the currently selected profile.

- `0.000000, 0.000000` is the default profile, you can't remove it.

#### Pre-scale blur (downscale only)
Uses separated (2 pass) Gaussian blur.

`Enable pre-scale blur`

`Radius`  
Blur kernel radius. The size of the kernel will be 2 * radius.

`Sigma`  
Blur spread, larger values increase the contribution of neighboring pixels.

#### Sigmoidize (upscale only)
Applies sigmoidal contrast curve. It can significantly improve upscaling results. Requires linearization.

`Enable sigmoidize`

`Contrast`  
Sets contrast of the sigmoidal curve.

`Midpoint`  
Sets midpoint of the sigmoidal curve.

#### Scale

`Use cylindrical filtering`  
Enables cylindrical filtering (Jinc based). If the option is unchecked orthogonal (2 pass) filtering will be used (Sinc based).

`Kernel function`  
Which kernel function will be used for calculation of kernel weights.

`Radius`  
Sampling kernel radius. In case of orthogonal filtering, kernel size will be 2 * radius. In case of cylindrical filtering, kernel size will be (2 * radius)^2.

`Parameter 1` and `Parameter 2`  
Some kernel functions take extra parameters, they are set here.

`Anti-ringing`   
Sets antiringing strenght.

#### Post-scale unsharp mask
Separated unsharp mask. It uses Gaussian blur to achieve sharpening.

`Enable post-scale unsharp mask`

`Radius`  
Blur kernel radius. The size of the kernel will be 2 * radius.

`Sigma`  
Blur spread, larger values increase the contribution of neighboring pixels.

`Amount`  
Amount (strenght) of sharpening.

### Color managment

`Enable color managment system`

`Display profile`  
Sets display color profile. `Auto` automatically picks installed color profile, `sRGB` uses internal sRGB profile as if its display profile, `AdobeRGB` uses internal AdobeRGB profile as if its display profile, and `Custom` enables the use of custom profile.

`Custom path`  
If you set `Display profile` to `Custom` here you can provide file path to custom profile (example C:\New Folder\display profile.icc).

`Rendering intent`

`Enable black point compensation`

#### Unatagged images

`Default to sRGB`  
Treat images without any embedded color profiles or color tags as sRGB.

### Transparency

`Tile size`

`First tile color`

`Second tile color`

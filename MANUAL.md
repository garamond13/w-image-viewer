# User manual

## Controls

**LMB double click** - toggle fullscreen  
**F11** - toggle fullscreen  
**ALT + ENTER** - toggle fullscreen  
**LMB hold and drag** - pan image  
**RMB** - open context menu  
**MW scroll up or down** - zoom in or out image  
**CTRL + NUM+** - zoom in  
**CTRL + NUM-** - zoom out  
**CTRL + 1** - zoom 100%  
**CTRL + 0** - zoom to fit  
**CTRL + O** - open file  
**LEFT ARROW** - previous image  
**RIGHT ARROW** - next image  
**CTRL + LEFT ARROW** - rotate CCW  
**CTRL + RIGHT ARROW** - rotate CW  
**ESC** - exit fullscreen or quit app  

## Settings

You can access the settings window by right clicking anywhere inside the application window to open the context menu, then chose `Settings...`.

- Currently inactive options will be dimmed out.
- You can save settings to config file by clicking `Write changes` at bottom of the settings window under `Changes` subsection. Config file `config.txt` should be automatically created in the same directory W Image Viewer .exe file is.
- Some changes will be applied only after application restart.

### Genearl

`Default window dimensions:`  
Window client area width and height in pixels.

`Use current dimensions`  
Set current window dimensions as the default window dimensions. 

`Enable window auto dimensions`  
Window dimensions will be set to the opened image dimensions. If the image dimensions are larger than the screen resolution, window dimensions will be set to 90% of the screen resolution while maintaining the same aspect ratio as the opened image.


`Window name`  
`Default name` sets the window name (title) to W Image Viewer, `Filename` sets window name to filename - W Image Viewer, and `Full filename` sets window name to file-path - W Image Viewer.

`Internal format`  
Sets the internal texture format. There shouldn't be a significant difference in visual quality between two options. `RGBA16F` may be significantly faster.

`Background color`  
Color of the background when no image is opened or when the opened image is not covering entire client area of the window.

`Read only thumbnail in RAW image`  
Reading thumbnail should be significantly faster than processing RAW image itself. Option if enabled reads only thumbnail if it exists, if doesn't or if the option is disabled RAW image will be processed.

### Scale

`Profiles:`  
- The left input field is for the lower bound of the scale factor range, and right input field is for the upper bound of the scale factor range. The range is right-open "[a, b)". After that you can click on `Add profile` to add it to the list of profiles, or `Edit profile` to edit currently selected profile (you can't edit the default profile `0.000000, 0.000000`).
- You can't have any overlapping profiles.
- Note that scale factor of 1.0 means no scaling, lower than 1.0 means downscaling and larger then 1.0 means upscaling. If current scaling factor is not covered by any profile the default profile `0.000000, 0.000000` will be applied.
- To remove a profile, select profile and click `Remove profile`. `0.000000, 0.000000` is the default profile, you can't remove it.
- Changes you make in `Scale section` will be applied to the currently selected profile.

#### Pre-scale blur (downscale only)
Uses separated (2 pass) Gaussian blur.

`Enable pre-scale blur`  
Enable or disable pre-scale blur.

`Radius`  
Blur kernel radius. The size of the kernel will be 2 * radius. Note that radius larger than 12 may be huge performance hit.

`Sigma`  
Blur spread, larger values increase the contribution of neighboring pixels.

#### Sigmoidize (upscale only)
Applies sigmoidal contrast curve. It can significantly improve upscaling results. Requires linearization.

`Enable sigmoidize`  
Enable or disable sigmoidization.

`Contrast`  
Sets contrast of the sigmoidal curve.

`Midpoint`  
Sets midpoint of the sigmoidal curve.

#### Scale

`Use cylindrical filtering (Jinc based)`  
Enables cylindrical filtering (Jinc based). If the option is unchecked orthogonal (2 pass) filtering will be used (Sinc based).

`Kernel function`  
Which kernel function will be used for calculation of kernel weights.

`Lanczos`  
Sinc windowed sinc. If `Use cylindrical filtering (Jinc based)` is checked it will be jinc windowed jinc.

`Ginseng`  
Jinc windowed sinc. If `Use cylindrical filtering (Jinc based)` is checked it will be sinc widnowed jinc.

`Hamming`  
Hamming windowed sinc or jinc.

`Power of cosine`  
Power of cosine windowed sinc or jinc.  
`Parameter 1` = n (affects window).  
Has to be satisfied: n >= 0.  
If n = 0: Box window.  
If n = 1: Cosine window.  
If n = 2: Hann window.  

`Kaiser`  
Kaiser windowed sinc or jinc.  
`Parameter 1` = beta (affects window).

`Power of Garamond`  
Power of Garamond windowed sinc or jinc.  
`Parameter 1` = n (affects window).  
`Parameter 2` = m (affects window).  
Has to be satisfied: n >= 0, m >= 0.  
If n == 0, should explicitly return 1 (Box window).  
If n = 1.0, m = 1.0: Linear window.  
If n = 2.0, m = 1.0: Welch window.  
If n -> inf, m <= 1.0: Box window.  
If m = 0.0: Box window.  
If m = 1.0: Garamond window.  

`Power of Blackman`  
Power of Blackman windowed sinc or jinc.  
`Parameter 1` = a or alpha (affects window).  
`Parameter 2` = n (affects window).  
Has to be satisfied: n >= 0.  
Has to be satisfied: if n != 1, a <= 0.16.  
If a = 0.0, n = 1.0: Hann window.  
If a = 0.0, n = 0.5: Cosine window.  
If n = 1.0: Blackman window.  
If n = 0.0: Box window.  

`GNW`    
Generalized Normal Window (GNW) windowed sinc or jinc.  
`Parameter 1` = s (affects window).  
`Parameter 2` = n (affects window).  
Has to be satisfied: s != 0, n >= 0.  
If n = 2: Gaussian window.  
If n -> inf: Box window.  

`Said`  
`Parameter 1` = eta (affects window).  
`Parameter 2` = chi (affects window). 
Has to be satisfied: eta != 2.  

`Nearest neighbor`  
Kernel function.  
Note that different function will be used if `Use cylindrical filtering (Jinc based)` is checked.  

`Linear`  
Kernel function.  
Note that different function will be used if `Use cylindrical filtering (Jinc based)` is checked.

`Bicubic`
Kernel function.  
`Parameter 1` = a.

`Modified FSR`  
Kernel function.  
`Parameter 1` = b.  
`Parameter 2` = c.  
Has to be satisfied: b != 0, b != 2, c != 0.  
If c = 1.0: FSR kernel.  

`BC-Spline`  
Kernel function.  
`Parameter 1` = B.  
`Parameter 2` = C.  
Keys kernels: B + 2C = 1.  
If B = 1.0, C = 0.0: Spline kernel.  
If B = 0.0, C = 0.0: Hermite kernel.  
If B = 0.0, C = 0.5: Catmull-Rom kernel.  
If B = 1 / 3, C = 1 / 3: Mitchell kernel.  
If B = 12 / (19 + 9 * sqrt(2)), C = 113 / (58 + 216 * sqrt(2)): Robidoux kernel.  
If B = 6 / (13 + 7 * sqrt(2)), C = 7 / (2 + 12 * sqrt(2)): RobidouxSharp kernel.  
If B = (9 - 3 * sqrt(2)) / 7, C = 0.1601886205085204: RobidouxSoft kernel.  

`Radius`  
Sampling kernel radius. In case of orthogonal filtering, kernel size will be 2 * radius. In case of cylindrical filtering, kernel size will be (2 * radius)^2. Note that radius larger than 12 may be huge performance hit.

`Blur`  
Kernel blur. Controls the width of the main lobe, value of 1 is neutral (no effect), values larger than one will increse the main lobe width (effectively it will blur the image), and values lower than 1 will decrease the main lobe width (effectively it will sharpen the image). Has to be not equal to 0.

`Parameter 1` and `Parameter 2`  
Some kernel functions take extra parameters, they are set here.

`Anti-ringing`   
Sets antiringing strenght.

#### Post-scale unsharp mask
Separated unsharp mask (2 pass). It uses Gaussian blur to achieve sharpening.

`Enable post-scale unsharp mask`  
Enable or disable post-scale unsharp mask.

`Radius`  
Blur kernel radius. The size of the kernel will be 2 * radius. Note that radius larger than 12 may be huge performance hit.

`Sigma`  
Blur spread, larger values increase the contribution of neighboring pixels.

`Amount`  
Amount (strenght) of sharpening.

### Color management

`Enable color management`  
Enable or disable color managment.

`Display profile`  
Sets display color profile. `Auto` automatically picks installed color profile, `sRGB` uses internal sRGB profile as if its display profile, `AdobeRGB` uses internal AdobeRGB profile as if its display profile, and `Custom` enables the use of custom profile.

`Custom...`  
If you set `Display profile` to `Custom` here you can provide file path to custom profile (example C:\New Folder\display profile.icc). Click on the `Custom...` button to bring up file open dialog.

`Rendering intent`  
Which rendering intent to use.

`Enable black point compensation`  
Enable or disable black point compensation.

`LUT size`  
What lut size will be used to apply color transfomations.

#### Color tags

`Linear tagged images default to ACEScg`  
Treat images with linear tags as ACEScg. If unchecked, images with linear tags will be treated as linear sRGB.

`Untagged images Default to sRGB`  
Treat images without any embedded color profiles or color tags as sRGB.

### Transparency

`Tile size`  
The size of the tiles.

`First tile color`  
Color of the first tile.

`Second tile color`  
Color of the second tile.

### Overlay

`Show overlay on start`
If enabled overlay will shown from application start, otherwise it will be hidden.

`Overlay position`
Overlay's postion inside the application window.

`Show:`
Select what do you want to be shown by overlay.
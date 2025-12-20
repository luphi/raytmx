# Smooth camera example

This program demonstrates smooth camera panning without artifacts like gaps between rows or columns. The map is drawn to
a render texture at its native resolution and then drawn as a texture with the screen's full resolution in combination
with the camera. The camera retains floating-point accuracy.

Note that the choice to render the map at its low, native resolution causes the text in the map to also be drawn at an
unreasonably low resolution, as seen in the bottom-right corner of the example TMX. In real use, maps with text should
use layers to separate high-resolution text from low-resolution tiles and should be drawn separately.

![smooth_camera.gif](smooth_camera.gif)
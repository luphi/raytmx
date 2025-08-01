# raytmx

Header-only raylib library for loading and drawing Tiled's TMX tilemap documents.

![example/raytmx-example.gif](example/raytmx-example.gif)


## Features

- Portable C99, tested with GCC (Windows and Linux), Clang (macOS), and MSVC
- Supports external tilesets and object templates
- Supports animations
- Supports ZLIB and GZIP compression for tile layer data
- Supports collision checks with Tile Collision Editor objects
- Supports collision checks with object groups
- Supports parallaxed scrolling of layers when a Camera2D is used
- Supports unencoded tile layer data and Base64- and CSV-encoded data
- Supports tile flipping flags and applies correct transforms
- Supports single-image and collection of images tilesets
- Supports drawing of all object types: ellipse, point, polygon, polyline, text, and tile objects
- Supports word wrapping and all alignment options, including horizontal justification, of text objects


## Limitations

- Only the orthogonal orientation is supported; isometric, staggered, and hexagonal are not
- JSON, which can optionally be used by Tiled, is not currently implemented
- ZStandard-compressed layer data decompression is not implemented
- Ellipses are currently treated as rectangles when doing collision checks
- Wangsets are not implemented
- Infinite maps are not supported and are treated as fixed-size
- Object rotations are parsed but currently ignored when drawing
- Tile object alignment (i.e. placement of tiles when used as objects) is parsed but currently ignored when drawing
- Text drawing is limited to raylib's default font although the desired font is available as a string
- Text drawing does not support bold, italics, underline, or strikeout styling
- Polygon objects currently may fail to draw if their vertices are not defined in counter-clockwise order
- Concave polygon objects may not be drawn correctly due to drawing with fan triangulation from the centroid
- Image transparency colors are parsed but their use is not implemented
- Nested `<properties>` are not supported; they are merged into a single list of properties


## Usage

Define the implementation before including *raytmx*.
``` c
#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"
```
As usual with header-only libraries, the implementation's definition can be limited to just a single file. This will depend on your specific build configuration.

Loading and unloading follows raylib's patterns.
```c
TmxMap* LoadTMX(const char* fileName);
void UnloadTMX(TmxMap* map);
```

Drawing also follows raylib's patterns.
```c
void DrawTMX(const TmxMap* map, const Camera2D* camera, int posX, int posY, Color tint);
void DrawTMXLayers(const TmxMap* map, const Camera2D* camera, const TmxLayer* layers, uint32_t layersLength, int posX,
    int posY, Color tint);
```

Animating a TMX is done by calling a specific function once per frame.
```c
void AnimateTMX(TmxMap* map);
```

Collision checks also follow raylib's patterns.
```c
bool CheckCollisionTMXObjects(TmxObject object1, TmxObject object2);
bool CheckCollisionTMXTileLayersRec(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength, Rectangle rec,
    TmxObject* outputObject);
bool CheckCollisionTMXTileLayersCircle(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength, Vector2 center,
    float radius, TmxObject* outputObject);
bool CheckCollisionTMXTileLayersPoint(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength, Vector2 point,
    TmxObject* outputObject);
bool CheckCollisionTMXLayersPoly(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength, Vector2* points,
    int pointCount, TmxObject* outputObject);
bool CheckCollisionTMXLayersPolyEx(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength, Vector2* points,
    int pointCount, Rectangle aabb, TmxObject* outputObject);
bool CheckCollisionTMXObjectGroupRec(TmxObjectGroup group, Rectangle rec, TmxObject* outputObject);
bool CheckCollisionTMXObjectGroupCircle(TmxObjectGroup group, Vector2 center, float radius, TmxObject* outputObject);
bool CheckCollisionTMXObjectGroupPoint(TmxObjectGroup group, Vector2 point, TmxObject* outputObject);
bool CheckCollisionTMXObjectGroupPoly(TmxObjectGroup group, Vector2* points, int pointCount, TmxObject* outputObject);
bool CheckCollisionTMXObjectGroupPolyEx(TmxObjectGroup group, Vector2* points, int pointCount, Rectangle aabb,
    TmxObject* outputObject);
```
Although raytmx doesn't do anything that would be considered collision response, the objects collided with are provided
as optional output variables, *outputObject*, to allow for it.

An example program that uses all of the above features is included.

A more minimal example program would look like:
```c
#include <stddef.h> /* NULL */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS */

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

int main(int argc, char **argv) {
    /* This map makes use of many TMX features making it useful for demonstrations. It will be adjacent to the */
    /* executable once built. */
    const char* tmx = "maps/raytmx-example.tmx";

    /* Configure the window with a resolution and title. This example will also target 60 frames per second. */
    const int screenWidth = 1024, screenHeight = 768;
    const float panSpeed = 150.0f;
    InitWindow(screenWidth, screenHeight, "raytmx example");
    SetTargetFPS(60);

    /* Load the map. If loading fails, NULL will be returned and details will be TraceLog()'d. */
    TmxMap* map = LoadTMX(tmx);
    if (map == NULL) {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", tmx);
        return EXIT_FAILURE;
    }

    /* Create a camera for efficient panning and zooming. The initial target will be the center of the map. */
    Camera2D camera;
    camera.zoom = 6.0f;
    camera.target.x = (float)(map->width * map->tileWidth) / 2.0f;
    camera.target.y = (float)(map->height * map->tileHeight) / 2.0f;
    camera.offset.x = (float)screenWidth / 2.0f;
    camera.offset.y = (float)screenHeight / 2.0f;
    camera.rotation = 0.0f;

    while (WindowShouldClose() == false) {
        if (IsKeyDown(KEY_RIGHT))
            camera.target.x += panSpeed * GetFrameTime();
        if (IsKeyDown(KEY_LEFT))
            camera.target.x -= panSpeed * GetFrameTime();
        if (IsKeyDown(KEY_DOWN))
            camera.target.y += panSpeed * GetFrameTime();
        if (IsKeyDown(KEY_UP))
            camera.target.y -= panSpeed * GetFrameTime();
        if (IsKeyDown(KEY_KP_ADD))
            camera.zoom += camera.zoom < 10.0f ? 0.25f : 0.0f;
        if (IsKeyDown(KEY_KP_SUBTRACT))
            camera.zoom -= camera.zoom > 1.0f ? 0.25f : 0.0f;

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginMode2D(camera);
            {
                AnimateTMX(map);
                DrawTMX(map, &camera, 0, 0, WHITE);
            }
            EndMode2D();
            DrawFPS(10, 10);
        }
        EndDrawing();
    }

    UnloadTMX(map);
    CloseWindow();

    return EXIT_SUCCESS;
}

```


## Dependency

*raytmx* depends on [hoxml](https://github.com/luphi/hoxml) for XML parsing and raylib for its graphical, file system, and time utilities.

# raytmx

Header-only raylib library for loading and drawing Tiled's TMX tilemap documents.

![examples/basic/basic.gif](examples/basic/basic.gif)

![examples/collisions/collisions.gif](examples/collisions/collisions.gif)

![examples/smooth_camera/smooth_camera.gif](examples/smooth_camera/smooth_camera.gif)

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
Only do this in one file. In other source files, include the header without defining the implemntation.

Loading and unloading follows raylib's patterns.
```c
TmxMap* LoadTMX(const char *fileName);
void UnloadTMX(TmxMap *map);
```

Drawing also follows raylib's patterns.
```c
void DrawTMX(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport, int posX, int posY, Color tint);
void DrawTMXLayers(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport, const TmxLayer *layers,
    uint32_t layersLength, int posX, int posY, Color tint);
```

Animating a TMX is done by calling a specific function once per frame.
```c
void AnimateTMX(TmxMap *map);
```

Collision checks also follow raylib's patterns.
```c
bool CheckCollisionTMXObjects(TmxObject object1, TmxObject object2);
bool CheckCollisionTMXTileLayersRec(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength, Rectangle rec,
    TmxObject *outputObject);
bool CheckCollisionTMXTileLayersCircle(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength, Vector2 center,
    float radius, TmxObject *outputObject);
bool CheckCollisionTMXTileLayersPoint(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength, Vector2 point,
    TmxObject *outputObject);
bool CheckCollisionTMXTileLayersPolyPoly(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 *points, int pointCount, TmxObject *outputObject);
bool CheckCollisionTMXTileLayersPolyPolyEx(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 *points, int pointCount, Rectangle aabb, TmxObject *outputObject);
bool CheckCollisionTMXObjectGroupRec(TmxObjectGroup group, Rectangle rec, TmxObject *outputObject);
bool CheckCollisionTMXObjectGroupCircle(TmxObjectGroup group, Vector2 center, float radius, TmxObject *outputObject);
bool CheckCollisionTMXObjectGroupPoint(TmxObjectGroup group, Vector2 point, TmxObject *outputObject);
bool CheckCollisionTMXObjectGroupPoly(TmxObjectGroup group, Vector2 *points, int pointCount, TmxObject *outputObject);
bool CheckCollisionTMXObjectGroupPolyEx(TmxObjectGroup group, Vector2 *points, int pointCount, Rectangle aabb,
    TmxObject *outputObject);
```
Although raytmx doesn't do anything that would be considered collision response, the objects collided with are provided
as optional output variables, *outputObject*, to allow for it.

Example programs that use all of the above features are included.

A more minimal example program would look like:
```c
#include <stddef.h> // Required for: NULL.
#include <stdlib.h> // Required for: EXIT_FAILURE, EXIT_SUCCESS.

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

int main(void)
{
    // Configure the window with a resolution and title. This example will also target 60 frames per second.
    const int screenWidth = 1024;
    const int screenHeight = 768;
    const float panSpeed = 150.0f; // Pixels per second.
    InitWindow(screenWidth, screenHeight, "raytmx example");
    SetTargetFPS(60);

    // Load the map from disk. If loading fails, NULL will be returned and details will be TraceLog()'d.
    TmxMap *map = LoadTMX("example.tmx");
    if (map == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load TMX \"example.tmx\"");
        CloseWindow();
        return EXIT_FAILURE;
    }

    // Create a camera that initially looks at the center of the map.
    Camera2D camera = { 0 };
    camera.target = (Vector2){ (float)(map->width*map->tileWidth)/2.0f, (float)(map->height*map->tileHeight)/2.0f };
    camera.offset = (Vector2){ (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 6.0f;

    while (!WindowShouldClose())
    {
        // Pan the camera based on which arrow key, if any, is pressed.
        if (IsKeyDown(KEY_RIGHT)) camera.target.x += panSpeed*GetFrameTime();
        if (IsKeyDown(KEY_LEFT)) camera.target.x -= panSpeed*GetFrameTime();
        if (IsKeyDown(KEY_DOWN)) camera.target.y += panSpeed*GetFrameTime();
        if (IsKeyDown(KEY_UP)) camera.target.y -= panSpeed*GetFrameTime();

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginMode2D(camera);
            {
                AnimateTMX(map);
                DrawTMX(map, &camera, NULL, 0, 0, WHITE);
            }
            EndMode2D();
        }
        EndDrawing();
    }

    UnloadTMX(map);
    CloseWindow();

    return EXIT_SUCCESS;
}
```


## Dependency

*raytmx* depends on [hoxml](https://github.com/luphi/hoxml) for XML parsing and raylib for its graphical, file system, 
and time utilities.

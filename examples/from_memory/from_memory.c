#include <stddef.h> // Required for: NULL.
#include <stdlib.h> // Required for: EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // Required for: strcmp().

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

#include "../assets/assets.h"

#define SPECIFY_WORKING_DIRECTORY false

static const char *GetFileText(const char *fileName)
{
    if (strcmp(fileName, "door.tx") == 0) return doorTxContent;
    else if (strcmp(fileName, "tilesets/shadows.tsx") == 0) return shadowsTsxContent;
    else if (strcmp(fileName, "tilesets/trees.tsx") == 0) return treesTsxContent;
    else return "";
}

static Texture2D LoadTexture2(const char* fileName)
{
    const unsigned char *imageData = NULL;
    int dataLength = 0;
    if (strcmp(fileName, "tilesets/fade.png") == 0)
    {
        imageData = fadePngData;
        dataLength = fadePngLength;
    }
    else if (strcmp(fileName, "tilesets/grass.png") == 0)
    {
        imageData = grassPngData;
        dataLength = grassPngLength;
    }
    else if (strcmp(fileName, "tilesets/ruins.png") == 0)
    {
        imageData = ruinsPngData;
        dataLength = ruinsPngLength;
    }
    else if (strcmp(fileName, "shadows.png") == 0)
    {
        imageData = shadowsPngData;
        dataLength = shadowsPngLength;
    }
    else if (strcmp(fileName, "trees.png") == 0)
    {
        imageData = treesPngData;
        dataLength = treesPngLength;
    }

    Texture2D texture = { ZERO_INIT };
    if ((imageData != NULL) && (dataLength > 0))
    {
        Image image = LoadImageFromMemory(".png", imageData, dataLength);
        texture = LoadTextureFromImage(image);
        UnloadImage(image);
    }

    return texture;
}

int main(void)
{
    // Configure the window with a resolution and title. This example will also target 60 frames per second.
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "raytmx load from memory example");
    SetTargetFPS(60);

    // Load the map from memory. If loading fails, NULL will be returned and details will be TraceLog()'d.
#if SPECIFY_WORKING_DIRECTORY
    // Borrow a couple functions from raytmx's private implementation to get the path for the assets directory.
#if defined _MSC_VER
    // The Visual Studio solution outputs the EXE a couple directories deep, like from_memory/x64/Debug. We need to go
    // up a few directories to the one with all example projects.
    char configurationDirectory[256] = { 0 }, platformDirectory[256] = { 0 }, projectDirectory[256] = { 0 };
    StringCopy(configurationDirectory, GetApplicationDirectory()); // e.g. "from_memory\x64\Debug"
    StringCopy(platformDirectory, GetPrevDirectoryPath2(configurationDirectory)); // e.g. "from_memory\x64"
    StringCopy(projectDirectory, GetPrevDirectoryPath2(platformDirectory)); // e.g. "from_memory"
    const char* examplesDirectory = GetPrevDirectoryPath2(projectDirectory);
#else
    // The Makefile outputs the executable adjacent to the source file. The examples directory is just up one.
    const char *examplesDirectory = GetPrevDirectoryPath2(GetApplicationDirectory());
#endif
    const char *assetsDirectory = JoinPath(examplesDirectory, "assets");
    TraceLog(LOG_INFO, "Loading in-memory TMX with assets directory \"%s\"", assetsDirectory);
    TmxMap *map = LoadTMXFromMemory(mapContent, assetsDirectory);
#else
    // Override some file-related functions. With this, raytmx will use the functions above when getting the content of
    // TSX/TX files and when loading textures. These functions use in-memory resources instead of those on disk.
    SetGetFileTextCallbackTMX(GetFileText);
    SetLoadTextureCallbackTMX(LoadTexture2);
    TraceLog(LOG_INFO, "Loading in-memory TMX with in-memory tilesets, templates, and textures");
    TmxMap *map = LoadTMXFromMemory(mapContent, NULL);
#endif
    if (map == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load TMX from memory");
        CloseWindow();
        return EXIT_FAILURE;
    }

    // Calculate how fast the camera will move relative to the size of a tile
    const float panVelocity = 10.0f*map->tileWidth; // 10 tiles per second.

    // Create a camera. Cameras use matrices to efficiently look at select parts of the map/world.
    Camera2D camera = { ZERO_INIT };
    camera.offset.x = (float)screenWidth/2.0f;
    camera.offset.y = (float)screenHeight/2.0f;
    camera.target.x = (float)(map->width*map->tileWidth)/2.0f;
    camera.target.y = (float)(map->height*map->tileHeight)/2.0f;
    camera.rotation = 0.0f;
    camera.zoom = 6.0f;

    while (!WindowShouldClose())
    {
        // If one or more arrow key is pressed.
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP))
        {
            // Create a velocity vector that will be the net sum of the arrow keys.
            Vector2 velocity = { 0.0f, 0.0f };
            if (IsKeyDown(KEY_RIGHT)) velocity.x += 1.0f;
            if (IsKeyDown(KEY_LEFT)) velocity.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN)) velocity.y += 1.0f;
            if (IsKeyDown(KEY_UP)) velocity.y -= 1.0f;

            // Change the camera's 'target' so it looks at some other point.
            camera.target.x += velocity.x*panVelocity*GetFrameTime();
            camera.target.y += velocity.y*panVelocity*GetFrameTime();
        }

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginMode2D(camera);
            {
                // Update animated tiles to new frames if enough time has passed.
                AnimateTMX(map);
                // Draw all layers of the map. The camera is passed to enable parallax scrolling.
                DrawTMX(map, &camera, NULL, 0, 0, WHITE);
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

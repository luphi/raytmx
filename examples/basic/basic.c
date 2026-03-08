#include <stddef.h> // Included for NULL.
#include <stdlib.h> // Included for EXIT_FAILURE and EXIT_SUCCESS.

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

int main(void)
{
    // This map makes use of many TMX features making it useful for demonstrations.
    const char *fileName = "../assets/example.tmx";

    // Configure the window with a resolution and title. This example will also target 60 frames per second.
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "raytmx basic example");
    SetTargetFPS(60);

    // Load the map from disk. If loading fails, NULL will be returned and details will be TraceLog()'d.
    TmxMap *map = LoadTMX(fileName);
    if (map == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", fileName);
        CloseWindow();
        return EXIT_FAILURE;
    }

    // Calculate how fast the camera will move relative to the size of a tile
    const float panVelocity = 10.0f*map->tileWidth; // 10 tiles per second.

    // Create a camera. Cameras use matrices to efficiently look at select parts of the map/world.
    Camera2D camera = { 0 };
    camera.offset = (Vector2){ (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    camera.target = (Vector2){ (float)(map->width*map->tileWidth)/2.0f, (float)(map->height*map->tileHeight)/2.0f };
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

#include <stddef.h> /* NULL */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS */

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

int main(int argc, char **argv) {
    /* This map makes use of many TMX features making it useful for demonstrations. */
    const char* tmx = "../assets/example.tmx";

    /* Configure the window with a resolution and title. This example will also target 60 frames per second. */
    const int screenWidthInPixels = 1024, screenHeightInPixels = 768;
    InitWindow(screenWidthInPixels, screenHeightInPixels, "raytmx basic example");
    SetTargetFPS(60);

    /* Load the map. If loading fails, NULL will be returned and details will be TraceLog()'d. */
    TmxMap* map = LoadTMX(tmx);
    if (map == NULL) {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", tmx);
        CloseWindow();
        return EXIT_FAILURE;
    }

    /* Calculate how fast the camera will move relative to the size of a tile */
    const float panVelocityInPixelsPerSecond = 10.0f * map->tileWidth; /* 10 tiles per second */

    /* Create a camera. Cameras use matrices to efficiently look at select parts of the map/world. */
    Camera2D camera;
    camera.offset.x = (float)screenWidthInPixels / 2.0f;
    camera.offset.y = (float)screenHeightInPixels / 2.0f;
    camera.target.x = (float)(map->width * map->tileWidth) / 2.0f;
    camera.target.y = (float)(map->height * map->tileHeight) / 2.0f;
    camera.rotation = 0.0f;
    camera.zoom = 6.0f;

    while (WindowShouldClose() == false) {
        /* If one or more arrow key is pressed. */
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP)) {
            /* Create a velocity vector that will be the net sum of the arrow keys. */
            Vector2 velocity = { .x = 0.0f, .y = 0.0f };
            if (IsKeyDown(KEY_RIGHT)) velocity.x += 1.0f;
            if (IsKeyDown(KEY_LEFT))  velocity.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN))  velocity.y += 1.0f;
            if (IsKeyDown(KEY_UP))    velocity.y -= 1.0f;

            /* Change the camera's 'target' so it looks at some other point. */
            camera.target.x += velocity.x * panVelocityInPixelsPerSecond * GetFrameTime();
            camera.target.y += velocity.y * panVelocityInPixelsPerSecond * GetFrameTime();
        }

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginMode2D(camera);
            {
                /* Progress the animated tiles. */
                AnimateTMX(map);
                /* Draw the the map (because no camera nor viewport is passed). */
                DrawTMX(map, NULL, NULL, 0, 0, WHITE);
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

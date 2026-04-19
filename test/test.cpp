#include <cstddef> // Required for: NULL.
#include <cstdlib> // Required for: atoi(), EXIT_FAILURE, EXIT_SUCCESS.

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

int main(int argc, char **argv)
{
    const char* tests[] =
    {
        "assets/collection_of_images_tileset.tmx", // Index 0.
        "assets/desert.tmx",                       // Index 1.
        "assets/gameart2d-desert.tmx",             // Index 2.
        "assets/gameart2d-desert_grouped.tmx",     // Index 3.
        "assets/image_layers.tmx",                 // Index 4.
        "assets/jb-32.tmx",                        // Index 5.
        "assets/jb-32_grouped.tmx",                // Index 6.
        "assets/level25.tmx",                      // Index 7.
        "assets/level25_unencoded.tmx",            // Index 8.
        "assets/MagicLand.tmx",                    // Index 9.
        "assets/MagicLand_uncompressed.tmx",       // Index 10.
        "assets/text.tmx",                         // Index 11.
        "assets/collision_tiles.tmx"               // Index 12.
    };

    if (argc < 2)
    {
        TraceLog(LOG_INFO, "Index number expected");
        return EXIT_FAILURE;
    }

    const int index = atoi(argv[1]);
    if ((index < 0) || (index > 12))
    {
        TraceLog(LOG_INFO, "%d is out-of-bounds", index);
        return EXIT_FAILURE;
    }

    const char* fileName = tests[index];
    const int screenWidth = 1600;
    const int screenHeight = 900;
    InitWindow(screenWidth, screenHeight, "raytmx test");
    SetTargetFPS(60);

    TmxMap *map = LoadTMX(fileName);
    if (map == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", fileName);
        CloseWindow();
        return EXIT_FAILURE;
    }

    const Color clearColor = { 137, 137, 137, 255 }; // The same grey Tiled uses.
    const int mapWidth = (int)(map->width*map->tileWidth); // Map's width in pixels.
    const int mapHeight = (int)(map->height*map->tileHeight); // Map's height in pixels.

    SetTraceLogFlagsTMX(LOG_SKIP_TILES | LOG_SKIP_OBJECTS);
    TraceLog(LOG_INFO, "----------------------------------");
    TraceLogTMX(LOG_INFO, map);
    TraceLog(LOG_INFO, "----------------------------------");

    const float panVelocity = 25.0f*map->tileWidth; // 25 tiles per second.
    const Vector2 offset = { (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    const Vector2 target = { (float)(map->width*map->tileWidth)/2.0f, (float)(map->height*map->tileHeight)/2.0f };
    Camera2D camera = { offset, target, 0.0f, 2.0f };
    const bool hasCollisions = index == 12; // Enable or disable collision checks. Unique to one test map.

    while (!WindowShouldClose())
    {
        if (IsKeyPressed(KEY_KP_ADD)) camera.zoom += (camera.zoom < 10.0f)? 1.0f : 0.0f;
        if (IsKeyPressed(KEY_KP_SUBTRACT)) camera.zoom -= (camera.zoom > 1.0f)? 1.0f : 0.0f;
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP))
        {
            Vector2 velocity = { 0.0f, 0.0f };
            if (IsKeyDown(KEY_RIGHT)) velocity.x += 1.0f;
            if (IsKeyDown(KEY_LEFT)) velocity.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN)) velocity.y += 1.0f;
            if (IsKeyDown(KEY_UP)) velocity.y -= 1.0f;
            camera.target.x += velocity.x*panVelocity*GetFrameTime();
            camera.target.y += velocity.y*panVelocity*GetFrameTime();
        }

        BeginDrawing();
        {
            ClearBackground(clearColor);
            BeginMode2D(camera);
            {
                AnimateTMX(map);
                DrawTMX(map, &camera, NULL, 0, 0, WHITE);
                DrawRectangleLines(0, 0, mapWidth, mapHeight, BLACK);
            }
            EndMode2D();
            DrawFPS(10, 10);

            if (hasCollisions)
            {
                const Vector2 mousePosition = GetScreenToWorld2D(GetMousePosition(), camera);
                const bool isColliding = CheckCollisionTMXTileLayersPoint(map, map->layers, 1, mousePosition, NULL);
                if (isColliding) DrawText("COLLIDING", 10, 40, 20, LIME);
                else DrawText("NOT COLLIDING", 10, 40, 20, RED);
            }
        }
        EndDrawing();
    }

    UnloadTMX(map);
    CloseWindow();

    return EXIT_SUCCESS;
}

#include <math.h> // Included for atan2f(), cosf(), roundf(), and sinf().
#include <stddef.h> // Included for NULL.
#include <stdlib.h> // Included for EXIT_FAILURE and EXIT_SUCCESS.

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

Rectangle GetCameraViewport(Camera2D camera, float mapWidth, float mapHeight)
{
    Rectangle viewport = { 0 };
    viewport.width = mapWidth/camera.zoom;
    viewport.height = mapHeight/camera.zoom;
    viewport.x = camera.target.x - (viewport.width/2.0f);
    viewport.y = camera.target.y - (viewport.height/2.0f);
    return viewport;
}

int main(void)
{
    // This map makes use of many TMX features making it useful for demonstrations.
    const char *fileName = "../assets/example.tmx";

    // Configure the window with a resolution and title. This example will also target 60 frames per second.
    const int mapWidth = 36;
    const int mapHeight = 52;
    const int tileWidth = 16;
    const int tileHeight = 16;
    const int screenWidth = mapWidth*tileWidth;
    const int screenHeight = mapHeight*tileHeight;
    const int panSpeed = 10*tileWidth; // 10 tiles per second.
    InitWindow(screenWidth, screenHeight, "raytmx smooth camera example");
    SetTargetFPS(60);

    // Load the map. If loading fails, NULL will be returned and details will be TraceLog()'d.
    TmxMap *map = LoadTMX(fileName);
    if (map == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", fileName);
        CloseWindow();
        return EXIT_FAILURE;
    }

    // Allocate a texture to draw the tilemap to each frame.
    const RenderTexture2D renderTexture = LoadRenderTexture(screenWidth, screenHeight);
    // Determine the region within the render texture to be drawn. This will draw the whole texture.
    Rectangle sourceRect = { 0.0f, 0.0f, (float)renderTexture.texture.width, (float)-renderTexture.texture.height };
    // Note: This source rectangle is Y-flipped due to raylib's relationship with OpenGL's coordinate system.

    // Create a camera. This will be a smooth-panning camera that moves with floating-point precision. It will be used
    // to pan over the render texture that the map is rendered to./
    Camera2D camera = { 0 };
    // 'offset' describes the camera's focus relative to the viewport's (0, 0) position. This means 'offset' should be
    // assigned with { <screen width / 2>, <screen height / 2> } in order for the camera's center to be in the center of
    // the screen.
    camera.offset = (Vector2){ (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    // 'target' describes the position the camera is looking at and will initially be the map's center.
    camera.target = (Vector2){ (float)screenWidth/2.0f , (float)screenHeight/2.0f };
    // 'rotation' is an angle (in degrees) the camera would be rotated and zero means no rotation.
    camera.rotation = 0.0f;
    // 'zoom' is the linear scale factor meaning a value of 6.0f would be a 6x zoom.
    camera.zoom = 6.0f;

    // Create an explicit viewport. This defines the region of the map that's visible with the current camera.
    Rectangle viewport = GetCameraViewport(camera, (float)screenWidth, (float)screenHeight);

    // Finally, create a flag that enables or disables the use of the camera. When 'true', the camera is used. When
    // 'false', the camera not used but the viewport is causing the culling to be visible.
    bool useCamera = true;

    while (!WindowShouldClose())
    {
        // If the Z key was briefly pressed once.
        if (IsKeyPressed(KEY_Z)) useCamera = !useCamera;

        // If the keypad + or keypad - key is currently pressed.
        if (IsKeyDown(KEY_KP_ADD) || IsKeyDown(KEY_KP_SUBTRACT))
        {
            // Zoom in or out if the + or - key is pressed, respectively.
            if (IsKeyDown(KEY_KP_ADD)) camera.zoom += (camera.zoom < 10.0f)? 0.25f : 0.0f;
            if (IsKeyDown(KEY_KP_SUBTRACT)) camera.zoom -= (camera.zoom > 1.0f) ? 0.25f : 0.0f;
            viewport = GetCameraViewport(camera, (float)screenWidth, (float)screenHeight);
        }

        // If one or more arrow key is currently pressed.
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP))
        {
            // Create a velocity vector that will be the net sum of the arrow keys.
            Vector2 velocity = { 0.0f, 0.0f };
            if (IsKeyDown(KEY_RIGHT)) velocity.x += 1.0f;
            if (IsKeyDown(KEY_LEFT)) velocity.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN)) velocity.y += 1.0f;
            if (IsKeyDown(KEY_UP)) velocity.y -= 1.0f;

            // Calculate the angle of the velocity vector and use it to calculate the X-axis and Y-axis compoents using
            // the camera's pan speed as its magnitude.
            const float theta = atan2f(velocity.y, velocity.x); // Angle of the vector in radians.
            camera.target.x += cosf(theta)*panSpeed*GetFrameTime();
            camera.target.y += sinf(theta)*panSpeed*GetFrameTime();
            viewport = GetCameraViewport(camera, (float)screenWidth, (float)screenHeight);
        }

        BeginDrawing();
        {
            ClearBackground(LIGHTGRAY);

            // Update animated tiles to new frames if enough time has passed.
            AnimateTMX(map);

            // Draw the map to the render texture at its native resolution.
            BeginTextureMode(renderTexture);
            {
                // Fill the render texture with a solid color different from the screen's.
                ClearBackground(BLACK);
                // Draw the region within the map that's visible in the viewport.
                DrawTMX(map, &camera, &viewport, 0, 0, WHITE);
            }
            EndTextureMode();

            // Draw the render texture with the high-precision, smooth camera (if currently enabled).
            if (useCamera) BeginMode2D(camera);

            // Draw the render texture, effectively drawing the map.
            DrawTexturePro(renderTexture.texture, sourceRect,
                (Rectangle){ 0.0, 0.0, (float)screenWidth, (float)screenHeight }, (Vector2){ 0.0f, 0.0f }, 0.0f, WHITE);

            // Draw a border around the viewport. This makes it visible if the camera is disabled.
            DrawRectangleLinesEx(viewport, 2.0f, RED);

            if (useCamera) EndMode2D();

            DrawFPS(20, 20);
        }
        EndDrawing();
    }

    UnloadRenderTexture(renderTexture);
    UnloadTMX(map);
    CloseWindow();

    return EXIT_SUCCESS;
}

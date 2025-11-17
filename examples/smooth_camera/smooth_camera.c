#include <math.h> /* atan2f(), cosf(), roundf(), sinf() */
#include <stddef.h> /* NULL */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS */

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

Rectangle GetCameraViewport(Camera2D camera, float mapWidthInPixels, float mapHeightInPixels) {
    Rectangle viewport;
    viewport.width = mapWidthInPixels / camera.zoom;
    viewport.height = mapHeightInPixels / camera.zoom;
    viewport.x = camera.target.x - (viewport.width / 2.0f);
    viewport.y = camera.target.y - (viewport.height / 2.0f);
    return viewport;
}

int main(int argc, char **argv) {
    /* This map makes use of many TMX features making it useful for demonstrations. */
    const char* tmx = "../assets/example.tmx";

    /* Configure the window with a resolution and title. This example will also target 60 frames per second. */
    const int tmxWidthInTiles = 36, tmxHeightInTiles = 52, tmxWidthOfTile = 16, tmxHeightOfTile = 16, zoom = 6;
    const int mapWidthInPixels = tmxWidthInTiles * tmxWidthOfTile;
    const int mapHeightInPixels = tmxHeightInTiles * tmxHeightOfTile;
    const int panSpeedInPixelsPerSecond = 10 * tmxWidthOfTile; /* 10 tiles per second. */
    InitWindow(mapWidthInPixels, mapHeightInPixels, "raytmx smooth camera example");
    SetTargetFPS(60);

    /* Load the map. If loading fails, NULL will be returned and details will be TraceLog()'d. */
    TmxMap* map = LoadTMX(tmx);
    if (map == NULL) {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", tmx);
        CloseWindow();
        return EXIT_FAILURE;
    }

    /* Allocate a texture to draw the tilemap to each frame. */
    const RenderTexture2D renderTexture = LoadRenderTexture(tmxWidthInTiles * tmxWidthOfTile,
        tmxHeightInTiles * tmxHeightOfTile);
    Rectangle renderSourceRect; /* Region within the render texture to be drawn (the whole thing). */
    renderSourceRect.x = 0.0f;
    renderSourceRect.y = 0.0f;
    renderSourceRect.width = (float)renderTexture.texture.width;
    renderSourceRect.height = (float)-renderTexture.texture.height;
    /* Note: This source rectangle is Y-flipped due to raylib's relationship with OpenGL's coordinate system. */

    /* Create a camera. This will be a smooth-panning camera that moves with floating-point precision. It will be */
    /* used to pan over the render texture that the map is rendered to. */
    Camera2D camera;
    /* 'offset' describes the camera's focus relative to the viewport's (0, 0) coordinate. This means 'offset' */
    /* should be assigned with { <screen width / 2>, <screen height / 2> } in order for the camera's center to be */
    /* be in the center of the screen. */
    camera.offset.x = (float)mapWidthInPixels / 2.0f;
    camera.offset.y = (float)mapHeightInPixels / 2.0f;
    /* 'target' describes the coordinate the camera is looking at and will initially be the map's center. */
    camera.target.x = (float)mapWidthInPixels / 2.0f;
    camera.target.y = (float)mapHeightInPixels / 2.0f;
    /* 'rotation' is an angle (in degrees) the camera would be rotated and zero means no rotation. */
    camera.rotation = 0.0f;
    /* 'zoom' is the linear scale factor meaning a value of 6.0f would be a 6x zoom. */
    camera.zoom = (float)zoom;

    /* Create an explicit viewport. This defines the region of the map that's visible with the current camera. */
    Rectangle viewport = GetCameraViewport(camera, (float)mapWidthInPixels, (float)mapHeightInPixels);

    /* Finally, create a flag that enables or disables the use of the camera. When `true`, the camera is used. When */
    /* `false`, the camera not used but the viewport is causing the culling to be visible. */
    bool useCamera = true;

    while (WindowShouldClose() == false) {
        /* If the Z key was briefly pressed once */
        if (IsKeyPressed(KEY_Z))
            useCamera = !useCamera;

        /* If the keypad + or keypad - key is pressed. */
        if (IsKeyDown(KEY_KP_ADD) || (IsKeyDown(KEY_KP_SUBTRACT))) {
            /* Zoom in or out if the + or - key is pressed, respectively. */
            if (IsKeyDown(KEY_KP_ADD))      camera.zoom += camera.zoom < 10.0f ? 0.25f : 0.0f;
            if (IsKeyDown(KEY_KP_SUBTRACT)) camera.zoom -= camera.zoom > 1.0f ? 0.25f : 0.0f;
            viewport = GetCameraViewport(camera, (float)mapWidthInPixels, (float)mapHeightInPixels);
        }

        /* If one or more arrow key is pressed. */
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP)) {
            /* Create a velocity vector that will be the net sum of the arrow keys. */
            Vector2 velocity = { 0.0f, 0.0f };
            if (IsKeyDown(KEY_RIGHT)) velocity.x += 1.0f;
            if (IsKeyDown(KEY_LEFT))  velocity.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN))  velocity.y += 1.0f;
            if (IsKeyDown(KEY_UP))    velocity.y -= 1.0f;

            /* Calculate the angle of the velocity vector and use it to calculate the X-axis and Y-axis compoents */
            /* using the camera's pan speed as its magnitude. */
            const float theta = atan2f(velocity.y, velocity.x); /* Angle of the vector in radians. */
            camera.target.x += cosf(theta) * panSpeedInPixelsPerSecond * GetFrameTime();
            camera.target.y += sinf(theta) * panSpeedInPixelsPerSecond * GetFrameTime();
            viewport = GetCameraViewport(camera, (float)mapWidthInPixels, (float)mapHeightInPixels);
        }

        BeginDrawing();
        {
            ClearBackground(LIGHTGRAY);

            /* Progress the animated tiles. */
            AnimateTMX(map);

            /* Draw the map to the render texture with the pixel precision camera. */
            BeginTextureMode(renderTexture);
            {
                /* Fill the render texture with a solid color different from the screen's. */
                ClearBackground(BLACK);
                /* Draw the a subset of map within the viewport. */
                DrawTMX(map, NULL, &viewport, 0, 0, WHITE);
            }
            EndTextureMode();

            /* Draw the render texture with the high-precision, smooth camera (if currently enabled). */
            if (useCamera)
                BeginMode2D(camera);
            {
                DrawTexturePro(/* texture: */ renderTexture.texture, /* source: */ renderSourceRect,
                    /* dest: */ (Rectangle) { 0.0, 0.0, (float)mapWidthInPixels, (float)mapHeightInPixels },
                    /* origin: */ (Vector2) { 0.0f, 0.0f }, /* rotation: */ 0.0f, /* tint: */ WHITE);

                DrawRectangleLinesEx(/* rec: */ viewport, /* lineThick: */ 2.0f, /* color: */ RED);
            }
            if (useCamera)
                EndMode2D();

            DrawFPS(20, 20);
        }
        EndDrawing();
    }

    UnloadRenderTexture(renderTexture);
    UnloadTMX(map);
    CloseWindow();

    return EXIT_SUCCESS;
}

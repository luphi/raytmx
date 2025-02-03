#include <math.h> /* atan2f(), cosf(), sinf() */
#include <stddef.h> /* NULL */
#include <stdlib.h> /* EXIT_FAILURE, EXIT_SUCCESS */
#include <string.h> /* strcmp() */

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

/* Collision checks can either be done with the objects associated with a tile via the Tile Collision Editor or, if */
/* this preprocessor is set to true, the object group within the TMX with the following name */
#define CHECK_COLLISION_OBJECT_GROUP false
#define COLLISION_OBJECT_GROUP_NAME "Walls"

/* The example TMX has an object group for doors, portals, etc. typical for games and within it is a spawn door. That */
/* object group and spawn object have the following names. */
#define DOORS_OBJECT_GROUP_NAME "Doors"
#define SPAWN_DOOR_NAME "spawn"

#define SCREEN_WIDTH_IN_PIXELS 1024
#define SCREEN_HEIGHT_IN_PIXELS 768
#define PLAYER_SPEED_IN_PIXELS_PER_SECOND 50.0f
#define POLYGON_POINTS_COUNT 6

Vector2 polygonCenter = {0.0f};
Vector2 polygonPoints[POLYGON_POINTS_COUNT] = {{0.0f}};
Rectangle polygonAabb = {0.0f};
float polygonRadius = 0.0f;

void CreatePolygon(Vector2 center, float radius) {
    polygonCenter = center; /* Global variable */
    polygonRadius = radius; /* Global variable */
    polygonAabb = (Rectangle) { /* Global variable */
        .x = center.x - radius,
        .y = center.y - radius,
        .width = 2.0f * radius,
        .height = 2.0f * radius
    };
    float theta = 0.0f;
    float dTheta = 2.0f * PI / (float)POLYGON_POINTS_COUNT;
    for (int i = 0; i < POLYGON_POINTS_COUNT; i++) {
        polygonPoints[i].x = center.x + (radius * cosf(theta));
        polygonPoints[i].y = center.y + (radius * sinf(theta));
        theta += dTheta;
    }
}

void TranslatePolygon(float dx, float dy) {
    /* Translate the center point */
    polygonCenter.x += dx;
    polygonCenter.y += dy;
    /* Translate each vertex */
    for (int i = 0; i < POLYGON_POINTS_COUNT; i++) {
        polygonPoints[i].x += dx;
        polygonPoints[i].y += dy;
    }
    /* Translate the AABB */
    polygonAabb.x += dx;
    polygonAabb.y += dy;
}

int main(int argc, char **argv) {
    /* This map makes use of many TMX features making it useful for demonstrations. It will be adjacent to the */
    /* executable once built. */
    const char* tmx = "maps/raytmx-example.tmx";

    /* Configure the window with a resolution and title. This example will also target 60 frames per second. */
    InitWindow(SCREEN_WIDTH_IN_PIXELS, SCREEN_HEIGHT_IN_PIXELS, "raytmx example");
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
    camera.offset.x = (float)SCREEN_WIDTH_IN_PIXELS / 2.0f;
    camera.offset.y = (float)SCREEN_HEIGHT_IN_PIXELS / 2.0f;
    camera.rotation = 0.0f;

#if CHECK_COLLISION_OBJECT_GROUP
    /* Loop through the layers to look for two things: 1) The object group that will determine where we can and */
    /* cannot go, and 2) the "spawn" door within the "Doors" object group where the player should spawn. */
    TmxObjectGroup wallsObjectGroup = {};
    for (size_t i = 0; i < map->layersLength; i++) {
        TmxLayer layer = map->layers[i];
        if (strcmp(layer.name, COLLISION_OBJECT_GROUP_NAME) == 0 && layer.type == LAYER_TYPE_OBJECT_GROUP) {
            wallsObjectGroup = layer.exact.objectGroup;
            break;
        }
    }
#endif

    /* Create a default player at the center of the map in case the expected layer doesn't exist */
    Vector2 mapCenter = {
        .x = (float)(map->width * map->tileWidth) / 2.0f,
        .y = (float)(map->height * map->tileHeight) / 2.0f
    };
    CreatePolygon(/* center: */ mapCenter, /* radius: */ (float)map->tileWidth / 3.0f);

    /* Loop through layers to look for the spawn door of the appropriate object group. The player will spawn there. */
    for (size_t i = 0; i < map->layersLength; i++) {
        TmxLayer layer = map->layers[i];
        if (strcmp(layer.name, DOORS_OBJECT_GROUP_NAME) == 0 && layer.type == LAYER_TYPE_OBJECT_GROUP) {
            TmxObjectGroup group = layer.exact.objectGroup;
            for (size_t j = 0; j < group.objectsLength; j++) {
                TmxObject object = group.objects[j];
                for (size_t k = 0; k < object.propertiesLength; k++) {
                    TmxProperty property = object.properties[k];
                    if (strcmp(property.name, "name") == 0 && property.type == PROPERTY_TYPE_STRING &&
                            strcmp(property.stringValue, SPAWN_DOOR_NAME) == 0) {
                        polygonCenter = (Vector2){(float)(object.x + (object.width / 2.0)),
                            (float)(object.y + (object.height / 2.0))};
                        CreatePolygon(/* center: */ polygonCenter, /* radius: */ polygonRadius);
                        camera.target = polygonCenter; /* Point the camera at the polygon's center */
                        break; /* Break from the innermost loop */
                    }
                }
            }
        }
    }

    while (WindowShouldClose() == false) {
        /* If one or more arrow key is pressed */
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP)) {
            /* Create a velocity vector with a magnitude equal to the player's allowed speed */
            Vector2 velocityVector = {0.0f};
            if (IsKeyDown(KEY_RIGHT))
                velocityVector.x += 1.0f;
            if (IsKeyDown(KEY_LEFT))
                velocityVector.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN))
                velocityVector.y += 1.0f;
            if (IsKeyDown(KEY_UP))
                velocityVector.y -= 1.0f;
            float theta = atan2f(velocityVector.y, velocityVector.x); /* Angle, in radians, of the velocity vector */
            velocityVector.x = cosf(theta) * PLAYER_SPEED_IN_PIXELS_PER_SECOND * GetFrameTime();
            velocityVector.y = sinf(theta) * PLAYER_SPEED_IN_PIXELS_PER_SECOND * GetFrameTime();

            /* Translate the player one axis at a time. If this movement leads the player to hit a wall, revert the */
            /* player's position for just that axis. */
            TranslatePolygon(velocityVector.x, 0.0f);
#if CHECK_COLLISION_OBJECT_GROUP
            if (CheckCollisionTMXObjectGroupPoly(wallsObjectGroup, polygonPoints, POLYGON_POINTS_COUNT, NULL))
#else
            if (CheckCollisionTMXLayersPolyEx(map, map->layers, map->layersLength, polygonPoints, POLYGON_POINTS_COUNT,
                    polygonAabb, NULL))
#endif
                TranslatePolygon(-velocityVector.x, 0.0f); /* Undo the X translation */
            TranslatePolygon(0.0f, velocityVector.y);
#if CHECK_COLLISION_OBJECT_GROUP
            if (CheckCollisionTMXObjectGroupPoly(wallsObjectGroup, polygonPoints, POLYGON_POINTS_COUNT, NULL))
#else
            if (CheckCollisionTMXLayersPolyEx(map, map->layers, map->layersLength, polygonPoints, POLYGON_POINTS_COUNT,
                    polygonAabb, NULL))
#endif
                TranslatePolygon(0.0f, -velocityVector.y); /* Undo the Y translation */
            /* Move the camera such that the player's center is in the center of the screen */
            camera.target = polygonCenter;
        }

        if (IsKeyDown(KEY_KP_ADD))
            camera.zoom += camera.zoom < 10.0f ? 0.25f : 0.0f;
        if (IsKeyDown(KEY_KP_SUBTRACT))
            camera.zoom -= camera.zoom > 1.0f ? 0.25f : 0.0f;

        BeginDrawing();
        {
            ClearBackground(BLACK);
            BeginMode2D(camera);
            {
                /* Progress the animated tiles */
                AnimateTMX(map);
                /* Draw the visible part of the map */
                DrawTMX(map, &camera, 0, 0, WHITE);
                /* Draw the "player" */
                DrawPoly(/* center: */ polygonCenter, /* sides: */ POLYGON_POINTS_COUNT, /* radius: */ polygonRadius,
                    /* rotation: */ 0.0f, /* color: */ DARKBLUE);
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

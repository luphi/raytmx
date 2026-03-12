#include <math.h> // Required for: atan2f(), cosf(), sinf().
#include <stddef.h> // Required for: NULL.
#include <stdlib.h> // Required for: EXIT_FAILURE, EXIT_SUCCESS.
#include <string.h> // Required for: strcmp().

#include "raylib.h"

#define RAYTMX_IMPLEMENTATION
#include "raytmx.h"

// Collision checks can either be done with the objects associated with a tile via the Tile Collision Editor or, if this
// preprocessor is set to true, the object group within the TMX with the following name.
#define CHECK_COLLISION_OBJECT_GROUP false
#define COLLISION_OBJECT_GROUP_NAME "Walls"

// The example TMX has an object group for doors, portals, etc. There's also a spawn door inside it. That object group
// and spawn object have the following names.
#define DOORS_OBJECT_GROUP_NAME "Doors"
#define SPAWN_DOOR_NAME "spawn"

#define PLAYER_SPEED_IN_PIXELS_PER_SECOND 50.0f
#define POLYGON_POINTS_COUNT 6

typedef struct Polygon {
    Vector2 center;
    float radius;
    Vector2 points[POLYGON_POINTS_COUNT];
    Rectangle aabb;
} Polygon;

// Get a Polygon with a given center point and radius.
static Polygon GetPolygon(Vector2 center, float radius)
{
    Polygon poly = { 0 };

    poly.center = center;
    poly.radius = radius;
    poly.aabb = (Rectangle){ center.x - radius, center.y - radius, 2.0f*radius, 2.0f*radius };

    float theta = 0.0f;
    const float dTheta = 2.0f*PI/(float)POLYGON_POINTS_COUNT;
    for (int i = 0; i < POLYGON_POINTS_COUNT; i++)
    {
        poly.points[i] = (Vector2){ center.x + (radius*cosf(theta)), center.y + (radius*sinf(theta)) };
        theta += dTheta;
    }

    return poly;
}

// Get a Polygon derived from a given one with the given deltas.
static Polygon TranslatePolygon(Polygon poly, float dx, float dy)
{
    // Translate the center point.
    poly.center.x += dx;
    poly.center.y += dy;

    // Translate each vertex the same amounts.
    for (int i = 0; i < POLYGON_POINTS_COUNT; i++)
    {
        poly.points[i].x += dx;
        poly.points[i].y += dy;
    }

    // Translate the Axis-Aligned Bounding Box (AABB).
    poly.aabb.x += dx;
    poly.aabb.y += dy;

    return poly;
}

int main(void)
{
    // This map makes use of many TMX features making it useful for demonstrations.
    const char *fileName = "../assets/example.tmx";

    // Configure the window with a resolution and title. This example will also target 60 frames per second.
    const int screenWidth = 1024;
    const int screenHeight = 768;
    InitWindow(screenWidth, screenHeight, "raytmx collisions example");
    SetTargetFPS(60);

    // Load the map. If loading fails, NULL will be returned and details will be TraceLog()'d.
    TmxMap *map = LoadTMX(fileName);
    if (map == NULL)
    {
        TraceLog(LOG_ERROR, "Failed to load TMX \"%s\"", fileName);
        CloseWindow();
        return EXIT_FAILURE;
    }

    // Calculate the map's center point. It's used to position the camera and the "player."
    const Vector2 mapCenter = { (float)(map->width*map->tileWidth)/2.0f, (float)(map->height*map->tileHeight)/2.0f };

    // Create a camera. Cameras use matrices to efficiently look at select parts of the map/world.
    Camera2D camera = { 0 };
    camera.offset = (Vector2){ (float)screenWidth/2.0f, (float)screenHeight/2.0f };
    camera.target = mapCenter;
    camera.rotation = 0.0f;
    camera.zoom = 6.0f;

#if CHECK_COLLISION_OBJECT_GROUP
    // Loop through the layers to look for two things: 1) The object group that will determine where we can and cannot
    // go, and 2) the "spawn" door within the "Doors" object group where the player should spawn.
    TmxObjectGroup wallsObjectGroup = { 0 };
    for (size_t i = 0; i < map->layersLength; i++)
    {
        TmxLayer layer = map->layers[i];

        if ((strcmp(layer.name, COLLISION_OBJECT_GROUP_NAME) == 0) && (layer.type == LAYER_TYPE_OBJECT_GROUP))
        {
            wallsObjectGroup = layer.exact.objectGroup;
            break;
        }
    }
#endif

    // Create a default "player" at the center of the map in case the expected layer doesn't exist.
    Polygon poly = GetPolygon(mapCenter, (float)map->tileWidth/3.0f);

    // Loop through layers to look for the spawn door of the appropriate object group. The player will spawn there.
    for (size_t i = 0; i < map->layersLength; i++)
    {
        const TmxLayer layer = map->layers[i];

        if ((strcmp(layer.name, DOORS_OBJECT_GROUP_NAME) == 0) && (layer.type == LAYER_TYPE_OBJECT_GROUP))
        {
            const TmxObjectGroup group = layer.exact.objectGroup;

            for (size_t j = 0; j < group.objectsLength; j++)
            {
                const TmxObject object = group.objects[j];

                for (size_t k = 0; k < object.propertiesLength; k++)
                {
                    const TmxProperty property = object.properties[k];

                    if ((strcmp(property.name, "name") == 0) && (property.type == PROPERTY_TYPE_STRING) &&
                        (strcmp(property.stringValue, SPAWN_DOOR_NAME) == 0))
                    {
                        Vector2 center = { (float)(object.x + (object.width/2.0)),
                            (float)(object.y + (object.height/2.0)) };
                        poly = GetPolygon(center, poly.radius);
                        camera.target = poly.center; // Point the camera at the polygon's center.
                        break; // Break from the innermost loop.
                    }
                }
            }
        }
    }

    while (!WindowShouldClose())
    {
        // If one or more arrow key is pressed.
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_UP))
        {
            // Create a velocity vector with a magnitude equal to the player's allowed speed.
            Vector2 velocity = { 0.0f, 0.0f };
            if (IsKeyDown(KEY_RIGHT)) velocity.x += 1.0f;
            if (IsKeyDown(KEY_LEFT)) velocity.x -= 1.0f;
            if (IsKeyDown(KEY_DOWN)) velocity.y += 1.0f;
            if (IsKeyDown(KEY_UP)) velocity.y -= 1.0f;
            const float theta = atan2f(velocity.y, velocity.x); // Angle of the vector in radians.
            velocity.x = cosf(theta)*PLAYER_SPEED_IN_PIXELS_PER_SECOND*GetFrameTime(); // X component.
            velocity.y = sinf(theta)*PLAYER_SPEED_IN_PIXELS_PER_SECOND*GetFrameTime(); // Y component.

            // Translate the player one axis at a time. If this movement leads the player to hit a wall, revert the
            // player's position for just that axis.
            poly = TranslatePolygon(poly, velocity.x, 0.0f);

#if CHECK_COLLISION_OBJECT_GROUP
            if (CheckCollisionTMXObjectGroupPoly(wallsObjectGroup, poly.points, POLYGON_POINTS_COUNT, NULL))
#else
            if (CheckCollisionTMXTileLayersPolyEx(map, map->layers, map->layersLength, poly.points,
                POLYGON_POINTS_COUNT, poly.aabb, NULL))
#endif
            {
                poly = TranslatePolygon(poly, -velocity.x, 0.0f); // Undo the X translation.
            }

            poly = TranslatePolygon(poly, 0.0f, velocity.y);
#if CHECK_COLLISION_OBJECT_GROUP
            if (CheckCollisionTMXObjectGroupPoly(wallsObjectGroup, poly.points, POLYGON_POINTS_COUNT, NULL))
#else
            if (CheckCollisionTMXTileLayersPolyEx(map, map->layers, map->layersLength, poly.points,
                POLYGON_POINTS_COUNT, poly.aabb, NULL))
#endif
            {
                poly = TranslatePolygon(poly, 0.0f, -velocity.y); // Undo the Y translation.
            }

            // Move the camera such that the player's center is in the center of the screen.
            camera.target = poly.center;
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
                // Draw the "player."
                DrawPoly(poly.center, POLYGON_POINTS_COUNT, poly.radius, 0.0f, DARKBLUE);
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

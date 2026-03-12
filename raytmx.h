/**********************************************************************************************
*   Copyright (c) 2024 Luke Philipsen
*
*   Permission to use, copy, modify, and/or distribute this software for
*   any purpose with or without fee is hereby granted.
*
*   THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
*   WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
*   OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
*   FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
*   DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
*   AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
*   OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
**********************************************************************************************/

/**********************************************************************************************
*   Usage
*
*   Do this:
*     #define RAYTMX_IMPLEMENTATION
*   before you include this file in *one* C or C++ file to create the implementation.

*   You can define RAYTMX_DEC with
*     #define RAYTMX_DEC static
*   or
*     #define RAYTMX_DEC extern
*   to specify raytmx function declarations as static or extern, respectively.
*   The default specifier is extern.
**********************************************************************************************/

#ifndef RAYTMX_H
    #define RAYTMX_H

#include <ctype.h> // Required for: isspace().
#include <math.h> // Required for: fabs(), floor(), INFINITY, roundf().
#include <stddef.h> // Required for: NULL.
#include <stdint.h> // Required for: int32_t, uint32_t.
#include <stdlib.h> // Required for: atoi(), strtoul().
#include <string.h> // Required for: memcpy(), memset(), strcpy(), strcpy_s() strlen(), strncpy(), strncpy_s().

#include "raylib.h"
#include "rlgl.h"

#ifndef RAYTMX_DEC
    #define RAYTMX_DEC
#endif // RAYTMX_DEC

#ifdef __cplusplus
    extern "C" {
#endif // __cpluspus

//----------------------------------------------------------------------------------
// Public types and functions.
//----------------------------------------------------------------------------------

// Function signature of raylib's LoadTexture() as a type. For use with SetLoadTextureTMX().
typedef Texture2D (*LoadTextureCallback)(const char *fileName);

// Bit flags passed to SetTraceLogFlagsTMX() that optionally disable the logging of specific TMX elements.
enum TmxLogFlags {
    LOG_SKIP_PROPERTIES = 1,     // Skip <properties> and child <property> elements.
    LOG_SKIP_LAYERS = 2,         // Skip <layer>, <objectgroup>, <imagelayer>, and <group> layers and children thereof.
    LOG_SKIP_TILE_LAYERS = 4,    // Skip <layer> layers and children thereof.
    LOG_SKIP_TILES = 8,          // Skip tiles (GIDs) of tile layers (<layer>s).
    LOG_SKIP_OBJECT_GROUPS = 16, // Skip <objectgroup> layers and children thereof.
    LOG_SKIP_OBJECTS = 32,       // Skip objects of object layers (<objectgroup>s).
    LOG_SKIP_IMAGE_LAYERS = 64,  // Skip <imagelayer> layers and children thereof.
    LOG_SKIP_IMAGES = 128,       // Skip images of image layers (<imagelayer>s).
    LOG_SKIP_WANG_SETS = 256,    // Skip <wangsets> and child <wangset> elements.
    LOG_SKIP_WANG_TILES = 512    // Skip Wang tiles of Wang sets (<wangset>s).
};

// Identifiers for the possible layer types.
typedef enum {
    LAYER_TYPE_TILE_LAYER = 0, // Layer containing a set number of tiles referenced by Global IDs (GIDs).
    LAYER_TYPE_OBJECT_GROUP,   // Layer containing an arbitrary number of various object types.
    LAYER_TYPE_IMAGE_LAYER,    // Layer consisting of a single image.
    LAYER_TYPE_GROUP           // Container layer, with no visuals of its own, that holds other, child layers.
} TmxLayerType;

// Identifiers for the possible property (data) types.
typedef enum {
    PROPERTY_TYPE_STRING = 0, // String, or a sequence of characters.
    PROPERTY_TYPE_INT,        // Integer number.
    PROPERTY_TYPE_FLOAT,      // Floating-point number.
    PROPERTY_TYPE_BOOL,       // Boolean, true or false.
    PROPERTY_TYPE_COLOR,      // Color with red, green, and blue values and a possible alpha value.
    PROPERTY_TYPE_FILE,       // File name or path to a file as a string.
    PROPERTY_TYPE_OBJECT      // Object (<object>) within the map as an integer ID.
} TmxPropertyType;

// Identifiers for the possible draw orders applicable to object layers.
typedef enum {
    OBJECT_GROUP_DRAW_ORDER_TOP_DOWN = 0, // Drawn in ascending order by Y coordinate.
    OBJECT_GROUP_DRAW_ORDER_INDEX         // Drawn in the order in which the appear in the document.
} TmxObjectGroupDrawOrder;

// Identifiers for the possible alignments of tiles with an object's bounds.
typedef enum {
    OBJECT_ALIGNMENT_UNSPECIFIED = 0, // For orthogonal, behaves like bottom-left. For isometric, like bottom.
    OBJECT_ALIGNMENT_TOP_LEFT,        // Tiles are snapped to the upper-left bound of objects.
    OBJECT_ALIGNMENT_TOP,             // Tiles are snapped to the upper-center bound of objects.
    OBJECT_ALIGNMENT_TOP_RIGHT,       // Tiles are snapped to the upper-right bound of objects.
    OBJECT_ALIGNMENT_LEFT,            // Tiles are snapped to the left-center bound of objects.
    OBJECT_ALIGNMENT_CENTER,          // Tiles are snapped to the horizontal and vertical center of objects.
    OBJECT_ALIGNMENT_RIGHT,           // Tiles are snapped to the right-center bound of objects.
    OBJECT_ALIGNMENT_BOTTOM_LEFT,     // Tiles are snapped to the lower-left bound of objects.
    OBJECT_ALIGNMENT_BOTTOM,          // Tiles are snapped to the lower-center bound of objects.
    OBJECT_ALIGNMENT_BOTTOM_RIGHT     // Tiles are snapped to the lower-right bound of objects.
} TmxObjectAlignment;

// Identifiers for the possible object types.
typedef enum {
    OBJECT_TYPE_RECTANGLE = 0, // Four-sided polygon with four right angles and axis-aligned edges.
    OBJECT_TYPE_ELLIPSE,       // Ellipse, or a circle when the axes are equal.
    OBJECT_TYPE_POINT,         // Individual (X, Y) position with no dimensions.
    OBJECT_TYPE_POLYGON,       // Filled polygon formed by an ordered series of points.
    OBJECT_TYPE_POLYLINE,      // Unfilled polygon formed by an ordered series of points.
    OBJECT_TYPE_TEXT,          // Text, or the visual representation of a string.
    OBJECT_TYPE_TILE           // Tile, referenced by a Global ID (GID), from a tileset known to the map.
} TmxObjectType;

// Identifiers for the possible horizontal alignments of text.
typedef enum {
    HORIZONTAL_ALIGNMENT_LEFT = 0, // Text is to be snapped to the left bound of its object.
    HORIZONTAL_ALIGNMENT_CENTER,   // Text is to be centered along the X-axis of its object.
    HORIZONTAL_ALIGNMENT_RIGHT,    // Text is to be snapped to the right bound of its object.
    HORIZONTAL_ALIGNMENT_JUSTIFY   // Text is to be evenly spaced, per line, filling the object's width.
} TmxHorizontalAlignment;

// Identifiers for the possible vertical alignments of text.
typedef enum {
    VERTICAL_ALIGNMENT_TOP = 0, // Text is to be snapped to the upper bound of its object.
    VERTICAL_ALIGNMENT_CENTER,  // Text is to be cnetered along the Y-axis of its object.
    VERTICAL_ALIGNMENT_BOTTOM   // Text is to be snapped to the lower bound of its object.
} TmxVerticalAlignment;

// Identifiers for the possible map orientations.
typedef enum {
    ORIENTATION_NONE = 0,   // Orientation was not specified. Assumed to be orthogonal.
    ORIENTATION_ORTHOGONAL, // Standard top-down view with rectanglular tiles.
    ORIENTATION_ISOMETRIC,  // Subset top-down view from a 45-degree angle. NOT supported.
    ORIENTATION_STAGGERED,  // Variation of isometric with staggered axes.
    ORIENTATION_HEXAGONAL   // Top-down view in which tiles are six-sided and alternative rows/columns are offset.
} TmxOrientation;

// Identifiers for the possible orders in which tiles in a tile layer are rendered/drawn.
typedef enum {
    RENDER_ORDER_RIGHT_DOWN = 0, // Tiles are rendered by row, from left to right, then column, from top to bottom.
    RENDER_ORDER_RIGHT_UP,       // Tiles are rendered by row, from left to right, then column, from bottom to top.
    RENDER_ORDER_LEFT_DOWN,      // Tiles are rendered by row, from right to left, then column, from top to bottom.
    RENDER_ORDER_LEFT_UP         // Tiles are rendered by row, from right to left, then column, from bottom to top.
} TmxRenderOrder;

// Model of an <image> element. Defines an image and relevant attributes along with a loaded texture.
typedef struct TmxImage {
    char *source;      // File name and/or path referencing the image on disk.
    Color trans;       // [optional] Color that defines is treated as transparent. Not currently implemented.
    bool hasTrans;     // When true, indicates 'trans' has been set with a color to be treated as transparent.
    uint32_t width;    // Width of the image in pixels.
    uint32_t height;   // Height of the image in pixels.
    Texture2D texture; // The image as a raylib texture loaded into VRAM, if loading was successful.
} TmxImage;

// Model of a <layer> element when combined with the 'TmxLayer' model. Defines a tile layer with a fixed-size list of
// tile Global IDs (GIDs).
typedef struct TmxTileLayer {
    uint32_t width;       // Width of the layer in tiles.
    uint32_t height;      // Height of the layer in tiles.
    char *encoding;       // [optional] Encoding used to encode tiles. May be NULL, "base64", or "csv".
    char *compression;    // [optional] Compression used to compress tiles. May be NULL, "gzip", "zlib", or "zstd".
    uint32_t *tiles;      // Array of tile Global IDs (GIDs) contained by this tile layer.
    uint32_t tilesLength; // Length of the 'tiles' array.
} TmxTileLayer;

// Contains the information needed to quickly draw a single line of a <text> element.
typedef struct TmxTextLine {
    char *content;    // The string to be drawn. This may be the whole content of the parent string or partial.
    Font font;        // The raylib Font to be used when drawing.
    Vector2 position; // Absolute position of this line. This is separate from its object layer's potential offset.
    float spacing;    // Spacing in pixels to be applied between each character when drawing.
} TmxTextLine;

// Model of a <text> element along with some pre-calculated objects for efficient drawing.
typedef struct TmxText {
    char *fontFamily;              // Font family (e.g. "sans-serif") to be used to render the text.
    uint32_t pixelSize;            // Size of the font in pixels.
    bool wrap;                     // When true, indicates word wrapping should be used when appropriate.
    Color color;                   // Color of the text.
    bool bold;                     // When true, indicates the text should be bolded.
    bool italic;                   // When true, indicates the text should be italicized.
    bool underline;                // When true, indicates the text should be underlined.
    bool strikeOut;                // When true, indicates the text should be struck/crossed out.
    bool kerning;                  // When true, indicates kerning should be used when drawing.
    TmxHorizontalAlignment halign; // Horizontal alignment of the text within its object.
    TmxVerticalAlignment valign;   // Vertical alignment of the text within its object.
    char *content;                 // The string to be drawn.
    TmxTextLine *lines;            // Array of pre-calculated lines with all values needed to quickly draw this text.
    uint32_t linesLength;          // Length of the 'lines' array.
} TmxText;

// Model of a <property> element. Describes a property of the model it's attached to with a name, type, and value.
typedef struct TmxProperty {
    TmxPropertyType type; // The specific (data) type of the property indicating which associated value to read.
    char *name;           // Name of the property.
    char *stringValue;    // The property's value for string-typed properties.
    int32_t intValue;     // The property's value for integer-typed properties.
    float floatValue;     // The property's value for floating point-typed properties.
    bool boolValue;       // The property's value for boolean-typed properties.
    Color colorValue;     // The property's value for color-typed properties.
} TmxProperty;

// Model of an <object> element within an <objectgroup> element. Objects are amorphous entities of varying type but all
// are potentially visible with positions and dimensions.
typedef struct TmxObject {
    TmxObjectType type;        // The specific object type indicating which optional fields have relevant information.
    uint32_t id;               // Unique ID of the object.
    char *name;                // Name of the object.
    char *typeString;          // The type/class of the object. ('type' is a reserved keyword hence 'typeString'.)
    double x;                  // Pixel X coordinate of the object. Separate from its object layer's potential offset.
    double y;                  // Pixel Y coordinate of the object. Separate from its object layer's potential offset.
    double width;              // Width of the object in pixels.
    double height;             // Height of the object in pixels.
    double rotation;           // Rotation of the object in (clockwise) degrees around the object's (X, Y) position.
    uint32_t gid;              // [semi-optional] Global ID of a tile drawn as the object. Zero if not a tile.
    bool visible;              // When true, indicates the object will be drawn.
    char *templateString;      // [optional] File name and/or path referencing an object template on disk applied to
                               //  this object. If NULL, no template is used.
    Vector2 *points;           // [optional] Array of *relative*, ordered points that define a poly(gon|line).
    uint32_t pointsLength;     // Length of the 'points' array.
    Vector2 *drawPoints;       // [optional] Array used as a buffer when drawing. Equal in length to the 'points' array.
    TmxText *text;             // [optional] Text to be drawn.
    TmxProperty *properties;   // Array of named, typed properties that apply to this object.
    uint32_t propertiesLength; // Length of the 'properties' array.
    Rectangle aabb;            // Axis-Aligned Bounding Box (AABB).
} TmxObject;

// Model of an <objectgroup> element when combined with the 'TmxLayer' model. Defines an object layer of an arbitrary
// number of objects of varying types.
typedef struct TmxObjectGroup {
    // uint32_t width;                 // Width of the object layer in tiles. Documentation calls it "meaningless."
    // uint32_t height;                // Height of the object layer in tiles. Documentation calls it "meaningless."
    Color color;                       // [optional] Color used to display objects within the layer.
    bool hasColor;                     // When true, indicates 'color' has been set.
    TmxObjectGroupDrawOrder drawOrder; // Indicates the order in which objects in this layer are drawn.
    TmxObject *objects;                // Array of objects contained by this object layer.
    uint32_t objectsLength;            // Length of the 'objects' array.
    uint32_t *ySortedObjects;          // Array of indexes of 'objects' sorted by the objects' Y coordinates.
} TmxObjectGroup;

// Model of an <imagelayer> element when combined with the 'TmxLayer' model. Defines a layer consisting of one image.
typedef struct TmxImageLayer {
    bool repeatX;   // When true, indicates the image is repeated along the X-axis.
    bool repeatY;   // When true, indicates the image is repeated along the Y-axis.
    TmxImage image; // Sole image of this layer.
    bool hasImage;  // When true, indicates 'image' has been set. Should always be true.
} TmxImageLayer;

struct TmxLayer; // Forward declaration of the following type. Contains children of the same type.
// Model of multiple layer elements: <layer>, <objectgroup>, <imagelayer>, or <group>. Defines a layer with attributes
// common to all, more-specific layer types. The more-specific attributes
typedef struct TmxLayer {
    TmxLayerType type;         // The layer type indicating which associated layer ('exact') has mspecific values.
    uint32_t id;               // Unique integer ID of the layer.
    char *name;                // Name of the layer.
    char *classString;         // [optional] Class of the layer. If unused, defaults to an empty string.
    bool visible;              // When true, indicates the layer and its children will be drawn.
    double opacity;            // Opacity of the layer and its children where 0.0 means the layer is fully transparent.
    Color tintColor;           // [optional] Tint color applied to the layer and its children.
    bool hasTintColor;         // When true, indicates 'tintColor' has been set.
    int32_t offsetX;           // Horizontal offset of the layer and its children in pixels.
    int32_t offsetY;           // Vertical offset of the layer and its children in pixels.
    double parallaxX;          // Horizontal parallax factor. 1.0 means the layers position on the screen changes at the
                               // same rate as the camera. 0.0 means the layer will not move with the camera.
    double parallaxY;          // Veritcal parallax factor. 1.0 means the layers position on the screen changes at the
                               // same rate as the camera. 0.0 means the layer will not move with the camera.
    TmxProperty *properties;   // Array of named, typed properties that apply to this layer.
    uint32_t propertiesLength; // Length of the 'properties' array.
    struct TmxLayer *layers;  // [optional] Array of child layers, may be NULL. Only used by group layers.
    uint32_t layersLength;     // Length of the 'layers' array.
    union {
        TmxTileLayer tileLayer;
        TmxObjectGroup objectGroup;
        TmxImageLayer imageLayer;
    } exact;                   // Additional information specific to a tile, object, or image layer but not groups.
} TmxLayer;

// Model of a <frame> element. Defines a temporal frame of an animation with the Global ID (GID) of the tile to be
// displayed and the duration thereof.
typedef struct TmxAnimationFrame {
    uint32_t gid;   // The Global ID (GID) of a tile from the animation's tileset to be drawn for some duration.
    float duration; // Duration in milliseconds that the frame should be displayed.
} TmxAnimationFrame;

// Model of an <animation> element. Defines a series of (tile) frames.
typedef struct TmxAnimation {
    TmxAnimationFrame *frames; // Array of frames. These frames identify tiles and durations to be displayed.
    uint32_t framesLength;     // Length of the 'frames' array.
} TmxAnimation;

// Model of a <tile> element within a <tileset> element. Contains information about tiles that are not or cannot be
// implicitly determined from the tileset.
typedef struct TmxTilesetTile {
    uint32_t id;                // Local ID of the tile within its tileset. Related to but separate from its Global ID.
    char *classString;          // [optional] Class of the tile. If unused, defaults to an empty string.
    int32_t x;                  // X coordinate, in pixels, of the sub-rectangle within the tileset's image to extract.
    int32_t y;                  // Y coordinate, in pixels, of the sub-rectangle within the tileset's image to extract.
    uint32_t width;             // Width, in pixels, of the sub-rectangle within the tileset's image to extract.
    uint32_t height;            // Height, in pixels, of the sub-rectangle within the tileset's image to extract.
    TmxImage image;             // [optional] Image to be used as the tile for "collection of images" tilesets.
    bool hasImage;              // When true, indicates 'image' is set.
    TmxAnimation animation;     // [optional] Animation. Lists GIDs to be drawn temporarily and periodically.
    bool hasAnimation;          // When true, indicates 'animation' is assigned.
    TmxProperty *properties;    // Array of named, typed properties that apply to this tileset tile.
    uint32_t propertiesLength;  // Length of the 'properties' array.
    TmxObjectGroup objectGroup; // [optional] 0+ objects representing collision information unique to the tile.
} TmxTilesetTile;

// Model of a <tileset> element. Defines an image, or series of images, from which tiles are drawn along with
// information on how to extract areas from within the image and/or how to align them within an object.
typedef struct TmxTileset {
    uint32_t firstGid;                  // First Global ID (GID) of a tile in this tileset.
    uint32_t lastGid;                   // Last Global ID (GID) of a tile in this tileset.
    char *source;                       // [optional] Source of this tileset, may be NULL. Only for external tilesets.
    char *name;                         // Name of the tileset.
    char *classString;                  // [optional] Class of the tileset. If unused, defaults to an empty string.
    uint32_t tileWidth;                 // Maximum, typically exact, width of the tiles in this tileset in pixels.
    uint32_t tileHeight;                // Maximum, typically exact, height of the tiles in this tileset in pixels.
    uint32_t spacing;                   // Spacing in pixels between tiles in this tileset.
    uint32_t margin;                    // Margin around the tiles in this tileset.
    uint32_t tileCount;                 // Number of tiles in this tileset.
                                        // Note: 'lastGid' - 'firstGid' is not always 'tileCount'.
    uint32_t columns;                   // Number of tile columsn in this tileset.
    TmxObjectAlignment objectAlignment; // Controls the alignment of tiles of this tileset when used as objects.
    int32_t tileOffsetX;                // Horizontal offset in pixels applied when drawing tiles from this tileset.
    int32_t tileOffsetY;                // Vertical offset in pixels applied when drawing tiles form this tileset.
    TmxImage image;                     // [optional] Image from which this tilesets tiles are extracted.
    bool hasImage;                      // When true, indicates 'image' is set.
    TmxProperty *properties;            // Array of named, typed properties that apply to this tileset.
    uint32_t propertiesLength;          // Length of the 'properties' array.
    TmxTilesetTile *tiles;              // Array of explicitly-defined tiles within the tileset.
    uint32_t tilesLength;               // Length of the 'tiles' array.
} TmxTileset;

// Contains the information and objects needed to quickly draw a <tile> in a raylib application.
typedef struct TmxTile {
    uint32_t gid;               // Global ID (GID) of the tile. Used for lookups. Zero means the tile is unused.
    Rectangle sourceRect;       // Sub-rectangle within a tileset to extract that is to be drawn.
    Texture2D texture;          // Texture to be drawn. May be used whole or as a source of a sub-rectangle.
    Vector2 offset;             // Offset in pixels to be applied to the tile, derived from the tileset.
    Vector2 dimensions;         // Width and height as drawn, in pixels. May differ from the maps tile width and height.
    TmxAnimation animation;     // [optional] Animation. Lists GIDs to be drawn temporarily and periodically.
    bool hasAnimation;          // When true, indicates 'animation' is assigned.
    uint32_t frameIndex;        // The current animation frame to draw for animations.
    float frameTime;            // Accumulator for animations. The time, in seconds, the current frame has been drawn.
    TmxObjectGroup objectGroup; // [optional] 0+ objects representing collision information unique to the tile.
} TmxTile;

// Model of a <map> element along with some pre-calculated objects for efficient drawing.
typedef struct TmxMap {
    char *fileName;             // File name of the TMX file with extension.
    TmxOrientation orientation; // Map orientation. May be orthogonal, isometric, staggered, or hexagonal.
    TmxRenderOrder renderOrder; // Order in which tiles on tile layers are rendered.
    uint32_t width;             // Width of this map in tiles.
    uint32_t height;            // Height of htis map in tiles.
    uint32_t tileWidth;         // Width of a tile in pixels.
    uint32_t tileHeight;        // Height of a tile in pixels.
    int32_t parallaxOriginX;    // X coordinate, in pixels, of the parallax origin.
    int32_t parallaxOriginY;    // Y coordinate, in pixels, of the parallax origin.
    Color backgroundColor;      // [optional] Background color to be drawn behind the map with its dimensions.
    bool hasBackgroundColor;    // When true, indicates 'backgroundColor' is set.
    TmxProperty *properties;    // Array of named, typed properties that apply to this map.
    uint32_t propertiesLength;  // Length of the 'properties' array.
    TmxTileset *tilesets;       // Array of tilesets used by the map.
    uint32_t tilesetsLength;    // Length of the 'tilesets' array.
    TmxLayer *layers;           // Array of layers and potential child layers that make up this map.
    uint32_t layersLength;      // Length of the 'layers' array.
    TmxTile *gidsToTiles;       // Array of pre-calculated tile metadata with values needed to quickly draw a tile given
                                // its GID. Allocated such that e.g. gidsToTiles[11] gets the data of tile GID eleven.
    uint32_t gidsToTilesLength; // Length of the 'gidsToTiles' array.
} TmxMap;

// Load a TMX map from disk. To clean up, use UnloadTMX().
 RAYTMX_DEC TmxMap *LoadTMX(const char *fileName);

// Unload a TMX map.
RAYTMX_DEC void UnloadTMX(TmxMap *map);

// Draw all visible layers of a map. 'camera' is required for parallax and may be used for culling. 'viewport' may be
// used for culling and takes priority over 'camera' if passed.
RAYTMX_DEC void DrawTMX(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport, int posX, int posY,
    Color tint);

// Draw spcific visible layers of a map. 'camera' is required for parallax and may be used for culling. 'viewport' may
// be used for culling and takes priority over 'camera' if passed. 'layers' is an array of length 'layersLength'.
RAYTMX_DEC void DrawTMXLayers(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport,
    const TmxLayer *layers, uint32_t layersLength, int posX, int posY, Color tint);

// Update animated tales based on draw times. Call this once between BeginDrawing() and EndDrawing().
RAYTMX_DEC void AnimateTMX(TmxMap *map);

// Check for collisions between two objects of arbitrary type. Text and tiles are treated as rectangles.
RAYTMX_DEC bool CheckCollisionTMXObjects(TmxObject object1, TmxObject object2);

// Check for collisions between a tile layer, or group layer, and a rectangle. The tiles must have collision information
// created with the Tiled Collision Editor. 'layers' is an array of length 'layersLength'. 'outputObject' is assigned
// with the object collided with, if not NULL.
RAYTMX_DEC bool CheckCollisionTMXTileLayersRec(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Rectangle rec, TmxObject *outputObject);

// Check for collisions between a tile layer, or group layer, and a circle. The tiles must have collision information
// created with the Tiled Collision Editor. 'layers' is an array of length 'layersLength'. 'outputObject' is assigned
// with the object collided with, if not NULL.
RAYTMX_DEC bool CheckCollisionTMXTileLayersCircle(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 center, float radius, TmxObject *outputObject);

// Check for collisions between a tile layer, or group layer, and a point. The tiles must have collision information
// created with the Tiled Collision Editor. // 'layers' is an array of length 'layersLength'. 'outputObject' is assigned
// with the object collided with, if not NULL.
RAYTMX_DEC bool CheckCollisionTMXTileLayersPoint(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 point, TmxObject *outputObject);

// Check for collisions between a tile layer, or group layer, and a polygon. The tiles must have collision information
// created with the Tiled Collision Editor. 'layers' is an array of length 'layersLength'. 'outputObject' is assigned
// with the object collided with, if not NULL.
RAYTMX_DEC bool CheckCollisionTMXTileLayersPoly(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 *points, int pointCount, TmxObject *outputObject);

// Check for collisions between a tile layer, or group layer, and a polygon. The tiles must have collision information
// created with the Tiled Collision Editor. 'layers' is an array of length 'layersLength'. 'outputObject' is assigned
// with the object collided with, if not NULL.
RAYTMX_DEC bool CheckCollisionTMXTileLayersPolyEx(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 *points, int pointCount, Rectangle aabb, TmxObject *outputObject);

// Check for collisions between an object group and a rectangle. 'outputObject' is assigned with the object collided
// with, if not NULL.
RAYTMX_DEC bool CheckCollisionTMXObjectGroupRec(TmxObjectGroup group, Rectangle rec, TmxObject *outputObject);

// Check for collisiosn between an object group and a circle. 'outputObject' is assigned with the object collided with,
// if not NULL.
RAYTMX_DEC bool CheckCollisionTMXObjectGroupCircle(TmxObjectGroup group, Vector2 center, float radius,
    TmxObject *outputObject);

// Check for collisions between an object group and a point. 'outputObject' is assigned with the object collided with,
// if not NULL.
RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoint(TmxObjectGroup group, Vector2 point, TmxObject *outputObject);

// Check for collisions between an object group and a polygon. 'outputObject' is assigned with the object collided with,
// if not NULL.
RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoly(TmxObjectGroup group, Vector2 *points, int pointCount,
    TmxObject *outputObject);

// Check for collisions between an object group and a polygon. 'outputObject' is assigned with the object collided with,
// if not NULL.
RAYTMX_DEC bool CheckCollisionTMXObjectGroupPolyEx(TmxObjectGroup group, Vector2 *points, int pointCount,
    Rectangle aabb, TmxObject *outputObject);

// Set a custom texture load. If set, used in place of raylib's LoadTexture().
RAYTMX_DEC void SetLoadTextureTMX(LoadTextureCallback callback);

// Log properties of a map. SetTraceLogFlagsTMX() may be used to exclude select properties.
RAYTMX_DEC void TraceLogTMX(int logLevel, const TmxMap *map);

// Set logging options for TraceLogTMX(). Consumes flag defined in the TmxLogFlags enumeration.
RAYTMX_DEC void SetTraceLogFlagsTMX(int logFlags);

#ifdef __cplusplus
    }
#endif // __cplusplus

#ifdef RAYTMX_IMPLEMENTATION

#ifndef HOXML_IMPLEMENTATION
    #define HOXML_IMPLEMENTATION
#endif
#include "hoxml.h"

//----------------------------------------------------------------------------------
// Private implementation of public types and functions.
//----------------------------------------------------------------------------------

// Thickness, in pixels, that outlines of specific objects are drawn with.
#define TMX_LINE_THICKNESS 3.0f

// Bit flags that GIDs may be masked with in order to indicate transformations for individual tiles.
enum TmxFlipFlags {
    FLIP_FLAG_HORIZONTAL = 0x80000000,
    FLIP_FLAG_VERTICAL = 0x40000000,
    FLIP_FLAG_DIAGONAL = 0x20000000,
    FLIP_FLAG_ROTATE_120 = 0x10000000
};

typedef enum {
    FORMAT_TMX = 0, // Tilemap with tilesets, layers, etc.
    FORMAT_TSX,     // External tilesets.
    FORMAT_TX       // Object templates.
} RaytmxDocumentFormat;

typedef struct RaytmxExternalTileset {
    TmxTileset tileset;
    bool isSuccess; // 'isSuccess' is true when the external tileset was successfully loaded.
} RaytmxExternalTileset;

typedef struct RaytmxObjectTemplate {
    TmxTileset tileset;
    TmxObject object;
    bool isSuccess, hasTileset; // 'isSuccess' is true when the object template was successfully loaded.
} RaytmxObjectTemplate;

struct RaytmxCachedTextureNode; // Forward declaration.
// Associates a file name with a Texture2D allowing for the reuse of textures in VRAM.
typedef struct RaytmxCachedTextureNode {
    char *fileName;
    Texture2D texture;
    struct RaytmxCachedTextureNode *next;
} RaytmxCachedTextureNode;

struct RaytmxCachedTemplateNode; // Forward declaration.
// Associates a file name with an object template.
typedef struct RaytmxCachedTemplateNode {
    char *fileName;
    RaytmxObjectTemplate objectTemplate;
    struct RaytmxCachedTemplateNode *next;
} RaytmxCachedTemplateNode;

struct RaytmxPropertyNode; // Forward declaration.
typedef struct RaytmxPropertyNode {
    TmxProperty property;
    struct RaytmxPropertyNode *next;
} RaytmxPropertyNode;

struct RaytmxTilesetNode; // Forward declaration.
typedef struct RaytmxTilesetNode {
    TmxTileset tileset;
    struct RaytmxTilesetNode *next;
} RaytmxTilesetNode;

struct RaytmxTilesetTileNode; // Forward declaration.
typedef struct RaytmxTilesetTileNode {
    TmxTilesetTile tile;
    struct RaytmxTilesetTileNode *next;
} RaytmxTilesetTileNode;

struct RaytmxAnimationFrameNode; // Forward declaration.
typedef struct RaytmxAnimationFrameNode {
    TmxAnimationFrame frame;
    struct RaytmxAnimationFrameNode *next;
} RaytmxAnimationFrameNode;

struct RaytmxLayerNode; // Forward declaration.
typedef struct RaytmxLayerNode {
    TmxLayer layer;
    uint32_t childrenLength;
    struct RaytmxLayerNode *next;
    struct RaytmxLayerNode *parent;
    struct RaytmxLayerNode *childrenRoot;
    struct RaytmxLayerNode *childrenTail;
} RaytmxLayerNode;

struct RaytmxTileLayerTileNode; // Forward declaration.
typedef struct RaytmxTileLayerTileNode {
    uint32_t gid;
    struct RaytmxTileLayerTileNode *next;
} RaytmxTileLayerTileNode;

struct RaytmxObjectNode; // Forward declaration.
typedef struct RaytmxObjectNode {
    TmxObject object;
    struct RaytmxObjectNode *next;
} RaytmxObjectNode;

struct RaytmxObjectSortingNode; // Forward declaration.
typedef struct RaytmxObjectSortingNode {
    double y;
    uint32_t index;
    struct RaytmxObjectSortingNode *next;
} RaytmxObjectSortingNode;

struct RaytmxPolyPointNode; // Forward declaration.
typedef struct RaytmxPolyPointNode {
    Vector2 point;
    struct RaytmxPolyPointNode *next;
} RaytmxPolyPointNode;

struct RaytmxTextLineNode; // Forward declaration.
typedef struct RaytmxTextLineNode {
    TmxTextLine line;
    struct RaytmxTextLineNode *next;
} RaytmxTextLineNode;

// Intermediate data used internally to parse TMX (map), TSX (tileset), and TX (template) files.
typedef struct RaytmxState {
    RaytmxDocumentFormat format;
    char documentDirectory[512];
    bool isSuccess;

    // Variables intended for TMX (map) parsing.
    RaytmxCachedTextureNode *texturesRoot;
    RaytmxCachedTemplateNode *templatesRoot;
    TmxOrientation mapOrientation;
    TmxRenderOrder mapRenderOrder;
    uint32_t mapWidth;
    uint32_t mapHeight;
    uint32_t mapTileWidth;
    uint32_t mapTileHeight;
    uint32_t mapPropertiesLength;
    int32_t mapParallaxOriginX;
    int32_t mapParallaxOriginY;
    Color mapBackgroundColor;
    bool mapHasBackgroundColor;
    TmxProperty *mapProperties;

    // These variables, when not NULL, are assigned to the current element(s) being parsed.
    TmxProperty *property;
    TmxTileset *tileset;
    TmxImage *image;
    TmxTilesetTile *tilesetTile;
    TmxAnimationFrame *animationFrame;
    // TmxWangSet *wangSet;     // TODO: Wang sets. Low priority.
    // TmxWangColor *wangColor; // TODO: Wang sets. Low priority.
    TmxLayer *layer;
    TmxTileLayer *tileLayer;
    TmxObjectGroup *objectGroup;
    TmxImageLayer *imageLayer;
    TmxObject *object;

    // These variables are linked lists containing various elements where an arbitrary amount are allowed, such as 1+
    // <object> elements in an <objectgroup>, that will be copied to arrays of known sizes later on.
    RaytmxPropertyNode *propertiesRoot;
    RaytmxPropertyNode *propertiesTail;
    uint32_t propertiesLength;
    uint32_t propertiesDepth;
    RaytmxTilesetNode *tilesetsRoot;
    RaytmxTilesetNode *tilesetsTail;
    uint32_t tilesetsLength;
    RaytmxTilesetTileNode *tilesetTilesRoot;
    RaytmxTilesetTileNode *tilesetTilesTail;
    uint32_t tilesetTilesLength;
    RaytmxAnimationFrameNode *animationFramesRoot;
    RaytmxAnimationFrameNode *animationFramesTail;
    uint32_t animationFramesLength;
    RaytmxLayerNode *layersRoot;
    RaytmxLayerNode *layersTail;
    uint32_t layersLength;
    RaytmxLayerNode *groupNode;
    RaytmxTileLayerTileNode *layerTilesRoot;
    RaytmxTileLayerTileNode *layerTilesTail;
    uint32_t layerTilesLength;
    RaytmxObjectNode *objectsRoot;
    RaytmxObjectNode *objectsTail;
    uint32_t objectsLength;
} RaytmxState;

typedef struct RaytmxTransform {
    Vector2 position;
    Vector2 parallax;
    Vector2 cameraOffset;
} RaytmxTransform;

// Forward declarations of functions used by the private implementation.
RaytmxExternalTileset LoadTSX(const char *fileName);
RaytmxObjectTemplate LoadTX(const char *fileName);
void ParseDocument(RaytmxState *state, const char *fileName);
void HandleElementBegin(RaytmxState *state, hoxml_context_t *hoxml);
void HandleAttribute(RaytmxState *state, hoxml_context_t *hoxml);
void HandleElementEnd(RaytmxState *state, hoxml_context_t *hoxml);
void FreeState(RaytmxState *state);
void FreeString(char *str);
void FreeTileset(TmxTileset tileset);
void FreeProperty(TmxProperty property);
void FreeLayer(TmxLayer layer);
void FreeObject(TmxObject object);
bool IterateTileLayer(const TmxMap *map, const TmxTileLayer *layer, Rectangle viewport, RaytmxTransform transform,
    uint32_t *rawGid, TmxTile *tile, Rectangle *tileRect);
void DrawTMXLayersInternal(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport, const TmxLayer *layers,
    uint32_t layersLength, RaytmxTransform transform, Color tint);
void DrawTMXTileLayer(const TmxMap *map, Rectangle viewport, TmxLayer layer, RaytmxTransform transform, Color tint);
void DrawTMXLayerTile(const TmxMap *map, Rectangle viewport, RaytmxTransform transform, uint32_t rawGid,
    Rectangle destRect, Color tint);
void DrawTMXObjectTile(const TmxMap *map, Rectangle viewport, uint32_t rawGid, RaytmxTransform transform, float width,
    float height, Color tint);
void DrawTMXObjectGroup(const TmxMap *map, Rectangle viewport, TmxLayer layer, RaytmxTransform transform, Color tint);
void DrawTMXImageLayer(const TmxMap *map, Rectangle viewport, TmxLayer layer, RaytmxTransform transform, Color tint);
bool CheckCollisionTMXTileLayerObject(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    TmxObject object, TmxObject *outputObject);
bool CheckCollisionTMXObjectGroupObject(TmxObjectGroup group, TmxObject object, TmxObject *outputObject);
void TraceLogTMXTilesets(int logLevel, TmxOrientation orientation, TmxTileset *tilesets, uint32_t tilesetsLength,
    int numSpaces);
void TraceLogTMXProperties(int logLevel, TmxProperty *properties, uint32_t propertiesLength, int numSpaces);
void TraceLogTMXLayers(int logLevel, TmxLayer *layers, uint32_t layersLength, int numSpaces);
void TraceLogTMXObject(int logLevel, TmxObject object, int numSpaces);
void StringCopy(char *destination, const char *source);
TmxProperty *AddProperty(RaytmxState *state);
void AddTileLayerTile(RaytmxState *state, uint32_t gid);
TmxTileset *AddTileset(RaytmxState *state);
TmxTilesetTile *AddTilesetTile(RaytmxState *state);
TmxAnimationFrame *AddAnimationFrame(RaytmxState *state);
TmxLayer *AddGenericLayer(RaytmxState *state, bool isGroup);
TmxObject *AddObject(RaytmxState *state);
void AppendLayerTo(TmxMap *map, RaytmxLayerNode *groupNode, RaytmxLayerNode *layersRoot, uint32_t layersLength);
RaytmxCachedTextureNode *LoadCachedTexture(RaytmxState *state, const char *fileName);
RaytmxCachedTemplateNode *LoadCachedTemplate(RaytmxState *state, const char *fileName);
Color GetColorFromHexString(const char *hex);
uint32_t GetGid(uint32_t rawGid, bool *flipX, bool *flipY, bool *flipDiag, bool *rotateHexag120);
void *MemAllocZero(unsigned int size);
const char *GetDirectoryPath2(const char *filePath);
const char *JoinPath(const char *prefix, const char *suffix);
void StringCopyN(char *destination, const char *source, size_t number);
void StringConcatenate(char *destination, const char *source);

RAYTMX_DEC TmxMap *LoadTMX(const char *fileName)
{
    TmxMap *map = (TmxMap *)MemAllocZero(sizeof(TmxMap));
    RaytmxState state;
    memset(&state, 0, sizeof(RaytmxState));
    state.format = FORMAT_TMX;

    // Do format-agnostic parsing of the document. The state object will be populated with raytmx's models of the
    // equivalent TMX, TSX, and/or TX elements. */
    ParseDocument(&state, fileName);
    if (!state.isSuccess)
    {
        UnloadTMX(map);
        return NULL;
    }

    // Copy some top-level map properties.
    map->fileName = (char *)MemAllocZero((unsigned int)strlen(fileName) + 1);
    StringCopy(map->fileName, GetFileName(fileName));
    map->orientation = state.mapOrientation;
    map->renderOrder = state.mapRenderOrder;
    map->width = state.mapWidth;
    map->height = state.mapHeight;
    map->tileWidth = state.mapTileWidth;
    map->tileHeight = state.mapTileHeight;
    map->backgroundColor = state.mapBackgroundColor;
    map->parallaxOriginX = state.mapParallaxOriginX;
    map->parallaxOriginY = state.mapParallaxOriginY;
    map->hasBackgroundColor = state.mapHasBackgroundColor;

    uint32_t gidsToTilesLength = 0; // Can also be seen as the last GID.
    if (state.tilesetsRoot != NULL) // If there is at least one tileset.
    {
        // Allocate the array of tilesets and zeroize every index.
        TmxTileset *tilesets = (TmxTileset *)MemAllocZero(sizeof(TmxTileset)*state.tilesetsLength);

        // Copy the TmxTileset pointers into the array.
        RaytmxTilesetNode *tilesetIter = state.tilesetsRoot;
        for (uint32_t i = 0; tilesetIter != NULL; i++)
        {
            TmxTileset tileset = tilesetIter->tileset;

            if (tileset.hasImage) // If the tileset has a shared image and implicitly defines tiles.
            {
                // These tilesets implicitly have X tiles where X = <width in tiles> * <height in tiles>. But even these
                // tilesets may have explicitly defined tiles for things like animations, or anything else. We need to
                // know the greatest ID of those explicit tiles because they are allowed to have IDs that exceed the
                // tileset's tile count and this affects global IDs, even in other tilesets.
                uint32_t greatestID = 0;
                for (uint32_t i = 0; i < tileset.tilesLength; i++)
                    if (greatestID < tileset.tiles[i].id) greatestID = tileset.tiles[i].id;
                // Determine the last GID, meaning the greatest GID value, from the greatest local ID and tile count.
                if (greatestID >= tileset.tileCount) tileset.lastGid = tileset.firstGid + greatestID;
                else tileset.lastGid = tileset.firstGid + tileset.tileCount - 1;
            }
            else if (tileset.tilesLength > 0) // If the tileset is a "collection of images" with explicit tiles.
                tileset.lastGid = tileset.firstGid + tileset.tiles[tileset.tilesLength - 1].id - 1;

            if (gidsToTilesLength < tileset.lastGid + 1) gidsToTilesLength = tileset.lastGid + 1;
            // Note: GIDs start at 1 so the length is the last GID + 1.

            tilesets[i] = tileset;
            tilesetIter = tilesetIter->next;
        }

        // Add the tilesets array to the map.
        map->tilesets = tilesets;
        map->tilesetsLength = state.tilesetsLength;
    }
    else TraceLog(LOG_WARNING, "RAYTMX: The map does not contain any tilesets");

    if (state.layersRoot != NULL) // If there is at least one layer within the map.
    {
        // Due to the existence of <group> layers, layers can have children of multiple generations. To form the
        // resulting tree-like structure, recursion is used.
        AppendLayerTo(map, NULL, state.layersRoot, state.layersLength);
    }
    else TraceLog(LOG_WARNING, "RAYTMX: The map does not contain any layers");

    if (gidsToTilesLength > 0)
    {
        TmxTile *gidsToTiles = (TmxTile *)MemAllocZero(sizeof(TmxTile)*gidsToTilesLength);

        for (uint32_t i = 0; i < map->tilesetsLength; i++)
        {
            TmxTileset *tileset = &(map->tilesets[i]);

            if (tileset->hasImage) // If the tileset has a shared image (i.e. not a "collection of images").
            {
                // First, iterate through the explicit tiles. These are explicitly defined with <tile> elements and are
                // used to, among other things, provide animations and specific source rectangles. Note: Their IDs may
                // exceed the tileset's tile count.
                for (uint32_t j = 0; j < tileset->tilesLength; j++)
                {
                    const TmxTilesetTile tilesetTile = tileset->tiles[j];
                    const uint32_t gid = tileset->firstGid + tilesetTile.id;

                    if (tilesetTile.hasAnimation) // If the tile is meta, pointing to a series of other tiles.
                    {
                        gidsToTiles[gid].hasAnimation = true;
                        gidsToTiles[gid].animation = tilesetTile.animation;

                        // Frames' tile (G)IDs are initially set with local values. Now that all tilesets are known and
                        // this tileset's first global ID is available, update the ID to be global.
                        for (uint32_t k = 0; k < gidsToTiles[gid].animation.framesLength; k++)
                            gidsToTiles[gid].animation.frames[k].gid += tileset->firstGid;
                    }

                    if (tilesetTile.x != 0 || tilesetTile.y != 0 || tilesetTile.width != 0 || tilesetTile.height != 0)
                    {
                        // This tile directly tells us the area within the tileset's image to use when drawing,
                        // overriding the implicit dimensions derived from the map's tile width and height.
                        gidsToTiles[gid].sourceRect.x = (float)tilesetTile.x;
                        gidsToTiles[gid].sourceRect.y = (float)tilesetTile.y;
                        gidsToTiles[gid].sourceRect.width = (float)tilesetTile.width;
                        gidsToTiles[gid].sourceRect.height = (float)tilesetTile.height;
                    }
                    gidsToTiles[gid].dimensions.x = (float)tileset->tileWidth;
                    gidsToTiles[gid].dimensions.y = (float)tileset->tileHeight;

                    // Tiles may have child object groups. These objects are a form of collision information. The object
                    // group may be empty or may have objects. A simple assignment covers both.
                    gidsToTiles[gid].objectGroup = tilesetTile.objectGroup;
                    gidsToTiles[gid].gid = gid; // Tell the tile its GID.
                }

                // Second, loop through the implicit tiles. These are inferred from knowing the dimensions of the
                // tileset's image, dimensiosn of tiles, and the (right-down) order of tiles within the image.
                for (uint32_t id = 0; id < tileset->tileCount; id++)
                {
                    const uint32_t gid = id + tileset->firstGid;
                    const uint32_t x = id%tileset->columns;
                    const uint32_t y = id/tileset->columns;
                    gidsToTiles[gid].gid = gid; // Tell the tile its GID.

                    // If that source renctangle within the image was NOT explicitly defined.
                    // Note: Float comparisons like this are typically unreliable but the GIDs-to-tiles array is
                    // initialized with zeroes making this accurate.
                    if ((gidsToTiles[gid].sourceRect.x == 0.0f) && (gidsToTiles[gid].sourceRect.y == 0.0f) &&
                        (gidsToTiles[gid].sourceRect.width == 0.0f) && (gidsToTiles[gid].sourceRect.height == 0.0f))
                    {
                        // Calculate the area within the texture to extract from some contextual information.
                        gidsToTiles[gid].sourceRect.x = (float)(tileset->margin + (x*tileset->tileWidth) +
                            (x*tileset->spacing));
                        gidsToTiles[gid].sourceRect.y = (float)(tileset->margin + (y*tileset->tileHeight) +
                            (y*tileset->spacing));
                        gidsToTiles[gid].sourceRect.width = (float)tileset->tileWidth;
                        gidsToTiles[gid].sourceRect.height = (float)tileset->tileHeight;
                    }

                    gidsToTiles[gid].texture = tileset->image.texture;
                    gidsToTiles[gid].offset.x = (float)tileset->tileOffsetX;
                    gidsToTiles[gid].offset.y = (float)tileset->tileOffsetY;
                    gidsToTiles[gid].dimensions.x = (float)tileset->tileWidth;
                    gidsToTiles[gid].dimensions.y = (float)tileset->tileHeight;
                }
            }
            else // If the tileset is a collection of images where each tile has its own image.
            {
                for (uint32_t j = 0; j < tileset->tilesLength; j++)
                {
                    const TmxTilesetTile tilesetTile = tileset->tiles[j];

                    if (!tilesetTile.hasImage)
                    {
                        TraceLog(LOG_WARNING, "RAYTMX: Skipping tile %d of image collection tileset \"%s\" because "
                            "it has no image", tilesetTile.id, tileset->name);
                        continue;
                    }

                    const int32_t gid = tileset->firstGid + tilesetTile.id;
                    gidsToTiles[gid].gid = gid;
                    gidsToTiles[gid].sourceRect.x = (float)tilesetTile.x; // Defaults to, and probably is, zero.
                    gidsToTiles[gid].sourceRect.y = (float)tilesetTile.y; // Defaults to, and probably is, zero.
                    gidsToTiles[gid].sourceRect.width = (tilesetTile.width != tilesetTile.image.width)?
                        (float)tilesetTile.width : (float)tilesetTile.image.width;
                    gidsToTiles[gid].sourceRect.height = (tilesetTile.height != tilesetTile.image.height)?
                        (float)tilesetTile.height : (float)tilesetTile.image.height;
                    gidsToTiles[gid].dimensions.x = gidsToTiles[gid].sourceRect.width;
                    gidsToTiles[gid].dimensions.y = gidsToTiles[gid].sourceRect.height;
                    gidsToTiles[gid].texture = tilesetTile.image.texture;
                }
            }
        }

        map->gidsToTiles = gidsToTiles;
        map->gidsToTilesLength = gidsToTilesLength;
    }

    // Free the linked lists and zeroize related values.
    FreeState(&state);

    return map;
}

RAYTMX_DEC void UnloadTMX(TmxMap *map)
{
    if (map == NULL) return;

    if (map->fileName != NULL) MemFree(map->fileName);

    if (map->properties != NULL)
    {
        for (uint32_t i = 0; i < map->propertiesLength; i++) FreeProperty(map->properties[i]);
        MemFree(map->properties);
    }

    if (map->tilesets != NULL)
    {
        for (uint32_t i = 0; i < map->tilesetsLength; i++) FreeTileset(map->tilesets[i]);
        MemFree(map->tilesets);
    }

    if (map->layers != NULL)
    {
        for (uint32_t i = 0; i < map->layersLength; i++) FreeLayer(map->layers[i]);
        MemFree(map->layers);
    }

    if (map->gidsToTiles != NULL) MemFree(map->gidsToTiles);

    MemFree(map);
}

RAYTMX_DEC void DrawTMX(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport, int posX, int posY,
    Color tint)
{
    if (map == NULL) return;

    if (map->hasBackgroundColor)
    {
        Rectangle backgroundRect;
        backgroundRect.x = (float)posX;
        backgroundRect.y = (float)posY;
        backgroundRect.width = (float)(map->width * map->tileWidth);
        backgroundRect.height = (float)(map->height * map->tileHeight);
        DrawRectangleRec(backgroundRect, map->backgroundColor);
    }

    DrawTMXLayers(map, camera, viewport, map->layers, map->layersLength, posX, posY, tint);
}

RAYTMX_DEC void DrawTMXLayers(const TmxMap* map, const Camera2D* camera, const Rectangle* viewport,
    const TmxLayer* layers, uint32_t layersLength, int posX, int posY, Color tint)
{
    // Pack some position and related information into an object to pass to various functions. At a minimum, this will
    // will determine the position each layer is drawn at. With a camera, this will also be used for parallax.
    RaytmxTransform transform;
    transform.position.x = (float)posX;
    transform.position.y = (float)posY;
    transform.parallax.x = 1.0f;
    transform.parallax.y = 1.0f;
    transform.cameraOffset.x = 0.0f;
    transform.cameraOffset.y = 0.0f;

    if ((map != NULL) && (camera != NULL)) // If the map, with its parallax origin, and a camera are available.
    {
        // Calculate the camera's distance from the parallax origin. This origin is the reference point for parallax
        // scrolling. The amount a parallaxed layer scrolls is proportion to this distance and some constant factor.
        transform.cameraOffset.x = camera->target.x - map->parallaxOriginX;
        transform.cameraOffset.y = camera->target.y - map->parallaxOriginY;
    }

    DrawTMXLayersInternal(map, camera, viewport, layers, layersLength, transform, tint);
}

RAYTMX_DEC void AnimateTMX(TmxMap *map)
{
    if (map == NULL) return;

    const float dt = GetFrameTime(); // Get the time, in seconds, since the last frame.

    // Iterate through the tiles, searching for those that are animations.
    for (uint32_t gid = 0; gid < map->gidsToTilesLength; gid++)
    {
        TmxTile *tile = &map->gidsToTiles[gid]; // A pointer is used in case the frame time needs to be reassigned.

        // If the GID maps to a valid tile, that tile is an animation, and the bounds check passes.
        if (tile->gid > 0 && tile->hasAnimation && tile->frameIndex < tile->animation.framesLength)
        {
            tile->frameTime += dt;

            // If the current frame has been displayed for its whole duration, or longer.
            if (tile->frameTime > tile->animation.frames[tile->frameIndex].duration)
            {
                tile->frameTime -= tile->animation.frames[tile->frameIndex].duration;

                // Increment the frame index to display the next one...
                tile->frameIndex += 1;

                // ...unless the last frame was "last" in both senses.
                if (tile->frameIndex == tile->animation.framesLength) tile->frameIndex = 0; // Wrap around.
            }
        }
    }
}

// Helper function that creates a TmxObject equivalent to a rectangle.
TmxObject CreateRectangleTMXObject(Rectangle rec)
{
    TmxObject object;
    memset(&object, 0, sizeof(TmxObject));

    object.type = OBJECT_TYPE_RECTANGLE;
    object.x = (double)rec.x;
    object.y = (double)rec.y;
    object.width = (double)rec.width;
    object.height = (double)rec.height;
    object.aabb = rec;

    return object;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersRec(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Rectangle rec, TmxObject *outputObject)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0)) return false;

    // Check the rectangle against objects associated with tiles in the layers for collisions.
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreateRectangleTMXObject(rec), outputObject);
}

// Helper function that creates a TmxObject equivalent to a circle.
TmxObject CreateCircularTMXObject(Vector2 center, float radius)
{
    TmxObject object;
    memset(&object, 0, sizeof(TmxObject));

    object.type = OBJECT_TYPE_ELLIPSE;
    object.x = (double)(center.x - radius);
    object.y = (double)(center.y - radius);
    object.width = 2.0*(double)radius;
    object.height = 2.0*(double)radius;
    object.aabb.x = center.x - radius;
    object.aabb.y = center.y - radius;
    object.aabb.width = 2.0f*radius;
    object.aabb.height = 2.0f*radius;

    return object;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersCircle(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 center, float radius, TmxObject *outputObject)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0)) return false;

    // Check the circle against objects associated with tiles in the layers for collisions.
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreateCircularTMXObject(center, radius),
        outputObject);
}

// Helper function that creates a TmxObject equivalent to a point.
TmxObject CreatePointTMXObject(Vector2 point)
{
    TmxObject object;
    memset(&object, 0, sizeof(TmxObject));

    object.type = OBJECT_TYPE_POINT;
    object.x = (double)point.x;
    object.y = (double)point.y;
    object.aabb.x = (float)point.x;
    object.aabb.y = (float)point.y;

    return object;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersPoint(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 point, TmxObject *outputObject)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0)) return false;

    // Check the point against objects associated with tiles in the layers for collisions.
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreatePointTMXObject(point), outputObject);
}

// Helper function that creates a polygon TmxObject from points with absolute positions.
// Note: If an AABB should be calculated from the vertices, pass NULL for 'aabb'.
TmxObject CreatePolygonTMXObject(Vector2 *points, int pointCount, Rectangle *aabb)
{
    TmxObject object;
    memset(&object, 0, sizeof(TmxObject));

    object.type = OBJECT_TYPE_POLYGON;

    if (aabb != NULL) object.aabb = *aabb; // If an AABB was provided, copy it.
    else // If no AABB was provided and, therefore, must be calculated.
    {
        // Calculate the bounding box by searching for the X-axis and Y-axis extremes.
        float minX = INFINITY;
        float maxX = -INFINITY;
        float minY = INFINITY;
        float maxY = -INFINITY;
        for (int i = 0; i < pointCount; i++)
        {
            if (points[i].x < minX) minX = points[i].x;
            if (points[i].x > maxX) maxX = points[i].x;
            if (points[i].y < minY) minY = points[i].y;
            if (points[i].y > maxY) maxY = points[i].y;
        }

        object.aabb.x = minX;
        object.aabb.y = minY;
        object.aabb.width = maxX - minX;
        object.aabb.height = maxY - minY;
    }

    // Polygons' points are relative to the object's position. Because this function takes points with absolute
    // positions, we'll account for this by making the object's position (0, 0).
    object.x = 0.0;
    object.y = 0.0;
    object.width = (double)object.aabb.width;
    object.height = (double)object.aabb.height;
    object.points = points;
    object.pointsLength = pointCount;

    return object;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersPoly(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 *points, int pointCount, TmxObject *outputObject)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0) || (points == NULL) || (pointCount < 3)) return false;

    // Check the polygon against objects associated with tiles in the layers for collisions.
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreatePolygonTMXObject(points, pointCount, NULL),
        outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersPolyEx(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    Vector2 *points, int pointCount, Rectangle aabb, TmxObject *outputObject)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0) || (points == NULL) || (pointCount < 3)) return false;

    // Check the polygon against objects associated with tiles in the layers for collisions.
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength,
        CreatePolygonTMXObject(points, pointCount, &aabb), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupRec(TmxObjectGroup group, Rectangle rec, TmxObject *outputObject)
{
    if ((group.objectsLength == 0) || (rec.width < 0.0f) || (rec.height < 0.0f)) return false;

    // Check the rectangle against TMX objects in the group for collisions.
    return CheckCollisionTMXObjectGroupObject(group, CreateRectangleTMXObject(rec), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupCircle(TmxObjectGroup group, Vector2 center, float radius,
    TmxObject *outputObject)
{
    if ((group.objectsLength == 0) || (radius < 0.0f)) return false;

    // Check the circle against TMX objects in the group for collisions.
    return CheckCollisionTMXObjectGroupObject(group, CreateCircularTMXObject(center, radius), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoint(TmxObjectGroup group, Vector2 point, TmxObject *outputObject)
{
    if (group.objectsLength == 0) return false;

    // Check the point against TMX objects in the group for collisions.
    return CheckCollisionTMXObjectGroupObject(group, CreatePointTMXObject(point), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoly(TmxObjectGroup group, Vector2 *points, int pointCount,
    TmxObject *outputObject)
{
    if ((group.objectsLength == 0) || (points == NULL) || (pointCount < 3)) return false;

    // Check the polygon TMX object against other TMX objects in the group for collisions.
    return CheckCollisionTMXObjectGroupObject(group, CreatePolygonTMXObject(points, pointCount, NULL), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupPolyEx(TmxObjectGroup group, Vector2 *points, int pointCount,
    Rectangle aabb, TmxObject *outputObject)
{
    if ((group.objectsLength == 0) || (points == NULL) || (pointCount < 3)) return false;

    // Check the polygon TMX object against other TMX objects in the group for collisions.
    return CheckCollisionTMXObjectGroupObject(group, CreatePolygonTMXObject(points, pointCount, &aabb), outputObject);
}

// LoadTexture() callback function pointer. NULL means no override.
static LoadTextureCallback loadTextureOverride = NULL;

RAYTMX_DEC void SetLoadTextureTMX(LoadTextureCallback callback)
{
    loadTextureOverride = callback;
}

// Bit flags that disable specific properties in TraceLogTMX(). Zero means nothing is disabled.
static int tmxLogFlags = 0;

RAYTMX_DEC void TraceLogTMX(int logLevel, const TmxMap *map)
{
    if (map == NULL) return;

    switch (map->orientation)
    {
        case ORIENTATION_NONE: TraceLog(logLevel, "orientation: none"); break;
        case ORIENTATION_ORTHOGONAL: TraceLog(logLevel, "orientation: orthogonal"); break;
        case ORIENTATION_ISOMETRIC: TraceLog(logLevel, "orientation: isometric"); break;
        case ORIENTATION_STAGGERED: TraceLog(logLevel, "orientation: staggered"); break;
        case ORIENTATION_HEXAGONAL: TraceLog(logLevel, "orientation: hexagonal"); break;
    }

    switch (map->renderOrder)
    {
        case RENDER_ORDER_RIGHT_DOWN: TraceLog(logLevel, "render order: right-down"); break;
        case RENDER_ORDER_RIGHT_UP: TraceLog(logLevel, "render order: right-up"); break;
        case RENDER_ORDER_LEFT_DOWN: TraceLog(logLevel, "render order: left-down"); break;
        case RENDER_ORDER_LEFT_UP: TraceLog(logLevel, "render order: left-up"); break;
    }

    TraceLog(logLevel, "width: %u tiles", map->width);
    TraceLog(logLevel, "height: %u tiles", map->height);
    TraceLog(logLevel, "tile width: %u pixels", map->tileWidth);
    TraceLog(logLevel, "tile height: %u pixels", map->tileHeight);
    TraceLog(logLevel, "parallax origin X: %d pixels", map->parallaxOriginX);
    TraceLog(logLevel, "parallax origin Y: %d pixels", map->parallaxOriginY);
    if (map->hasBackgroundColor) TraceLog(logLevel, "background color: 0x%08X", map->backgroundColor);

    TraceLogTMXTilesets(logLevel, map->orientation, map->tilesets, map->tilesetsLength, 0);
    TraceLogTMXProperties(logLevel, map->properties, map->propertiesLength, 0);
    TraceLogTMXLayers(logLevel, map->layers, map->layersLength, 0);
}

RAYTMX_DEC void SetTraceLogFlagsTMX(int logFlags)
{
    tmxLogFlags = logFlags;
}

RaytmxExternalTileset LoadTSX(const char *fileName)
{
    RaytmxExternalTileset externalTileset = { 0 };
    RaytmxState state;
    memset(&state, 0, sizeof(RaytmxState));
    state.format = FORMAT_TSX;

    // Do format-agnostic parsing of the document. The state object will be populated with raytmx's models of the
    // equivalent TMX, TSX, and/or TX elements.
    ParseDocument(&state, fileName);
    if (!state.isSuccess) return externalTileset;

    if (state.tilesetsRoot != NULL) // If there is at least one tileset.
    {
        // Copy the root tileset so it can be returned.
        externalTileset.tileset = state.tilesetsRoot->tileset;
        externalTileset.isSuccess = true;

        // TSX files should have only one tileset so any others will be freed/unloaded immediately.
        RaytmxTilesetNode *tilesetIter = state.tilesetsRoot->next;
        while (tilesetIter != NULL)
        {
            FreeTileset(tilesetIter->tileset);
            tilesetIter = tilesetIter->next;
        }
    }
    else TraceLog(LOG_WARNING, "RAYTMX: TSX file (external tileset) \"%s\" does not contain any tilesets", fileName);

    // Free the linked lists and zeroize related values.
    FreeState(&state);

    return externalTileset;
}

RaytmxObjectTemplate LoadTX(const char *fileName)
{
    RaytmxObjectTemplate objectTemplate = { 0 };
    RaytmxState state;
    memset(&state, 0, sizeof(RaytmxState));
    state.format = FORMAT_TX;

    // Do format-agnostic parsing of the document. The state object will be populated with raytmx's models of the
    // equivalent TMX, TSX, and/or TX elements.
    ParseDocument(&state, fileName);
    if (!state.isSuccess) return objectTemplate;

    if (state.objectsRoot != NULL) // If there is at least one object.
    {
        // Copy the root object so it can be returned.
        objectTemplate.object = state.objectsRoot->object;
        objectTemplate.isSuccess = true;

        // TX files should have only one object so any others will be freed/unloaded immediately.
        RaytmxObjectNode *objectIter = state.objectsRoot->next;
        while (objectIter != NULL)
        {
            FreeObject(objectIter->object);
            objectIter = objectIter->next;
        }
    }
    else TraceLog(LOG_WARNING, "RAYTMX: TX file (object template) \"%s\" does not contain any objects", fileName);

    if (state.tilesetsRoot != NULL) // If there is at least one tileset.
    {
        // Object templates may have a tileset. This is for cases where the object references a tile (i.e. its 'gid'
        // attribute is set).  Copy the root tileset so it can be returned.
        objectTemplate.tileset = state.tilesetsRoot->tileset;
        objectTemplate.hasTileset = true;

        // TX files should have at most one tileset so any others will be freed/unloaded immediately.
        RaytmxTilesetNode *tilesetsIter = state.tilesetsRoot->next;
        while (tilesetsIter != NULL)
        {
            FreeTileset(tilesetsIter->tileset);
            tilesetsIter = tilesetsIter->next;
        }
    }

    // Free the linked lists and zeroize related values.
    FreeState(&state);

    return objectTemplate;
}

void ParseDocument(RaytmxState *state, const char *fileName)
{
    char *content = LoadFileText(fileName);
    if (content == NULL)
    {
        TraceLog(LOG_ERROR, "RAYTMX: Failed to open \"%s\"", fileName);
        return;
    }
    size_t contentLength = strlen(content);

    StringCopy(state->documentDirectory, GetDirectoryPath2(fileName));

    hoxml_context_t hoxml = { 0 };
    size_t bufferLength = contentLength;
    char *buffer = (char *)MemAlloc((unsigned int)bufferLength);
    hoxml_init(&hoxml, buffer, bufferLength);

    hoxml_code_t code;
    while ((code = hoxml_parse(&hoxml, content, contentLength)) != HOXML_END_OF_DOCUMENT)
    {
        if (code > HOXML_END_OF_DOCUMENT) // If there's information about an element, attribute, whatever.
        {
            switch (code)
            {
                case HOXML_ELEMENT_BEGIN: HandleElementBegin(state, &hoxml); break;
                case HOXML_ELEMENT_END: HandleElementEnd(state, &hoxml); break;
                case HOXML_ATTRIBUTE: HandleAttribute(state, &hoxml);
                case HOXML_PROCESSING_INSTRUCTION_BEGIN: break;
                case HOXML_PROCESSING_INSTRUCTION_END: break;
                default: break; // No other cases to handle but compilers like to complain.
            }
        }
        else if (code < HOXML_END_OF_DOCUMENT) // If there was an error, recoverable or not.
        {
            switch (code)
            {
                case HOXML_ERROR_INSUFFICIENT_MEMORY:
                {
                    // This is one we can recover from by expanding the buffer. In this case, it will be doubled.
                    TraceLog(LOG_DEBUG, "RAYTMX: Allocating a new XML parsing buffer due to insufficient memory");
                    bufferLength *= 2;
                    char *newBuffer = (char *)MemAlloc((unsigned int)bufferLength);
                    hoxml_realloc(&hoxml, newBuffer, bufferLength);
                    MemFree(buffer);
                    buffer = newBuffer;
                } continue;
                case HOXML_ERROR_UNEXPECTED_EOF: TraceLog(LOG_ERROR, "RAYTMX: Unexpected end of file"); break;
                case HOXML_ERROR_SYNTAX:
                {
                    TraceLog(LOG_ERROR, "RAYTMX: Invalid syntax: line %d, column %d", hoxml.line, hoxml.column);
                } break;
                case HOXML_ERROR_ENCODING:
                {
                    TraceLog(LOG_ERROR, "RAYTMX: Character encoding error: line %d, column %d", hoxml.line,
                        hoxml.column);
                } break;
                case HOXML_ERROR_TAG_MISMATCH:
                {
                    TraceLog(LOG_ERROR, "RAYTMX: Close tag does not match open tag: line %d, column %d", hoxml.line,
                        hoxml.column);
                } break;
                case HOXML_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION:
                case HOXML_ERROR_INVALID_DOCUMENT_DECLARATION:
                {
                    TraceLog(LOG_ERROR, "RAYTMX: Document (type) declaration error: line %d, column %d", hoxml.line,
                        hoxml.column);
                } break;
                default: break; // Keep compilers happy.
            }

            UnloadFileText(content);
            return;
        }
    }

    UnloadFileText(content);
    MemFree(buffer);
    state->isSuccess = true;
}

void HandleElementBegin(RaytmxState *state, hoxml_context_t *hoxml)
{
    if ((state == NULL) || (hoxml == NULL)) return;

    if (strcmp(hoxml->tag, "map") == 0) // Unused. Included for readability.
        ;
    else if (strcmp(hoxml->tag, "properties") == 0)
    {
        // TMX allows nested properties but they are not (currently?) supported. To avoid memory leaks <properties>
        // depth is tracked.
        state->propertiesDepth += 1;
    }
    else if (strcmp(hoxml->tag, "property") == 0) state->property = AddProperty(state);
    else if (strcmp(hoxml->tag, "tileset") == 0) state->tileset = AddTileset(state);
    else if (strcmp(hoxml->tag, "image") == 0)
    {
        // If any of the elements that may have an image is/are open.
        if ((state->tilesetTile != NULL) || (state->tileset != NULL) || (state->imageLayer != NULL))
        {
            // If the open element already has an image.
            // Note: In all cases, the element can contain at most one <image>.
            if (((state->tilesetTile) != NULL && state->tilesetTile->hasImage) ||
                ((state->tileset != NULL) && state->tileset->hasImage) ||
                ((state->imageLayer != NULL) && state->imageLayer->hasImage))
            {
                TraceLog(LOG_WARNING, "RAYTMX: an element contained multiple images; the image on line %d will be "
                    "dropped", hoxml->line);
            }
            else
            {
                // <image> elements can be children of <tileset>, <tile>, or <imagelayer> elements.
                if (state->tilesetTile != NULL)
                {
                    state->tilesetTile->hasImage = true;
                    state->image = &(state->tilesetTile->image);
                }
                else if (state->tileset != NULL)
                {
                    state->tileset->hasImage = true;
                    state->image = &(state->tileset->image);
                }
                else if (state->imageLayer != NULL)
                {
                    state->imageLayer->hasImage = true;
                    state->image = &(state->imageLayer->image);
                }
            }
        }
    }
    else if (strcmp(hoxml->tag, "tile") == 0)
    {
        // <tile> elements can be children of <tileset> or <layer>. They are also not the same element in that they have
        // entirely different attributes and a tileset's <tile> may have children.
        if (state->tileset != NULL) state->tilesetTile = AddTilesetTile(state);
        // Layer <tile>s are added during attribute handling because they provide a GID attribute and nothing else.
    }
    else if (strcmp(hoxml->tag, "animation") == 0)
    {
        if (state->tilesetTile != NULL) state->tilesetTile->hasAnimation = true;
    }
    else if (strcmp(hoxml->tag, "frame") == 0) state->animationFrame = AddAnimationFrame(state);
    else if (strcmp(hoxml->tag, "layer") == 0)
    {
        // Allocate a new layer with 'tileLayer' allocated and append it to the current group, if it exists.
        state->layer = AddGenericLayer(state, false); // False means the layer is _not_ a group.
        state->layer->type = LAYER_TYPE_TILE_LAYER;
        state->tileLayer = &(state->layer->exact.tileLayer);
    }
    else if (strcmp(hoxml->tag, "objectgroup") == 0)
    {
        if (state->tilesetTile != NULL) // If the object group is a child of a <tile>, it's collision info.
        {
            state->objectGroup = &(state->tilesetTile->objectGroup);
            // Child objects (rectangles, points, ellipses, or polygons) are expected to follow.
        }
        else
        {
            // Allocate a new layer with 'objectGroup' allocated and append it to the current group, if it exists.
            state->layer = AddGenericLayer(state, false); // False means the layer is _not_ a group.
            state->layer->type = LAYER_TYPE_OBJECT_GROUP;
            state->objectGroup = &(state->layer->exact.objectGroup);
        }
    }
    else if (strcmp(hoxml->tag, "object") == 0)
    {
        // <object> elements are typically only allowable as children of <objectgroup>s but object templates, TX files,
        /// contain them as children of root <template> */
        if ((state->objectGroup != NULL) || (state->format == FORMAT_TX)) state->object = AddObject(state);
    }
    else if (strcmp(hoxml->tag, "ellipse") == 0)
    {
        if (state->object != NULL)
        {
            // An <ellipse> within an <object> indicates its type but the <object>'s 'x,' 'y,' 'width,' and 'height'
            // attributes are used to define the ellipse so assigning the type is all that's necessary.
            state->object->type = OBJECT_TYPE_ELLIPSE;
        }
    }
    else if (strcmp(hoxml->tag, "point") == 0)
    {
        if (state->object != NULL)
        {
            // A <point> within an <object> indicates its type but the <object>'s 'x' and 'y' attributes are used to
            // define the point so assigning the type is all that's necessary.
            state->object->type = OBJECT_TYPE_POINT;
        }
    }
    else if (strcmp(hoxml->tag, "polygon") == 0)
    {
        if (state->object != NULL)
        {
            // Note: <polygon>s and <polyline>s have a list of points/vertices defined in a 'points' attribute.
            state->object->type = OBJECT_TYPE_POLYGON;
        }
    }
    else if (strcmp(hoxml->tag, "polyline") == 0)
    {
        if (state->object != NULL)
        {
            // Note: <polyline>s and <polygone>s have a list of points/vertices defined in a 'points' attribute.
            state->object->type = OBJECT_TYPE_POLYLINE;
        }
    }
    else if (strcmp(hoxml->tag, "text") == 0)
    {
        if (state->object != NULL)
        {
            state->object->type = OBJECT_TYPE_TEXT;
            state->object->text = (TmxText *)MemAllocZero(sizeof(TmxText));

            // There are a couple non-zero default values for <text> attributes.
            state->object->text->pixelSize = 16;
            state->object->text->color.a = 255; // Full opacity black.
            state->object->text->kerning = 1;
            // The font family will also default to "sans-serif" when the element ends if there is no attribute.
        }
    }
    else if (strcmp(hoxml->tag, "imagelayer") == 0)
    {
        // Allocate a new layer with 'imageLayer' allocated and append it to the current group, if it exists.
        state->layer = AddGenericLayer(state, false); // False means the layer is _not_ a group.
        state->layer->type = LAYER_TYPE_IMAGE_LAYER;
        state->imageLayer = &(state->layer->exact.imageLayer);
    }
    else if (strcmp(hoxml->tag, "group") == 0)
    {
        // Allocate a new layer and append it to the current group, if it exists.
        state->layer = AddGenericLayer(state, true); // True means the layer is a group.
        state->layer->type = LAYER_TYPE_GROUP;
    }
}

void HandleAttribute(RaytmxState *state, hoxml_context_t *hoxml)
{
    if ((state == NULL) || (hoxml == NULL)) return;

    if (strcmp(hoxml->tag, "map") == 0)
    {
        if (strcmp(hoxml->attribute, "orientation") == 0)
        {
            if (strcmp(hoxml->value, "orthogonal") == 0) state->mapOrientation = ORIENTATION_ORTHOGONAL;
            else if (strcmp(hoxml->value, "isometric") == 0) state->mapOrientation = ORIENTATION_ISOMETRIC;
            else if (strcmp(hoxml->value, "staggered") == 0) state->mapOrientation = ORIENTATION_STAGGERED;
            else if (strcmp(hoxml->value, "hexagonal") == 0) state->mapOrientation = ORIENTATION_HEXAGONAL;
        }
        else if (strcmp(hoxml->attribute, "renderorder") == 0)
        {
            if (strcmp(hoxml->value, "right-down") == 0) state->mapRenderOrder = RENDER_ORDER_RIGHT_DOWN;
            else if (strcmp(hoxml->value, "right-up") == 0) state->mapRenderOrder = RENDER_ORDER_RIGHT_UP;
            else if (strcmp(hoxml->value, "left-down") == 0) state->mapRenderOrder = RENDER_ORDER_LEFT_DOWN;
            else if (strcmp(hoxml->value, "left-up") == 0) state->mapRenderOrder = RENDER_ORDER_LEFT_UP;
        }
        else if (strcmp(hoxml->attribute, "width") == 0) state->mapWidth = atoi(hoxml->value);
        else if (strcmp(hoxml->attribute, "height") == 0) state->mapHeight = atoi(hoxml->value);
        else if (strcmp(hoxml->attribute, "tilewidth") == 0) state->mapTileWidth = atoi(hoxml->value);
        else if (strcmp(hoxml->attribute, "tileheight") == 0) state->mapTileHeight = atoi(hoxml->value);
        else if (strcmp(hoxml->attribute, "parallaxoriginx") == 0) state->mapParallaxOriginX = atoi(hoxml->value);
        else if (strcmp(hoxml->attribute, "parallaxoriginy") == 0) state->mapParallaxOriginY = atoi(hoxml->value);
        else if (strcmp(hoxml->attribute, "backgroundcolor") == 0)
        {
            state->mapBackgroundColor = GetColorFromHexString(hoxml->value);
            state->mapHasBackgroundColor = true;
        }
    }
    else if (strcmp(hoxml->tag, "property") == 0)
    {
        if (state->property != NULL)
        {
            if (strcmp(hoxml->attribute, "name") == 0)
            {
                state->property->name = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->property->name, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "type") == 0)
            {
                if (strcmp(hoxml->value, "string") == 0) state->property->type = PROPERTY_TYPE_STRING;
                else if (strcmp(hoxml->value, "int") == 0) state->property->type = PROPERTY_TYPE_INT;
                else if (strcmp(hoxml->value, "float") == 0) state->property->type = PROPERTY_TYPE_FLOAT;
                else if (strcmp(hoxml->value, "bool") == 0) state->property->type = PROPERTY_TYPE_BOOL;
                else if (strcmp(hoxml->value, "color") == 0) state->property->type = PROPERTY_TYPE_COLOR;
                else if (strcmp(hoxml->value, "file") == 0) state->property->type = PROPERTY_TYPE_FILE;
                else if (strcmp(hoxml->value, "object") == 0) state->property->type = PROPERTY_TYPE_OBJECT;
                // TMX documentation also mentions a "class" type but doesn't describe what it is nor does Tiled list
                // it as an option when adding a property. So what is it? Unsupported, that's what.
            }
            else if (strcmp(hoxml->attribute, "value") == 0)
            {
                // Although unlikley, it's possible that 'value' attribute will be parsed before the 'type' attribute.
                // In that case, doing a cast/conversion now may not be possible. To avoid this, the raw string value is
                // copied to 'stringValue' temporarily, or permanently for string and file types, and the
                // cast/conversion will happen at the end of the element if needed.
                state->property->stringValue = (char *)MemAlloc((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->property->stringValue, hoxml->value);
            }
        }
    }
    else if (strcmp(hoxml->tag, "tileset") == 0)
    {
        if (state->tileset != NULL)
        {
            if (strcmp(hoxml->attribute, "firstgid") == 0) state->tileset->firstGid = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "source") == 0)
            {
                state->tileset->source = (char *)MemAlloc((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->tileset->source, hoxml->value);
                // 'source' points to an external TSX file that defines the majority of the tileset. Try to load it.
                RaytmxExternalTileset externalTileset = LoadTSX(JoinPath(state->documentDirectory, hoxml->value));
                if (externalTileset.isSuccess)
                {
                    // A <tileset> within a <map> will have two attributes: 'firstgid' and 'source.' The rest of the
                    // tileset's details are in the external TSX that 'source' points to. They need to be merged.
                    // Remember the two internal attributes.
                    uint32_t tempFirstGid = state->tileset->firstGid;
                    char *tempSource = state->tileset->source;
                    // Assign all values from the TSX's tileset to the one within the state object. This will overwrite
                    // the values of 'firstGid' and 'source'.
                    *(state->tileset) = externalTileset.tileset;
                    // Reassign the original 'firstGid' and 'source' values.
                    state->tileset->firstGid = tempFirstGid;
                    state->tileset->source = tempSource;
                }
            }
            else if (strcmp(hoxml->attribute, "name") == 0)
            {
                state->tileset->name = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->tileset->name, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "class") == 0)
            {
                state->tileset->classString = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->tileset->classString, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "tilewidth") == 0) state->tileset->tileWidth = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "tileheight") == 0) state->tileset->tileHeight = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "spacing") == 0) state->tileset->spacing = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "margin") == 0) state->tileset->margin = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "tilecount") == 0) state->tileset->tileCount = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "columns") == 0) state->tileset->columns = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "objectalignment") == 0)
            {
                if (strcmp(hoxml->value, "unspecified") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_UNSPECIFIED;
                else if (strcmp(hoxml->value, "topleft") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_TOP_LEFT;
                else if (strcmp(hoxml->value, "top") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_TOP;
                else if (strcmp(hoxml->value, "topright") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_TOP_RIGHT;
                else if (strcmp(hoxml->value, "left") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_LEFT;
                else if (strcmp(hoxml->value, "center") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_CENTER;
                else if (strcmp(hoxml->value, "right") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_RIGHT;
                else if (strcmp(hoxml->value, "bottomleft") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM_LEFT;
                else if (strcmp(hoxml->value, "bottom") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM;
                else if (strcmp(hoxml->value, "bottomright") == 0)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM_RIGHT;
            }
        }
    }
    else if (strcmp(hoxml->tag, "tileoffset") == 0)
    {
        if (state->tileset != NULL)
        {
            if (strcmp(hoxml->attribute, "x") == 0) state->tileset->tileOffsetX = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "y") == 0) state->tileset->tileOffsetY = atoi(hoxml->value);
        }
    }
    else if (strcmp(hoxml->tag, "image") == 0)
    {
        if (state->image != NULL)
        {
            if (strcmp(hoxml->attribute, "source") == 0)
            {
                state->image->source = (char*)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->image->source, hoxml->value);
                RaytmxCachedTextureNode *cachedTexture = LoadCachedTexture(state, hoxml->value);
                if (cachedTexture != NULL) state->image->texture = cachedTexture->texture;
            }
            else if (strcmp(hoxml->attribute, "trans") == 0)
            {
                state->image->trans = GetColorFromHexString(hoxml->value);
                state->image->hasTrans = true;
            }
            else if (strcmp(hoxml->attribute, "width") == 0) state->image->width = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "height") == 0) state->image->height = atoi(hoxml->value);
        }
    }
    else if (strcmp(hoxml->tag, "tile") == 0)
    {
        if (state->tilesetTile != NULL) // If the <tile> corresponds to a tileset tile.
        {
            if (strcmp(hoxml->attribute, "id") == 0) state->tilesetTile->id = atoi(hoxml->value);
            else if ((strcmp(hoxml->attribute, "type") == 0) || (strcmp(hoxml->attribute, "class") == 0))
            {
                state->tilesetTile->classString = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->tilesetTile->classString, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "x") == 0) state->tilesetTile->x = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "y") == 0) state->tilesetTile->y = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "width") == 0) state->tilesetTile->width = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "height") == 0) state->tilesetTile->height = atoi(hoxml->value);
        }
        else // If the <tile> corresponds to a layer tile.
            if (strcmp(hoxml->attribute, "gid") == 0) AddTileLayerTile(state, atoi(hoxml->value));
    }
    else if (strcmp(hoxml->tag, "frame") == 0)
    {
        if (state->animationFrame != NULL)
        {
            if (strcmp(hoxml->attribute, "tileid") == 0) state->animationFrame->gid = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "duration") == 0)
                state->animationFrame->duration = (float)atoi(hoxml->value) / 1000.0f;
        }
    }
    else if (strcmp(hoxml->tag, "layer") == 0)
    {
        if (state->tileLayer != NULL)
        {
            // Check for attributes specific to <layer> layers.
            if (strcmp(hoxml->attribute, "width") == 0) state->tileLayer->width = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "height") == 0) state->tileLayer->height = atoi(hoxml->value);
        }
    }
    else if (strcmp(hoxml->tag, "data") == 0)
    {
        if (state->tileLayer != NULL) // If this <data> applies to a <layer>.
        {
            if (strcmp(hoxml->attribute, "encoding") == 0)
            {
                state->tileLayer->encoding = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->tileLayer->encoding, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "compression") == 0)
            {
                state->tileLayer->compression = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->tileLayer->compression, hoxml->value);
            }
        }
        else if (state->image != NULL) // If this <data> applies to an <image>.
        {
            // TODO (?): TMX documentation says an <image> can contain a <data> element but doesn't provide any more
            // information than that. Tiled doesn't seem to have a feature for this either. Context suggests its the
            // image's binary data as it would exist in a file (e.g. a PNG), but it's not clear.
        }
    }
    else if (strcmp(hoxml->tag, "objectgroup") == 0)
    {
        if (state->objectGroup != NULL)
        {
            // Check for attributes specific to <objectgroup> layers */
            if (strcmp(hoxml->attribute, "color") == 0)
            {
                state->objectGroup->color = GetColorFromHexString(hoxml->value);
                state->objectGroup->hasColor = true;
            }
            // else if (strcmp(hoxml->attribute, "width") == 0)
            //    state->objectGroup->width = atoi(hoxml->value); // "Meaningless" according to documentation.
            // else if (strcmp(hoxml->attribute, "height") == 0)
            //    state->objectGroup->height = atoi(hoxml->value); // "Meaningless" according to documentation.
            else if (strcmp(hoxml->attribute, "draworder") == 0)
            {
                if (strcmp(hoxml->value, "index") == 0) state->objectGroup->drawOrder = OBJECT_GROUP_DRAW_ORDER_INDEX;
                else if (strcmp(hoxml->value, "topdown") == 0)
                    state->objectGroup->drawOrder = OBJECT_GROUP_DRAW_ORDER_TOP_DOWN;
           }
        }
    }
    else if (strcmp(hoxml->tag, "object") == 0)
    {
        if (state->object != NULL)
        {
            if (strcmp(hoxml->attribute, "id") == 0) state->object->id = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "name") == 0)
            {
                state->object->name = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->object->name, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "type") == 0)
            {
                state->object->typeString = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->object->typeString, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "x") == 0) state->object->x = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "y") == 0) state->object->y = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "width") == 0) state->object->width = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "height") == 0) state->object->height = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "rotation") == 0) state->object->rotation = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "gid") == 0)
            {
                state->object->gid = atoi(hoxml->value);
                // The presence of a 'gid' attribute also indicates the object's type is that of a tile.
                state->object->type = OBJECT_TYPE_TILE;
            }
            else if (strcmp(hoxml->attribute, "visible") == 0) state->object->visible = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "template") == 0)
            {
                state->object->templateString = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->object->templateString, hoxml->value);
            }
        }
    }
    else if ((strcmp(hoxml->tag, "polygon") == 0) || (strcmp(hoxml->tag, "polyline") == 0))
    {
        // <polygon> and <polyline>, children of <object>, both have just one attribute: 'points'.
        if ((state->object != NULL) && (strcmp(hoxml->attribute, "points") == 0))
        {
            if (state->object->points != NULL) // If there's already an array of points.
            {
                TraceLog(LOG_WARNING, "RAYTMX: object \"%s\", has multiple 'points' attributes; points listed in any "
                    "latter 'points' attributes will be dropped", state->object->name);
                return;
            }

            // The 'points' attribute's value is a string containing all points of the poly(gon|line) in the form
            // "0,0 31.25,-0.75 49.5,-16.5 93.25,-17.25" where the points are [0, 0], [31.25, -0.75], etc. and these
            // points are relative to the object's position (its 'x' and 'y' attributes).
            char x[32] = { 0 };
            char y[32] = { 0 };
            char *terminator = NULL;
            char *comma = NULL;
            char *iter = hoxml->value;
            RaytmxPolyPointNode *pointsRoot = NULL;
            RaytmxPolyPointNode *pointsTail = NULL;
            uint32_t pointsLength = 0;
            // Specific to polygons, the centroid is calculated requiring a sum of all vertices.
            Vector2 vertexSum = { 0.0f, 0.0f };
            while (iter != NULL)
            {
                terminator = strstr(iter, " "); // This pointer marks the end of the current point.
                if (terminator == NULL) // There are no space delimiters this far into the value string.
                {
                    // This is the end of the value string so we'll use a regular null terminator instead.
                    terminator = iter + strlen(iter);
                }

                comma = strstr(iter, ","); // This pointer marks the location of the comma between X and Y.
                if (comma == NULL)
                {
                    // The string is malformed. Cannot continue.
                    TraceLog(LOG_WARNING, "RAYTMX: The 'points' attribute on line %d has a malformed value; at least "
                        "one point is lost as a result", hoxml->line);
                    break;
                }

                // 'iter' is pointing to the first digit, 'comma' is pointing to the comma, and 'terminator' is pointing
                // to either the terminating space or a typical null terminator.
                StringCopyN(x, iter, comma - iter); // Copy from 'iter' up to but excluding 'comma'.
                x[comma - iter] = '\0';
                iter = comma + 1; // Point the iterator right after the ',' where Y begins.
                StringCopyN(y, iter, terminator - iter); // Copy 'iter' up to but excluding 'terminator'.
                y[terminator - iter] = '\0';

                // Create a linked list node to hold the point and append it to the linked list.
                RaytmxPolyPointNode *node = (RaytmxPolyPointNode *)MemAllocZero(sizeof(RaytmxPolyPointNode));
                // Note: These values may be negative. A poly(gon|line) object's position is determined by the first
                // vertex added leading to the first entry to be "0,0" and all other vertices relative to it.
                node->point.x = (float)atof(x);
                node->point.y = (float)atof(y);
                vertexSum.x += node->point.x;
                vertexSum.y += node->point.y;
                if (pointsRoot == NULL) pointsRoot = node; // If this is the first point to add to the list.
                else pointsTail->next = node; // If adding to the tail of the list.
                pointsTail = node;
                pointsLength += 1;
                iter = (terminator[0] != '\0')? terminator + 1 : NULL;
            }

            if (pointsRoot != NULL) // If a list with at least one node was created from the 'points' attribute.
            {
                // The first vertex will be duplicated and appended to the end of the list, for drawing purposes, so the
                // length of the points list is incremented by one.
                pointsLength += 1;

                const bool isPolygon = strcmp(hoxml->tag, "polygon") == 0;
                if (isPolygon) // If the object is a polygon, not polyline.
                {
                    // Polygons will be drawn using raylib's DrawTriangleFan() function in which the first point is the
                    // centroid. It must also end with the first, non-centroid point. So, for polygons, the list will
                    // have two more points.
                    pointsLength += 1;
                }

                // Allocate the array and assign NULL to every index to be safe.
                Vector2 *points = (Vector2 *)MemAllocZero(sizeof(Vector2)*pointsLength);
                if (isPolygon) // If the centroid should be included as a vertex.
                {
                    // Finish calculating the centroid by averaging the sum of the vertices keeping in mind that
                    // 'pointsLength' is equal to N + 2.
                    points[0].x = vertexSum.x/(pointsLength - 2);
                    points[0].y = vertexSum.y/(pointsLength - 2);
                }

                // Copy the points as Vector2s into the array and free the nodes while we're at it.
                RaytmxPolyPointNode *nodeIter = pointsRoot;
                uint32_t i = isPolygon? 1 : 0; // Skip over the first element, the centroid, for polygons only.
                while (nodeIter != NULL)
                {
                    points[i] = nodeIter->point;
                    RaytmxPolyPointNode *parent = nodeIter;
                    nodeIter = nodeIter->next;
                    i += 1;
                    MemFree(parent);
                }

                // End the list with the first point. Both polygons and polylines use this when drawing.
                points[pointsLength - 1].x = points[isPolygon? 1 : 0].x;
                points[pointsLength - 1].y = points[isPolygon? 1 : 0].y;
                // TODO: Sort the vertices into counter-clockwise order as DrawTriangleFan() requires it.
                state->object->points = points;
                state->object->pointsLength = pointsLength;
                state->object->drawPoints = (Vector2 *)MemAllocZero(sizeof(Vector2)*pointsLength);
            }
        }
    }
    else if (strcmp(hoxml->tag, "text") == 0)
    {
        if ((state->object != NULL) && (state->object->text != NULL))
        {
            if (strcmp(hoxml->attribute, "fontfamily") == 0)
            {
                state->object->text->fontFamily = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->object->text->fontFamily, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "pixelsize") == 0) state->object->text->pixelSize = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "wrap") == 0) state->object->text->wrap = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "color") == 0)
                state->object->text->color = GetColorFromHexString(hoxml->value);
            else if (strcmp(hoxml->attribute, "bold") == 0) state->object->text->bold = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "italic") == 0) state->object->text->italic = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "underline") == 0)
                state->object->text->underline = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "strikeout") == 0)
                state->object->text->strikeOut = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "kerning") == 0) state->object->text->kerning = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "halign") == 0)
            {
                if (strcmp(hoxml->value, "left") == 0) state->object->text->halign = HORIZONTAL_ALIGNMENT_LEFT;
                else if (strcmp(hoxml->value, "center") == 0) state->object->text->halign = HORIZONTAL_ALIGNMENT_CENTER;
                else if (strcmp(hoxml->value, "right") == 0) state->object->text->halign = HORIZONTAL_ALIGNMENT_RIGHT;
                else if (strcmp(hoxml->value, "justify") == 0)
                    state->object->text->halign = HORIZONTAL_ALIGNMENT_JUSTIFY;
            }
            else if (strcmp(hoxml->attribute, "valign") == 0)
            {
                if (strcmp(hoxml->value, "top") == 0) state->object->text->valign = VERTICAL_ALIGNMENT_TOP;
                else if (strcmp(hoxml->value, "center") == 0) state->object->text->valign = VERTICAL_ALIGNMENT_CENTER;
                else if (strcmp(hoxml->value, "bottom") == 0) state->object->text->valign = VERTICAL_ALIGNMENT_BOTTOM;
            }
        }
    }
    else if (strcmp(hoxml->tag, "imagelayer") == 0)
    {
        if (state->imageLayer != NULL)
        {
            // Check for attributes specific to <imagelayer> layers.
            if (strcmp(hoxml->attribute, "repeatx") == 0) state->imageLayer->repeatX = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "repeaty") == 0) state->imageLayer->repeatY = atoi(hoxml->value) != 0;
        }
    }

    if ((strcmp(hoxml->tag, "layer") == 0) || (strcmp(hoxml->tag, "objectgroup") == 0) ||
        (strcmp(hoxml->tag, "imagelayer") == 0) || (strcmp(hoxml->tag, "group") == 0))
    {
        if (state->layer != NULL)
        {
            // Check for attributes common to all layer types.
            if (strcmp(hoxml->attribute, "id") == 0) state->layer->id = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "name") == 0)
            {
                state->layer->name = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->layer->name, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "class") == 0)
            {
                state->layer->classString = (char *)MemAllocZero((unsigned int)strlen(hoxml->value) + 1);
                StringCopy(state->layer->classString, hoxml->value);
            }
            else if (strcmp(hoxml->attribute, "opacity") == 0) state->layer->opacity = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "visible") == 0) state->layer->visible = atoi(hoxml->value) != 0;
            else if (strcmp(hoxml->attribute, "tintcolor") == 0)
            {
                state->layer->tintColor = GetColorFromHexString(hoxml->value);
                state->layer->hasTintColor = true;
            }
            else if (strcmp(hoxml->attribute, "offsetx") == 0) state->layer->offsetX = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "offsety") == 0) state->layer->offsetY = atoi(hoxml->value);
            else if (strcmp(hoxml->attribute, "parallaxx") == 0) state->layer->parallaxX = atof(hoxml->value);
            else if (strcmp(hoxml->attribute, "parallaxy") == 0) state->layer->parallaxY = atof(hoxml->value);
        }
    }
}

void HandleElementEnd(RaytmxState *state, hoxml_context_t *hoxml)
{
    if ((state == NULL) || (hoxml == NULL)) return;

    // If the element is one of the layer types which share some common attributes that may need default strings.
    if ((strcmp(hoxml->tag, "layer") == 0) || (strcmp(hoxml->tag, "objectgroup") == 0) ||
        (strcmp(hoxml->tag, "imagelayer") == 0) || (strcmp(hoxml->tag, "group") == 0))
    {
        TmxLayer *layer = state->layer;
        if ((layer == NULL) && (state->groupNode != NULL)) layer = &(state->groupNode->layer);

        if (layer != NULL)
        {
            // Apply default values for the attribute(s) that have default values that aren't zero or null.
            if (layer->name == NULL) // If this layer didn't have a 'name' attribute.
            {
                // The default value for 'name' is "" (an empty string).
                layer->name = (char *)MemAlloc(1);
                layer->name[0] = '\0';
            }
            if (layer->classString == NULL) // If this layer didn't have a 'class' attribute.
            {
                // The default value for 'class' is "" (an empty string).
                layer->classString = (char *)MemAlloc(1);
                layer->classString[0] = '\0';
            }
        }
    }

    if (strcmp(hoxml->tag, "properties") == 0)
    {
        if (state->propertiesRoot == NULL) return;

        // TMX allows nested properties (e.g. <properties><properties><property/></properties></properties>) but that is
        // unsupported. Depth is tracked such that all properties, included nested ones, are appended to one array when
        // the outermost element ends.
        state->propertiesDepth -= 1;
        if (state->propertiesDepth > 0) return; // If the outermost <properties> has not yet ended.

        // Allocate the array and assign NULL to every index to be safe.
        TmxProperty *properties = (TmxProperty *)MemAllocZero(sizeof(TmxProperty)*state->propertiesLength);

        // Copy the TmxProperty pointers into the array and free the nodes while we're at it.
        RaytmxPropertyNode *iter = state->propertiesRoot;
        for (uint32_t i = 0; iter != NULL; i++)
        {
            properties[i] = iter->property;
            RaytmxPropertyNode *parent = iter;
            iter = iter->next;
            MemFree(parent);
        }

        // Add the properties array to the element it applies to.
        // A <property>, or rather its parent <properties>, can be within 10+ other elements. The order of the checks
        // here is slightly important as the most-nested elements must come first in cases where one may contain another
        // (e.g. an <object> within an <objectgroup> layer).
        if ((state->tilesetTile != NULL) && (state->tilesetTile->properties == NULL))
        {
            state->tilesetTile->properties = properties;
            state->tilesetTile->propertiesLength = state->propertiesLength;
        }
        // else if ((state->wangColor != NULL) && (state->wangColor->properties == NULL))
        // {
        //     state->wangColor->properties = properties;
        //     state->wangColor->propertiesLength = state->propertiesLength;
        // }
        // else if ((state->wangSet != NULL) && (state->wangSet->properties == NULL))
        // {
        //     state->wangSet->properties = properties;
        //     state->wangSet->propertiesLength = state->propertiesLength;
        // }
        else if ((state->tileset != NULL) && (state->tileset->properties == NULL))
        {
            state->tileset->properties = properties;
            state->tileset->propertiesLength = state->propertiesLength;
        }
        else if ((state->object != NULL) && (state->object->properties == NULL))
        {
            state->object->properties = properties;
            state->object->propertiesLength = state->propertiesLength;
        }
        else if ((state->layer != NULL) && (state->layer->properties == NULL))
        {
            state->layer->properties = properties;
            state->layer->propertiesLength = state->propertiesLength;
        }
        else
        {
            state->mapProperties = properties;
            state->mapPropertiesLength = state->propertiesLength;
        }

        // Clean up the state object.
        state->propertiesRoot = NULL;
        state->propertiesTail = NULL;
        state->propertiesLength = 0;
    }
    else if (strcmp(hoxml->tag, "property") == 0)
    {
        if (state->property != NULL)
        {
            // Apply default values for the attribute(s) that have default values that aren't zero or null.
            // Properties are cast and assigned to type-specific variables at the end of the element due to the order of
            // the property's attributes not being guaranteed, so we wait until we have all the information.
            switch (state->property->type)
            {
                case PROPERTY_TYPE_STRING:
                default: // The default type of a property is 'string'.
                {
                    if (state->property->stringValue == NULL)
                    {
                        if (hoxml->content != NULL) // If Tiled opted to put the value in the element's content.
                        {
                            // From the documentation: "When a string property contains newlines, the current version of
                            // Tiled will write out the value as characters contained inside the property element rather
                            // than as the value attribute."
                            state->property->stringValue = (char *)MemAlloc((unsigned int)strlen(hoxml->content) + 1);
                            StringCopy(state->property->stringValue, hoxml->content);
                        }
                        else // If the string's value was neither provided as an attribute nor content.
                        {
                            // The default value for 'string' is an empty string.
                            state->property->stringValue = (char *)MemAlloc(1);
                            state->property->stringValue[0] = '\0';
                        }
                    }
                } break;
                case PROPERTY_TYPE_INT:
                case PROPERTY_TYPE_OBJECT:
                {
                    // The default value for 'int' and 'object' is zero.
                    state->property->intValue = atoi(state->property->stringValue); // atoi() defaults to 0.
                } break;
                case PROPERTY_TYPE_FLOAT:
                {
                    // The default value for 'float' is zero.
                    state->property->floatValue = (float)atof(state->property->stringValue); // atof() defaults to 0.0f.
                } break;
                case PROPERTY_TYPE_BOOL:
                {
                    // The default value for 'bool' is false.
                    if ((state->property->stringValue == NULL) || (strcmp(state->property->stringValue, "true") != 0))
                        state->property->boolValue = false;
                    else state->property->boolValue = true;
                } break;
                case PROPERTY_TYPE_COLOR:
                {
                    // The default value for 'color' is the color #00000000.
                    if (state->property->stringValue == NULL) state->property->colorValue = BLANK; // { 0, 0, 0, 0 }.
                    else state->property->colorValue = GetColorFromHexString(state->property->stringValue);
                } break;
                case PROPERTY_TYPE_FILE:
                {
                    // The default value for 'file' is ".".
                    if (state->property->stringValue == NULL)
                    {
                        state->property->stringValue = (char *)MemAlloc(2);
                        state->property->stringValue[0] = '.';
                        state->property->stringValue[1] = '\0';
                    }
                } break;
            }

            // If the type was neither 'string' nor 'file' and 'stringValue' is set.
            if ((state->property->type != PROPERTY_TYPE_STRING) && (state->property->type != PROPERTY_TYPE_FILE) &&
                (state->property->stringValue != NULL))
            {
                // Properties of types other than 'string' and 'file' are placed in 'stringValue' temporarily. Now that
                // they have been cast and assigned appropriately, 'stringValue' can be freed.
                MemFree(state->property->stringValue);
                state->property->stringValue = NULL;
            }
        }
        state->property = NULL;
    }
    else if (strcmp(hoxml->tag, "tileset") == 0)
    {
        if (state->tileset != NULL)
        {
            // Apply default values for the attribute(s) that have default values that aren't zero or null.
            if (state->tileset->name == NULL) // If this <tileset> didn't have a 'name' attribute.
            {
                // The default value for 'name' is "" (an empty string).
                state->tileset->name = (char *)MemAlloc(1);
                state->tileset->name[0] = '\0';
            }

            if (state->tileset->classString == NULL) // If this <tileset> didn't have a 'class' attribute.
            {
                // The default value for 'class' is "" (an empty string).
                state->tileset->classString = (char *)MemAlloc(1);
                state->tileset->classString[0] = '\0';
            }

            if (state->tileset->objectAlignment == OBJECT_ALIGNMENT_UNSPECIFIED)
            {
                // There are default object alignments for orthogonal and isometric modes.
                if (state->mapOrientation == ORIENTATION_ORTHOGONAL)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM_LEFT;
                else if (state->mapOrientation == ORIENTATION_ISOMETRIC)
                    state->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM;
            }

            if (state->tilesetTilesRoot != NULL)
            {
                // Allocate the array and zeroize every index as initialization.
                TmxTilesetTile *tiles =
                    (TmxTilesetTile *)MemAllocZero(sizeof(TmxTilesetTile)*state->tilesetTilesLength);
                // Copy the TmxTilesetTile pointers into the array and free the nodes while we're at it.
                RaytmxTilesetTileNode *iter = state->tilesetTilesRoot;
                for (uint32_t i = 0; (i < state->tilesetTilesLength) && (iter != NULL); i++)
                {
                    tiles[i] = iter->tile;
                    RaytmxTilesetTileNode *parent = iter;
                    iter = iter->next;
                    MemFree(parent);
                }

                // Add the tiles array to the tileset.
                state->tileset->tiles = tiles;
                state->tileset->tilesLength = state->tilesetTilesLength;

                // Clean up the state object.
                state->tilesetTilesRoot = NULL;
                state->tilesetTilesTail = NULL;
                state->tilesetTilesLength = 0;
            }
        }
        state->tileset = NULL;
    }
    else if (strcmp(hoxml->tag, "image") == 0) state->image = NULL;
    else if (strcmp(hoxml->tag, "animation") == 0)
    {
        if ((state->tilesetTile != NULL) && state->tilesetTile->hasAnimation)
        {
            if (state->animationFramesRoot == NULL) return;

            // Allocate the array and zeroize every index as initialization.
            TmxAnimationFrame *frames =
                (TmxAnimationFrame *)MemAllocZero(sizeof(TmxAnimationFrame)*state->animationFramesLength);
            // Copy the TmxAnimationFrame pointers into the array and free the nodes while we're at it.
            RaytmxAnimationFrameNode *iter = state->animationFramesRoot;
            for (uint32_t i = 0; iter != NULL; i++)
            {
                frames[i] = iter->frame;
                RaytmxAnimationFrameNode *parent = iter;
                iter = iter->next;
                MemFree(parent);
            }

            // Add the frames array to the tile's animation.
            state->tilesetTile->animation.frames = frames;
            state->tilesetTile->animation.framesLength = state->animationFramesLength;

            // Clean up the state object.
            state->animationFramesRoot = NULL;
            state->animationFramesTail = NULL;
            state->animationFramesLength = 0;
        }
    }
    else if (strcmp(hoxml->tag, "frame") == 0) state->animationFrame = NULL;
    else if (strcmp(hoxml->tag, "layer") == 0)
    {
        if (state->tileLayer != NULL)
        {
            // If there were 1+ <tile>s within this <layer> but this <layer> already has tiles (from a <data>?).
            if ((state->layerTilesRoot != NULL) && (state->tileLayer->tiles != NULL))
            {
                TraceLog(LOG_WARNING, "RAYTMX: layer \"%s\" has more than one source of tile data - the latter tiles "
                    "for this layer will be dropped", state->layer->name);
                // Free the nodes and tiles therein.
                RaytmxTileLayerTileNode *iter = state->layerTilesRoot;
                while (iter != NULL)
                {
                    RaytmxTileLayerTileNode *parent = iter;
                    iter = iter->next;
                    MemFree(parent);
                }
            }
            else
            {
                // Allocate the array and zeroize every index as initialization.
                uint32_t *tiles = (uint32_t *)MemAllocZero(sizeof(uint32_t)*state->layerTilesLength);
                // Copy the GID into the array and free the nodes while we're at it.
                RaytmxTileLayerTileNode *iter = state->layerTilesRoot;
                for (uint32_t i = 0;  iter != NULL; i++)
                {
                    tiles[i] = iter->gid;
                    RaytmxTileLayerTileNode *parent = iter;
                    iter = iter->next;
                    MemFree(parent);
                }

                // Add the tiles array to the tile layer.
                state->tileLayer->tiles = tiles;
                state->tileLayer->tilesLength = state->layerTilesLength;
            }

            // Clean up the state object.
            state->layerTilesRoot = NULL;
            state->layerTilesTail = NULL;
            state->layerTilesLength = 0;
        }
        state->tileLayer = NULL;
        state->layer = NULL;
    }
    else if (strcmp(hoxml->tag, "tile") == 0)
    {
        if (state->tilesetTile != NULL)
        {
            // Apply default values for the attribute(s) that have default values that aren't zero or null.
            if (state->tilesetTile->classString == NULL) // If this <tile> didn't have a 'class' attribute.
            {
                // The default value for 'class' is "" (an empty string).
                state->tilesetTile->classString = (char *)MemAlloc(1);
                state->tilesetTile->classString[0] = '\0';
            }

            if (state->tilesetTile->hasImage)
            {
                // The 'width' and 'height' attributes default to the tile's image's width and height, respectively.
                if (state->tilesetTile->width == 0) state->tilesetTile->width = state->tilesetTile->image.width;
                if (state->tilesetTile->height == 0) state->tilesetTile->height = state->tilesetTile->image.height;
            }
            state->tilesetTile = NULL;
        }
    }
    else if (strcmp(hoxml->tag, "data") == 0)
    {
        if (state->image != NULL)
        {
            // TODO (?): If a child <data> element of an <image> is parsed in some future version, it will almost
            // certainly need to be freed here.
        }
        else if ((state->tileLayer != NULL) && (state->tileLayer->tiles != NULL))
        {
            TraceLog(LOG_WARNING, "RAYTMX: layer \"%s\" has more than one source of tile data - the latter tiles for "
                "this layer will be dropped", state->layer->name);
        }
        else if ((state->tileLayer != NULL) && (state->tileLayer->encoding != NULL))
        {
            RaytmxTileLayerTileNode *tiles = NULL;

            if (strcmp(state->tileLayer->encoding, "base64") == 0)
            {
                // The layer's data is a series of unsigned, 32-bit integers encoded as a Base64 string. But, XML
                // considers everything between <data> and </data> to be content meaning there is probably some
                // whitespace on both ends of the content we need to ignore. So, find the actual start and stop:.
                char *encodedStart = hoxml->content;
                while (isspace(*encodedStart)) encodedStart++;
                char *encodedEnd = encodedStart + strlen(encodedStart) - 1;
                while ((encodedEnd > encodedStart) && isspace(*encodedEnd)) encodedEnd--;

                // With the string of encoded Base64 data trimmed, decode it.
                int decodedLength = 0;
                unsigned char *decoded = DecodeDataBase64((const unsigned char *)encodedStart, &decodedLength);
                if (decoded != NULL)
                {
                    if (state->tileLayer->compression == NULL) // If the Base64-encoded data is uncompressed.
                    {
                        // Iterate through N bytes ('decodedLength') with every four bytes being a single GID resulting
                        // in N / 4 tiles.
                        uint32_t *iter = (uint32_t *)decoded;
                        for (int i = 0; i < decodedLength/4; i++)
                        {
                            AddTileLayerTile(state, *iter);
                            iter += 1; // Point to the next unsigned, 32-bit integer in the decoded data.
                        }
                    }
                    else // If the Base-64encoded data is also compressed.
                    {
                        if ((strcmp(state->tileLayer->compression, "gzip") == 0) ||
                            (strcmp(state->tileLayer->compression, "zlib") == 0))
                        {
                            unsigned char *postHeaderDecoded = NULL;
                            if (strcmp(state->tileLayer->compression, "gzip") == 0)
                            {
                                // The first two bytes of a GZIP header are expected to be a magic number, 0x1F8B,
                                // identifying the format and the third is expected to indicate the compression method
                                // where 0x08 is DEFLATE.
                                // If these values are found, decompression can continue.
                                if ((decoded[0] == 0x1F) && (decoded[1] == 0x8B) && (decoded[2] == 0x08))
                                {
                                    // Skip past the GZIP header. The header is typically ten bytes. The bytes not
                                    // checked are unimportant things like a timestamp and OS ID. Additional optional
                                    // headers are possible but not used by Tiled so they are assumed to be missing.
                                    postHeaderDecoded = decoded + 10;
                                }
                                else // If the GZIP header doesn't match a decompressable one.
                                {
                                    TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" uses GZIP compression but the stream's "
                                        "header doesn't indicate DEFLATE compression", state->layer->name);
                                }
                            }
                            else // if (strcmp(state->tileLayer->compression, "zlib") == 0)
                            {
                                // The first byte of a ZLIB header is expected to be 0x78 where the 8 indicates the
                                // DEFLATE compression method and the 7 is "compression info" that indicates a 32K LZ77
                                // window size and, in practice, cannot be anything else.
                                // If these values are found, decompression can continue.
                                if (decoded[0] == 0x78)
                                {
                                    // Skip past the ZLIB header. The header is two bytes.
                                    postHeaderDecoded = decoded + 2;
                                }
                                else // If the ZLIB header doesn't match a decompressable one.
                                {
                                    TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" uses ZLIB compression but the stream's "
                                        "header doesn't indicate DEFLATE compression", state->layer->name);
                                }
                            }

                            if (postHeaderDecoded != NULL)
                            {
                                // "zlib" and "gzip" both use the DEFLATE algorithm and raylib provides a decompression
                                // function when it's built with SUPPORT_COMPRESSION_API (default).
                                int decompressedLength = 0;
                                unsigned char *decompressed = DecompressData(postHeaderDecoded, decodedLength,
                                    &decompressedLength);
                                if ((decompressed != NULL) && (decompressedLength > 0))
                                {
                                    uint32_t *iter = (uint32_t *)decompressed;
                                    for (int i = 0; i < decompressedLength/4; i++)
                                    {
                                        AddTileLayerTile(state, *iter);
                                        iter += 1; // Point to the next unsigned integer in the decompressed data.
                                    }
                                    MemFree(decompressed); // Free the memory allocated by DecompressData().
                                }
                                else // raylib wasn't built with compression or allocation failed.
                                {
                                    TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" compressed with \"%s\" cannot be parsed "
                                        "because DEFLATE decompression failed - either raylib was not built with "
                                        "SUPPORT_COMPRESSION_API or memory allocation failed", state->layer->name,
                                        state->tileLayer->compression);
                                }
                            }
                        }
                        else
                        {
                            TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" cannot be parsed because the compression method "
                                "\"%s\" is unsupported", state->layer->name, state->tileLayer->compression);
                        }
                    }
                    MemFree(decoded); // Free the memory allocated by DecodeDataBase64().
                }
                else TraceLog(LOG_ERROR, "RAYTMX: Unable to decode Base64 data for layer \"%s\"", state->layer->name);
            }
            else if (strcmp(state->tileLayer->encoding, "csv") == 0)
            {
                // The Comma-Separated Value (CSV) list herein is a series of Global IDs (GIDs) of tiles in the form
                // "31,32,33" where 31, 32, and 33 are GIDs.
                char valueStr[16] = { 0 }; // Must fit all digits of a single value. 16 should be more than enough.
                char *iter = hoxml->content;
                while ((iter != NULL) && (*iter != '\0')) // While not pointing to the end of the string.
                {
                    memset(valueStr, '\0', 16); // Fill the value-as-a-string buffer with null terminators.

                    // Copy each character into the buffer until either a comma or the terminator is reached.
                    for (int i = 0; (*iter != ',') && (*iter != '\0'); i++)
                    {
                        memcpy(valueStr + i, iter, 1);
                        iter++;
                    }

                    // If iteration was paused by a comma, move to the next character so it's ignored next iteration.
                    if (*iter == ',') iter++;

                    AddTileLayerTile(state, atoi(valueStr)); // Read the value as an integer GID.
                }
            }

            if (tiles != NULL) // If there was no error in parsing the data and there's a linked list of tiles.
            {
                uint32_t *tiles = (uint32_t *)MemAllocZero(sizeof(uint32_t)*state->layerTilesLength);

                // Copy the GIDs into the array and free the nodes while we're at it.
                RaytmxTileLayerTileNode *layerTilesIter = state->layerTilesRoot;
                for (uint32_t i = 0; (i < state->layerTilesLength) && (layerTilesIter != NULL); i++)
                {
                    tiles[i] = layerTilesIter->gid;
                    RaytmxTileLayerTileNode *layerTilesTemp = layerTilesIter;
                    layerTilesIter = layerTilesIter->next;
                    MemFree(layerTilesTemp);
                }

                // Add the tiles array to the element it applies to.
                state->tileLayer->tiles = tiles;
                state->tileLayer->tilesLength = state->layerTilesLength;

                // Clean up the state object.
                state->layerTilesRoot = NULL;
                state->layerTilesTail = NULL;
                state->layerTilesLength = 0;
            }
        }
    }
    else if (strcmp(hoxml->tag, "objectgroup") == 0)
    {
        if (state->objectGroup != NULL)
        {
            if (state->objectsRoot == NULL) return;

            // Allocate the arrays and zeroize every index as initialization.
            TmxObject *objects = (TmxObject *)MemAllocZero(sizeof(TmxObject)*state->objectsLength);
            uint32_t *ySortedObjects = (uint32_t *)MemAllocZero(sizeof(uint32_t)*state->objectsLength);

            // Create a contiguous array of TmxObjects, create a sorted linked list of indexes within that array of
            // TmxObjects (sorted by ascending Y coordinate), and free the object linked list.
            RaytmxObjectNode *objectsIter = state->objectsRoot;
            RaytmxObjectNode *objectsTemp = NULL;
            RaytmxObjectSortingNode *sortingRoot = NULL;
            RaytmxObjectSortingNode *sortingIter = NULL;
            RaytmxObjectSortingNode *sortingTemp = NULL;
            RaytmxObjectSortingNode *newSortingNode = NULL;
            for (uint32_t i = 0; objectsIter != NULL; i++)
            {
                objects[i] = objectsIter->object;

                // Add a new node into the sorted list.
                newSortingNode = (RaytmxObjectSortingNode *)MemAllocZero(sizeof(RaytmxObjectSortingNode));
                newSortingNode->y = objects[i].y;
                newSortingNode->index = i;
                if (sortingRoot == NULL) sortingRoot = newSortingNode; // If this is the first node.
                else if (sortingRoot->y >= newSortingNode->y) // If replacing the root node.
                {
                    newSortingNode->next = sortingRoot;
                    sortingRoot = newSortingNode;
                }
                else // If inserting at some location after the root.
                {
                    sortingIter = sortingRoot;
                    while ((sortingIter->next != NULL) && (newSortingNode->y > sortingIter->next->y))
                        sortingIter = sortingIter->next;
                    newSortingNode->next = sortingIter->next;
                    sortingIter->next = newSortingNode;
                }

                // Free the object node.
                objectsTemp = objectsIter;
                objectsIter = objectsIter->next;
                MemFree(objectsTemp);
            }

            // Create a contiguous array from the sorted linked list such that index 0 of this array points to the
            // TmxObject (via its index in 'objects') with the lowest (visually, highest) Y coordinate.
            sortingIter = sortingRoot;
            for (uint32_t i = 0; sortingIter != NULL; i++)
            {
                ySortedObjects[i] = sortingIter->index;
                sortingTemp = sortingIter;
                sortingIter = sortingIter->next;
                MemFree(sortingTemp);
            }

            // Add the 'objects' and 'ySortedObjects' array to the object layer.
            state->objectGroup->objects = objects;
            state->objectGroup->objectsLength = state->objectsLength;
            state->objectGroup->ySortedObjects = ySortedObjects;

            // Clean up the state object.
            state->objectsRoot = NULL;
            state->objectsTail = NULL;
            state->objectsLength = 0;
        }
        state->objectGroup = NULL;
        state->layer = NULL;
    }
    else if (strcmp(hoxml->tag, "object") == 0)
    {
        if (state->object != NULL)
        {
            // Apply default values for the attribute(s) that have default values that aren't zero or null.
            if (state->object->name == NULL) // If this <object> didn't have a 'name' attribute.
            {
                // The default value for 'name' is "" (an empty string).
                state->object->name = (char *)MemAlloc(1);
                state->object->name[0] = '\0';
            }

            if (state->object->typeString == NULL) // If this <object> didn't have a 'type' attribute.
            {
                // The default value for 'type' is "" (an empty string).
                state->object->typeString = (char *)MemAlloc(1);
                state->object->typeString[0] = '\0';
            }

            if (state->object->templateString != NULL)
            {
                // During editing, objects can be created from templates with pre-defined values and/or <properties>, a
                // concept similar to copying and pasting or instancing. These templates are saved in external TX files
                // that the 'template' attribute points to. Try to load it.
                RaytmxCachedTemplateNode *cachedTemplate = LoadCachedTemplate(state, state->object->templateString);
                if (cachedTemplate != NULL)
                {
                    const RaytmxObjectTemplate objectTemplate = cachedTemplate->objectTemplate;

                    // The template's <object> will contain some subset of values and/or <properties> that apply to the
                    // instanced <object> referencing it. For example, if the template's <object> has a 'name' attribute
                    // with the value "cactus" then the instanced <object> will also have a 'name' attribute. "cactus"
                    // is a default value that will be used if the instanced <object> does not define one of its own.
                    // The template's <object> needs to be checked for non-default values and <properties> and they need
                    // to be applied to the instanced <object> where none exist.
                    if ((objectTemplate.object.name != NULL) && (state->object->name == NULL))
                    {
                        state->object->name =
                            (char *)MemAllocZero((unsigned int)strlen(objectTemplate.object.name) + 1);
                        StringCopy(state->object->name, objectTemplate.object.name);
                    }

                    if ((objectTemplate.object.typeString != NULL) && (state->object->typeString != NULL))
                    {
                        state->object->typeString =
                            (char *)MemAllocZero((unsigned int)strlen(objectTemplate.object.typeString) + 1);
                        StringCopy(state->object->typeString, objectTemplate.object.typeString);
                    }

                    if ((objectTemplate.object.x != 0.0) && (state->object->x == 0.0))
                        state->object->x = objectTemplate.object.x;
                    if ((objectTemplate.object.y != 0.0) && (state->object->y == 0.0))
                        state->object->y = objectTemplate.object.y;
                    if ((objectTemplate.object.width != 0.0) && (state->object->width == 0.0))
                        state->object->width = objectTemplate.object.width;
                    if ((objectTemplate.object.height != 0.0) && (state->object->height == 0.0))
                        state->object->height = objectTemplate.object.height;
                    if ((objectTemplate.object.rotation != 0.0) && (state->object->rotation == 0.0))
                        state->object->rotation = objectTemplate.object.rotation;
                    if ((objectTemplate.object.gid != 0) && (state->object->gid == 0))
                        state->object->gid = objectTemplate.object.gid;
                    if (!objectTemplate.object.visible && state->object->visible) state->object->visible = false;

                    // TMX documentation doesn't specify some particulars of templates, namely if it is possible for a
                    // template to be type-specific. In other words, can a template be a polygon? If so, are points
                    // merged or replaced? Based on Tiled's behavior, templates do not have type-specific values or, at
                    // least, they are ignored if they exist. However, <properties> are definitely merged.
                    // If the template's <object> has <properties>.
                    if (objectTemplate.object.properties != NULL)
                    {
                        // There are two cases here: the instanced <object> already has properties, or it doesn't.
                        if (state->object->properties == NULL) // If the instance doesn't have <properties>.
                        {
                            // This is the easy case. Just copy the pointer to the existing array.
                            state->object->properties = objectTemplate.object.properties;
                            state->object->propertiesLength = objectTemplate.object.propertiesLength;
                        }
                        else
                        {
                            // The two <properties> need to be merged keeping in mind that they may, or probably, have
                            // overlapping entries. (When both have properties of the same name, the instanced
                            // <object>'s takes priority.) Create a linked list from which to create this new, merged
                            // properties array.
                            RaytmxPropertyNode *propertiesRoot = NULL;
                            RaytmxPropertyNode *propertiesTail = NULL;
                            RaytmxPropertyNode *node = NULL;
                            uint32_t propertiesLength = 0;
                            // Add the properties from the instanced <object>.
                            for (uint32_t i = 0; i < state->object->propertiesLength; i++)
                            {
                                node = (RaytmxPropertyNode *)MemAllocZero(sizeof(RaytmxPropertyNode));
                                node->property = state->object->properties[i];
                                if (propertiesRoot == NULL) propertiesRoot = node;
                                else propertiesTail->next = node;
                                propertiesTail = node;
                                propertiesLength += 1;
                            }

                            // Add the properties from the template's <object> if they do not already exist.
                            for (uint32_t i = 0; i < objectTemplate.object.propertiesLength; i++)
                            {
                                RaytmxPropertyNode *propertiesIter = propertiesRoot;
                                bool isNew = true;
                                while (propertiesIter != NULL)
                                {
                                    if (strcmp(objectTemplate.object.properties[i].name,
                                        propertiesIter->property.name) == 0)
                                    {
                                        isNew = false;
                                        break;
                                    }
                                    propertiesIter = propertiesIter->next;
                                }

                                if (isNew)
                                {
                                    node = (RaytmxPropertyNode *)MemAllocZero(sizeof(RaytmxPropertyNode));
                                    node->property = objectTemplate.object.properties[i];
                                    if (propertiesRoot == NULL) propertiesRoot = node;
                                    else propertiesTail->next = node;
                                    propertiesTail = node;
                                    propertiesLength += 1;
                                }
                            }

                            // Free the separate array that was previously allocated.
                            MemFree(state->object->properties);

                            // Allocate a new array to be populated with the merged properties */
                            state->object->properties =
                                (TmxProperty *)MemAllocZero(sizeof(TmxProperty)*propertiesLength);
                            state->object->propertiesLength = propertiesLength;

                            // Copy the TmxProperty entires into the array and free the nodes while we're at it.
                            RaytmxPropertyNode *propertiesIter = propertiesRoot;
                            for (uint32_t i = 0; propertiesIter != NULL; i++)
                            {
                                state->object->properties[i] = propertiesIter->property;
                                RaytmxPropertyNode *propertiesTemp = propertiesIter;
                                propertiesIter = propertiesIter->next;
                                MemFree(propertiesTemp);
                            }
                        }
                    }
                }
                else TraceLog(LOG_WARNING, "RAYTMX: Unable to apply template to object ID %u", state->object->id);
            }

            // Calculate the Axis-Aligned Bounding Box (AABB) of the object. This can vary by object type. Due to the
            // possibility of 'width' and 'height' being derived from a template, these must be calculated after a
            // template is applied to the object.
            switch (state->object->type)
            {
                case OBJECT_TYPE_RECTANGLE:
                case OBJECT_TYPE_ELLIPSE:
                case OBJECT_TYPE_TEXT:
                {
                    state->object->aabb.x = (float)state->object->x;
                    state->object->aabb.y = (float)state->object->y;
                    state->object->aabb.width = (float)state->object->width;
                    state->object->aabb.height = (float)state->object->height;
                } break;
                case OBJECT_TYPE_POINT:
                {
                    state->object->aabb.x = (float)state->object->x;
                    state->object->aabb.y = (float)state->object->y;
                    state->object->aabb.width = 0.0f;
                    state->object->aabb.height = 0.0f;
                } break;
                case OBJECT_TYPE_POLYGON:
                case OBJECT_TYPE_POLYLINE:
                {
                    float minX = INFINITY;
                    float maxX = -INFINITY;
                    float minY = INFINITY;
                    float maxY = -INFINITY;
                    for (uint32_t i = 1; i < state->object->pointsLength; i++)
                    {
                        if (state->object->points[i].x < minX) minX = state->object->points[i].x;
                        if (state->object->points[i].x > maxX) maxX = state->object->points[i].x;
                        if (state->object->points[i].y < minY) minY = state->object->points[i].y;
                        if (state->object->points[i].y > maxY) maxY = state->object->points[i].y;
                    }

                    // Note: Poly(gon|line) objects' vertices are stored with relative positions whereas AABBs use
                    // absolute values. The object's X and Y values must be added.
                    state->object->aabb.x = minX + (float)state->object->x;
                    state->object->aabb.y = minY + (float)state->object->y;
                    state->object->aabb.width = maxX - minX;
                    state->object->aabb.height = maxY - minY;
                } break;
                case OBJECT_TYPE_TILE:
                    // The tile object type can have varying sizes, depending on the tile. While most will have the
                    // width and height defined in the top-level <map>, a "collection of images" tileset will have tiles
                    // with arbitrary dimensions. For that reason, an AABB cannot be calculated.
                break;
            }
        }
        state->object = NULL;
    }
    else if (strcmp(hoxml->tag, "text") == 0)
    {
        // Apply default values for the attribute(s) that have default values that aren't zero or null.
        if ((state->object != NULL) && (state->object->text != NULL))
        {
            TmxObject *object = state->object;
            TmxText *objectText = object->text;

            if (hoxml->content != NULL) // If the element had content e.g. <text>Content here</text>.
            {
                objectText->content = (char *)MemAllocZero((unsigned int)strlen(hoxml->content) + 1);
                StringCopy(objectText->content, hoxml->content);
            }

            if (objectText->fontFamily == NULL) // If this <text> didn't have a 'fontfamily' attribute.
            {
                // The default value for 'fontfamily' is "sans-serif".
                objectText->fontFamily = (char *)MemAllocZero((unsigned int)strlen("sans-serif") + 1);
                StringCopy(objectText->fontFamily, "sans-serif");
            }

            if (objectText->content != NULL) // If there's text to be drawn.
            {
                RaytmxTextLineNode *linesRoot = NULL;
                RaytmxTextLineNode *linesTail = NULL;
                uint32_t linesLength = 0;

                // There's some aligning and allocating to be done in order to draw the text. This isn't something that
                // should be done per-draw and, given the text is static, can be done ahead of time.
                const Font font = GetFontDefault();
                const float spacing = objectText->kerning? 1.0f : 0.0f;

                const unsigned int bufferLength = (unsigned int)strlen(objectText->content) + 1;
                // This buffer will hold hold subsets of the content while iterating through it. The string in this
                // buffer may exceed the bounds.
                char *testingBuffer = (char *)MemAllocZero(bufferLength);
                // This one will hold the last known good string whose graphical text would fit within the bounds.
                char *validBuffer = (char *)MemAllocZero(bufferLength);
                // This one will hold space-delimited subsets of the above.
                char *delimitedBuffer = (char *)MemAllocZero(bufferLength);
                bool isDelimited = false;

                char *start = objectText->content;
                char *end = start;
                char *validEnd = start;
                char *delimitedEnd = start;
                // While the 'end' iterator hasn't reached the end of the content AND further lines will fit within the
                // Y bounds of the object.
                while ((*end != '\0') &&
                        (object->y + (objectText->pixelSize*(linesLength + 1)) <= object->y + object->height))
                {
                    end++;
                    const int length = (int)(end - start);
                    if (length <= 0) continue; // Quick error check for a case that is hopefully impossible in practice.

                    StringCopyN(testingBuffer, start, length);
                    // Remove any trailing whitespace by iterating backwards and adding a terminator in place of any
                    // whitespace character.
                    for (char *i = testingBuffer + length - 1; (i > testingBuffer) && isspace(*i); i--) *i = '\0';

                    // Measure the dimensions of the now-widest text.
                    const Vector2 textSize = MeasureTextEx(font, testingBuffer, (float)objectText->pixelSize, spacing);
                    // If this text still fits within the width of the object (the bounds).
                    if (textSize.x <= object->width)
                    {
                        // This string is still valid. Remember the string and where it ends. Iteration may return to
                        // last known good position.
                        validEnd = end;
                        StringCopy(validBuffer, testingBuffer);
                        // Note: Because the testing buffer is stripped of trailing whitespace, so too is this one.

                        // If the end of the current string is whitespace but the last character wasn't.
                        if (isspace(*end) && !isspace(*(end - 1)))
                        {
                            // Taking "Hello, from TMX!" as an example, this would be triggered by the first space with
                            // "Hello," being the string to remember. In the event that the would-be text is too wide,
                            // meaning "Hello, from" is too long to fit width-wise, then this delimted "Hello," will be
                            // the line and iteration will return to "from".
                            isDelimited = true;
                            delimitedEnd = end;
                            StringCopy(delimitedBuffer, validBuffer);
                        }
                    }

                    // If it's time to create a visual line of text from the string either because: 1) the string has
                    // become too wide to fit in the bounds of the object, or two) the end of the content was reached.
                    if ((textSize.x > object->width) || (*end == '\0'))
                    {
                        char *sourceBuffer = NULL;
                        if ((*end == '\0') || !isDelimited)
                        {
                            // The valid buffer is used in cases where the would-be text is simply the remainder of the
                            // content or the line does not have a clear separator (whitespace), like an extra long word
                            // that continues to the next line.
                            sourceBuffer = validBuffer;
                            start = validEnd;
                        }
                        else
                        {
                            // The delimited buffer is used in cases where one or more words were identified using space
                            // between them as a delimiter.
                            sourceBuffer = delimitedBuffer;
                            start = delimitedEnd;

                            // Skip over any whitespace at the delimiter.
                            while (isspace(*start) && (*start != '\0')) start++;
                        }

                        // The 'start' pointer is pointing to the first character of the next line. Point 'end' to the
                        // same place and continue from there so nothing is skipped. Note: 'end' will be incremented at
                        // at the start of the next loop.
                        end = start;

                        TmxTextLine line = { 0 };
                        line.content = (char *)MemAllocZero((unsigned int)strlen(sourceBuffer) + 1);
                        StringCopy(line.content, sourceBuffer);
                        line.font = font;
                        line.spacing = spacing;
                        // Note: The number of lines is not yet known but needs to be for Y positioning.

                        RaytmxTextLineNode *node = (RaytmxTextLineNode *)MemAllocZero(sizeof(RaytmxTextLineNode));
                        node->line = line;
                        if (linesRoot == NULL) linesRoot = node;
                        else linesTail->next = node;
                        linesTail = node;
                        linesLength += 1;

                        // Reset variables.
                        memset(testingBuffer, '\0', bufferLength);
                        memset(validBuffer, '\0', bufferLength);
                        memset(delimitedBuffer, '\0', bufferLength);
                        isDelimited = false;

                        if (!objectText->wrap) // If word wrapping is disabled.
                        {
                            // It's unclear why this would be done but, having hit the end of what can be displayed on a
                            // single line, no more text can be appended.
                            break;
                        }
                    }
                }

                MemFree(testingBuffer);
                MemFree(validBuffer);
                MemFree(delimitedBuffer);

                if (linesRoot != NULL)
                {
                    // Allocate the array and zero out every value as initialization.
                    TmxTextLine *lines = (TmxTextLine *)MemAllocZero(sizeof(TmxTextLine)*linesLength);
                    // Copy the TmxTextLines into the array and free the nodes while we're at it.
                    RaytmxTextLineNode *iter = linesRoot;
                    for (uint32_t i = 0; (i < linesLength) && (iter != NULL); i++)
                    {
                        lines[i] = iter->line;
                        Vector2 textSize = MeasureTextEx(font, lines[i].content, (float)objectText->pixelSize,
                            lines[i].spacing);

                        // Horizontal alignment.
                        if (objectText->halign == HORIZONTAL_ALIGNMENT_RIGHT)
                            lines[i].position.x = (float)(object->x + object->width) - textSize.x;
                        else if (objectText->halign == HORIZONTAL_ALIGNMENT_CENTER)
                            lines[i].position.x = (float)(object->x + (object->width/2.0)) - (textSize.x/2.0f);
                        else if (objectText->halign == HORIZONTAL_ALIGNMENT_JUSTIFY)
                        {
                            // Horizontally justified text extends from the left bound to the right bound. Typically,
                            // this is done by adding space betweens words or, where there is only one word, adding
                            // space between letters. All additional space is distributed evenly. However, the method
                            // here is a hybrid: because control over spacing between words is coarse, spacing between
                            // letters is added on top of it.
                            lines[i].position.x = (float)object->x; // Place the text on the left bound.
                            // Count the number of spaces between words.
                            uint32_t numSpaces = 0;
                            for (uint32_t j = 0; lines[i].content[j] != '\0'; j++)
                                if (isspace(lines[i].content[j])) numSpaces++;
                            size_t length = strlen(lines[i].content);
                            if (numSpaces > 0) // If there's more than one word in the line.
                            {
                                // Measure the dimensions of a single space using this line's configuration.
                                Vector2 originalTextSize = textSize;
                                textSize = MeasureTextEx(font, " ", (float)objectText->pixelSize, lines[i].spacing);

                                // Calculate the number of new spaces to add between words, per existing space.
                                const float idealNumAdditionalSpaces =
                                    ((float)object->width - originalTextSize.x)/textSize.x;
                                const uint32_t numSpacesToAddPer =
                                    (uint32_t)floor((idealNumAdditionalSpaces - (float)numSpaces)/(float)numSpaces);

                                // Create a new string with the additional space.
                                const size_t justifiedLength = length + (numSpacesToAddPer*numSpaces);
                                char *justifiedContent = (char *)MemAllocZero((unsigned int)justifiedLength + 1);
                                uint32_t sourceIndex = 0;
                                uint32_t destinationIndex = 0;
                                while (lines[i].content[sourceIndex] != '\0')
                                {
                                    justifiedContent[destinationIndex++] = lines[i].content[sourceIndex];

                                    // If the current character is whitespace but the next one is not.
                                    if ((sourceIndex < length) && !isspace(lines[i].content[sourceIndex]) &&
                                        isspace(lines[i].content[sourceIndex + 1]))
                                    {
                                        // Add 'numSpacesToAddPer' spaces to the justified string.
                                        for (uint32_t j = 0; j < numSpacesToAddPer; j++)
                                            justifiedContent[destinationIndex++] = ' ';
                                    }
                                    sourceIndex++;
                                }

                                // Free the original content buffer and replace it with the justified one.
                                MemFree(lines[i].content);
                                lines[i].content = justifiedContent;
                                length = justifiedLength;
                            }

                            // Calculate a spacing, between each letter, with which the drawn text will span the full
                            // width of the bounds.
                            textSize = MeasureTextEx(font, lines[i].content, (float)objectText->pixelSize, 0.0f);
                            lines[i].spacing = (float)((object->width - textSize.x)/(double)(length - 1));
                        }
                        else // if (objectText->halign == HORIZONTAL_ALIGNMENT_LEFT)
                            lines[i].position.x = (float)object->x;

                        // Vertical alignment.
                        if (objectText->valign == VERTICAL_ALIGNMENT_BOTTOM)
                        {
                            lines[i].position.y = (float)(object->y + object->height) -
                                (float)(objectText->pixelSize*(i + 1));
                        }
                        else if (objectText->valign == VERTICAL_ALIGNMENT_CENTER)
                        {
                            const float totalLineHeight = (float)(objectText->pixelSize*linesLength); // All N lines.
                            lines[i].position.y = (float)object->y + // Top of the <object>'s bounds.
                                ((float)object->height/2.0f) + // Vertical center of the <object>'s bounds.
                                (totalLineHeight/2.0f) - // Bottom of the centered lines' bounds.
                                (float)(objectText->pixelSize*(i + 1));
                        }
                        else // if (objectText->valign == VERTICAL_ALIGNMENT_TOP)
                            lines[i].position.y = (float)object->y + (float)(objectText->pixelSize*i);

                        RaytmxTextLineNode *parent = iter;
                        iter = iter->next;
                        MemFree(parent);
                    }

                    // Add the lines array to the text object.
                    objectText->lines = lines;
                    objectText->linesLength = linesLength;
                }
            }
        }
    }
    else if (strcmp(hoxml->tag, "imagelayer") == 0)
    {
        state->imageLayer = NULL;
        state->layer = NULL;
    }
    else if (strcmp(hoxml->tag, "group") == 0)
    {
        // <group>s can be nested so we must return to processing its parent, if it exists.
        if (state->groupNode != NULL) state->groupNode = state->groupNode->parent;
    }
}

void FreeStateLayers(RaytmxLayerNode *layers)
{
    RaytmxLayerNode *layersIter = layers;
    RaytmxLayerNode *layersTemp = NULL;
    while (layersIter != NULL)
    {
        // Group layers may have children, forming a tree-like structure. Free the children.
        FreeStateLayers(layersIter->childrenRoot);

        // Iterate to the next node and free this one.
        layersTemp = layersIter;
        layersIter = layersIter->next;
        MemFree(layersTemp);
    }
}

void FreeState(RaytmxState *state)
{
    if (state == NULL) return;

    // Clear the caches. These allow for quick lookups of previously-loaded textures and object templates. They aren't
    // needed once loading is complete.
    RaytmxCachedTextureNode *cachedTextureIter = state->texturesRoot;
    RaytmxCachedTextureNode *cachedTextureTemp = NULL;
    while (cachedTextureIter != NULL)
    {
        cachedTextureTemp = cachedTextureIter;
        cachedTextureIter = cachedTextureIter->next;
        if (cachedTextureTemp->fileName != NULL) MemFree(cachedTextureTemp->fileName);
        MemFree(cachedTextureTemp);
    }
    state->texturesRoot = NULL;

    RaytmxCachedTemplateNode *cachedTemplateIter = state->templatesRoot;
    RaytmxCachedTemplateNode *cachedTemplateTemp = NULL;
    while (cachedTemplateIter != NULL)
    {
        cachedTemplateTemp = cachedTemplateIter;
        cachedTemplateIter = cachedTemplateIter->next;
        FreeObject(cachedTemplateTemp->objectTemplate.object);
        if (cachedTemplateTemp->fileName != NULL) MemFree(cachedTemplateTemp->fileName);
        MemFree(cachedTemplateTemp);
    }
    state->templatesRoot = NULL;

    state->property = NULL;
    state->tileset = NULL;
    state->image = NULL;
    state->tilesetTile = NULL;
    state->animationFrame = NULL;
    // state->wangSet = NULL; // TODO:  Wang sets. Low priority.
    // state->wangColor = NULL; // TODO:  Wang sets. Low priority.
    state->layer = NULL;
    state->tileLayer = NULL;
    state->objectGroup = NULL;
    state->imageLayer = NULL;
    state->object = NULL;

    // Free each property in the linked list of properties.
    RaytmxPropertyNode *propertiesIter = state->propertiesRoot;
    RaytmxPropertyNode *propertiesTemp = NULL;
    while (propertiesIter != NULL)
    {
        propertiesTemp = propertiesIter;
        propertiesIter = propertiesIter->next;
        MemFree(propertiesTemp);
    }
    // Zeroize this linked list's properties.
    state->propertiesRoot = NULL;
    state->propertiesTail = NULL;
    state->propertiesLength = 0;

    // Free each node in the linked list of tilesets.
    RaytmxTilesetNode *tilesetsIter = state->tilesetsRoot;
    RaytmxTilesetNode *tilesetsTemp = NULL;
    while (tilesetsIter != NULL)
    {
        tilesetsTemp = tilesetsIter;
        tilesetsIter = tilesetsIter->next;
        MemFree(tilesetsTemp);
    }
    // Zeroize this linked list's properties.
    state->tilesetsRoot = NULL;
    state->tilesetsTail = NULL;
    state->tilesetsLength = 0;

    // Free each node in the linked list of tileset tiles.
    RaytmxTilesetTileNode *tilesetTilesIter = state->tilesetTilesRoot;
    RaytmxTilesetTileNode *tilesetTilesTemp = NULL;
    while (tilesetTilesIter != NULL)
    {
        tilesetTilesTemp = tilesetTilesIter;
        tilesetTilesIter = tilesetTilesIter->next;
        MemFree(tilesetTilesTemp);
    }
    // Zeroize this linked list's properties.
    state->tilesetTilesRoot = NULL;
    state->tilesetTilesTail = NULL;
    state->tilesetTilesLength = 0;

    // Free each node in the linked list of animation frames.
    RaytmxAnimationFrameNode *animationFramesIter = state->animationFramesRoot;
    RaytmxAnimationFrameNode *animationFramesTemp = NULL;
    while (animationFramesIter != NULL)
    {
        animationFramesTemp = animationFramesIter;
        animationFramesIter = animationFramesIter->next;
        MemFree(animationFramesTemp);
    }
    // Zeroize this linked list's properties.
    state->animationFramesRoot = NULL;
    state->animationFramesTail = NULL;
    state->animationFramesLength = 0;

    // Layers may be groups. The resulting collection isn't a link list as much as it is a tree. So freeing layers is
    // best done with a recursive approach.
    FreeStateLayers(state->layersRoot);
    // Zeroize this linked list's properties.
    state->layersRoot = NULL;
    state->layersTail = NULL;
    state->layersLength = 0;

    // Free each node in the linked list of layer tiles.
    RaytmxTileLayerTileNode *layerTilesIter = state->layerTilesRoot;
    RaytmxTileLayerTileNode *layerTilesTemp = NULL;
    while (layerTilesIter != NULL)
    {
        layerTilesTemp = layerTilesIter;
        layerTilesIter = layerTilesIter->next;
        MemFree(layerTilesTemp);
    }
    // Zeroize this linked list's properties.
    state->layerTilesRoot = NULL;
    state->layerTilesTail = NULL;
    state->layerTilesLength = 0;

    // Free each node in the linked list of objects.
    RaytmxObjectNode *objectsIter = state->objectsRoot;
    RaytmxObjectNode *objectsTemp = NULL;
    while (objectsIter != NULL)
    {
        objectsTemp = objectsIter;
        objectsIter = objectsIter->next;
        MemFree(objectsTemp);
    }
    // Zeroize this linked list's properties.
    state->objectsRoot = NULL;
    state->objectsTail = NULL;
    state->objectsLength = 0;
}

void inline FreeString(char *str)
{
    if (str != NULL) MemFree(str);
}

void FreeTileset(TmxTileset tileset)
{
    FreeString(tileset.source);
    FreeString(tileset.name);
    FreeString(tileset.classString);

    if (tileset.hasImage)
    {
        FreeString(tileset.image.source);
        UnloadTexture(tileset.image.texture);
    }

    if (tileset.properties != NULL)
    {
        for (uint32_t i = 0; i < tileset.propertiesLength; i++) FreeProperty(tileset.properties[i]);
        MemFree(tileset.properties);
    }

    for (uint32_t i = 0; i < tileset.tilesLength; i++)
    {
        TmxTilesetTile tile = tileset.tiles[i];

        FreeString(tile.classString);
        if (tile.hasImage)
        {
            FreeString(tile.image.source);
            UnloadTexture(tile.image.texture);
            if (tile.properties != NULL)
            {
                for (uint32_t j = 0; j < tile.propertiesLength; j++) FreeProperty(tile.properties[j]);
                MemFree(tile.properties);
            }
        }

        if (tile.animation.frames != NULL) MemFree(tile.animation.frames);
    }
}

void FreeProperty(TmxProperty property)
{
    FreeString(property.name);
    FreeString(property.stringValue);
}

void FreeLayer(TmxLayer layer)
{
    FreeString(layer.name);
    FreeString(layer.classString);

    if (layer.properties != NULL)
    {
        for (uint32_t i = 0; i < layer.propertiesLength; i++) FreeProperty(layer.properties[i]);
        MemFree(layer.properties);
    }

    switch (layer.type)
    {
        case LAYER_TYPE_TILE_LAYER:
        {
            FreeString(layer.exact.tileLayer.encoding);
            FreeString(layer.exact.tileLayer.compression);
            MemFree(layer.exact.tileLayer.tiles);
        } break;
        case LAYER_TYPE_OBJECT_GROUP:
        {
            for (uint32_t j = 0; j < layer.exact.objectGroup.objectsLength; j++)
                FreeObject(layer.exact.objectGroup.objects[j]);
            MemFree(layer.exact.objectGroup.objects);
        } break;
        case LAYER_TYPE_IMAGE_LAYER:
        {
            if (layer.exact.imageLayer.hasImage) UnloadTexture(layer.exact.imageLayer.image.texture);
        } break;
        case LAYER_TYPE_GROUP: break; // Nothing to do for this case but compilers like to complain.
    }

    // <group> layers are expected to have child layers, or child <group>s, so recursively free them too.
    for (uint32_t i = 0; i < layer.layersLength; i++) FreeLayer(layer.layers[i]);
}

void FreeObject(TmxObject object)
{
    FreeString(object.name);
    FreeString(object.typeString);
    FreeString(object.templateString);

    if (object.points != NULL) MemFree(object.points);

    if (object.text != NULL)
    {
        if (object.text->lines != NULL)
        {
            for (uint32_t j = 0; j < object.text->linesLength; j++) FreeString(object.text->lines[j].content);
            MemFree(object.text->lines);
        }

        MemFree(object.text);
    }
}

#define SIGN(x) ((x < 0)? -1 : +1)

// Helper function that keeps an integer within a range, between some (inclusive) minimum and maximum values.
int Clampi(int value, int minimum, int maximum)
{
    if (value < minimum) return minimum;
    else if (value > maximum) return maximum;
    else return value;
}

// Scary-looking helper function that does something simple: iterates through the visible tiles of a layer.
// Returns true while still iterating so it can be used like "while (IterateTileLayer()) { ... }".
// Details of the current tile are assigned to output parameters, if passed a non-NULL address.
// Iteration is done row-by-row.
bool IterateTileLayer(const TmxMap *map, const TmxTileLayer *layer, Rectangle viewport, RaytmxTransform transform,
    uint32_t *rawGid, TmxTile *tile, Rectangle *tileRect)
{
    // Static variables whose values will persist between calls. These are needed to initialize and iterate.
    static const TmxTileLayer *currentLayer = NULL; // Tile layer being iterated.
    static int fromX = 0; // Initial X position, tile not pixel, that row-by-row iteration begins at.
    static int fromY = 0; // Initial Y position, tile not pixel, that row-by-row iteration begins at.
    static int toX = 0; // Final X position, tile not pixel, that iteration ends at.
    static int toY = 0; // Final Y position, tile not pixel, that iteration ends at.
    static int currentX = 0; // Current tile X position (column) within the iteration.
    static int currentY = 0; // Current tile Y position (row) within the iteration.

    // If iteration is impossible or there are effectively no tiles to iterate over.
    if ((map == NULL) || (map->width == 0) || (map->height == 0) || (map->tileWidth == 0) || (map->tileHeight == 0) ||
        (layer == NULL) || (layer->tilesLength == 0))
    {
        return false;
    }

    if (currentLayer != layer) // If the layer has changed (i.e. iteration should initialize).
    {
        currentLayer = layer; // Remember this layer.

        // Create an adjusted viewport that effectively removes the map's drawn position. With this, it doesn't need to
        // be a factor in the math below.
        const Rectangle viewport2 = { viewport.x - transform.position.x, viewport.y - transform.position.y,
            viewport.width, viewport.height };

        switch (map->renderOrder)
        {
            case RENDER_ORDER_RIGHT_DOWN:
            {
                // Start at the top-left, iterate right, then iterate down, ending at the bottom-right.
                // In other words, this is the order in which English is read.
                fromX = (int)viewport2.x/(int)map->tileWidth;
                fromY = (int)viewport2.y/(int)map->tileHeight;
                toX = (int)(viewport2.x + viewport2.width)/(int)map->tileWidth;
                toY = (int)(viewport2.y + viewport2.height)/(int)map->tileHeight;
            } break;
            case RENDER_ORDER_RIGHT_UP:
            {
                // Start at the bottom-left, iterate right, then iterate up, ending at the top-right.
                fromX = (int)viewport2.x/(int)map->tileWidth;
                fromY = (int)(viewport2.y + viewport2.height)/(int)map->tileHeight;
                toX = (int)(viewport2.x + viewport2.width)/(int)map->tileWidth;
                toY = (int)viewport2.y/(int)map->tileHeight;
            } break;
            case RENDER_ORDER_LEFT_DOWN:
            {
                // Start at the top-right, iterate left, then iterate down, ending at the bottom-left.
                fromX = (int)(viewport2.x + viewport2.width)/(int)map->tileWidth;
                fromY = (int)viewport2.y/(int)map->tileHeight;
                toX = (int)viewport2.x/(int)map->tileWidth;
                toY = (int)(viewport2.y + viewport2.height)/(int)map->tileHeight;
            } break;
            case RENDER_ORDER_LEFT_UP:
            {
                // Start at the bottom-right, iterate left, then iterate up, ending at the top-left.
                fromX = (int)(viewport2.x + viewport2.width)/(int)map->tileWidth;
                fromY = (int)(viewport2.y + viewport2.height)/(int)map->tileHeight;
                toX = (int)viewport2.x/(int)map->tileWidth;
                toY = (int)viewport2.y/(int)map->tileHeight;
            } break;
        }

        // Restrain the the tile positions to those within the map in case of rounding mistakes.
        fromX = Clampi(fromX, 0, (int)map->width - 1);
        fromY = Clampi(fromY, 0, (int)map->height - 1);
        toX = Clampi(toX, 0, (int)map->width - 1);
        toY = Clampi(toY, 0, (int)map->height - 1);

        // Begin iteration from both "from" tile positions.
        currentX = fromX;
        currentY = fromY;
    }
    else if (currentX == toX) // If the end of the current row was reached.
    {
        // Rendering is done row-by-row. This row is done so move to the next one.
        currentX = fromX;
        currentY += SIGN(toY - fromY); // Either +1 or -1.
    }
    else // If still iterating through the current row.
    {
        // Move to the right or left by one tile.
        currentX += SIGN(toX - fromX); // Either +1 or -1.
    }

    // If iteration has gone beyond the final row.
    if (((toY - fromY > 0) && (currentY > toY)) || ((toY - fromY < 0) && (currentY < toY)))
    {
        // This is the termination condition. Zero all values and return false so the caller exits its loop.
        currentLayer = NULL;
        fromX = 0;
        fromY = 0;
        toX = 0;
        toY = 0;
        currentX = 0;
        currentY = 0;
        return false;
    }

    // Calculate the index in the tile layer from knowing the tile's X and Y position (in tiles, not pixels).
    int index = (currentY*(int)map->width) + currentX;
    if ((index < 0) || (index >= (int)layer->tilesLength)) // Bounds check.
    {
        currentLayer = NULL;
        fromX = 0;
        fromY = 0;
        toX = 0;
        toY = 0;
        currentX = 0;
        currentY = 0;
        return false;
    }

    // Get the raw Global ID (GID) of the tile. This is "raw" in that it may have flipping (bit) flags.
    const uint32_t rawGid2 = layer->tiles[index];
    // Get the actual GID value by stripping any potential flipping flags.
    const uint32_t gid = GetGid(rawGid2, NULL, NULL, NULL, NULL);
    // Get the tile's metadata from knowing its GID.
    TmxTile tile2 = { 0 };
    if (gid < map->gidsToTilesLength) tile2 = map->gidsToTiles[gid];

    // If the raw GID should be assigned to its output parameter.
    if (rawGid != NULL) *rawGid = rawGid2; // Assign it.

    // If the tile's metadata should be assigned to its output parameter and the GID is in-bounds.
    if (tile != NULL) *tile = tile2; // Assign it.

    // If the tile's destination rectangle should be assigned to its output parameter.
    if (tileRect != NULL)
    {
        // Calculate the tile's destination rectangle, in pixels.
        // Note: The map's tile width and height determine the grid size and, therefore, the (X, y) position. However,
        // the tileset may define a different width and height which are assigned to the metadata's 'dimensions'.
        tileRect->x = (float)((uint32_t)currentX*map->tileWidth);
        tileRect->y = (float)((uint32_t)currentY*map->tileHeight);
        tileRect->width = tile2.dimensions.x;
        tileRect->height = tile2.dimensions.y;
    }

    return true;
}

void DrawTMXLayersInternal(const TmxMap *map, const Camera2D *camera, const Rectangle *viewport, const TmxLayer *layers,
    uint32_t layersLength, RaytmxTransform transform, Color tint)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0)) return;

    for (uint32_t i = 0; i < layersLength; i++)
    {
        const TmxLayer layer = layers[i];

        // If the layer is not visible.
        if (!layer.visible) continue; // Skip it.

        // All types of layers can have a couple attributes that affect color: 'opacity' and 'tintcolor'.
        Color layerTint = tint;
        layerTint.a = (unsigned char)((double)layerTint.a*layer.opacity);
        if (layer.hasTintColor) layerTint = ColorTint(layerTint, layer.tintColor);

        // Determine the viewport. This will depend on a couple parameters. If 'viewport' was assigned, it's used
        // directly. If 'camera' was assigned, its target and zoom are used to derive a reasonable viewport from the
        // screen's dimensions. If neither is assigned, the map's bounds are used.
        Rectangle viewport2 = { 0 };
        if (viewport != NULL) viewport2 = *viewport;
        else if (camera != NULL)
        {
            viewport2.width = (float)GetScreenWidth()/camera->zoom;
            viewport2.height = (float)GetScreenHeight()/camera->zoom;
            viewport2.x = camera->target.x - (viewport2.width/2.0f);
            viewport2.y = camera->target.y - (viewport2.height/2.0f);
        }
        else
        {
            viewport2.x = transform.position.x;
            viewport2.y = transform.position.y;
            viewport2.width = (float)(map->width*map->tileWidth);
            viewport2.height = (float)(map->height*map->tileHeight);
        }

        // Create an updated transform for this layer. This will be the sum of the position the map is drawn at, the
        // layer's offset, and the effect of parallax scrolling if conditions are met.
        RaytmxTransform transform2 = transform;

        // "The effective parallax scrolling factor of a layer is determined by multiplying the parallax scrolling
        // factor by the scrolling factors of all parent layers."
        transform2.parallax.x *= (float)layer.parallaxX;
        transform2.parallax.y *= (float)layer.parallaxY;

        // Apply the offset specific to this layer and/or parallax scrolling. Also applies to child layers.
        transform2.position.x += (float)layer.offsetX + (transform2.cameraOffset.x*(1.0f - transform2.parallax.x));
        transform2.position.y += (float)layer.offsetY + (transform2.cameraOffset.y*(1.0f - transform2.parallax.y));
        // Enforce integer values. This is necessary in order to prevent errors resulting from rounding. In particular,
        // parallax factors are rarely integers which leads to non-integer positions and gaps between rows or columns.
        transform2.position.x = roundf(transform2.position.x);
        transform2.position.y = roundf(transform2.position.y);

        switch (layer.type)
        {
            case LAYER_TYPE_TILE_LAYER: DrawTMXTileLayer(map, viewport2, layer, transform2, layerTint); break;
            case LAYER_TYPE_OBJECT_GROUP: DrawTMXObjectGroup(map, viewport2, layer, transform2, layerTint); break;
            case LAYER_TYPE_IMAGE_LAYER: DrawTMXImageLayer(map, viewport2, layer, transform2, layerTint); break;
            case LAYER_TYPE_GROUP:
                DrawTMXLayersInternal(map, camera, &viewport2, layer.layers, layer.layersLength, transform2, layerTint);
                break;
        }
    }
}

void DrawTMXTileLayer(const TmxMap *map, Rectangle viewport, TmxLayer layer, RaytmxTransform transform, Color tint)
{
    if ((map == NULL) || (layer.type != LAYER_TYPE_TILE_LAYER) || (layer.exact.tileLayer.tilesLength == 0)) return;

    // Iterate through each tile that overlaps with the viewport and draw them.
    uint32_t rawGid = 0;
    Rectangle destRect = { 0 };
    while (IterateTileLayer(map, &(layer.exact.tileLayer), viewport, transform, &rawGid, NULL, &destRect))
        DrawTMXLayerTile(map, viewport, transform, rawGid, destRect, tint); // Draw the individual tile.
}

void DrawTextureTile(Texture2D texture, Rectangle source, Rectangle dest, bool flipX, bool flipY, bool flipDiag,
    Color tint)
{
    // If the texture is invalid.
    if (texture.id == 0) return;

    // Determine the area within the texture to be drawn.
    // Note: The coordinates here are in the [0.0, 1.0] range where (0.0, 0.0) is the bottom-left corner of the texture,
    // (1.0, 0.0) is the bottom-right, and (1.0, 1.0) is the top-right. In other words, the coordinates are a ratio of
    // the dimensions making (0.5, 0.5) the center of the texture regardless of its aspect ratio.
    const float width = (float)texture.width;
    const float height = (float)texture.height;
    const Vector2 sourceTopLeft = { source.x/width, source.y/height };
    Vector2 sourceTopRight = { (source.x + source.width)/width, source.y/height };
    Vector2 sourceBottomLeft = { source.x/width, (source.y + source.height)/height };
    const Vector2 sourceBottomRight = { (source.x + source.width)/width, (source.y + source.height)/height };

    if (flipDiag) // If the tile uses a diagonal flip.
    {
        // "The diagonal flip should flip the bottom left and top right corners of the tile..."
        const Vector2 temp = sourceBottomLeft;
        sourceBottomLeft = sourceTopRight;
        sourceTopRight = temp;
    }

    // Determine the area on the screen to be drawn to.
    const Vector2 destTopLeft = { dest.x, dest.y };
    const Vector2 destTopRight = { dest.x + dest.width, dest.y };
    const Vector2 destBottomLeft = { dest.x, dest.y + dest.height };
    const Vector2 destBottomRight = { dest.x + dest.width, dest.y + dest.height };

    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    {
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f); // Normal vector pointing towards viewer.

        // Top-left corner of the quad.
        if (flipX && !flipY) rlTexCoord2f(sourceTopRight.x, sourceTopRight.y);
        else if (flipY && !flipX) rlTexCoord2f(sourceBottomLeft.x, sourceBottomLeft.y);
        else rlTexCoord2f(sourceTopLeft.x, sourceTopLeft.y);

        if (flipX && flipY) rlVertex2f(destBottomRight.x, destBottomRight.y);
        else rlVertex2f(destTopLeft.x, destTopLeft.y);

        // Bottom-left corner of the quad.
        if (flipX && !flipY) rlTexCoord2f(sourceBottomRight.x, sourceBottomRight.y);
        else if (flipY && !flipX) rlTexCoord2f(sourceTopLeft.x, sourceTopLeft.y);
        else rlTexCoord2f(sourceBottomLeft.x, sourceBottomLeft.y);

        if (flipX && flipY) rlVertex2f(destTopRight.x, destTopRight.y);
        else rlVertex2f(destBottomLeft.x, destBottomLeft.y);

        // Bottom-right corner of the quad.
        if (flipX && !flipY) rlTexCoord2f(sourceBottomLeft.x, sourceBottomLeft.y);
        else if (flipY && !flipX) rlTexCoord2f(sourceTopRight.x, sourceTopRight.y);
        else rlTexCoord2f(sourceBottomRight.x, sourceBottomRight.y);

        if (flipX && flipY) rlVertex2f(destTopLeft.x, destTopLeft.y);
        else rlVertex2f(destBottomRight.x, destBottomRight.y);

        // Top-right corner of the quad.
        if (flipX && !flipY) rlTexCoord2f(sourceTopLeft.x, sourceTopLeft.y);
        else if (flipY && !flipX) rlTexCoord2f(sourceBottomRight.x, sourceBottomRight.y);
        else rlTexCoord2f(sourceTopRight.x, sourceTopRight.y);

        if (flipX && flipY) rlVertex2f(destBottomLeft.x, destBottomLeft.y);
        else rlVertex2f(destTopRight.x, destTopRight.y);
    }
    rlEnd();
    rlSetTexture(0);
}

void DrawTMXLayerTile(const TmxMap *map, Rectangle viewport, RaytmxTransform transform, uint32_t rawGid,
    Rectangle destRect, Color tint)
{
    if ((map == NULL) || (tint.a == 0)) return;

    // Tile Global IDs (GIDs) can have several bit flags that indicate transforms. This function is used to both get
    // those possible transform flags as well as the actual GID value without those bit flags.
    bool flipX = false;
    bool flipY = false;
    bool flipDiag = false;
    bool rotateHexag120 = false;
    uint32_t gid = GetGid(rawGid, &flipX, &flipY, &flipDiag, &rotateHexag120);

    // If the GID is outside the range of known GIDs, do not attempt to draw the tile.
    if (gid >= map->gidsToTilesLength) return;

    // With the GID, grab the relevant tile information (texture, animation, etc.) from the global mapping.
    TmxTile tile = map->gidsToTiles[gid];

    // If the GID is not known to exist in any tilesets within the map, do not attempt to draw the tile.
    if (tile.gid == 0) return;

    if (tile.hasAnimation && (tile.frameIndex < tile.animation.framesLength) &&
        (tile.animation.frames[tile.frameIndex].gid < map->gidsToTilesLength)) // If the tile is/has an animation.
    {
        // Animation tiles are meta; they contain a list of other GIDs to be drawn. Get the actual tile to draw this
        // frame from that list.
        gid = tile.animation.frames[tile.frameIndex].gid;
        if (gid < map->gidsToTilesLength) tile = map->gidsToTiles[gid];
    }

    // Determine where the tile will be drawn. raylib's coordinates consider (X, Y) to be the top-left corner of the
    // rectangle being drawn. The TMX documentation complicates things a bit saying "Larger tiles will extend at the top
    // and right (anchored to the bottom left)" meaning that TMX considers (X, Y) to be the bottom-left corner. The
    // simplest way to reconcile the Y coordinate differences is to substract the texture's height at Y + 1. This way,
    // tiles larger than the map's tile height values will be drawn further up (negative Y direction). */
    destRect.x += transform.position.x + tile.offset.x;
    destRect.y += transform.position.y + tile.offset.y + map->tileHeight - tile.sourceRect.height;

    // If the viewport and destination rectangles are overlapping to any degree (i.e. if the tile is visible).
    if (CheckCollisionRecs(viewport, destRect))
        DrawTextureTile(tile.texture, tile.sourceRect, destRect, flipX, flipY, flipDiag, tint); // Draw the tile.
}

void DrawTMXObjectTile(const TmxMap *map, Rectangle viewport, uint32_t rawGid, RaytmxTransform transform, float width,
    float height, Color tint)
{
    if ((map == NULL) || (width <= 0) || (height <= 0) || (tint.a == 0)) return;

    bool flipX = false;
    bool flipY = false;
    bool flipDiag = false;
    bool rotateHexag120 = false;
    // Tile Global IDs (GIDs) can have several bit flags that indicate transforms. This function is used to both get
    // those possible transform flags as well as the actual GID value without those bit flags.
    uint32_t gid = GetGid(rawGid, &flipX, &flipY, &flipDiag, &rotateHexag120);

    // If the GID is outside the range of known GIDs, do not attempt to draw the tile.
    if (gid >= map->gidsToTilesLength) return;

    // With the GID, grab the relevant tile information (texture, animation, etc.) from the global mapping.
    TmxTile tile = map->gidsToTiles[gid];

    // If the GID is not known to exist in any tilesets within the map, do not attempt to draw the tile.
    if (tile.gid == 0) return;

    if (tile.hasAnimation && (tile.frameIndex < tile.animation.framesLength) &&
        (tile.animation.frames[tile.frameIndex].gid < map->gidsToTilesLength)) // If the tile is/has an animation.
    {
        // Animation tiles are meta; they contain a list of other GIDs to be drawn. Get the actual tile to draw this
        // frame from that list.
        gid = tile.animation.frames[tile.frameIndex].gid;
        if (gid < map->gidsToTilesLength) tile = map->gidsToTiles[gid];
    }

    // Determine the area in which to draw, and potentially stretch, the texture. This area matches that of the
    // <object>, not the tile size. This also means that the Y coordinate needs consideration because raylib considers
    // (X, Y) to the be top-left corner of any area but the TMX format considers it the bottom-left.
    const Rectangle destRect = { transform.position.x + tile.offset.x, transform.position.y + tile.offset.y - height,
        width, height };

    // If the viewport and destination rectangles are overlapping to any degree (i.e. if the tile is visible).
    if (CheckCollisionRecs(viewport, destRect))
        DrawTextureTile(tile.texture, tile.sourceRect, destRect, flipX, flipY, flipDiag, tint); // Draw the tile.
}

void DrawTMXObjectGroup(const TmxMap *map, Rectangle viewport, TmxLayer layer, RaytmxTransform transform, Color tint)
{
    if ((map == NULL) || (layer.type != LAYER_TYPE_OBJECT_GROUP) || (tint.a == 0)) return;

    const TmxObjectGroup objectGroup = layer.exact.objectGroup;

    for (int32_t i = 0; i < (int32_t)objectGroup.objectsLength; i++)
    {
        // Select the object to draw based on the <objectgroup>'s draw order.
        const TmxObject object = (objectGroup.drawOrder == OBJECT_GROUP_DRAW_ORDER_INDEX)? objectGroup.objects[i] :
            objectGroup.objects[objectGroup.ySortedObjects[i]];

        if (object.type == OBJECT_TYPE_TILE) // If the object is a tile with an abitrary GID and dimensions.
        {
            RaytmxTransform transform2 = transform;
            transform2.position.x += (float)object.x;
            transform2.position.y += (float)object.y;
            // Note: This draw method handles culling so it doesn't need to be done here.
            DrawTMXObjectTile(map, viewport, object.gid, transform2, (float)object.width, (float)object.height, tint);
        }
        else // If the object is any type other than a tile.
        {
            Rectangle aabb2 = object.aabb;
            aabb2.x += transform.position.x;
            aabb2.y += transform.position.y;
            // If the viewport and the polygon's AABB are overlapping to any degree (i.e. it is visible).
            if (CheckCollisionRecs(viewport, aabb2))
            {
                switch (object.type)
                {
                    case OBJECT_TYPE_RECTANGLE:
                    {
                        const Rectangle rectangle = { transform.position.x + (float)object.x,
                            transform.position.y + (float)object.y, (float)object.width, (float)object.height };
                        DrawRectangleRec(rectangle, objectGroup.color);
                    } break;
                    case OBJECT_TYPE_ELLIPSE:
                    {
                        // The width and height of the object determine the semi major and minor axes. That means the
                        // object's width is twice the horizontal radius and its height is twice the vertical radius.
                        const float radiusX = (float)object.width/2.0f;
                        const float radiusY = (float)object.height/2.0f;
                        DrawEllipse((int)(transform.position.x + (float)object.x + radiusX),
                            (int)(transform.position.y + (float)object.y + radiusY), radiusX, radiusY, objectGroup.color);
                    } break;
                    case OBJECT_TYPE_POINT:
                    {
                        // A point is a single pixel but the TMX format doesn't require the point must be drawn any
                        // particular way. Tiled uses a pin icon. Drawing the exact pixel is tempting but hard to see so
                        // the point is drawn with a circle, with a diameter equal to a quarter of a tile's width.
                        const Vector2 center = { transform.position.x + (float)object.x,
                            transform.position.y + (float)object.y };
                        DrawCircleV(center, (float)map->tileWidth/4.0f, objectGroup.color);
                    } break;
                    case OBJECT_TYPE_POLYGON:
                    case OBJECT_TYPE_POLYLINE:
                    {
                        // Copy the 'points' array to the 'drawPoints' array and apply the drawing position, an offset
                        // applied by the layer and/or draw call. The 'drawPoints' array was allocated at the same time
                        // as 'points' with the same size. This improves draw call times by reducing memory allocations.
                        memcpy(object.drawPoints, object.points, sizeof(Vector2)*object.pointsLength);
                        for (uint32_t i = 0; i < object.pointsLength; i++)
                        {
                            // Polygons' and polyglines' vertices are stored with relative positions. To get the
                            // absolute position needed for drawing, just add the object's position and offset.
                            object.drawPoints[i].x += transform.position.x + (float)object.x;
                            object.drawPoints[i].y += transform.position.y + (float)object.y;
                        }

                        // Use the offset points to draw the poly(gon|line).
                        if (object.type == OBJECT_TYPE_POLYGON)
                        {
                            // Note: Polygons' first elements are their centroids. DrawTriangleFan() requires this.
                            // And, the last element in 'drawPoints' is a duplicate of the first, non-centroid point.
                            DrawTriangleFan(object.drawPoints, object.pointsLength, objectGroup.color);
                        }
                        else // if (object.type == OBJECT_TYPE_POLYLINE)
                        {
                            // Note: The last element in 'drawPoints' is a duplicate of the first point.
                            for (uint32_t i = 1; i < object.pointsLength; i++)
                            {
                                DrawLineEx(object.drawPoints[i - 1], object.drawPoints[i], TMX_LINE_THICKNESS,
                                    objectGroup.color);
                            }
                        }
                    } break;
                    case OBJECT_TYPE_TEXT:
                    {
                        for (uint32_t i = 0; i < object.text->linesLength; i++)
                        {
                            Vector2 position = object.text->lines[i].position;
                            position.x += transform.position.x;
                            position.y += transform.position.y;
                            DrawTextEx(object.text->lines[i].font, object.text->lines[i].content, position,
                                (float)object.text->pixelSize, object.text->lines[i].spacing, object.text->color);
                        }
                    } break;
                    case OBJECT_TYPE_TILE:
                        // Object tiles are handled in the 'if' case of this 'else' block because the use of an AABB for
                        // culling is not reliable for them.
                    break;
                }
            }
        }
    }
}

void DrawTMXImageLayer(const TmxMap *map, Rectangle viewport, TmxLayer layer, RaytmxTransform transform, Color tint)
{
    if ((map == NULL) || (layer.type != LAYER_TYPE_IMAGE_LAYER) || !layer.exact.imageLayer.hasImage ||
        (layer.exact.imageLayer.image.width == 0) || (layer.exact.imageLayer.image.height == 0) || (tint.a == 0))
    {
        return;
    }

    const TmxImageLayer imageLayer = layer.exact.imageLayer;
    // Determine where the image of this image layer would be drawn, assuming no repetitions.
    Rectangle imageRect = { transform.position.x, transform.position.y, (float)imageLayer.image.width,
        (float)imageLayer.image.height };

    if (!imageLayer.repeatX && !imageLayer.repeatY && CheckCollisionRecs(viewport, imageRect)) // If visible.
        DrawTextureV(imageLayer.image.texture, transform.position, tint);
    else if (imageLayer.repeatX || imageLayer.repeatY) // If the image might be drawn across a whole axis, or both.
    {
        // Use integer division to determine the X and Y positions at which a the image would appear if it were repeated
        // across the whole axis (i.e. if "Repeat X" and/or "Repeat Y" are enabled).
        const int coefficientX = (int)(viewport.x - imageRect.x)/(int)imageRect.width;
        const int coefficientY = (int)(viewport.y - imageRect.y)/(int)imageRect.height;
        float x0 = imageRect.x + (imageRect.width*(float)coefficientX); // Initial X position.
        float y0 = imageRect.y + (imageRect.height*(float)coefficientY); // Initial Y position.

        // If repeating along the X-axis, move the image's representative rectangle to that X.
        if (imageLayer.repeatX) imageRect.x = x0;

        if (imageLayer.repeatY) imageRect.y = y0;

        if (CheckCollisionRecs(viewport, imageRect)) // If the repeating image would be visible in the viewport.
        {
            // Take a step back on each axis that the image repeats on. This ensures we don't leave any empty space
            // along the left and/or top edge of the viewport.
            if (imageLayer.repeatX) x0 -= imageRect.width;
            if (imageLayer.repeatY) y0 -= imageRect.height;

            // Create some unchanging objects that the draw function will need.
            // Region within the texture to be drawn. We'll use the whole texture.
            const Rectangle sourceRect = { 0.0f, 0.0f, (float)imageLayer.image.width, (float)imageLayer.image.height };
            // Reference point used for rotations. We're not rotating so we'll just use all zeroes.
            const Vector2 origin = { 0.0f, 0.0f };

            if (imageLayer.repeatX && imageLayer.repeatY) // If repeating on both axes.
            {
                // Loop over both the X and Y axes to draw an array of repeated images.
                for (float x = x0; x <= viewport.x + viewport.width; x += imageRect.width)
                {
                    for (float y = y0; y <= viewport.y + viewport.height; y += imageRect.height)
                    {
                        imageRect.x = x;
                        imageRect.y = y;
                        DrawTexturePro(imageLayer.image.texture, sourceRect, imageRect, origin, 0.0f, tint);
                    }
                }
            }
            else if (imageLayer.repeatX) // If repeating on just the X-axis.
            {
                // Loop over just the X-axis to draw a horizontal line of repeated images.
                for (float x = x0; x <= viewport.x + viewport.width; x += imageRect.width)
                {
                    imageRect.x = x;
                    DrawTexturePro(imageLayer.image.texture, sourceRect, imageRect, origin, 0.0f, tint);
                }
            }
            else if (imageLayer.repeatY) // If repeating on just the Y-axis.
            {
                // Loop over just the Y-axis to draw a vertical line of repeated images.
                for (float y = y0; y <= viewport.y + viewport.height; y += imageRect.height)
                {
                    imageRect.y = y;
                    DrawTexturePro(imageLayer.image.texture,sourceRect, imageRect, origin, 0.0f, tint);
                }
            }
        }
    }
}

// Check for collisions between a line and a polygon.
bool CheckCollisionLinePoly(Vector2 startPos, Vector2 endPos, Vector2 polyPos, Vector2 *points, int pointCount)
{
    if (pointCount < 3) return false;

    // Cycle through each edge of the polygon.
    int nextIndex = 0;
    for (int currentIndex = 0; currentIndex < pointCount; currentIndex++)
    {
        // Get the next index. If the current index is the last, wrap around.
        nextIndex = currentIndex + 1;
        if (nextIndex == pointCount) nextIndex = 0;

        // Get the current and next points. These two points form an edge of the polygon.
        const Vector2 currentPoint = points[currentIndex];
        const Vector2 nextPoint = points[nextIndex];

        // Check these edges for collisions. Note: The last parameter is unused, hence NULL.
        if (CheckCollisionLines(startPos, endPos, currentPoint, nextPoint, NULL)) return true;
    }

    return false;
}

// Check for collisions between two polygons.
bool CheckCollisionPolyPoly(Vector2 polyPos1, Vector2 *points1, int pointCount1, Vector2 polyPos2, Vector2 *points2,
    int pointCount2)
{
    // If either vertex array is missing or does not contain enough vertices to define a polygon.
    if ((points1 == NULL) || (pointCount1 < 3) || (points2 == NULL) || (pointCount2 < 3)) return false;

    // Cycle through each edge of polygon 1.
    int nextIndex = 0;
    for (int currentIndex = 0; currentIndex < pointCount1; currentIndex++)
    {
        // Get the next index. If the current index is the last, wrap around.
        nextIndex = currentIndex + 1;
        if (nextIndex == pointCount1) nextIndex = 0;

        // Get the current and next points. These two points form an edge of polygon 1.
        Vector2 currentPoint = points1[currentIndex];
        currentPoint.x += polyPos1.x;
        currentPoint.y += polyPos1.y;
        Vector2 nextPoint = points1[nextIndex];
        nextPoint.x += polyPos1.x;
        nextPoint.y += polyPos1.y;

        // Check this edge for collisions against edges of polygon 2.
        if (CheckCollisionLinePoly(currentPoint, nextPoint, polyPos2, points2, pointCount2)) return true;
    }

    // Check if polygon 1 is fully inside polygon 2.
    if (CheckCollisionPointPoly(points1[0], points2, pointCount2)) return true;

    // Check if polygon 2 is fully inside polygon 1.
    if (CheckCollisionPointPoly(points2[0], points1, pointCount1)) return true;

    return false;
}

RAYTMX_DEC bool CheckCollisionTMXObjects(TmxObject object1, TmxObject object2)
{
    // Perform a quick collision check on the Axis-Aligned Bounding Boxes (AABB) before more accurate checks.
    if (!CheckCollisionRecs(object1.aabb,object2.aabb)) return false; // AABBs don't collide so the objects don't.

    switch (object1.type)
    {
        case OBJECT_TYPE_RECTANGLE: // Object 1's type.
        case OBJECT_TYPE_ELLIPSE: // Object 1's type (treated as a rectangle due to difficulty).
        case OBJECT_TYPE_TEXT: // Object 1's type.
        case OBJECT_TYPE_TILE: // Object 1's type.
            switch (object2.type)
            {
                case OBJECT_TYPE_RECTANGLE: // Object 2's type.
                case OBJECT_TYPE_ELLIPSE: // Object 2's type (treated as a rectangle due to difficulty).
                case OBJECT_TYPE_TEXT: // Object 2's type.
                case OBJECT_TYPE_TILE: // Object 2's type.
                {
                    // The objects' shapes, as far as collisions are concerned, are identical to the AABB. We already
                    // determined the AABBs collide so these objects collide.
                    return true;
                }

                case OBJECT_TYPE_POINT: // Object 2's type.
                {
                    Vector2 point;
                    point.x = (float)object2.x;
                    point.y = (float)object2.y;
                    return CheckCollisionPointRec(point, object1.aabb);
                }

                case OBJECT_TYPE_POLYGON: // Object 2's type.
                case OBJECT_TYPE_POLYLINE: // Object 2's type.
                {
                    // A rectangle is a polygon. Create an array of points to treat it as a polygon keeping in mind
                    // polygon vertices are relative so the top-left corner is always (0, 0).
                    Vector2 points[4];
                    points[0].x = 0.0f;
                    points[0].y = 0.0f;
                    points[1].x = (float)object1.width;
                    points[1].y = 0.0f;
                    points[2].x = (float)object1.width;
                    points[2].y = (float)object1.height;
                    points[3].x = 0.0f;
                    points[3].y = (float)object1.height;
                    Vector2 position1;
                    position1.x = (float)object1.x;
                    position1.y = (float)object1.y;
                    Vector2 position2;
                    position2.x = (float)object2.x;
                    position2.y = (float)object2.y;
                    return CheckCollisionPolyPoly(position1, points, 4, position2, object2.points,
                        object2.pointsLength);
                }
        } break;

        case OBJECT_TYPE_POINT: // Object 1's type.
            switch (object2.type)
            {
                case OBJECT_TYPE_RECTANGLE: // Object 2's type.
                case OBJECT_TYPE_ELLIPSE: // Object 2's type (treated as a rectangle due to difficulty).
                case OBJECT_TYPE_TEXT: // Object 2's type.
                case OBJECT_TYPE_TILE: // Object 2's type.
                {
                    Vector2 point;
                    point.x = (float)object1.x;
                    point.y = (float)object1.y;
                    return CheckCollisionPointRec(point, object2.aabb);
                }

                case OBJECT_TYPE_POINT: // Object 2's type.
                {
                    // Check for approximate equality. Floating-point values tend to have some margin of error so we'll
                    // look for X and Y positions that are very close rather than bit-for-bit identical.
                    return (fabs(object1.x - object2.x) < 0.01) && (fabs(object1.y - object2.y) < 0.01);
                }

                case OBJECT_TYPE_POLYGON: // Object 2's type.
                case OBJECT_TYPE_POLYLINE: // Object 2's type.
                {
                    Vector2 point;
                    point.x = (float)object1.x;
                    point.y = (float)object1.y;
                    return CheckCollisionPointPoly(point, object2.points, object2.pointsLength);
                }
            } break;

        case OBJECT_TYPE_POLYGON: // Object 1's type.
        case OBJECT_TYPE_POLYLINE: // Object 1's type.
            switch (object2.type)
            {
                case OBJECT_TYPE_RECTANGLE: // Object 2's type.
                case OBJECT_TYPE_ELLIPSE: // Object 2's type (treated as a rectangle due to difficulty).
                case OBJECT_TYPE_TEXT: // Object 2's type.
                case OBJECT_TYPE_TILE: // Object 2's type.
                {
                    // A rectangle is a polygon. Create an array of points to treat it as a polygon keeping in mind
                    // polygon vertices are relative so the top-left corner is always (0, 0).
                    Vector2 points[4];
                    points[0].x = 0.0f;
                    points[0].y = 0.0f;
                    points[1].x = (float)object2.width;
                    points[1].y = 0.0f;
                    points[2].x = (float)object2.width;
                    points[2].y = (float)object2.height;
                    points[3].x = 0.0f;
                    points[3].y = (float)object2.height;
                    Vector2 position1;
                    position1.x = (float)object1.x;
                    position1.y = (float)object1.y;
                    Vector2 position2;
                    position2.x = (float)object2.x;
                    position2.y = (float)object2.y;
                    return CheckCollisionPolyPoly(position1, points, 4, position2, object2.points,
                        object2.pointsLength);
                }

                case OBJECT_TYPE_POINT: // Object 2's type.
                {
                    Vector2 point;
                    point.x = (float)object2.x;
                    point.y = (float)object2.y;
                    return CheckCollisionPointPoly(point, object1.points, object1.pointsLength);
                }

                case OBJECT_TYPE_POLYGON: // Object 2's type.
                case OBJECT_TYPE_POLYLINE: // Object 2's type.
                {
                    Vector2 position1;
                    position1.x = (float)object1.x;
                    position1.y = (float)object1.y;
                    Vector2 position2;
                    position2.x = (float)object2.x;
                    position2.y = (float)object2.y;
                    return CheckCollisionPolyPoly(position1, object1.points, object1.pointsLength, position2,
                        object2.points, object2.pointsLength);
                }
            } break;
    }

    return false;
}

// Helper function that applies a translation to an object.
TmxObject TranslateObject(TmxObject object, float dx, float dy)
{
    // Translate the position of the object by the different X and Y deltas.
    object.x += (double)dx;
    object.y += (double)dy;

    // Translate the Axis-Aligned Bounding Box (AABB) to match.
    object.aabb.x += dx;
    object.aabb.y += dy;

    // Note: Although polygons and polylines have a series of vertices, they are relative to the object. This means
    // nothing more needs to be done. Translating the object's X and Y effectively translates each vertex.
    return object;
}

// Check for collisions between a tile layer, or group layer, and an arbitrary object. The tiles must have collision
// information created with the Tiled Collision Editor. 'layers' is an array of length 'layersLength'. 'outputObject' is
// assigned with the object collided with, if not NULL.
bool CheckCollisionTMXTileLayerObject(const TmxMap *map, const TmxLayer *layers, uint32_t layersLength,
    TmxObject object, TmxObject *outputObject)
{
    if ((map == NULL) || (layers == NULL) || (layersLength == 0)) return false;

    // Iterate through each layer and check their tiles for collisions with the given object.
    for (uint32_t i = 0; i < layersLength; i++)
    {
        if (layers[i].type == LAYER_TYPE_TILE_LAYER) // If the layer has tiles.
        {
            // Iterate through each tile that the object's Axis-Aligned Bounding Box (AABB) overlaps with.
            TmxTile tile = { 0 };
            Rectangle tileRect = { 0 };
            RaytmxTransform transform;
            transform.position.x = 0.0f;
            transform.position.y = 0.0f;
            transform.parallax.x = 1.0f;
            transform.parallax.y = 1.0f;
            transform.cameraOffset.x = 0.0f;
            transform.cameraOffset.y = 0.0f;
            while (IterateTileLayer(map, &(layers[i].exact.tileLayer), object.aabb, transform, NULL, &tile, &tileRect))
            {
                // Iterate through each object associated with the tile.
                for (uint32_t j = 0; j < tile.objectGroup.objectsLength; j++)
                {
                    // This object, the tile's collision information, has a relative position so this object must be
                    // translated to the position of the tile as it would be drawn with the layer.
                    const TmxObject object2 = TranslateObject(tile.objectGroup.objects[j], tileRect.x, tileRect.y);
                    // If this tile's object collides with the given object.
                    if (CheckCollisionTMXObjects(object2, object))
                    {
                        if (outputObject != NULL) *outputObject = object2;
                        return true; // Found a collision.
                    }
                }
            }
        }
        else if (layers[i].type == LAYER_TYPE_GROUP) // If the layer contains other layers.
        {
            if (CheckCollisionTMXTileLayerObject(map, layers[i].layers, layers[i].layersLength, object, outputObject))
                return true;
        }
    }

    return false;
}

// Check for collisions between an object group and an arbitrary object.
bool CheckCollisionTMXObjectGroupObject(TmxObjectGroup group, TmxObject object, TmxObject *outputObject)
{
    for (size_t i = 0; i < group.objectsLength; i++)
    {
        if (CheckCollisionTMXObjects(group.objects[i], object))
        {
            if (outputObject != NULL) *outputObject = group.objects[i];
            return true;
        }
    }

    return false;
}

void TraceLogTMXTilesets(int logLevel, TmxOrientation orientation, TmxTileset *tilesets, uint32_t tilesetsLength,
    int numSpaces)
{
    for (uint32_t i = 0; i < tilesetsLength; i++)
    {
        const TmxTileset tileset = tilesets[i];

        if (i == 0) TraceLog(logLevel, "tilesets:");
        TraceLog(logLevel, "  \"%s\":", tileset.name);
        TraceLog(logLevel, "    first GID: %u", tileset.firstGid);
        TraceLog(logLevel, "    last GID: %u", tileset.lastGid);
        if (tileset.source != NULL) TraceLog(logLevel, "    source: \"%s\"", tileset.source);
        if (tileset.classString[0] != '\0') TraceLog(logLevel, "    class: \"%s\"", tileset.classString);
        TraceLog(logLevel, "    tile width: %u", tileset.tileWidth);
        TraceLog(logLevel, "    tile height: %u", tileset.tileHeight);
        if (tileset.spacing != 0) TraceLog(logLevel, "    spacing: %u", tileset.spacing);
        if (tileset.margin != 0) TraceLog(logLevel, "    margin: %u", tileset.margin);
        TraceLog(logLevel, "    tile count: %u", tileset.tileCount);
        TraceLog(logLevel, "    columns: %u", tileset.columns);
        if (tileset.tileOffsetX != 0) TraceLog(logLevel, "    tile offset X: %d", tileset.tileOffsetX);
        if (tileset.tileOffsetY != 0) TraceLog(logLevel, "    tile offset Y: %d", tileset.tileOffsetY);

        if (((orientation == ORIENTATION_ORTHOGONAL) && (tileset.objectAlignment != OBJECT_ALIGNMENT_BOTTOM_LEFT)) ||
            ((orientation == ORIENTATION_ISOMETRIC) && (tileset.objectAlignment != OBJECT_ALIGNMENT_BOTTOM)))
        {
            switch (tileset.objectAlignment)
            {
                case OBJECT_ALIGNMENT_UNSPECIFIED: TraceLog(logLevel, "    object alignment: unspecified"); break;
                case OBJECT_ALIGNMENT_TOP_LEFT: TraceLog(logLevel, "    object alignment: top-left"); break;
                case OBJECT_ALIGNMENT_TOP: TraceLog(logLevel, "    object alignment: top"); break;
                case OBJECT_ALIGNMENT_TOP_RIGHT: TraceLog(logLevel, "    object alignment: top-right"); break;
                case OBJECT_ALIGNMENT_LEFT: TraceLog(logLevel, "    object alignment: left"); break;
                case OBJECT_ALIGNMENT_CENTER: TraceLog(logLevel, "    object alignment: center"); break;
                case OBJECT_ALIGNMENT_RIGHT: TraceLog(logLevel, "    object alignment: right"); break;
                case OBJECT_ALIGNMENT_BOTTOM_LEFT: TraceLog(logLevel, "    object alignment: bottom-left"); break;
                case OBJECT_ALIGNMENT_BOTTOM: TraceLog(logLevel, "    object alignment: bottom"); break;
                case OBJECT_ALIGNMENT_BOTTOM_RIGHT: TraceLog(logLevel, "    object alignment: bottom-right"); break;
            }
        }

        if (tileset.hasImage)
        {
            TraceLog(logLevel, "    image:");
            TraceLog(logLevel, "      source: \"%s\"", tileset.image.source);
            if (tileset.image.hasTrans) TraceLog(logLevel, "      trans: 0x%08X", tileset.image.trans);
            if (tileset.image.width != 0) TraceLog(logLevel, "      width: %u", tileset.image.width);
            if (tileset.image.height != 0) TraceLog(logLevel, "      height: %u", tileset.image.height);
            TraceLog(logLevel, "      texture (ID): %u", tileset.image.texture.id);
        }

        for (uint32_t j = 0; j < tileset.tilesLength; j++)
        {
            const TmxTilesetTile tile = tileset.tiles[j];

            if (j == 0) TraceLog(logLevel, "    tiles:");
            TraceLog(logLevel, "      ID: %u", tile.id);

            if (tile.hasAnimation)
            {
                for (uint32_t i = 0; i < tile.animation.framesLength; i++)
                {
                    if (i == 0) TraceLog(logLevel, "      frames:");
                    TraceLog(logLevel, "        (G)ID: %u", tile.animation.frames[i].gid);
                    if (tileset.classString[0] != '\0')
                        TraceLog(logLevel, "          class: \"%s\"", tileset.classString);
                    TraceLog(logLevel, "          duration: %f", tile.animation.frames[i].duration);
                }
            }

            TraceLogTMXProperties(logLevel, tile.properties, tile.propertiesLength, 8);
            if (tile.hasImage && (tile.image.texture.id != tileset.image.texture.id))
            {
                // The 'x,' 'y,' 'width,' and 'height' attributes relate to the image so only log them if one exists.
                if (tile.x != 0) TraceLog(logLevel, "      x: %d", tile.x);
                if (tile.y != 0) TraceLog(logLevel, "      y: %d", tile.y);
                if (tile.width != tile.image.width) TraceLog(logLevel, "      width: %u", tile.width);
                if (tile.height != tile.image.height) TraceLog(logLevel, "      height: %u", tile.height);
                TraceLog(logLevel, "      image:");
                TraceLog(logLevel, "        source: \"%s\"", tile.image.source);
                if (tile.image.hasTrans) TraceLog(logLevel, "        trans: 0x%08X", tile.image.trans);
                if (tile.image.width != 0) TraceLog(logLevel, "        width: %u", tile.image.width);
                if (tile.image.height != 0) TraceLog(logLevel, "        height: %u", tile.image.height);
                TraceLog(logLevel, "        texture (ID): %u", tile.image.texture.id);
            }

            if (tile.objectGroup.objectsLength > 0)
            {
                if (tmxLogFlags & LOG_SKIP_OBJECTS)
                    TraceLog(logLevel, "      skipping %u objects", tile.objectGroup.objectsLength);
                else
                {
                    TraceLog(logLevel, "      objects:");
                    for (uint32_t k = 0; k < tile.objectGroup.objectsLength; k++)
                        TraceLogTMXObject(logLevel, tile.objectGroup.objects[k], numSpaces + 2);
                }
            }
        }

        TraceLogTMXProperties(logLevel, tileset.properties, tileset.propertiesLength, 2);
    }
}

void TraceLogTMXProperties(int logLevel, TmxProperty *properties, uint32_t propertiesLength, int numSpaces)
{
    char padding[16] = { 0 };
    StringCopyN(padding, "                ", numSpaces);

    if (tmxLogFlags & LOG_SKIP_PROPERTIES) TraceLog(logLevel, "%sskipped %u properties", padding, propertiesLength);
    else
    {
        for (uint32_t i = 0; i < propertiesLength; i++)
        {
            const TmxProperty property = properties[i];

            if (i == 0) TraceLog(logLevel, "%sproperties:", padding);

            switch (property.type)
            {
                case PROPERTY_TYPE_STRING:
                case PROPERTY_TYPE_FILE:
                {
                    TraceLog(logLevel, "%s  \"%s\": \"%s\"", padding, property.name, property.stringValue);
                } break;
                case PROPERTY_TYPE_INT:
                case PROPERTY_TYPE_OBJECT:
                {
                    TraceLog(logLevel, "%s  \"%s\": %d", padding, property.name, property.intValue);
                } break;
                case PROPERTY_TYPE_FLOAT:
                {

                    TraceLog(logLevel, "%s  \"%s\": %f", padding, property.name, property.floatValue);
                } break;
                case PROPERTY_TYPE_BOOL:
                {

                    TraceLog(logLevel, "%s  \"%s\": %s", padding, property.name, property.boolValue? "true" : "false");
                } break;
                case PROPERTY_TYPE_COLOR:
                {

                    TraceLog(logLevel, "%s  \"%s\": 0x%08X", padding, property.name, property.colorValue);
                } break;
            }
        }
    }
}

void TraceLogTMXLayers(int logLevel, TmxLayer *layers, uint32_t layersLength, int numSpaces)
{
    char padding[16] = { 0 };
    StringCopyN(padding, "                ", numSpaces);

    if (tmxLogFlags & LOG_SKIP_LAYERS) TraceLog(logLevel, "%sskipped %u layers", padding, layersLength);
    else
    {
        uint32_t numTileLayers = 0;
        uint32_t numObjectGroups = 0;
        uint32_t numImageLayers = 0;
        for (uint32_t i = 0; i < layersLength; i++)
        {
            const TmxLayer layer = layers[i];

            if (i == 0) TraceLog(logLevel, "%slayers:", padding);

            // If, based on the layer type, this layer isn't one that should be skipped.
            if ((layer.type == LAYER_TYPE_GROUP) ||
                ((layer.type == LAYER_TYPE_TILE_LAYER) && !(tmxLogFlags & LOG_SKIP_TILE_LAYERS)) ||
                ((layer.type == LAYER_TYPE_OBJECT_GROUP) && !(tmxLogFlags & LOG_SKIP_OBJECT_GROUPS)) ||
                ((layer.type == LAYER_TYPE_IMAGE_LAYER) && !(tmxLogFlags & LOG_SKIP_IMAGE_LAYERS)))
            {
                // Log the attributes of this layer common to all layers.
                TraceLog(logLevel, "%s  \"%s\":", padding, layer.name);
                switch (layer.type)
                {
                    case LAYER_TYPE_TILE_LAYER: TraceLog(logLevel, "%s    type: tile layer", padding); break;
                    case LAYER_TYPE_OBJECT_GROUP: TraceLog(logLevel, "%s    type: object layer", padding); break;
                    case LAYER_TYPE_IMAGE_LAYER: TraceLog(logLevel, "%s    type: image layer", padding); break;
                    case LAYER_TYPE_GROUP: TraceLog(logLevel, "%s    type: group", padding); break;
                }
                if (layer.id > 0) TraceLog(logLevel, "%s    ID: %u", padding, layer.id);
                if (!layer.visible) TraceLog(logLevel, "%s    visible: false", padding);
                if (layer.classString[0] != '\0') TraceLog(logLevel, "%s    class: \"%s\"", padding, layer.classString);
                if (layer.offsetX != 0) TraceLog(logLevel, "%s    offset X: %d", padding, layer.offsetX);
                if (layer.offsetY != 0) TraceLog(logLevel, "%s    offset Y: %d", padding, layer.offsetY);
                if (layer.parallaxX != 1.0) TraceLog(logLevel, "%s    parallax X: %f", padding, layer.parallaxX);
                if (layer.parallaxY != 1.0) TraceLog(logLevel, "%s    parallax Y: %f", padding, layer.parallaxY);
                if (layer.opacity != 1.0) TraceLog(logLevel, "%s    opacity: %f", padding, layer.opacity);
                if (layer.hasTintColor) TraceLog(logLevel, "%s    tint color: 0x%08X", padding, layer.tintColor);
                TraceLogTMXProperties(logLevel, layer.properties, layer.propertiesLength, numSpaces + 4);
            }

            // Log attributes specific to the layer's type (tile layer, object layer, image layer, or group).
            switch (layer.type)
            {
                case LAYER_TYPE_TILE_LAYER:
                {
                    numTileLayers += 1;
                    if (tmxLogFlags & LOG_SKIP_TILE_LAYERS) continue;

                    if (layer.exact.tileLayer.width != 0)
                        TraceLog(logLevel, "%s    width: %u", padding, layer.exact.tileLayer.width);
                    if (layer.exact.tileLayer.height != 0)
                        TraceLog(logLevel, "%s    height: %u", padding, layer.exact.tileLayer.height);
                    if (tmxLogFlags & LOG_SKIP_TILES)
                        TraceLog(logLevel, "%s    skipping %u tiles", padding, layer.exact.tileLayer.tilesLength);
                    else
                    {
                        for (uint32_t j = 0; j < layer.exact.tileLayer.tilesLength; j++)
                        {
                            if (j == 0) TraceLog(logLevel, "%s    tiles:", padding);
                            TraceLog(logLevel, "%s      GID: %u", padding, layer.exact.tileLayer.tiles[j]);
                        }
                    }
                } break;
                case LAYER_TYPE_OBJECT_GROUP:
                {
                    numObjectGroups += 1;
                    if (tmxLogFlags & LOG_SKIP_OBJECT_GROUPS) continue;
                    if (layer.exact.objectGroup.hasColor)
                        TraceLog(logLevel, "%s    color: 0x%08X", padding, layer.exact.objectGroup.color);
                    // if (layer.exact.objectGroup.width != 0)
                    //     TraceLog(logLevel, "%s    width: %u", padding, layer.exact.objectGroup.width);
                    // if (layer.exact.objectGroup.height != 0)
                    //     TraceLog(logLevel, "%s    height: %u", padding, layer.exact.objectGroup.height);
                    if (layer.exact.objectGroup.drawOrder != OBJECT_GROUP_DRAW_ORDER_TOP_DOWN)
                    {
                        switch (layer.exact.objectGroup.drawOrder)
                        {
                            case OBJECT_GROUP_DRAW_ORDER_TOP_DOWN:
                            default:
                            {
                                TraceLog(logLevel, "%s    draw order: top-down", padding);
                            } break;
                            case OBJECT_GROUP_DRAW_ORDER_INDEX:
                            {
                                TraceLog(logLevel, "%s    draw order: index", padding);
                            } break;
                        }
                    }
                    if (tmxLogFlags & LOG_SKIP_OBJECTS)
                        TraceLog(logLevel, "%s    skipping %u objects", padding, layer.exact.objectGroup.objectsLength);
                    else
                    {
                        for (uint32_t j = 0; j < layer.exact.objectGroup.objectsLength; j++)
                        {
                            if (j == 0) TraceLog(logLevel, "%s    objects:", padding);
                            TraceLogTMXObject(logLevel, layer.exact.objectGroup.objects[j], numSpaces);
                        }
                    }
                } break;
                case LAYER_TYPE_IMAGE_LAYER:
                {
                    numImageLayers += 1;
                    if (tmxLogFlags & LOG_SKIP_IMAGE_LAYERS) continue;

                    if (layer.exact.imageLayer.repeatX) TraceLog(logLevel, "%s    repeat X: true", padding);
                    if (layer.exact.imageLayer.repeatY) TraceLog(logLevel, "%s    repeat Y: true", padding);
                    if (layer.exact.imageLayer.hasImage)
                    {
                        const TmxImage image = layer.exact.imageLayer.image;

                        TraceLog(logLevel, "%s    image:", padding);
                        TraceLog(logLevel, "%s      source: \"%s\"", padding, image.source);
                        if (layer.exact.imageLayer.image.hasTrans)
                            TraceLog(logLevel, "%s      trans: 0x%08X", padding, image.trans);
                        if (layer.exact.imageLayer.image.width != 0)
                            TraceLog(logLevel, "%s      width: %u", padding, image.width);
                        if (layer.exact.imageLayer.image.height != 0)
                            TraceLog(logLevel, "%s      height: %u", padding, image.height);
                        TraceLog(logLevel, "%s      texture (ID): %u", padding, image.texture.id);
                    }
                } break;
                case LAYER_TYPE_GROUP:
                {
                    TraceLogTMXLayers(logLevel, layer.layers, layer.layersLength, numSpaces + 4);
                } break;
            }
        }
        if (tmxLogFlags & LOG_SKIP_TILE_LAYERS && (numTileLayers > 0))
            TraceLog(logLevel, "%s  skipped %u tile layers", padding, numTileLayers);
        if (tmxLogFlags & LOG_SKIP_OBJECT_GROUPS && (numObjectGroups > 0))
            TraceLog(logLevel, "%s  skipped %u object layers", padding, numObjectGroups);
        if (tmxLogFlags & LOG_SKIP_IMAGE_LAYERS && (numImageLayers > 0))
            TraceLog(logLevel, "%s  skipped %u image layers", padding, numImageLayers);
    }
}

void TraceLogTMXObject(int logLevel, TmxObject object, int numSpaces)
{
    char padding[16] = { 0 };
    StringCopyN(padding, "                ", numSpaces);

    TraceLog(logLevel, "%s      ID: %u", padding, object.id);
    TraceLog(logLevel, "%s        name: \"%s\"", padding, object.name);
    switch (object.type)
    {
        case OBJECT_TYPE_RECTANGLE: TraceLog(logLevel, "%s        type: quad", padding); break;
        case OBJECT_TYPE_ELLIPSE: TraceLog(logLevel, "%s        type: ellipse", padding); break;
        case OBJECT_TYPE_POINT: TraceLog(logLevel, "%s        type: point", padding); break;
        case OBJECT_TYPE_POLYGON: TraceLog(logLevel, "%s        type: polygon", padding); break;
        case OBJECT_TYPE_POLYLINE: TraceLog(logLevel, "%s        type: polyline", padding); break;
        case OBJECT_TYPE_TEXT: TraceLog(logLevel, "%s        type: text", padding); break;
        case OBJECT_TYPE_TILE: TraceLog(logLevel, "%s        type: tile", padding); break;
    }

    if (object.typeString[0] != '\0') TraceLog(logLevel, "%s        type: \"%s\"", padding, object.typeString);
    if (object.x != 0.0) TraceLog(logLevel, "%s        x: %f", padding, object.x);
    if (object.y != 0.0) TraceLog(logLevel, "%s        y: %f", padding, object.y);
    if (object.width != 0.0) TraceLog(logLevel, "%s        width: %f", padding, object.width);
    if (object.height != 0.0) TraceLog(logLevel, "%s        height: %f", padding, object.height);
    if (object.rotation != 0.0) TraceLog(logLevel, "%s        rotation: %f", padding, object.rotation);
    if (object.gid > 0) TraceLog(logLevel, "%s        GID: %u", padding, object.gid);
    if (!object.visible) TraceLog(logLevel, "%s        visible: false", padding);
    if (object.templateString != NULL) TraceLog(logLevel, "%s        template: \"%s\"", padding, object.templateString);
    for (uint32_t k = 0; k < object.pointsLength; k++)
    {
        if (k == 0) TraceLog(logLevel, "%s        points:", padding);
        TraceLog(logLevel, "%s          [%f, %f]", padding, object.points[k].x, object.points[k].y);
    }

    TraceLogTMXProperties(logLevel, object.properties, object.propertiesLength, numSpaces + 8);
    if (object.text != NULL)
    {
        TraceLog(logLevel, "%s        font family: \"%s\"", padding, object.text->fontFamily);
        TraceLog(logLevel, "%s        pixel size: %u", padding, object.text->pixelSize);
        if (object.text->wrap) TraceLog(logLevel, "%s        wrap: true", padding);
        TraceLog(logLevel, "%s        color: 0x%08X", padding, object.text->color);
        if (object.text->bold) TraceLog(logLevel, "%s        bold: true", padding);
        if (object.text->italic) TraceLog(logLevel, "%s        italic: true", padding);
        if (object.text->underline) TraceLog(logLevel, "%s        underline: true", padding);
        if (object.text->strikeOut) TraceLog(logLevel, "%s        strike out: true", padding);
        if (object.text->kerning) TraceLog(logLevel, "%s        kerning: true", padding);
        switch (object.text->halign)
        {
            case HORIZONTAL_ALIGNMENT_LEFT: TraceLog(logLevel, "%s        horizontal align: left", padding); break;
            case HORIZONTAL_ALIGNMENT_CENTER: TraceLog(logLevel, "%s        horizontal align: center", padding); break;
            case HORIZONTAL_ALIGNMENT_RIGHT: TraceLog(logLevel, "%s        horizontal align: right", padding); break;
            case HORIZONTAL_ALIGNMENT_JUSTIFY:
            {
                TraceLog(logLevel, "%s        horizontal align: justify", padding);
            } break;
        }
        switch (object.text->valign)
        {
            case VERTICAL_ALIGNMENT_TOP: TraceLog(logLevel, "%s        vertical align: top", padding); break;
            case VERTICAL_ALIGNMENT_CENTER: TraceLog(logLevel, "%s        vertical align: center", padding); break;
            case VERTICAL_ALIGNMENT_BOTTOM: TraceLog(logLevel, "%s        vertical align: bottom", padding); break;
        }

        if (object.text->content[0] != '\0')
            TraceLog(logLevel, "%s        content: \"%s\"", padding, object.text->content);
    }
}

TmxProperty *AddProperty(RaytmxState *state)
{
    RaytmxPropertyNode *node = (RaytmxPropertyNode *)MemAllocZero(sizeof(RaytmxPropertyNode));

    // Use this node as the root if there is no root. Append it to the tail otherwise.
    if (state->propertiesRoot == NULL) state->propertiesRoot = node;
    else state->propertiesTail->next = node;
    state->propertiesTail = node;
    state->propertiesLength += 1;

    return &(node->property);
}

void AddTileLayerTile(RaytmxState *state, uint32_t gid)
{
    RaytmxTileLayerTileNode *node = (RaytmxTileLayerTileNode *)MemAllocZero(sizeof(RaytmxTileLayerTileNode));
    node->gid = gid;

    // Use this node as the root if there is no root. Append it to the tail otherwise.
    if (state->layerTilesRoot == NULL) state->layerTilesRoot = node;
    else state->layerTilesTail->next = node;
    state->layerTilesTail = node;
    state->layerTilesLength += 1;
}

TmxTileset *AddTileset(RaytmxState *state)
{
    RaytmxTilesetNode *node = (RaytmxTilesetNode *)MemAllocZero(sizeof(RaytmxTilesetNode));

    // Use this node as the root if there is no root. Append it to the tail otherwise.
    if (state->tilesetsRoot == NULL) state->tilesetsRoot = node;
    else state->tilesetsTail->next = node;
    state->tilesetsTail = node;
    state->tilesetsLength += 1;

    return &(node->tileset);
}

TmxTilesetTile *AddTilesetTile(RaytmxState *state)
{
    RaytmxTilesetTileNode *node = (RaytmxTilesetTileNode *)MemAllocZero(sizeof(RaytmxTilesetTileNode));

    // Use this node as the root if there is no root. Append it to the tail otherwise.
    if (state->tilesetTilesRoot == NULL) state->tilesetTilesRoot = node;
    else state->tilesetTilesTail->next = node;
    state->tilesetTilesTail = node;
    state->tilesetTilesLength += 1;

    return &(node->tile);
}

TmxAnimationFrame *AddAnimationFrame(RaytmxState *state)
{
    RaytmxAnimationFrameNode *node = (RaytmxAnimationFrameNode *)MemAllocZero(sizeof(RaytmxAnimationFrameNode));

    // Use this node as the root if there is no root. Append it to the tail otherwise.
    if (state->animationFramesRoot == NULL) state->animationFramesRoot = node;
    else state->animationFramesTail->next = node;
    state->animationFramesTail = node;
    state->animationFramesLength += 1;

    return &(node->frame);
}

TmxLayer *AddGenericLayer(RaytmxState *state, bool isGroup)
{
    RaytmxLayerNode *node = (RaytmxLayerNode *)MemAllocZero(sizeof(RaytmxLayerNode));
    // There are some non-zero default values for several layer attributes.
    node->layer.opacity = 1.0;
    node->layer.visible = true;
    node->layer.parallaxX = 1.0;
    node->layer.parallaxY = 1.0;

    if (state->groupNode != NULL) // If the layer is being appended to a <group>.
    {
        node->parent = state->groupNode;
        // Use this node as the root if there is no root. Append it to the tail otherwise.
        if (state->groupNode->childrenRoot == NULL) state->groupNode->childrenRoot = node;
        else state->groupNode->childrenTail->next = node;
        state->groupNode->childrenTail = node;
        state->groupNode->childrenLength += 1;
    }
    else // If there is no <group> to append to, meaning the layer belongs to the top-level <map>.
    {
        // Use this node as the root if there is no root. Append it to the tail otherwise.
        if (state->layersRoot == NULL) state->layersRoot = node;
        else state->layersTail->next = node;
        state->layersTail = node;
        state->layersLength += 1;
    }

    if (isGroup) state->groupNode = node;

    return &(node->layer);
}

TmxObject *AddObject(RaytmxState *state)
{
    RaytmxObjectNode *node = (RaytmxObjectNode *)MemAllocZero(sizeof(RaytmxObjectNode));
    // <object> elements have one non-zero default value.
    node->object.visible = true;

    // Use this node as the root if there is no root. Append it to the tail otherwise.
    if (state->objectsRoot == NULL) state->objectsRoot = node;
    else state->objectsTail->next = node;
    state->objectsTail = node;
    state->objectsLength += 1;

    return &(node->object);
}

void AppendLayerTo(TmxMap *map, RaytmxLayerNode *groupNode, RaytmxLayerNode *layersRoot, uint32_t layersLength)
{
    if ((map == NULL) || (layersRoot == NULL) || (layersLength == 0)) return;

    TmxLayer *groupLayer = NULL;
    if (groupNode != NULL) groupLayer = &(groupNode->layer);

    // Allocate the array and zerioze every index as initialization.
    TmxLayer *layers = (TmxLayer *)MemAllocZero(sizeof(TmxLayer)*layersLength);

    // Copy the TmxLayers into the array.
    RaytmxLayerNode *layersIter = layersRoot;
    for (uint32_t i = 0; layersIter != NULL; i++)
    {
        if (layersIter->childrenRoot != NULL)
            AppendLayerTo(map, layersIter, layersIter->childrenRoot, layersIter->childrenLength);
        layers[i] = layersIter->layer;
        layersIter = layersIter->next;
    }

    if (groupLayer != NULL) // If the list of layers is being appended to a group.
    {
        groupLayer->layers = layers;
        groupLayer->layersLength = layersLength;
    }
    else // If the list of layers is being appended to the top-level map.
    {
        map->layers = layers;
        map->layersLength = layersLength;
    }
}

RaytmxCachedTextureNode *LoadCachedTexture(RaytmxState *raytmxState, const char *fileName)
{
    if ((raytmxState == NULL) || (fileName == NULL)) return NULL;

    // First try to find an already-loaded texture identified by the file name.
    RaytmxCachedTextureNode *cachedTextureNode = raytmxState->texturesRoot;
    while (cachedTextureNode != NULL)
    {
        // If the file name associated with the node matches the given file name.
        if (strcmp(cachedTextureNode->fileName, fileName) == 0) return cachedTextureNode;
        cachedTextureNode = cachedTextureNode->next;
    }
    // NOTE: If the function hasn't returned by this line, the texture is new and needs to be cached.

    // Try to load the texture.
    const char *fullPath = JoinPath(raytmxState->documentDirectory, fileName);
    const Texture2D texture = loadTextureOverride? loadTextureOverride(fullPath) : LoadTexture(fullPath);
    if (texture.id == 0) // If loading the texture failed.
    {
        TraceLog(LOG_ERROR, "RAYTMX: Unable to load texture \"%s\"", fullPath);
        return NULL;
    }

    // Create a new node in the list of known textures.
    cachedTextureNode = (RaytmxCachedTextureNode *)MemAllocZero(sizeof(RaytmxCachedTextureNode));
    cachedTextureNode->fileName = (char *)MemAllocZero((unsigned int)strlen(fileName) + 1);
    StringCopy(cachedTextureNode->fileName, fileName);
    cachedTextureNode->texture = texture;

    // Add to the cache. Use this node as the root if there is no root. Append it to the tail otherwise.
    if (raytmxState->texturesRoot == NULL) raytmxState->texturesRoot = cachedTextureNode;
    else
    {
        // Iterate to the tail and append this new node.
        RaytmxCachedTextureNode *cachedTextureIter = raytmxState->texturesRoot;
        while (cachedTextureIter->next != NULL) cachedTextureIter = cachedTextureIter->next;
        cachedTextureIter->next = cachedTextureNode;
    }

    return cachedTextureNode;
}

RaytmxCachedTemplateNode *LoadCachedTemplate(RaytmxState *raytmxState, const char *fileName)
{
    if ((raytmxState == NULL) || (fileName == NULL)) return NULL;

    // First try to find an already-loaded template identified by the file name.
    RaytmxCachedTemplateNode *cachedTemplateNode = raytmxState->templatesRoot;
    while (cachedTemplateNode != NULL)
    {
        // If the file name associated with the node matches the given file name.
        if (strcmp(cachedTemplateNode->fileName, fileName) == 0) return cachedTemplateNode;
        cachedTemplateNode = cachedTemplateNode->next;
    }
    // NOTE: If the function hasn't returned by this line, the template is new and needs to be cached.

    // Load the template from the external TX file.
    const char *fullPath = JoinPath(raytmxState->documentDirectory, fileName);
    const RaytmxObjectTemplate objectTemplate = LoadTX(fullPath);
    if (!objectTemplate.isSuccess) // If loading the template failed.
    {
        TraceLog(LOG_ERROR, "RAYTMX: Unable to load template \"%s\"", fullPath);
        return NULL;
    }

    // Create a new node in the list of known templates.
    cachedTemplateNode = (RaytmxCachedTemplateNode *)MemAllocZero(sizeof(RaytmxCachedTemplateNode));
    cachedTemplateNode->fileName = (char *)MemAllocZero((unsigned int)strlen(fileName) + 1);
    StringCopy(cachedTemplateNode->fileName, fileName);
    cachedTemplateNode->objectTemplate = objectTemplate;

    if (objectTemplate.hasTileset) // If the template contains a tileset in addition to an object.
    {
        // In cases where the template's object references a tile (i.e. its 'gid' attribute is set), the template will
        // have at most one tileset. Search the state object's list of tilesets and add this one if it's new.
        RaytmxTilesetNode *tilesetsIter = raytmxState->tilesetsRoot;
        bool isNew = true;
        while (tilesetsIter != NULL)
        {
            // If the existing tileset has a name, the template's tileset has a name, and they match OR if the existing
            // tileset has a source, template's tileset has a source, and they match.
            // TODO: The comparison of sources without respect to directory should be reviewed as it could result in
            // false negatives and duplicate tileset instances.
            if (((tilesetsIter->tileset.name != NULL) && (objectTemplate.tileset.name != NULL) &&
                (strcmp(tilesetsIter->tileset.name, objectTemplate.tileset.name) == 0)) ||
                ((tilesetsIter->tileset.source != NULL) && (objectTemplate.tileset.source != NULL) &&
                (strcmp(tilesetsIter->tileset.source, objectTemplate.tileset.source) == 0)))
            {
                isNew = false;
                break;
            }

            tilesetsIter = tilesetsIter->next;
        }

        if (isNew)
        {
            TmxTileset *tileset = AddTileset(raytmxState);
            *tileset = objectTemplate.tileset;
        }
    }

    // Add to the cache.
    if (raytmxState->templatesRoot == NULL) raytmxState->templatesRoot = cachedTemplateNode;
    else
    {
        // Iterate to the tail and append this new node.
        RaytmxCachedTemplateNode *cachedTemplateIter = raytmxState->templatesRoot;
        while (cachedTemplateIter->next != NULL) cachedTemplateIter = cachedTemplateIter->next;
        cachedTemplateIter->next = cachedTemplateNode;
    }

    return cachedTemplateNode;
}

Color GetColorFromHexString(const char *hex)
{
    Color color = BLACK; // { 0, 0, 0, 255 }.

    const size_t length = strlen(hex);
    // If the hex string is too short to contain at least R, G, and B components.
    if (length < 6) return color;

    // Hex strings are in the form #AARRGGBB or #RRGGBB meaning alpha is optional. To avoid any special logic for the
    // two cases where alpha is or isn't given, parsing will just be done backwards. Here, 'hex' is used as an iterator
    // so it will begin at the end of the string. For example, for "#789abc" this begins at 'c'.
    hex += length - 1;

    char component[] = "\0\0\0"; // Holds the two-digit component (e.g. "55" or "ff").
    for (size_t i = 0; i < length/2; i++) // Iterate three or four times with four meaning alpha was included.
    {
        component[1] = hex[0]; // Store the char 'hex' points to. For "#789abc" this is 'c', 'a', or '8'.
        hex--; // Point to the previous char in the string. For "#789abc" this now points to 'b', '9' or '7'.
        component[0] = hex[0]; // Store the other char of the current component.
        hex--; // Iterate backwards again in preparation for the next component.
        // For "#789abc" the 'component' array is now "bc", "9a", or "78".
        switch (i)
        {
            case 0: color.b = (unsigned char)strtoul(component, NULL, 16); break; // e.g. "ff" -> 255 for blue...
            case 1: color.g = (unsigned char)strtoul(component, NULL, 16); break; // ...for green...
            case 2: color.r = (unsigned char)strtoul(component, NULL, 16); break; // ...for red...
            case 3: color.a = (unsigned char)strtoul(component, NULL, 16); break; // ...for alpha. Maybe.
        }
    }

    return color;
}

uint32_t GetGid(uint32_t rawGid, bool *flipX, bool *flipY, bool *flipDiag, bool *rotateHexag120)
{
    // If the output parameters can be written to, output the status of the flip flags.
    if (flipX != NULL) *flipX = rawGid & FLIP_FLAG_HORIZONTAL;
    if (flipY != NULL) *flipY = rawGid & FLIP_FLAG_VERTICAL;
    if (flipDiag != NULL) *flipDiag = rawGid & FLIP_FLAG_DIAGONAL;
    if (rotateHexag120 != NULL) *rotateHexag120 = rawGid & FLIP_FLAG_ROTATE_120;

    // Return the integer with flip flags removed.
    return rawGid & ~(FLIP_FLAG_HORIZONTAL | FLIP_FLAG_VERTICAL | FLIP_FLAG_DIAGONAL | FLIP_FLAG_ROTATE_120);
}

void *MemAllocZero(unsigned int size)
{
    void *buffer = MemAlloc(size); // Reserve 'size' bytes of memory.
    memset(buffer, 0, size); // Initialize any values to zero, NULL, false, or an equivalent enum value.
    return buffer;
}

// Get an absolute directory path for a given file path. Returns a static string.
// raylib's GetDirectoryPath() doesn't work as described so this is used in its place.
const char *GetDirectoryPath2(const char *filePath)
{
    static char directoryPath[260] = { 0 }; // Max path length on Windows, the bottleneck, is 260 characters.
    memset(directoryPath, '\0', 260);

    size_t length = strlen(filePath);
    // Paths beginning with a Windows drive letter (C:\, D:\, etc.) or beginning with a slash are absolute paths.
    if ((length >= 2) && ((filePath[1] == ':') || (filePath[0] == '\\') || (filePath[0] == '/'))) // If absolute.
    {
        const bool isDirectory = !IsPathFile(filePath);
        StringCopy(directoryPath, filePath);

        // If the given file path is already an absolute path to a directory, return here.
        if (isDirectory) return directoryPath;
    }
    else StringCopy(directoryPath, JoinPath(GetWorkingDirectory(), filePath)); // If 'filePath' is relative.

    // The goal is to return part of 'filePath', up to the last slash.
    length = strlen(directoryPath);
    char *iter = directoryPath + length - 1;

    // Iterate backwards until a slash is found.
    while ((iter != directoryPath) && (*iter != '\0') && (*iter != '\\') && (*iter != '/')) iter -= 1;
    *(iter + 1) = '\0'; // Place a null terminator after the slash to effectively end the string there.
    return directoryPath;
}

const char *JoinPath(const char *prefix, const char *suffix)
{
    static char joinedPath[260] = { 0 }; // Max path length on Windows, the bottleneck, is 260 characters.
    memset(joinedPath, '\0', 260);
    StringCopy(joinedPath, prefix);

    const size_t prefixLength = strlen(prefix);
    if ((prefixLength >= 1) && (joinedPath[prefixLength - 1] != '/') && (joinedPath[prefixLength - 1] != '\\'))
#ifdef _WIN32
        joinedPath[prefixLength] = '\\'; // Append the path with a '\\' separator.
#else
        joinedPath[prefixLength] = '/'; // Append the path with a '/' separator.
#endif

    const char *suffixStart = suffix;
    const size_t suffixLength = strlen(suffix);
    // Skip over the "this directory" part (e.g. "./a.tsx" -> "a.tsx").
    if ((suffixLength >= 2) && (suffix[0] == '.') && ((suffix[1] == '/') || (suffix[1] == '\\'))) suffixStart += 2;

    // Note: ".." is kept in the joined path intentionally although it is possible to exceed 260 characters. TODO?
    StringConcatenate(joinedPath, suffixStart);
    return joinedPath;
}

void StringCopy(char *destination, const char *source)
{
#if (!defined _MSC_VER || defined _CRT_SECURE_NO_WARNINGS)
    // This is for build environments where "[M]icro[S]oft [C]ompiler [VER]sion" is not defined, meaning the compiler is
    // one other than MSVC, or where MSVC is being used but the security deprecation warning has been disabled. MSVC
    // does not consider strcpy() to be secure because it's somewhat unbounded. In practice, it's fine.
    strcpy(destination, source);
#else
    // This keeps MSVC happy by providing the supposed size of the destination buffer. However, these buffers are
    // dynamically allocated so there's no quick way to get the actual size. The best solution is to use strlen() but
    // this makes the function O(2n) vs. the alternative's O(n). */
    strcpy_s(destination, strlen(source) + 1, source); // + 1 for the terminator.
#endif
}

void StringCopyN(char *destination, const char *source, size_t number)
{
#if (!defined _MSC_VER || defined _CRT_SECURE_NO_WARNINGS)
    strncpy(destination, source, number);
#else
    strncpy_s(destination, number + 1, source, number);
#endif
}

void StringConcatenate(char *destination, const char *source)
{
#if (!defined _MSC_VER || defined _CRT_SECURE_NO_WARNINGS)
    strcat(destination, source);
#else
    strcat_s(destination, strlen(destination) + strlen(source) + 1, source);
#endif
}

#endif // RAYTMX_IMPLEMENTATION

#endif // RAYTMX_H

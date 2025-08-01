/*
Copyright (c) 2024-2025 Luke Philipsen

Permission to use, copy, modify, and/or distribute this software for
any purpose with or without fee is hereby granted.

THE SOFTWARE IS PROVIDED “AS IS” AND THE AUTHOR DISCLAIMS ALL
WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES
OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE
FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY
DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN
AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
*/

/* Usage

  Do this:
    #define RAYTMX_IMPLEMENTATION
  before you include this file in *one* C or C++ file to create the implementation.

  You can define RAYTMX_DEC with
    #define RAYTMX_DEC static
  or
    #define RAYTMX_DEC extern
  to specify raytmx function declarations as static or extern, respectively.
  The default specifier is extern.
*/

#ifndef RAYTMX_H
    #define RAYTMX_H

#include <ctype.h> /* isspace() */
#include <math.h> /* floor(), INFINITY */
#include <stddef.h> /* NULL */
#include <stdint.h> /* int32_t, uint32_t */
#include <stdlib.h> /* atoi(), strtoul() */
#include <string.h> /* memcpy(), memset(), strcpy(), strcpy_s() strlen(), strncpy(), strncpy_s() */

#include "raylib.h"
#include "rlgl.h"

#ifndef RAYTMX_DEC
    #define RAYTMX_DEC
#endif /* RAYTMX_DEC */

#ifdef __cplusplus
    extern "C" {
#endif /* __cpluspus */

/***************/
/* Definitions */

/**
 * Function signature of raylib's LoadTexture() as a type. For use with SetLoadTextureTMX().
 */
typedef Texture2D (*LoadTextureCallback)(const char* fileName);

/**
 * Bit flags passed to SetTraceLogFlagsTMX() that optionally disable the logging of specific TMX elements.
 */
enum tmx_log_flags {
    LOG_SKIP_PROPERTIES = 1, /**< Skip <properties> and child <property> elements. */
    LOG_SKIP_LAYERS = 2, /**< Skip <layer>, <objectgroup>, <imagelayer>, and <group> layers and children thereof. */
    LOG_SKIP_TILE_LAYERS = 4, /**< Skip <layer> layers and children thereof. */
    LOG_SKIP_TILES = 8, /**< Skip tiles (GIDs) of tile layers (<layer>s). */
    LOG_SKIP_OBJECT_GROUPS = 16, /**< Skip <objectgroup> layers and children thereof. */
    LOG_SKIP_OBJECTS = 32, /**< Skip objects of object layers (<objectgroup>s). */
    LOG_SKIP_IMAGE_LAYERS = 64, /**< Skip <imagelayer> layers and children thereof. */
    LOG_SKIP_IMAGES = 128, /**< Skip images of image layers (<imagelayer>s). */
    LOG_SKIP_WANG_SETS = 256, /**< Skip <wangsets> and child <wangset> elements. */
    LOG_SKIP_WANG_TILES = 512 /**< Skip Wang tiles of Wang sets (<wangset>s). */
};

/**
 * Identifiers for the possible layer types.
 */
typedef enum tmx_layer_type {
    LAYER_TYPE_TILE_LAYER = 0, /**< Layer containing a set number of tiles referenced by Global IDs (GIDs). */
    LAYER_TYPE_OBJECT_GROUP, /**< Layer containing an arbitrary number of various object types. */
    LAYER_TYPE_IMAGE_LAYER, /**< Layer consisting of a single image. */
    LAYER_TYPE_GROUP /**< Container layer, with no visuals of its own, that holds other, child layers. */
} TmxLayerType;

/**
 * Identifiers for the possible property (data) types.
 */
typedef enum tmx_property_type {
    PROPERTY_TYPE_STRING = 0, /**< String, or a sequence of characters. */
    PROPERTY_TYPE_INT, /**< Integer number. */
    PROPERTY_TYPE_FLOAT, /**< Floating-point number. */
    PROPERTY_TYPE_BOOL, /**< Boolean, true or false. */
    PROPERTY_TYPE_COLOR, /**< Color with red, green, and blue values and a possible alpha value. */
    PROPERTY_TYPE_FILE, /**< File name or path to a file as a string. */
    PROPERTY_TYPE_OBJECT /**< Object (<object>) within the map as an integer ID. */
} TmxPropertyType;

/**
 * Identifiers for the possible draw orders applicable to object layers.
 */
typedef enum tmx_object_group_draw_order {
    OBJECT_GROUP_DRAW_ORDER_TOP_DOWN = 0, /**< Drawn in ascending order by y-coordinate. */
    OBJECT_GROUP_DRAW_ORDER_INDEX /**< Drawn in the order in which the appear in the document. */
} TmxObjectGroupDrawOrder;

/**
 * Identifiers for the possible alignments of tiles with an object's bounds.
 */
typedef enum tmx_object_alignment {
    OBJECT_ALIGNMENT_UNSPECIFIED = 0, /**< For orthogonal, behaves like bottom-left. For isometric, like bottm. */
    OBJECT_ALIGNMENT_TOP_LEFT, /**< Tiles are snapped to the upper-left bound of objects. */
    OBJECT_ALIGNMENT_TOP, /**< Tiles are snapped to the upper-center bound of objects. */
    OBJECT_ALIGNMENT_TOP_RIGHT, /**< Tiles are snapped to the upper-right bound of objects. */
    OBJECT_ALIGNMENT_LEFT, /**< Tiles are snapped to the left-center bound of objects. */
    OBJECT_ALIGNMENT_CENTER, /**< Tiles are snapped to the horizontal and vertical center of objects. */
    OBJECT_ALIGNMENT_RIGHT, /**< Tiles are snapped to the right-center bound of objects. */
    OBJECT_ALIGNMENT_BOTTOM_LEFT, /**< Tiles are snapped to the lower-left bound of objects. */
    OBJECT_ALIGNMENT_BOTTOM, /**< Tiles are snapped to the lower-center bound of objects. */
    OBJECT_ALIGNMENT_BOTTOM_RIGHT /**< Tiles are snapped to the lower-right bound of objects. */
} TmxObjectAlignment;

/**
 * Identifiers for the possible object types.
 */
typedef enum tmx_object_type {
    OBJECT_TYPE_RECTANGLE = 0, /**< Four-sided polygon four right angles and axis-aligned edges. */
    OBJECT_TYPE_ELLIPSE, /**< Ellipse, or a circle when the axes are equal. */
    OBJECT_TYPE_POINT, /**< Individual (X, Y) coordinate with no dimensions. */
    OBJECT_TYPE_POLYGON, /**< Filled polygon formed by an ordered series of points. */
    OBJECT_TYPE_POLYLINE, /**< Unfilled polygon formed by an ordered series of points. */
    OBJECT_TYPE_TEXT, /**< Text, or the visual representation of a string. */
    OBJECT_TYPE_TILE /**< Tile, referenced by a Global ID (GID), from a tileset known to the map. */
} TmxObjectType;

/**
 * Identifiers for the possible horizontal alignments of text.
 */
typedef enum tmx_horizontal_alignment {
    HORIZONTAL_ALIGNMENT_LEFT = 0, /**< Text is to be snapped to the left bound of its object. */
    HORIZONTAL_ALIGNMENT_CENTER, /**< Text is to be centered along the X axis of its object. */
    HORIZONTAL_ALIGNMENT_RIGHT, /**< Text is to be snapped to the right bound of its object. */
    HORIZONTAL_ALIGNMENT_JUSTIFY /**< Text is to be evenly spaced, per line, filling the object's width. */
} TmxHorizontalAlignment;

/**
 * Identifiers for the possible vertical alignments of text.
 */
typedef enum tmx_vertical_alignment {
    VERTICAL_ALIGNMENT_TOP = 0, /**< Text is to be snapped to the upper bound of its object. */
    VERTICAL_ALIGNMENT_CENTER, /**< Text is to be cnetered along the Y axis of its object. */
    VERTICAL_ALIGNMENT_BOTTOM /**< Text is to be snapped to the lower bound of its object. */
} TmxVerticalAlignment;

/**
 * Identifiers for the possible map orientations.
 */
typedef enum tmx_orientation {
    ORIENTATION_NONE = 0, /**< Orientation was not specified. Assumed to be orthogonal. */
    ORIENTATION_ORTHOGONAL, /**< Standard top-down view with rectanglular tiles. */
    ORIENTATION_ISOMETRIC, /**< Subset top-down view from a 45-degree angle. NOT supported. */
    ORIENTATION_STAGGERED, /**< Variation of isometric with staggered axes. */
    ORIENTATION_HEXAGONAL /**< Top-down view in which tiles are six-sided and alternative rows/columns are offset. */
} TmxOrientation;

/**
 * Identifiers for the possible orders in which tiles in a tile layer are rendered/drawn.
 */
typedef enum tmx_render_order {
    RENDER_ORDER_RIGHT_DOWN = 0, /**< Tiles are rendered by row, from left to right, then column, from top to bottom. */
    RENDER_ORDER_RIGHT_UP, /**< Tiles are rendered by row, from left to right, then column, from bottom to top. */
    RENDER_ORDER_LEFT_DOWN, /**< Tiles are rendered by row, from right to left, then column, from top to bottom. */
    RENDER_ORDER_LEFT_UP /**< Tiles are rendered by row, from right to left, then column, from bottom to top. */
} TmxRenderOrder;

/**
 * Model of an <image> element. Defines an image and relevant attributes along with a loaded texture.
 */
typedef struct tmx_image {
    char* source; /**< File name and/or path referencing the image on disk. */
    Color trans; /**< (Optional) color that defines is treated as transparent. Not currently implemented. */
    bool hasTrans; /**< When true, indicates 'trans' has been set with a color to be treated as transparent. */
    uint32_t width; /**< Width of the image in pixels. */
    uint32_t height; /**< Height of the image in pixels. */
    Texture2D texture; /**< The image as a raylib texture loaded into VRAM, if loading was successful. */
} TmxImage;

/**
 * Model of a <layer> element when combined with the 'TmxLayer' model. Defines a tile layer with a fixed-size list of
 * tile Global IDs (GIDs).
 */
typedef struct tmx_tile_layer {
    uint32_t width; /**< Width of the layer in tiles. */
    uint32_t height; /**< Height of the layer in tiles. */
    char* encoding; /**< (Optional) encoding used to encode tiles. May be NULL, "base64," or "csv." */
    char* compression; /**< (Optional) compression used to compress tiles. May be NULL, "gzip," "zlib," or "zstd." */
    uint32_t* tiles; /**< Array of tile Global IDs (GIDs) contained by this tile layer. */
    uint32_t tilesLength; /**< Length of the 'tiles' array. */
} TmxTileLayer;

/**
 * Contains the information needed to quickly draw a single line of a <text> element.
 */
typedef struct tmx_text_line {
    char* content; /**< The string to be drawn. This may be the whole content of the parent string or partial. */
    Font font; /**< The raylib Font to be used when drawing. */
    Vector2 position; /**< Absolute position of this line. This is separate from its object layer's potential offset. */
    float spacing; /**< Spacing in pixels to be applied between each character when drawing. */
} TmxTextLine;

/**
 * Model of a <text> element along with some pre-calculated objects for efficient drawing.
 */
typedef struct tmx_text {
    char* fontFamily; /**< Font family (e.g. "sans-serif") to be used to render the text. */
    uint32_t pixelSize; /**< Size of the font in pixels. */
    bool wrap; /**< When true, indicates word wrapping should be used when appropriate. */
    Color color; /**< Color of the text. */
    bool bold; /**< When true, indicates the text should be bolded. */
    bool italic; /**< When true, indicates the text should be italicized. */
    bool underline; /**< When true, indicates the text should be underlined. */
    bool strikeOut; /**< When true, indicates the text should be struck/crossed out. */
    bool kerning; /**< When true, indicates kerning should be used when drawing. */
    TmxHorizontalAlignment halign; /**< Horizontal alignment of the text within its object. */
    TmxVerticalAlignment valign; /**< Vertical alignment of the text within its object. */
    char* content; /**< The string to be drawn. */
    TmxTextLine* lines; /**< Array of pre-calculated lines with all values needed to quickly draw this text. */
    uint32_t linesLength; /**< Length of the 'lines' array. */
} TmxText;

/**
 * Model of a <property> element. Describes a property of the model it's attached to with a name, type, and value.
 */
typedef struct tmx_property {
    TmxPropertyType type; /**< The specific (data) type of the property indicating which associated value to read. */
    char* name; /**< Name of the property. */
    char* stringValue; /**< The property's value for string-typed properties. */
    int32_t intValue; /**< The property's value for integer-typed properties. */
    float floatValue; /**< The property's value for floating point-typed properties. */
    bool boolValue; /**< The property's value for boolean-typed properties. */
    Color colorValue; /**< The property's value for color-typed properties. */
} TmxProperty;

/**
 * Model of an <object> element within an <objectgroup> element. Objects are amorphous entities of varying type but all
 * are potentially visible with positions and dimensions, although points' dimensions are effectively zero.
 */
typedef struct tmx_object {
    TmxObjectType type; /**< The specific object type indicating which optional fields have relevant information. */
    uint32_t id; /**< Unique ID of the object. */
    char* name; /**< Name of the object. */
    char* typeString; /**< The type/class of the object. */ /* 'type' is a reserved keyword hence 'typeString' */
    double x; /**< X coordinate, in pixels, of the object. This is separate from its object layer's potential offset. */
    double y; /**< Y coordinate, in pixels, of the object. This is separate from its object layer's potential offset. */
    double width; /**< Width of the object in pixels. */
    double height; /**< Height of the object in pixels. */
    double rotation; /**< Rotation of the object in (clockwise) degrees around the object's (x, y) position. */
    uint32_t gid; /**< (Semi-optional) Global ID of a tile drawn as the object. If zero, the object is not a tile. */
    bool visible; /**< When true, indicates the object will be drawn. */
    char* templateString; /**< (Optional) file name and/or path referencing an object template on disk applied to this
                                object. If NULL, no template is used. */
    Vector2* points; /**< (Optional) array of ordered points that define a poly(gon|line). Relative, not absolute. */
    uint32_t pointsLength; /**< Length of the 'points' array. */
    Vector2* drawPoints; /**< (Optional) array used as a buffer when drawing. Equal in length to the 'points' array. */
    TmxText* text; /**< (Optional) text to be drawn. */
    TmxProperty* properties; /**< Array of named, typed properties that apply to this object. */
    uint32_t propertiesLength; /**< Length of the 'properties' array. */
    Rectangle aabb; /**< Axis-Aligned Bounding Box (AABB). */
} TmxObject;

/**
 * Model of an <objectgroup> element when combined with the 'TmxLayer' model. Defines an object layer of an arbitrary
 * number of objects of varying types.
 */
typedef struct tmx_object_group {
    /* uint32_t width; */ /**< Width of the object layer in tiles. TMX documentation describes it as "meaningless." */
    /* uint32_t height; */ /**< Height of the object layer in tiles. TMX documentation describes it as "meaningless." */
    Color color; /**< (Optional) color used to display objects within the layer. */
    bool hasColor; /**< When true, indicates 'color' has been set. */
    TmxObjectGroupDrawOrder drawOrder; /**< Indicates the order in which objects in this layer are drawn. */
    TmxObject* objects; /**< Array of objects contained by this object layer. */
    uint32_t objectsLength; /**< Length of the 'objects' array. */
    uint32_t* ySortedObjects; /**< Array of indexes of 'objects' sorted by the objects' y-coordinates. */
} TmxObjectGroup;

/**
 * Model of an <imagelayer> element when combined with the 'TmxLayer' model. Defines a layer consisting of one image.
 */
typedef struct tmx_image_layer {
    bool repeatX; /**< When true, indicates the image is repeated along the X axis. */
    bool repeatY; /**< When true, indicates the image is repeated along the Y axis. */
    TmxImage image; /**< Sole image of this layer. */
    bool hasImage; /**< When true, indicates 'image' has been set. Should always be true. */
} TmxImageLayer;

struct tmx_layer; /* Forward declaration of the following type. Contains children of the same type. */

/**
 * Model of multiple layer elements: <layer>, <objectgroup>, <imagelayer>, or <group>. Defines a layer with attributes
 * common to all, more-specific layer types. The more-specific attributes
 */
typedef struct tmx_layer {
    TmxLayerType type; /**< The specific layer type indicating which associated layer ('exact') has mspecific values. */
    uint32_t id; /**< Unique integer ID of the layer. */
    char* name; /**< Name of the layer. */
    char* classString; /**< (Optional) class of the layer, may be NULL. */ /* 'class' is reserved hence 'classString' */
    bool visible; /**< When true, indicates the layer and its children will be drawn. */
    double opacity; /**< Opacity of the layer and its children where 0.0 means the layer is fully transparent. */
    Color tintColor; /**< (Optional) tint color applied to the layer and its chilren. */
    bool hasTintColor; /**< When true, indicates 'tintColor' has been set. */
    int32_t offsetX; /**< Horizontal offset of the layer and its children in pixels. */
    int32_t offsetY; /**< Vertical offset of the layer and its children in pixels. */
    double parallaxX; /**< Horizontal parallax factor. 1.0 means the layers position on the screen changes at the same
                           rate as the camera. 0.0 means the layer will not move with the camera. */
    double parallaxY; /**< Veritcal parallax factor. 1.0 means the layers position on the screen changes at the same
                           rate as the camera. 0.0 means the layer will not move with the camera. */
    TmxProperty* properties; /**< Array of named, typed properties that apply to this layer. */
    uint32_t propertiesLength; /**< Length of the 'properties' array. */
    struct tmx_layer* layers; /**< (Optional) array of child layers, may be NULL. Only used by group layers. */
    uint32_t layersLength; /**< Length of the 'layers' array. */
    union layer_type_union {
        TmxTileLayer tileLayer;
        TmxObjectGroup objectGroup;
        TmxImageLayer imageLayer;
    } exact; /**< Additional layer information specific to a tile, object, or image layer but not groups. */
} TmxLayer;

/**
 * Model of a <frame> element. Defines a temporal frame of an animation with the Global ID (GID) of the tile to be
 * displayed and the duration thereof.
 */
typedef struct tmx_animation_frame {
    uint32_t id; /**< The local ID, not Global ID (GID), of a tile within the animation's tileset. */
    float duration; /**< Duration in milliseconds that the frame should be displayed. */
} TmxAnimationFrame;

/**
 * Model of an <animation> element. Defines a series of (tile) frames.
 */
typedef struct tmx_animation {
    TmxAnimationFrame* frames; /**< Array of frames. These frames identify tiles and durations to be displayed. */
    uint32_t framesLength; /**< Length of the 'frames' array. */
} TmxAnimation;

/**
 * Model of a <tile> element within a <tileset> element. Contains information about tiles that are not or cannot be
 * implicitly determined from the tileset.
 */
typedef struct tmx_tileset_tile {
    uint32_t id; /**< Local ID of the tile within its tileset. This is a factor in but different from its Global ID. */
    int32_t x; /**< X coordinate, in pixels, of the sub-rectangle within the tileset's image to extract. */
    int32_t y; /**< Y coordinate, in pixels, of the sub-rectangle within the tileset's image to extract. */
    uint32_t width; /**< Width, in pixels, of the sub-rectangle within the tileset's image to extract. */
    uint32_t height; /**< Height, in pixels, of the sub-rectangle within the tileset's image to extract. */
    TmxImage image; /**< (Optional) image to be used as the tile for "collection of images" tilesets. */
    bool hasImage; /**< When true, indicates 'image' is set. */
    TmxAnimation animation; /**< (Optional) animation, may be NULL. */
    bool hasAnimation; /**< When true, indicates 'animation' is set. */
    TmxProperty* properties; /**< Array of named, typed properties that apply to this tileset tile. */
    uint32_t propertiesLength; /**< Length of the 'properties' array. */
    TmxObjectGroup objectGroup; /**< (Optional) 0+ objects representing collision information unique to the tile. */
} TmxTilesetTile;

/**
 * Model of a <tileset> element. Defines an image, or serious of images, from which tiles are drawn along with
 * information on how to extract areas from within the image and/or how to align them within an object.
 */
typedef struct tmx_tileset {
    uint32_t firstGid; /**< First Global ID (GID) of a tile in this tileset. */
    uint32_t lastGid; /**< Last Global ID (GID) of a tile in this tileset. */
    char* source; /**< (Optional) source of this tileset, may be NULL. Only used for external tilesets. */
    char* name; /**< Name of the tileset. */
    char* classString; /**< (Optional) class of the tileset, may be NULL */
    uint32_t tileWidth; /**< Maximum, although typically exact, width of the tiles in this tileset in pixels. */
    uint32_t tileHeight; /**< Maximum, although typically exact, height of the tiles in this tileset in pixels. */
    uint32_t spacing; /**< Spacing in pixels between tiles in this tileset. */
    uint32_t margin; /**< Margin around the tiles in this tileset. */
    uint32_t tileCount; /**< Number of tiles in this tileset. Note: 'lastGid' - 'firstGid' is not always 'tileCount.' */
    uint32_t columns; /**< Number of tile columsn in this tileset. */
    TmxObjectAlignment objectAlignment; /**< Controls the alignment of tiles of this tileset when used as objects. */
    int32_t tileOffsetX; /**< Horizontal offset in pixels applied when drawing tiles from this tileset. */
    int32_t tileOffsetY; /**< Vertical offset in pixels applied when drawing tiles form this tileset. */
    TmxImage image; /**< (Optional) image from which this tilesets tiles are extracted. */
    bool hasImage;  /**< When true, indicates 'image' is set. */
    TmxProperty* properties; /**< Array of named, typed properties that apply to this tileset. */
    uint32_t propertiesLength; /**< Length of the 'properties' array. */
    TmxTilesetTile* tiles; /**< Array of explicitly-defined tiles within the tileset. */
    uint32_t tilesLength; /**< Length of the 'tiles' array. */
} TmxTileset;

/**
 * Contains the information and objects needed to quickly draw a <tile> in a raylib application.
 */
typedef struct tmx_tile {
    uint32_t gid; /**< Three possible uses: 1) If zero, indicates this tile is unused and the GID mapping to it doesn't
                       exist within the map, 2) if the tile is an animation, indicates the first GID of the tileset the
                       animation's frames reference, or 3) just the GID of the tile. */
    Rectangle sourceRect; /**< Sub-rectangle within a tileset to extract that is to be drawn. */
    Texture2D texture; /**< Texture in VRAM to be used to draw. May be used whole or as a source of a sub-rectangle. */
    Vector2 offset; /**< Offset in pixels to be applied to the tile, derived from the tileset. */
    TmxAnimation animation; /**< (Optional) animation. */
    bool hasAnimation; /**< When true, indicates 'animation' is set. */
    uint32_t frameIndex; /**< For animations, the current animation frame to draw. */
    float frameTime; /**< For animations, an accumulator. The time, in seconds, the current frame has been drawn. */
    TmxObjectGroup objectGroup; /**< (Optional) 0+ objects representing collision information unique to the tile. */
} TmxTile;

/**
 * Model of a <map> element along with some pre-calculated objects for efficient drawing.
 */
typedef struct tmx_map {
    char* fileName; /**< File name of the TMX file with extension. */
    TmxOrientation orientation; /**< Map orientation. May be orthogonal, isometric, staggered, or hexagonal. */
    TmxRenderOrder renderOrder; /**< Order in which tiles on tile layers are rendered. */
    uint32_t width; /**< Width of this map in tiles. */
    uint32_t height; /**< Height of htis map in tiles. */
    uint32_t tileWidth; /**< Width of a tile in pixels. */
    uint32_t tileHeight; /**< Height of a tile in pixels. */
    int32_t parallaxOriginX; /**< X coordinate, in pixels, of the parallax origin. */
    int32_t parallaxOriginY; /**< Y coordinate, in pixels, of hte parallax origin. */
    Color backgroundColor; /**< (Optional) background color to be drawn behind the map with its dimensions. */
    bool hasBackgroundColor; /**< When true, indicates 'backgroundColor' is set. */
    TmxProperty* properties; /**< Array of named, typed properties that apply to this map. */
    uint32_t propertiesLength; /**< Length of the 'properties' array. */
    TmxTileset* tilesets; /**< Array of tilesets used by the map. */
    uint32_t tilesetsLength; /**< Length of the 'tilesets' array. */
    TmxLayer* layers; /**< Array of layers and potential child layers that make up this map. */
    uint32_t layersLength; /**< Length of the 'layers' array. */
    TmxTile* gidsToTiles; /**< Array of pre-calculated tile metadata with all the values needed to quickly draw a tile
                               given its GID. Allocated such that gidsToTiles[1] returns the data of tile GID 1. */
    uint32_t gidsToTilesLength; /**< Length of the 'gidsToTiles' array. */
} TmxMap;

/**
 * Given a path to TMX document, parse it and create an equivalent model that can be, among other uses, quickly drawn.
 * This function allocates memory and loads textures into VRAM. To clean up, use UnloadTMX().
 *
 * @param fileName File name and/or path referencing a TMX document on disk to be loaded.
 * @return A model of the map as defined by the given TMX document, or NULL if loading failed for any reason.
 */
RAYTMX_DEC TmxMap* LoadTMX(const char* fileName);

/**
 * Unload a given map model by freeing memory allocations and unloading textures. In other words, free the resources
 * reserved by LoadTMX().
 *
 * @param map A previously-loaded map model to be freed/unloaded.
 */
RAYTMX_DEC void UnloadTMX(TmxMap* map);

/**
 * Draw the entirety of the given map at the given position.
 * When a camera is also passed to this function, parallaxed scrolling can be applied to layers with parallax factors
 * that are not 1.0 and the screen's surface can be known allowing for occlusion culling and the performance gains that
 * come with it.
 * The given tint is applied to map and its layers. When layers have their own tints, the two colors are combined. If no
 * tint is needed, passing WHITE effectively means no tint is applied.
 *
 * @param map A loaded map model to be drawn in whole at the given coordinates.
 * @param camera (Optional) camera to be used for parallax and occlusion.
 * @param posX X coordinate at which to draw the map. This corresponds to the top-left corner of the map.
 * @param posY Y coordinate at which to draw the map. This corresponds to the top-left corner of the map.
 * @param tint A tint to be applied to the map and its layers. This tint is combined with any individual layer tints.
 */
RAYTMX_DEC void DrawTMX(const TmxMap* map, const Camera2D* camera, int posX, int posY, Color tint);

/**
 * Draw the given layers at the given position.
 * When a camera is also passed to this function, parallaxed scrolling can be applied to layers with parallax factors
 * that are not 1.0 and the screen's surface can be known allowing for occlusion culling and the performance gains that
 * come with it.
 * The given tint is applied to the layers. When layers have their own tints, the two colors are combined. If no tint is
 * needed, passing WHITE effectively means no tint is applied.
 *
 * @param map A loaded map model to be drawn in part at the given coordinates.
 * @param camera (Optional) camera to be used for parallax and occlusion.
 * @param layers An array of select layers to be drawn.
 * @param layersLength Length of the given array of layers.
 * @param posX X coordinate at which to draw the layers. This corresponds to the top-left corner of the layers.
 * @param posY Y coordinate at which to draw the layers. This corresponds to the top-left corner of the layers.
 * @param tint A tint to be applied to the layers. This tint is combined with any individual layer tints.
 */
RAYTMX_DEC void DrawTMXLayers(const TmxMap* map, const Camera2D* camera, const TmxLayer* layers, uint32_t layersLength,
    int posX, int posY, Color tint);

/**
 * Progress the animations of the given map in real-time. This is intended to be called once per frame, or once per
 * BeginDrawing() an EndDrawing() call. If called more or less frequently, animation speeds will be affected.
 *
 * @param map A loaded map model to be animated.
 */
RAYTMX_DEC void AnimateTMX(TmxMap* map);

/**
 * Check for collisions between two objects of arbitrary type. Objects that are not primitive shapes, namely text and
 * tiles, are treated as rectangles.
 *
 * @param object1 A TMX <object> to be checked for a collision.
 * @param object2 Another TMX <object> to be checked for a collision.
 * @return True if the given objects collide with one another, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXObjects(TmxObject object1, TmxObject object2);

/**
 * Check for collisions between the given tile or group layers and the given rectangle. The tiles must have collision
 * information created with the Tiled Collision Editor.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param map A loaded map model containing the given layers.
 * @param layers An array of select tile or group layers to be checked for collisions.
 * @param layersLength Length of the given array of layers.
 * @param rec The rectangle to perform collision checks on.
 * @param outputObject Output parameter assigned with the object the rectangle collided with. NULL if not wanted.
 * @return True if the given rectangle collides with any tile in the given layers, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXTileLayersRec(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
    Rectangle rec, TmxObject* outputObject);

/**
 * Check for collisions between the given tile or group layers and the given circle. The tiles must have collision
 * information created with the Tiled Collision Editor.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param map A loaded map model containing the given layers.
 * @param layers An array of select tile or group layers to be checked for collisions.
 * @param layersLength Length of the given array of layers.
 * @param center The center point of the circle.
 * @param radius The radius of the circle.
 * @param outputObject Output parameter assigned with the object the circle collided with. NULL if not wanted.
 * @return True if the given circle collides with any tile in the given layers, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXTileLayersCircle(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
    Vector2 center, float radius, TmxObject* outputObject);

/**
 * Check for collisions between the given tile or group layers and the given point. The tiles must have collision
 * information created with the Tiled Collision Editor.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param map A loaded map model containing the given layers.
 * @param layers An array of select tile or group layers to be checked for collisions.
 * @param layersLength Length of the given array of layers.
 * @param point The point to perform collision checks on.
 * @param outputObject Output parameter assigned with the object the point collided with. NULL if not wanted.
 * @return True if the given point collides with any tile in the given layers, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXTileLayersPoint(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
    Vector2 point, TmxObject* outputObject);

/**
 * Check for collisions between the given tile or group layers and the given polygon. The tiles must have collision
 * information created with the Tiled Collision Editor.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * This function calculates the Axis-Aligned Bounding Box (AABB) of the polygon each time it's called. If the polygon's
 * AABB is known, CheckCollisionTMXLayersPolyEx() can be used for better performance.
 *
 * @param map A loaded map model containing the given layers.
 * @param layers An array of select tile or group layers to be checked for collisions.
 * @param layersLength Length of the given array of layers.
 * @param points An array of vertices defining the polygon. No repeats.
 * @param pointCount The length of the array of vertices.
 * @param outputObject Output parameter assigned with the object the polygon collided with. NULL if not wanted.
 * @return True if the given polygon collides with any tile in the given layers, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXLayersPoly(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
    Vector2* points, int pointCount, TmxObject* outputObject);

/**
 * Check for collisions between the given tile or group layers and the given polygon with the given Axis-Aligned
 * Bounding Box (AABB). The tiles must have collision information created with the Tiled Collision Editor.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 * 
 * @param map A loaded map model containing the given layers.
 * @param layers An array of select tile or group layers to be checked for collisions.
 * @param layersLength Length of the given array of layers.
 * @param points An array of vertices defining the polygon. No repeats.
 * @param pointCount The length of the array of vertices.
 * @param aabb Bounding box of the polygon. Used for quicker, broad collision checks.
 * @param outputObject Output parameter assigned with the object the polygon collided with. NULL if not wanted.
 * @return True if the given polygon collides with any tile in the given layers, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXLayersPolyEx(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
    Vector2* points, int pointCount, Rectangle aabb, TmxObject* outputObject);

/**
 * Check for collisions between the given object group, with 0+ objects of arbitrary shape, and the given rectangle.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param group The object group whose 0+ objects will be checked for collisions.
 * @param rec The rectangle to perform collision checks on.
 * @param outputObject Output parameter assigned with the object the rectangle collided with. NULL if not wanted.
 * @return True if the given rectangle collides with any object in the object group, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXObjectGroupRec(TmxObjectGroup group, Rectangle rec, TmxObject* outputObject);

/**
 * Check for collisions between the given object group, with 0+ objects of arbitrary shape, and the given circle.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param group The object group whose 0+ objects will be checked for collisions.
 * @param center The center point of the circle.
 * @param radius The radius of the circle.
 * @param outputObject Output parameter assigned with the object the circle collided with. NULL if not wanted.
 * @return True if the given circle collides with any object in the object group, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXObjectGroupCircle(TmxObjectGroup group, Vector2 center, float radius,
    TmxObject* outputObject);

/**
 * Check for collisions between the given object group, with 0+ objects of arbitrary shape, and the given point.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param group The object group whose 0+ objects will be checked for collisions.
 * @param point The point to perform collision checks on.
 * @param outputObject Output parameter assigned with the object the point collided with. NULL if not wanted.
 * @return True if the given point collides with any object in the object group, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoint(TmxObjectGroup group, Vector2 point, TmxObject* outputObject);

/**
 * Check for collisions between the given object group, with 0+ objects of arbitrary shape, and the given polygon.
 * This function calculates the Axis-Aligned Bounding Box (AABB) of the polygon each time it's called. If the polygon's
 * AABB is known, CheckCollisionTMXObjectGroupPolyEx() can be used for better performance.
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param group The object group whose 0+ objects will be checked for collisions.
 * @param points An array of vertices defining the polygon. No repeats.
 * @param pointCount The length of the array of vertices.
 * @param outputObject Output parameter assigned with the object the polygon collided with. NULL if not wanted.
 * @return True if the given polygon collides with any object in the object group, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoly(TmxObjectGroup group, Vector2* points, int pointCount,
    TmxObject* outputObject);

/**
 * Check for collisions between the given object group, with 0+ objects of arbitrary shape, and the given polygon with
 * the given Axis-Aligned Bound Box (AABB).
 * Note: This function assumes the map is positioned at (0, 0). If the map is drawn with an offset, normalize.
 *
 * @param group The object group whose 0+ objects will be checked for collisions.
 * @param points An array of vertices defining the polygon. No repeats.
 * @param pointCount The length of the array of vertices.
 * @param aabb Bounding box of the polygon. Used for quicker, broad collision checks.
 * @param outputObject Output parameter assigned with the object the polygon collided with. NULL if not wanted.
 * @return True if the given polygon collides with any object in the object group, or false if there is no collision.
 */
RAYTMX_DEC bool CheckCollisionTMXObjectGroupPolyEx(TmxObjectGroup group, Vector2* points, int pointCount,
    Rectangle aabb, TmxObject* outputObject);

/**
 * Set a custom callback in place of raylib's LoadTexture(). The callback must return a Texture2D and take a const char*
 * as the sole parameter. To unset, pass NULL to this function.
 *
 * @param callback A function pointer with a "Texture2D CustLoadTexture(const char* fileName)" signature, or NULL if
 *                 unsetting the custom callback.
 */
RAYTMX_DEC void SetLoadTextureTMX(LoadTextureCallback callback);

/**
 * Log properties of the given map as a formatted string.
 * SetTraceLogFlagsTMX() may be used to exclude select information.
 *
 * @param logLevel The level/severity with which to log the string (e.g. LOG_DEBUG, LOG_INFO, etc.).
 * @param map A loaded map to be logged.
 */
RAYTMX_DEC void TraceLogTMX(int logLevel, const TmxMap* map);

/**
 * Globally set logging options for TraceLogTMX() allowing for select types of information to be excluded.
 * The flags used by this function are defined in the tmx_log_flags enumeration.
 *
 * @param logFlags Logically OR'd bit flags to be applied to all logging following this call.
 */
RAYTMX_DEC void SetTraceLogFlagsTMX(int logFlags);

#ifdef __cplusplus
    }
#endif /* __cplusplus */

#ifdef RAYTMX_IMPLEMENTATION

#ifndef HOXML_IMPLEMENTATION
    #define HOXML_IMPLEMENTATION
#endif
#include "hoxml.h"

/******************/
/* Implementation */

#define TMX_LINE_THICKNESS 3.0f /* Thickness, in pixels, that outlines of specific objects are drawn with */

/* Bit flags that GIDs may be masked with in order to indicate transformations for individual tiles */
enum tmx_flip_flags {
    FLIP_FLAG_HORIZONTAL = 0x80000000,
    FLIP_FLAG_VERTICAL = 0x40000000,
    FLIP_FLAG_DIAGONAL = 0x20000000,
    FLIP_FLAG_ROTATE_120 = 0x10000000
};

typedef enum raytmx_document_format {
    FORMAT_TMX = 0, /* Tilemap with tilesets, layers, etc. */
    FORMAT_TSX, /* External tilesets */
    FORMAT_TX /* Object templates */
} RaytmxDocumentFormat;

typedef struct raytmx_external_tileset {
    TmxTileset tileset;
    bool isSuccess; /* 'isSuccess' is true when the external tileset was successfully loaded */
} RaytmxExternalTileset;

typedef struct raytmx_object_template {
    TmxTileset tileset;
    TmxObject object;
    bool isSuccess, hasTileset; /* 'isSuccess' is true when the object template was successfully loaded */
} RaytmxObjectTemplate;

struct raytmx_cached_texture; /* Forward declaration */
typedef struct raytmx_cached_texture {
    char* fileName;
    Texture2D texture;
    struct raytmx_cached_texture* next;
} RaytmxCachedTextureNode; /* Associates a file name with a Texture2D allowing for the reuse of textures in VRAM */

struct raytmx_cached_template; /* Forward declaration */
typedef struct raytmx_cached_template {
    char* fileName;
    RaytmxObjectTemplate objectTemplate;
    struct raytmx_cached_template* next;
} RaytmxCachedTemplateNode; /* Associates a file name with an object template */

struct raytmx_property_node; /* Forward declaration */
typedef struct raytmx_property_node {
    TmxProperty property;
    struct raytmx_property_node* next;
} RaytmxPropertyNode;

struct raytmx_tileset_node; /* Forward declaration */
typedef struct raytmx_tileset_node {
    TmxTileset tileset;
    struct raytmx_tileset_node* next;
} RaytmxTilesetNode;

struct raytmx_tileset_tile_node; /* Forward declaration */
typedef struct raytmx_tileset_tile_node {
    TmxTilesetTile tile;
    struct raytmx_tileset_tile_node* next;
} RaytmxTilesetTileNode;

struct raytmx_animation_frame_node; /* Forward declaration */
typedef struct raytmx_animation_frame_node {
    TmxAnimationFrame frame;
    struct raytmx_animation_frame_node* next;
} RaytmxAnimationFrameNode;

struct raytmx_layer_node; /* Forward declaration */
typedef struct raytmx_layer_node {
    TmxLayer layer;
    uint32_t childrenLength;
    struct raytmx_layer_node *next, *parent, *childrenRoot, *childrenTail;
} RaytmxLayerNode;

struct raytmx_tile_layer_tile_node; /* Forward declaration */
typedef struct raytmx_tile_layer_tile_node {
    uint32_t gid;
    struct raytmx_tile_layer_tile_node* next;
} RaytmxTileLayerTileNode;

struct raytmx_object_node; /* Forward declaration */
typedef struct raytmx_object_node {
    TmxObject object;
    struct raytmx_object_node* next;
} RaytmxObjectNode;

struct raytmx_object_sorting_node; /* Forward declaration */
typedef struct raytmx_object_sorting_node {
    double y;
    uint32_t index;
    struct raytmx_object_sorting_node* next;
} RaytmxObjectSortingNode;

struct raytmx_poly_point_node; /* Forward declaration */
typedef struct raytmx_poly_point_node {
    Vector2 point;
    struct raytmx_poly_point_node* next;
} RaytmxPolyPointNode;

struct raytmx_text_line_node; /* Forward declaration */
typedef struct raytmx_text_line_node {
    TmxTextLine line;
    struct raytmx_text_line_node* next;
} RaytmxTextLineNode;

typedef struct raytmx_state {
    RaytmxDocumentFormat format;
    char documentDirectory[512];
    bool isSuccess;

    /* Variables intended for TMX (map) parsing */
    RaytmxCachedTextureNode* texturesRoot;
    RaytmxCachedTemplateNode* templatesRoot;
    TmxOrientation mapOrientation;
    TmxRenderOrder mapRenderOrder;
    uint32_t mapWidth, mapHeight, mapTileWidth, mapTileHeight, mapPropertiesLength;
    int32_t mapParallaxOriginX, mapParallaxOriginY;
    Color mapBackgroundColor;
    bool mapHasBackgroundColor;
    TmxProperty* mapProperties;

    /* These variables, when not NULL, are assigned to the current element(s) being parsed */
    TmxProperty* property;
    TmxTileset* tileset;
    TmxImage* image;
    TmxTilesetTile* tilesetTile;
    TmxAnimationFrame* animationFrame;
    /* TmxWangSet* wangSet; */ /* TODO: Wang sets. Low priority. */
    /* TmxWangColor* wangColor; */ /* TODO: Wang sets. Low priority. */
    TmxLayer* layer;
    TmxTileLayer* tileLayer;
    TmxObjectGroup* objectGroup;
    TmxImageLayer* imageLayer;
    TmxObject* object;

    /* These variables are linked lists containing various elements where an arbitrary amount are allowed, such as */
    /* 1+ <object> elements in an <objectgroup>, that will be copied to arrays of known sizes later on */
    RaytmxPropertyNode *propertiesRoot, *propertiesTail;
    RaytmxTilesetNode *tilesetsRoot, *tilesetsTail;
    RaytmxTilesetTileNode *tilesetTilesRoot, *tilesetTilesTail;
    RaytmxAnimationFrameNode *animationFramesRoot, *animationFramesTail;
    RaytmxLayerNode *layersRoot, *layersTail, *groupNode;
    RaytmxTileLayerTileNode *layerTilesRoot, *layerTilesTail;
    RaytmxObjectNode *objectsRoot, *objectsTail;
    uint32_t tilesetsLength, tilesetTilesLength, animationFramesLength, propertiesLength, layersLength,
        layerTilesLength, objectsLength, propertiesDepth;
} RaytmxState; /* Intermediate data used internally to parse TMX (map), TSX (tileset), and TX (template) files */

RaytmxExternalTileset LoadTSX(const char* fileName);
RaytmxObjectTemplate LoadTX(const char* fileName);
void ParseDocument(RaytmxState* raytmxState, const char* fileName);
void HandleElementBegin(RaytmxState* raytmxState, hoxml_context_t* hoxmlContext);
void HandleAttribute(RaytmxState* raytmxState, hoxml_context_t* hoxmlContext);
void HandleElementEnd(RaytmxState* raytmxState, hoxml_context_t* hoxmlContext);
void FreeState(RaytmxState* raytmxState);
void FreeString(char* str);
void FreeTileset(TmxTileset tileset);
void FreeProperty(TmxProperty property);
void FreeLayer(TmxLayer layer);
void FreeObject(TmxObject object);
bool IterateTileLayer(const TmxMap* map, const TmxTileLayer* layer, Rectangle screenRect, uint32_t* rawGid,
    TmxTile* tile, Rectangle* tileRect);
void DrawTMXTileLayer(const TmxMap* map, Rectangle screenRect, TmxLayer layer, int posX, int posY, Color tint);
void DrawTMXLayerTile(const TmxMap* map, Rectangle screenRect, uint32_t rawGid, int posX, int posY, Color tint);
void DrawTMXObjectTile(const TmxMap* map, Rectangle screenRect, uint32_t rawGid, int posX, int posY, float width,
    float height, Color tint);
void DrawTMXObjectGroup(const TmxMap* map, Rectangle screenRect, TmxLayer layer, int posX, int posY, Color tint);
void DrawTMXImageLayer(const TmxMap* map, Rectangle screenRect, TmxLayer layer, int posX, int posY, Color tint);
bool CheckCollisionTMXTileLayerObject(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
    TmxObject object, TmxObject* outputObject);
bool CheckCollisionTMXObjectGroupObject(TmxObjectGroup group, TmxObject object, TmxObject* outputObject);
void TraceLogTMXTilesets(int logLevel, TmxOrientation orientation, TmxTileset* tilesets, uint32_t tilesetsLength,
    int numSpaces);
void TraceLogTMXProperties(int logLevel, TmxProperty* properties, uint32_t propertiesLength, int numSpaces);
void TraceLogTMXLayers(int logLevel, TmxLayer* layers, uint32_t layersLength, int numSpaces);
void TraceLogTMXObject(int logLevel, TmxObject object, int numSpaces);
void StringCopy(char* destination, const char* source);
TmxProperty* AddProperty(RaytmxState* raytmxState);
void AddTileLayerTile(RaytmxState* raytmxState, uint32_t gid);
TmxTileset* AddTileset(RaytmxState* raytmxState);
TmxTilesetTile* AddTilesetTile(RaytmxState* raytmxState);
TmxAnimationFrame* AddAnimationFrame(RaytmxState* raytmxState);
TmxLayer* AddGenericLayer(RaytmxState* raytmxState, bool isGroup);
TmxObject* AddObject(RaytmxState* raytmxState);
void AppendLayerTo(TmxMap* map, RaytmxLayerNode* groupNode, RaytmxLayerNode* layersRoot, uint32_t layersLength);
RaytmxCachedTextureNode* LoadCachedTexture(RaytmxState* raytmxState, const char* fileName);
RaytmxCachedTemplateNode* LoadCachedTemplate(RaytmxState* raytmxState, const char* fileName);
Color GetColorFromHexString(const char* hex);
uint32_t GetGid(uint32_t rawGid, bool* isFlippedHorizontally, bool* isFlippedVertically, bool* isFlippedDiagonally,
    bool* isRotatedHexagonal120);
void* MemAllocZero(unsigned int size);
char* GetDirectoryPath2(const char* filePath);
char* JoinPath(const char* prefix, const char* suffix);
void StringCopyN(char* destination, const char* source, size_t number);
void StringConcatenate(char* destination, const char* source);

/**********************************************************************************************************************/
/* Public implementation.                                                                                             */

RAYTMX_DEC TmxMap* LoadTMX(const char* fileName) {
    RaytmxState raytmxState[1];
    memset(raytmxState, 0, sizeof(RaytmxState)); /* Initialize all values to zero, NULL, or an equivalent enum value */
    raytmxState->format = FORMAT_TMX;

    /* Initialize the map object */
    TmxMap* map = (TmxMap*)MemAllocZero(sizeof(TmxMap));

    /* Do format-agnostic parsing of the document. The state object will be populated with raytmx's models of the */
    /* equivalent TMX, TSX, and/or TX elements. */
    ParseDocument(raytmxState, fileName);
    if (!raytmxState->isSuccess) {
        UnloadTMX(map);
        return NULL;
    }

    /* Copy some top-level map properties */
    map->fileName = (char*)MemAllocZero((unsigned int)strlen(fileName) + 1);
    StringCopy(map->fileName, GetFileName(fileName));
    map->orientation = raytmxState->mapOrientation;
    map->renderOrder = raytmxState->mapRenderOrder;
    map->width = raytmxState->mapWidth;
    map->height = raytmxState->mapHeight;
    map->tileWidth = raytmxState->mapTileWidth;
    map->tileHeight = raytmxState->mapTileHeight;
    map->backgroundColor = raytmxState->mapBackgroundColor;
    map->parallaxOriginX = raytmxState->mapParallaxOriginX;
    map->parallaxOriginY = raytmxState->mapParallaxOriginY;
    map->hasBackgroundColor = raytmxState->mapHasBackgroundColor;

    uint32_t gidsToTilesLength = 0; /* Can also be seen as the last GID */
    if (raytmxState->tilesetsRoot != NULL) { /* If there is at least one tileset */
        /* Allocate the array of tilesets and zeroize every index */
        TmxTileset* tilesets = (TmxTileset*)MemAllocZero(sizeof(TmxTileset) * raytmxState->tilesetsLength);
        /* Copy the TmxTileset pointers into the array */
        RaytmxTilesetNode* tilesetIterator = raytmxState->tilesetsRoot;
        for (uint32_t i = 0; tilesetIterator != NULL; i++) {
            TmxTileset tileset = tilesetIterator->tileset;
            if (tileset.hasImage) /* If the tileset has a shared image and implicitly defines tiles */
                tileset.lastGid = tileset.firstGid + tileset.tileCount - 1;
            else if (tileset.tilesLength > 0) /* If the tileset is a "collection of images" with explicit tiles */
                tileset.lastGid = tileset.firstGid + tileset.tiles[tileset.tilesLength - 1].id - 1;

            if (gidsToTilesLength < tileset.lastGid + 1)
                gidsToTilesLength = tileset.lastGid + 1; /* GIDs start at 1 so the length is the last GID + 1 */
            tilesets[i] = tileset;
            tilesetIterator = tilesetIterator->next;
        }
        /* Add the tilesets array to the map */
        map->tilesets = tilesets;
        map->tilesetsLength = raytmxState->tilesetsLength;
    } else
        TraceLog(LOG_WARNING, "RAYTMX: The map does not contain any tilesets");

    if (raytmxState->layersRoot != NULL) { /* If there is at least one layer within the map */
        /* Due to the existence of <group> layers, layers can have children of multiple generations. To form the */
        /* resulting tree-like structure, recursion is used. */
        AppendLayerTo(map, NULL, raytmxState->layersRoot, raytmxState->layersLength);
    } else
        TraceLog(LOG_WARNING, "RAYTMX: The map does not contain any layers");

    if (gidsToTilesLength > 0) {
        TmxTile* gidsToTiles = (TmxTile*)MemAllocZero(sizeof(TmxTile) * gidsToTilesLength);

        for (uint32_t i = 0; i < map->tilesetsLength; i++) {
            TmxTileset* tileset = &map->tilesets[i];
            if (tileset->hasImage) { /* If the tileset has a shared image (i.e. not a "collection of images") */
                for (uint32_t id = 0; id < tileset->tileCount; id++) {
                    uint32_t gid = id + tileset->firstGid, x = id % tileset->columns, y = id/ tileset->columns;
                    bool hasExplicitSourceRect = false;
                    gidsToTiles[gid].gid = gid;

                    /* Search through the explicit tileset tiles for one with a matching local ID. Whereas most */
                    /* tiles in a tile layer are implicit, some may have information given directly, like */
                    /* animation frames or sub-rectangle values, as well as less relevant information. */
                    for (uint32_t j = 0; j < tileset->tilesLength; j++) {
                        TmxTilesetTile tilesetTile = tileset->tiles[j];
                        if (tilesetTile.id == id) { /* If this tileset tile has explicitly-defined information */
                            /* Typical tiles are implicit since everything that must be known about them can be */
                            /* inferred from knowing the dimensions the tileset's image, dimensions of tiles, and */
                            /* the (right-down) order of tiles within the tilest's image. However, tiles can have */
                            /* additional, non-inferable information. This is particularly true for animations. */
                            if (tilesetTile.hasAnimation) { /* If the tile is meta, pointing to other tiles */
                                gidsToTiles[gid].hasAnimation = true;
                                gidsToTiles[gid].animation = tilesetTile.animation;
                                /* 'gid' is slightly repurposed for animations in that it's assigned with the */
                                /* tileset's first GID rather than the tiles'. This is done because frames use */
                                /* local IDs and the tileset's first GID is needed to get the frame's GID. */
                                gidsToTiles[gid].gid = tileset->firstGid;
                            } else if (tilesetTile.x != 0 || tilesetTile.y != 0 || tilesetTile.width != 0 ||
                                    tilesetTile.height != 0) {
                                /* This tile directly tells us the area within the tileset's image to use when */
                                /* drawing, overriding the implicit dimensions derived from the map's 'tilewidth' */
                                /* and 'tileheight' attributes. */
                                hasExplicitSourceRect = true;
                                gidsToTiles[gid].sourceRect.x = (float)tilesetTile.x;
                                gidsToTiles[gid].sourceRect.y = (float)tilesetTile.y;
                                gidsToTiles[gid].sourceRect.width = (float)tilesetTile.width;
                                gidsToTiles[gid].sourceRect.height = (float)tilesetTile.height;
                            }

                            /* Tiles may have child object groups. These objects are a form of collision information. */
                            /* The object group may be empty or may have objects. A simple assignment covers both. */
                            gidsToTiles[gid].objectGroup = tilesetTile.objectGroup;

                            break; /* The tile was found - no need to check the rest */
                        }
                    }

                    if (!gidsToTiles[gid].hasAnimation) { /* If the tile is of the typical, static variety */
                        if (!hasExplicitSourceRect) { /* If that section was not explicitly defined */
                            /* Calculate the area within the texture to be drawn from contextual information */
                            gidsToTiles[gid].sourceRect.x = (float)(tileset->margin + (x * tileset->tileWidth) +
                                (x * tileset->spacing));
                            gidsToTiles[gid].sourceRect.y = (float)(tileset->margin + (y * tileset->tileHeight) +
                                (y * tileset->spacing));
                            gidsToTiles[gid].sourceRect.width = (float)tileset->tileWidth;
                            gidsToTiles[gid].sourceRect.height = (float)tileset->tileHeight;
                        }
                        gidsToTiles[gid].texture = tileset->image.texture;
                        gidsToTiles[gid].offset.x = (float)tileset->tileOffsetX;
                        gidsToTiles[gid].offset.y = (float)tileset->tileOffsetY;
                    }
                }
            } else { /* If the tileset is a collection of images where each tile has its own image */
                for (uint32_t j = 0; j < tileset->tilesLength; j++) {
                    TmxTilesetTile tilesetTile = tileset->tiles[j];
                    if (!tilesetTile.hasImage) {
                        TraceLog(LOG_WARNING, "RAYTMX: Skipping tile %d of image collection tileset \"%s\" because "
                            "it has no image", tilesetTile.id, tileset->name);
                        continue;
                    }

                    int32_t gid = tileset->firstGid + tilesetTile.id;
                    gidsToTiles[gid].gid = gid;
                    gidsToTiles[gid].sourceRect.x = (float)tilesetTile.x; /* Defaults to and probably is zero */
                    gidsToTiles[gid].sourceRect.y = (float)tilesetTile.y; /* Defaults to and probably is zero */
                    if (tilesetTile.width != tilesetTile.image.width)
                        gidsToTiles[gid].sourceRect.width = (float)tilesetTile.width;
                    else
                        gidsToTiles[gid].sourceRect.width = (float)tilesetTile.image.width;
                    if (tilesetTile.height != tilesetTile.image.height)
                        gidsToTiles[gid].sourceRect.height = (float)tilesetTile.height;
                    else
                        gidsToTiles[gid].sourceRect.height = (float)tilesetTile.image.height;
                    gidsToTiles[gid].texture = tilesetTile.image.texture;
                }
            }
        }

        map->gidsToTiles = gidsToTiles;
        map->gidsToTilesLength = gidsToTilesLength;
    } /* gidsToTilesLength > 0 */

    /* Free the linked lists and zeroize related values */
    FreeState(raytmxState);

    return map;
}

RAYTMX_DEC void UnloadTMX(TmxMap* map) {
    if (map == NULL)
        return;

    if (map->fileName != NULL)
        MemFree(map->fileName);

    if (map->properties != NULL) {
        for (uint32_t i = 0; i < map->propertiesLength; i++)
            FreeProperty(map->properties[i]);
        MemFree(map->properties);
    }

    if (map->tilesets != NULL) {
        for (uint32_t i = 0; i < map->tilesetsLength; i++)
            FreeTileset(map->tilesets[i]);
        MemFree(map->tilesets);
    }

    if (map->layers != NULL) {
        for (uint32_t i = 0; i < map->layersLength; i++)
            FreeLayer(map->layers[i]);
        MemFree(map->layers);
    }

    if (map->gidsToTiles != NULL)
        MemFree(map->gidsToTiles);

    MemFree(map);
}

RAYTMX_DEC void DrawTMX(const TmxMap* map, const Camera2D* camera, int posX, int posY, Color tint) {
    if (map == NULL)
        return;

    if (map->hasBackgroundColor) {
        DrawRectangle(/* posX: */ posX, /* posY: */ posY, /* width: */ map->width, /*height: */ map->height,
            /* color: */ map->backgroundColor);
    }

    DrawTMXLayers(map, camera, map->layers, map->layersLength, posX, posY, tint);
}

RAYTMX_DEC void DrawTMXLayers(const TmxMap* map, const Camera2D* camera, const TmxLayer* layers, uint32_t layersLength,
        int posX, int posY, Color tint) {
    if (map == NULL || layers == NULL || layersLength == 0)
        return;

    for (uint32_t i = 0; i < layersLength; i++) {
        TmxLayer layer = layers[i];
        if (!layers[i].visible) /* If the layer is not visible */
            continue; /* Skip it - it's literally invisible */

        /* All types of layers can have a couple attributes that affect color: 'opacity' and 'tintcolor' */
        Color layerTint = tint;
        layerTint.a = (unsigned char)((double)layerTint.a * layer.opacity);
        if (layer.hasTintColor)
            layerTint = ColorTint(layerTint, layer.tintColor);

        Rectangle screenRect;
        if (camera != NULL) {
            screenRect.width = GetScreenWidth() / camera->zoom;
            screenRect.height = GetScreenHeight() / camera->zoom;
            screenRect.x = camera->target.x - (screenRect.width / 2.0f);
            screenRect.y = camera->target.y - (screenRect.height / 2.0f);
        } else {
            screenRect.x = 0.0f;
            screenRect.y = 0.0f;
            screenRect.width = (float)GetScreenWidth();
            screenRect.height = (float)GetScreenHeight();
        }

        int32_t parallaxOffsetX = 0, parallaxOffsetY = 0;
        if (camera != NULL) {
            parallaxOffsetX = (int32_t)((double)(camera->target.x - map->parallaxOriginX) * (layer.parallaxX - 1.0));
            parallaxOffsetY = (int32_t)((double)(camera->target.y - map->parallaxOriginY) * (layer.parallaxY - 1.0));
        }

        switch (layer.type) {
        case LAYER_TYPE_TILE_LAYER:
            DrawTMXTileLayer(map, screenRect, layer, posX + layer.offsetX + parallaxOffsetX,
                posY + layer.offsetY + parallaxOffsetY, layerTint);
            break;
        case LAYER_TYPE_OBJECT_GROUP:
            DrawTMXObjectGroup(map, screenRect, layer, posX + layer.offsetX + parallaxOffsetX,
                posY + layer.offsetY + parallaxOffsetY, layerTint);
            break;
        case LAYER_TYPE_IMAGE_LAYER:
            DrawTMXImageLayer(map, screenRect, layer, posX + layer.offsetX + parallaxOffsetX,
                posY + layer.offsetY + parallaxOffsetY, layerTint);
        break;
        case LAYER_TYPE_GROUP:
            DrawTMXLayers(map, camera, layer.layers, layer.layersLength, posX + layer.offsetX + parallaxOffsetX,
                posY + layer.offsetY + parallaxOffsetY, layerTint);
            break;
        }
    }
}

RAYTMX_DEC void AnimateTMX(TmxMap* map) {
    if (map == NULL)
        return;

    float dt = GetFrameTime(); /* Returns the duration, in seconds, of the last frame drawn */
    /* Iterate through the tiles, searching for those that are animations */
    for (uint32_t gid = 0; gid < map->gidsToTilesLength; gid++) {
        TmxTile* tile = &map->gidsToTiles[gid]; /* A pointer is used in case the frame time needs to be reassigned */
        if (tile->gid > 0 && tile->hasAnimation) { /* If the GID maps to a valid tile and that tile is an animation */
            tile->frameTime += dt;
            /* If the current frame has been displayed for its whole duration, or longer */
            if (tile->frameTime > tile->animation.frames[tile->frameIndex].duration) {
                tile->frameTime -= tile->animation.frames[tile->frameIndex].duration;
                /* Increment the frame index to display the next one... */
                tile->frameIndex += 1;
                /* ...unless the last frame was "last" in both senses */
                if (tile->frameIndex == tile->animation.framesLength)
                    tile->frameIndex = 0; /* Wrap around to the first frame */
            }
        }
    }
}

/**
 * Helper function that creates a TmxObject equivalent to the given rectangle.
 *
 * @param rec A rectangle as raylib's Rectangle type.
 * @return An equivalent TmxObject.
 */
TmxObject CreateRectangularTMXObject(Rectangle rec) {
    TmxObject recAsObject;
    memset(&recAsObject, 0, sizeof(TmxObject)); /* Zero initialize */

    recAsObject.type = OBJECT_TYPE_RECTANGLE;
    recAsObject.x = (double)rec.x;
    recAsObject.y = (double)rec.y;
    recAsObject.width = (double)rec.width;
    recAsObject.height = (double)rec.height;
    recAsObject.aabb = rec;

    return recAsObject;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersRec(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
        Rectangle rec, TmxObject* outputObject) {
    if (map == NULL || layers == NULL || layersLength == 0)
        return false;

    /* Check the rectangle against objects associated with tiles in the layers for collisions */
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreateRectangularTMXObject(rec), outputObject);
}

/**
 * Helper function that creates a TmxObject equivalent to the given circle.
 *
 * @param center The center point of the circle.
 * @param radius The radius of the circle.
 * @return An equivalent TmxObject.
 */
TmxObject CreateCircularTMXObject(Vector2 center, float radius) {
    TmxObject circleAsObject;
    memset(&circleAsObject, 0, sizeof(TmxObject)); /* Zero initialize */

    circleAsObject.type = OBJECT_TYPE_ELLIPSE;
    circleAsObject.x = (double)(center.x - radius);
    circleAsObject.y = (double)(center.y - radius);
    circleAsObject.width = 2.0 * (double)radius;
    circleAsObject.height = 2.0 * (double)radius;
    circleAsObject.aabb.x = center.x - radius;
    circleAsObject.aabb.y = center.y - radius;
    circleAsObject.aabb.width = 2.0f * radius;
    circleAsObject.aabb.height = 2.0f * radius;

    return circleAsObject;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersCircle(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
        Vector2 center, float radius, TmxObject* outputObject) {
    if (map == NULL || layers == NULL || layersLength == 0)
        return false;

    /* Check the circle against objects associated with tiles in the layers for collisions */
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreateCircularTMXObject(center, radius),
        outputObject);
}

/**
 * Helper function that creates a TmxObject equivalent to the given point.
 *
 * @param point The point.
 * @return An equivalent TmxObject.
 */
TmxObject CreatePointTMXObject(Vector2 point) {
    TmxObject pointAsObject;
    memset(&pointAsObject, 0, sizeof(TmxObject)); /* Zero initialize */

    pointAsObject.type = OBJECT_TYPE_POINT;
    pointAsObject.x = (float)point.x;
    pointAsObject.y = (float)point.y;
    pointAsObject.aabb.x = (float)point.x;
    pointAsObject.aabb.y = (float)point.y;

    return pointAsObject;
}

RAYTMX_DEC bool CheckCollisionTMXTileLayersPoint(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
        Vector2 point, TmxObject* outputObject) {
    if (map == NULL || layers == NULL || layersLength == 0)
        return false;

    /* Check the point against objects associated with tiles in the layers for collisions */
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength, CreatePointTMXObject(point), outputObject);
}

/**
 * Helper function that creates a TmxObject equivalent to the given polygon with the given Axis-Aligned Bounding Box
 * (AABB). If the AABB should be calculated from the given vertices, pass a rectangle with a zero or negative width or
 * height.
 *
 * @param points An array of vertices defining the polygon. No repeats.
 * @param pointCount The length of the array of vertices.
 * @param aabb Bounding box of the polygon. Used for quicker, broad collision checks.
 * @return An equivalent TmxObject.
 */
TmxObject CreatePolygonTMXObject(Vector2* points, int pointCount, Rectangle aabb) {
    /* Create a polygon TMX object to represent the given polygon */
    TmxObject polygonAsObject;
    memset(&polygonAsObject, 0, sizeof(TmxObject)); /* Zero initialize */

    polygonAsObject.type = OBJECT_TYPE_POLYGON;

    if (aabb.width <= 0.0f || aabb.height <= 0.0f) { /* If the AABB wasn't pre-calculated or is just wrong */
        /* Calculate the bounding box by searching for minimum and maximum coordinates of the vertices */
        float minX = INFINITY, maxX = -INFINITY, minY = INFINITY, maxY = -INFINITY;
        for (int i = 0; i < pointCount; i++) {
            Vector2 point = points[i];
            if (point.x < minX)
                minX = point.x;
            if (point.x > maxX)
                maxX = point.x;
            if (point.y < minY)
                minY = point.y;
            if (point.y > maxY)
                maxY = point.y;
        }
        aabb.x = minX;
        aabb.y = minY;
        aabb.width = maxX - minX;
        aabb.height = maxY - minY;
    }

    /* Polygons' points are relative to the object's position. Because this function takes points with absolute */
    /* positions, we'll account for this by making the object's position zero. */
    polygonAsObject.x = 0.0;
    polygonAsObject.y = 0.0;
    polygonAsObject.width = (double)aabb.width;
    polygonAsObject.height = (double)aabb.height;
    polygonAsObject.aabb = aabb;
    polygonAsObject.points = points;
    polygonAsObject.pointsLength = pointCount;

    return polygonAsObject;
}

RAYTMX_DEC bool CheckCollisionTMXLayersPoly(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
        Vector2* points, int pointCount, TmxObject* outputObject) {
    if (map == NULL || layers == NULL || layersLength == 0 || points == NULL || pointCount < 3)
        return false;

    /* Check the polygon against objects associated with tiles in the layers for collisions */
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength,
        CreatePolygonTMXObject(points, pointCount, (Rectangle){0.0f}), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXLayersPolyEx(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
        Vector2* points, int pointCount, Rectangle aabb, TmxObject* outputObject) {
    if (map == NULL || layers == NULL || layersLength == 0 || points == NULL || pointCount < 3)
        return false;

    /* Check the polygon against objects associated with tiles in the layers for collisions */
    return CheckCollisionTMXTileLayerObject(map, layers, layersLength,
        CreatePolygonTMXObject(points, pointCount, aabb), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupRec(TmxObjectGroup group, Rectangle rec, TmxObject* outputObject) {
    if (group.objectsLength == 0 || rec.width < 0.0f || rec.height < 0.0f)
        return false; /* Early-out opportunity. These cases would always return false. */

    /* Check the rectangle against TMX objects in the group for collisions */
    return CheckCollisionTMXObjectGroupObject(group, CreateRectangularTMXObject(rec), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupCircle(TmxObjectGroup group, Vector2 center, float radius,
        TmxObject* outputObject) {
    if (group.objectsLength == 0 || radius < 0.0f)
        return false; /* Early-out opportunity. These cases would always return false. */

    /* Check the circle against TMX objects in the group for collisions */
    return CheckCollisionTMXObjectGroupObject(group, CreateCircularTMXObject(center, radius), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoint(TmxObjectGroup group, Vector2 point, TmxObject* outputObject) {
    if (group.objectsLength == 0)
        return false; /* Early-out opportunity. This case would always return false. */

    /* Check the point against TMX objects in the group for collisions */
    return CheckCollisionTMXObjectGroupObject(group, CreatePointTMXObject(point), outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupPoly(TmxObjectGroup group, Vector2* points, int pointCount,
        TmxObject* outputObject) {
    if (group.objectsLength == 0 || points == NULL || pointCount < 3)
        return false; /* Early-out opportunity. These cases would always return false. */

    /* Check the polygon TMX object against other TMX objects in the group for collisions */
    return CheckCollisionTMXObjectGroupObject(group, CreatePolygonTMXObject(points, pointCount, (Rectangle){0.0f}),
        outputObject);
}

RAYTMX_DEC bool CheckCollisionTMXObjectGroupPolyEx(TmxObjectGroup group, Vector2* points, int pointCount,
        Rectangle aabb, TmxObject* outputObject) {
    if (group.objectsLength == 0 || points == NULL || pointCount < 3)
        return false; /* Early-out opportunity. These cases would always return false. */

    /* Check the polygon TMX object against other TMX objects in the group for collisions */
    return CheckCollisionTMXObjectGroupObject(group, CreatePolygonTMXObject(points, pointCount, aabb), outputObject);
}

static LoadTextureCallback loadTextureOverride = NULL;

RAYTMX_DEC void SetLoadTextureTMX(LoadTextureCallback callback) {
    loadTextureOverride = callback;
}

static int tmxLogFlags = 0;

RAYTMX_DEC void TraceLogTMX(int logLevel, const TmxMap* map) {
    if (map == NULL)
        return;

    switch (map->orientation) {
    case ORIENTATION_NONE: TraceLog(logLevel, "orientation: none"); break;
    case ORIENTATION_ORTHOGONAL: TraceLog(logLevel, "orientation: orthogonal"); break;
    case ORIENTATION_ISOMETRIC: TraceLog(logLevel, "orientation: isometric"); break;
    case ORIENTATION_STAGGERED: TraceLog(logLevel, "orientation: staggered"); break;
    case ORIENTATION_HEXAGONAL: TraceLog(logLevel, "orientation: hexagonal"); break;
    }

    switch (map->renderOrder) {
    case RENDER_ORDER_RIGHT_DOWN: TraceLog(logLevel, "render order: right-down"); break;
    case RENDER_ORDER_RIGHT_UP: TraceLog(logLevel, "render order: right-up"); break;
    case RENDER_ORDER_LEFT_DOWN: TraceLog(logLevel, "render order: left-down"); break;
    case RENDER_ORDER_LEFT_UP: TraceLog(logLevel, "render order: left-up"); break;
    }

    TraceLog(logLevel, "width: %u tiles", map->width);
    TraceLog(logLevel, "height: %u tiles", map->height);
    TraceLog(logLevel, "tile width: %u pixels", map->tileWidth);
    TraceLog(logLevel, "tile height: %u pixels", map->tileHeight);
    if (map->parallaxOriginX != 0)
        TraceLog(logLevel, "parallax origin X: %d pixels", map->parallaxOriginX);
    if (map->parallaxOriginY != 0)
        TraceLog(logLevel, "parallax origin Y: %d pixels", map->parallaxOriginY);
    if (map->hasBackgroundColor)
        TraceLog(logLevel, "background color: 0x%08X", map->backgroundColor);

    TraceLogTMXTilesets(logLevel, map->orientation, map->tilesets, map->tilesetsLength, 0);
    TraceLogTMXProperties(logLevel, map->properties, map->propertiesLength, 0);
    TraceLogTMXLayers(logLevel, map->layers, map->layersLength, 0);
}

RAYTMX_DEC void SetTraceLogFlagsTMX(int logFlags) {
    tmxLogFlags = logFlags;
}

/**********************************************************************************************************************/
/* Private implementation.                                                                                            */

RaytmxExternalTileset LoadTSX(const char* fileName) {
    RaytmxState raytmxState[1];
    memset(raytmxState, 0, sizeof(RaytmxState)); /* Initialize all values to zero, NULL, or an equivalent enum value */
    raytmxState->format = FORMAT_TSX;

    /* Initialize an external tileset object */
    RaytmxExternalTileset externalTileset;
    memset(&externalTileset, 0, sizeof(RaytmxExternalTileset));

    /* Do format-agnostic parsing of the document. The state object will be populated with raytmx's models of the */
    /* equivalent TMX, TSX, and/or TX elements. */
    ParseDocument(raytmxState, fileName);
    if (!raytmxState->isSuccess)
        return externalTileset; /* Will have 'isSuccess' set to false to indicate a failure */

    if (raytmxState->tilesetsRoot != NULL) { /* If there is at least one tileset */
        /* Copy the root tileset so it can be returned */
        externalTileset.tileset = raytmxState->tilesetsRoot->tileset;
        externalTileset.isSuccess = true;
        /* TSX files should have only one tileset so any others will be freed/unloaded immediately */
        RaytmxTilesetNode* tilesetIterator = raytmxState->tilesetsRoot->next;
        while (tilesetIterator != NULL) {
            FreeTileset(tilesetIterator->tileset);
            tilesetIterator = tilesetIterator->next;
        }
    } else
        TraceLog(LOG_WARNING, "RAYTMX: TSX file (external tileset) \"%s\" does not contain any tilesets", fileName);

    /* Free the linked lists and zeroize related values */
    FreeState(raytmxState);

    return externalTileset;
}

RaytmxObjectTemplate LoadTX(const char* fileName) {
    RaytmxState raytmxState[1];
    memset(raytmxState, 0, sizeof(RaytmxState)); /* Initialize all values to zero, NULL, or an equivalent enum value */
    raytmxState->format = FORMAT_TX;

    /* Initialize an object template object */
    RaytmxObjectTemplate objectTemplate;
    memset(&objectTemplate, 0, sizeof(RaytmxObjectTemplate));

    /* Do format-agnostic parsing of the document. The state object will be populated with raytmx's models of the */
    /* equivalent TMX, TSX, and/or TX elements. */
    ParseDocument(raytmxState, fileName);
    if (!raytmxState->isSuccess)
        return objectTemplate; /* Will have 'isSuccess' set to false to indicate a failure */

    if (raytmxState->objectsRoot != NULL) { /* If there is at least one object */
        /* Copy the root object so it can be returned */
        objectTemplate.object = raytmxState->objectsRoot->object;
        objectTemplate.isSuccess = true;
        /* TX files should have only one object so any others will be freed/unloaded immediately */
        RaytmxObjectNode* objectIterator = raytmxState->objectsRoot->next;
        while (objectIterator != NULL) {
            FreeObject(objectIterator->object);
            objectIterator = objectIterator->next;
        }
    } else
        TraceLog(LOG_WARNING, "RAYTMX: TX file (object template) \"%s\" does not contain any objects", fileName);

    if (raytmxState->tilesetsRoot != NULL) { /* If there is at least one tileset */
        /* Object templates may have a tileset. This is for cases where the object references a tile (i.e. its 'gid' */
        /* attribute is set).  Copy the root tileset so it can be returned */
        objectTemplate.tileset = raytmxState->tilesetsRoot->tileset;
        objectTemplate.hasTileset = true;
        /* TX files should have at most one tileset so any others will be freed/unloaded immediately */
        RaytmxTilesetNode* tilesetsIterator = raytmxState->tilesetsRoot->next;
        while (tilesetsIterator != NULL) {
            FreeTileset(tilesetsIterator->tileset);
            tilesetsIterator = tilesetsIterator->next;
        }
    }

    /* Free the linked lists and zeroize related values */
    FreeState(raytmxState);

    return objectTemplate;
}

void ParseDocument(RaytmxState* raytmxState, const char* fileName) {
    char* content = LoadFileText(fileName);
    if (content == NULL) {
        TraceLog(LOG_ERROR, "RAYTMX: Failed to open \"%s\"", fileName);
        return;
    }
    size_t contentLength = strlen(content);

    StringCopy(raytmxState->documentDirectory, GetDirectoryPath2(fileName));

    hoxml_context_t hoxmlContext[1];
    size_t bufferLength = contentLength;
    char* buffer = (char*)MemAlloc((unsigned int)bufferLength);
    hoxml_init(hoxmlContext, buffer, bufferLength);

    hoxml_code_t code;
    while ((code = hoxml_parse(hoxmlContext, content, contentLength)) != HOXML_END_OF_DOCUMENT) {
        if (code > HOXML_END_OF_DOCUMENT) { /* If there's information about an element, attribute, whatever */
            switch (code) {
            case HOXML_ELEMENT_BEGIN: HandleElementBegin(raytmxState, hoxmlContext); break;
            case HOXML_ELEMENT_END: HandleElementEnd(raytmxState, hoxmlContext); break;
            case HOXML_ATTRIBUTE: HandleAttribute(raytmxState, hoxmlContext);
            case HOXML_PROCESSING_INSTRUCTION_BEGIN: break;
            case HOXML_PROCESSING_INSTRUCTION_END: break;
            default: break; /* No other cases to handle but compilers like to complain */
            }
        } else if (code < HOXML_END_OF_DOCUMENT) { /* If there was an error, recoverable or not */
            switch (code) {
            case HOXML_ERROR_INSUFFICIENT_MEMORY: {
                /* This is one we can recover from by expanding the buffer. In this case, it will be doubled. */
                TraceLog(LOG_DEBUG, "RAYTMX: Allocating a new XML parsing buffer due to insufficient memory");
                bufferLength *= 2;
                char* newBuffer = (char*)MemAlloc((unsigned int)bufferLength);
                hoxml_realloc(hoxmlContext, newBuffer, bufferLength);
                MemFree(buffer);
                buffer = newBuffer;
                continue;
            } case HOXML_ERROR_UNEXPECTED_EOF:
                TraceLog(LOG_ERROR, "RAYTMX: Unexpected end of file");
            break;
            case HOXML_ERROR_SYNTAX:
                TraceLog(LOG_ERROR, "RAYTMX: Invalid syntax: line %d, column %d", hoxmlContext->line,
                    hoxmlContext->column);
            break;
            case HOXML_ERROR_ENCODING:
                TraceLog(LOG_ERROR, "RAYTMX: Character encoding error: line %d, column %d", hoxmlContext->line,
                    hoxmlContext->column);
            break;
            case HOXML_ERROR_TAG_MISMATCH:
                TraceLog(LOG_ERROR, "RAYTMX: Close tag does not match open tag: line %d, column %d",
                    hoxmlContext->line, hoxmlContext->column);
            break;
            case HOXML_ERROR_INVALID_DOCUMENT_TYPE_DECLARATION:
            case HOXML_ERROR_INVALID_DOCUMENT_DECLARATION:
                TraceLog(LOG_ERROR, "RAYTMX: Document (type) declaration error: line %d, column %d",
                    hoxmlContext->line, hoxmlContext->column);
            break;
            default: break; /* Keep the compiler happy */
            }
            UnloadFileText(content);
            return;
        }
    }

    UnloadFileText(content);
    MemFree(buffer);
    raytmxState->isSuccess = true;
}

void HandleElementBegin(RaytmxState* raytmxState, hoxml_context_t* hoxmlContext) {
    if (raytmxState == NULL || hoxmlContext == NULL)
        return;

    if (strcmp(hoxmlContext->tag, "map") == 0)
        ;
    else if (strcmp(hoxmlContext->tag, "properties") == 0) {
        /* TMX allows nested properties but they are not (currently?) supported. To avoid memory leaks <properties> */
        /* depth is tracked. */
        raytmxState->propertiesDepth += 1;
    } else if (strcmp(hoxmlContext->tag, "property") == 0)
        raytmxState->property = AddProperty(raytmxState);
    else if (strcmp(hoxmlContext->tag, "tileset") == 0)
        raytmxState->tileset = AddTileset(raytmxState);
    else if (strcmp(hoxmlContext->tag, "image") == 0) {
        /* If any of the elements that may have an image is/are open */
        if (raytmxState->tilesetTile != NULL || raytmxState->tileset != NULL || raytmxState->imageLayer != NULL) {
            /* If the open element already has an image */
            /* Note: In all cases, the element can contain at most one <image> */
            if ((raytmxState->tilesetTile != NULL && raytmxState->tilesetTile->hasImage) ||
                    (raytmxState->tileset != NULL && raytmxState->tileset->hasImage) ||
                    (raytmxState->imageLayer != NULL && raytmxState->imageLayer->hasImage)) {
                TraceLog(LOG_WARNING, "RAYTMX: an element contained multiple images; the image on line %d will be "
                    "dropped", hoxmlContext->line);
            } else {
                /* <image> elements can be children of <tileset>, <tile>, or <imagelayer> elements */
                if (raytmxState->tilesetTile != NULL) {
                    raytmxState->tilesetTile->hasImage = true;
                    raytmxState->image = &raytmxState->tilesetTile->image;
                } else if (raytmxState->tileset != NULL) {
                    raytmxState->tileset->hasImage = true;
                    raytmxState->image = &raytmxState->tileset->image;
                } else if (raytmxState->imageLayer != NULL) {
                    raytmxState->imageLayer->hasImage = true;
                    raytmxState->image = &raytmxState->imageLayer->image;
                }
            }
        }
    } /* strcmp(hoxmlContext->tag, "image") == 0 */
    else if (strcmp(hoxmlContext->tag, "tile") == 0) {
        /* <tile> elements can be children of <tileset> or <layer>. They are also not the same element in that they */
        /* have entirely different attributes and a tileset's <tile> may have children. */
        if (raytmxState->tileset != NULL)
            raytmxState->tilesetTile = AddTilesetTile(raytmxState);
        /* Layer <tile>s are added during attribute handling because they provide a GID attribute and nothing else */
    } /* strcmp(hoxmlContext->tag, "tile") == 0 */
    else if (strcmp(hoxmlContext->tag, "animation") == 0) {
        if (raytmxState->tilesetTile != NULL)
            raytmxState->tilesetTile->hasAnimation = true;
    } else if (strcmp(hoxmlContext->tag, "frame") == 0)
        raytmxState->animationFrame = AddAnimationFrame(raytmxState);
    else if (strcmp(hoxmlContext->tag, "layer") == 0) {
        /* Allocate a new layer with 'tileLayer' allocated and append it to the current group, if it exists */
        raytmxState->layer = AddGenericLayer(raytmxState, /* isGroup: */ false);
        raytmxState->layer->type = LAYER_TYPE_TILE_LAYER;
        raytmxState->tileLayer = &raytmxState->layer->exact.tileLayer;
    } else if (strcmp(hoxmlContext->tag, "objectgroup") == 0) {
        if (raytmxState->tilesetTile != NULL) { /* If the object group is a child of a <tile>, it's collision info */
            raytmxState->objectGroup = &raytmxState->tilesetTile->objectGroup;
            /* Child objects (rectangles, points, ellipses, or polygons) are expected to follow */
        } else {
            /* Allocate a new layer with 'objectGroup' allocated and append it to the current group, if it exists */
            raytmxState->layer = AddGenericLayer(raytmxState, /* isGroup: */ false);
            raytmxState->layer->type = LAYER_TYPE_OBJECT_GROUP;
            raytmxState->objectGroup = &raytmxState->layer->exact.objectGroup;
        }
    } else if (strcmp(hoxmlContext->tag, "object") == 0) {
        /* <object> elements are typically only allowable as children of <objectgroup>s but object templates, TX */
        /* files, contain them as children of root <template> */
        if (raytmxState->objectGroup != NULL || raytmxState->format == FORMAT_TX)
            raytmxState->object = AddObject(raytmxState);
    } else if (strcmp(hoxmlContext->tag, "ellipse") == 0) {
        if (raytmxState->object != NULL) {
            /* An <ellipse> within an <object> indicates its type but the <object>'s 'x,' 'y,' 'width,' and 'height' */
            /* attributes are used to define the ellipse so assigning the type is all that's necessary */
            raytmxState->object->type = OBJECT_TYPE_ELLIPSE;
        }
    } else if (strcmp(hoxmlContext->tag, "point") == 0) {
        if (raytmxState->object != NULL) {
            /* A <point> within an <object> indicates its type but the <object>'s 'x' and 'y' attributes are used to */
            /* define the point so assigning the type is all that's necessary */
            raytmxState->object->type = OBJECT_TYPE_POINT;
        }
    } else if (strcmp(hoxmlContext->tag, "polygon") == 0) {
        if (raytmxState->object != NULL) {
            /* Note: <polygon>s and <polyline>s have a list of points/vertices defined in a 'points' attribute */
            raytmxState->object->type = OBJECT_TYPE_POLYGON;
        }
    } else if (strcmp(hoxmlContext->tag, "polyline") == 0) {
        if (raytmxState->object != NULL) {
            /* Note: <polyline>s and <polygone>s have a list of points/vertices defined in a 'points' attribute */
            raytmxState->object->type = OBJECT_TYPE_POLYLINE;
        }
    } else if (strcmp(hoxmlContext->tag, "text") == 0) {
        if (raytmxState->object != NULL) {
            raytmxState->object->type = OBJECT_TYPE_TEXT;
            raytmxState->object->text = (TmxText*)MemAllocZero(sizeof(TmxText));
            /* There are a couple non-zero default values for <text> attributes: */
            raytmxState->object->text->pixelSize = 16;
            raytmxState->object->text->color.a = 255; /* Full opacity black */
            raytmxState->object->text->kerning = 1;
            /* The font family will also default to "sans-serif" when the element ends if there is no attribute */
        }
    } else if (strcmp(hoxmlContext->tag, "imagelayer") == 0) {
        /* Allocate a new layer with 'imageLayer' allocated and append it to the current group, if it exists */
        raytmxState->layer = AddGenericLayer(raytmxState, /* isGroup: */ false);
        raytmxState->layer->type = LAYER_TYPE_IMAGE_LAYER;
        raytmxState->imageLayer = &raytmxState->layer->exact.imageLayer;
    } else if (strcmp(hoxmlContext->tag, "group") == 0) {
        /* Allocate a new layer and append it to the current group, if it exists */
        raytmxState->layer = AddGenericLayer(raytmxState, /* isGroup: */ true);
        raytmxState->layer->type = LAYER_TYPE_GROUP;
    }
}

void HandleAttribute(RaytmxState* raytmxState, hoxml_context_t* hoxmlContext) {
    if (raytmxState == NULL || hoxmlContext == NULL)
        return;

    if (strcmp(hoxmlContext->tag, "map") == 0) {
        if (strcmp(hoxmlContext->attribute, "orientation") == 0) {
            if (strcmp(hoxmlContext->value, "orthogonal") == 0)
                raytmxState->mapOrientation = ORIENTATION_ORTHOGONAL;
            else if (strcmp(hoxmlContext->value, "isometric") == 0)
                raytmxState->mapOrientation = ORIENTATION_ISOMETRIC;
            else if (strcmp(hoxmlContext->value, "staggered") == 0)
                raytmxState->mapOrientation = ORIENTATION_STAGGERED;
            else if (strcmp(hoxmlContext->value, "hexagonal") == 0)
                raytmxState->mapOrientation = ORIENTATION_HEXAGONAL;
        } /* strcmp(hoxmlContext->attribute, "orientation") == 0 */
        else if (strcmp(hoxmlContext->attribute, "renderorder") == 0) {
            if (strcmp(hoxmlContext->value, "right-down") == 0)
                raytmxState->mapRenderOrder = RENDER_ORDER_RIGHT_DOWN;
            else if (strcmp(hoxmlContext->value, "right-up") == 0)
                raytmxState->mapRenderOrder = RENDER_ORDER_RIGHT_UP;
            else if (strcmp(hoxmlContext->value, "left-down") == 0)
                raytmxState->mapRenderOrder = RENDER_ORDER_LEFT_DOWN;
            else if (strcmp(hoxmlContext->value, "left-up") == 0)
                raytmxState->mapRenderOrder = RENDER_ORDER_LEFT_UP;
        } /* strcmp(hoxmlContext->attribute, "renderorder") == 0 */
        else if (strcmp(hoxmlContext->attribute, "width") == 0)
            raytmxState->mapWidth = atoi(hoxmlContext->value);
        else if (strcmp(hoxmlContext->attribute, "height") == 0)
            raytmxState->mapHeight = atoi(hoxmlContext->value);
        else if (strcmp(hoxmlContext->attribute, "tilewidth") == 0)
            raytmxState->mapTileWidth = atoi(hoxmlContext->value);
        else if (strcmp(hoxmlContext->attribute, "tileheight") == 0)
            raytmxState->mapTileHeight = atoi(hoxmlContext->value);
        else if (strcmp(hoxmlContext->attribute, "parallaxoriginx") == 0)
            raytmxState->mapParallaxOriginX = atoi(hoxmlContext->value);
        else if (strcmp(hoxmlContext->attribute, "parallaxoriginy") == 0)
            raytmxState->mapParallaxOriginY = atoi(hoxmlContext->value);
        else if (strcmp(hoxmlContext->attribute, "backgroundcolor") == 0) {
            raytmxState->mapBackgroundColor = GetColorFromHexString(hoxmlContext->value);
            raytmxState->mapHasBackgroundColor = true;
        }
    } /* strcmp(hoxmlContext->tag, "map") == 0 */
    else if (strcmp(hoxmlContext->tag, "property") == 0) {
        if (raytmxState->property != NULL) {
            if (strcmp(hoxmlContext->attribute, "name") == 0) {
                raytmxState->property->name = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->property->name, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "type") == 0) {
                if (strcmp(hoxmlContext->value, "string") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_STRING;
                else if (strcmp(hoxmlContext->value, "int") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_INT;
                else if (strcmp(hoxmlContext->value, "float") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_FLOAT;
                else if (strcmp(hoxmlContext->value, "bool") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_BOOL;
                else if (strcmp(hoxmlContext->value, "color") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_COLOR;
                else if (strcmp(hoxmlContext->value, "file") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_FILE;
                else if (strcmp(hoxmlContext->value, "object") == 0)
                    raytmxState->property->type = PROPERTY_TYPE_OBJECT;
                /* TMX documentation also mentions a "class" type but doesn't describe what it is nor does Tiled list */
                /* it as an option when adding a property. So what is it? Unsupported, that's what. */
            } /* strcmp(hoxmlContext->attribute, "type") == 0 */
            else if (strcmp(hoxmlContext->attribute, "value") == 0) {
                /* Although unlikley, it's possible that 'value' attribute will be parsed before the 'type' */
                /* attribute. In that case, doing a cast/conversion now may not be possible. To avoid this, the raw */
                /* string value is copied to 'stringValue' temporarily, or permanently for string and file types, and */
                /* the cast/conversion will happen at the end of the element if needed. */
                raytmxState->property->stringValue = (char*)MemAlloc((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->property->stringValue, hoxmlContext->value);
            } /* strcmp(hoxmlContext->attribute, "value") == 0 */
        } /* raytmxState->property != NULL */
    } /* strcmp(hoxmlContext->tag, "property") == 0 */
    else if (strcmp(hoxmlContext->tag, "tileset") == 0) {
        if (raytmxState->tileset != NULL) {
            if (strcmp(hoxmlContext->attribute, "firstgid") == 0)
                raytmxState->tileset->firstGid = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "source") == 0) {
                raytmxState->tileset->source = (char*)MemAlloc((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->tileset->source, hoxmlContext->value);
                /* 'source' points to an external TSX file that defines the majority of the tileset. Try to load it. */
                RaytmxExternalTileset externalTileset = LoadTSX(JoinPath(raytmxState->documentDirectory,
                    hoxmlContext->value));
                if (externalTileset.isSuccess) {
                    /* A <tileset> within a <map> will have two attributes: 'firstgid' and 'source.' The rest of */
                    /* the tileset's details are in the external TSX that 'source' points to. They need to be merged. */
                    /* Remember the two internal attributes. */
                    uint32_t tempFirstGid = raytmxState->tileset->firstGid;
                    char* tempSource = raytmxState->tileset->source;
                    /* Assign all values from the TSX's tileset to the one within the state object. This will */
                    /* overrite the values of 'firstGid' and 'source.' */
                    *raytmxState->tileset = externalTileset.tileset;
                    /* Reassign the original 'firstGid' and 'source' values */
                    raytmxState->tileset->firstGid = tempFirstGid;
                    raytmxState->tileset->source = tempSource;
                }
            } else if (strcmp(hoxmlContext->attribute, "name") == 0) {
                raytmxState->tileset->name = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->tileset->name, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "class") == 0) {
                raytmxState->tileset->classString = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->tileset->classString, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "tilewidth") == 0)
                raytmxState->tileset->tileWidth = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "tileheight") == 0)
                raytmxState->tileset->tileHeight = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "spacing") == 0)
                raytmxState->tileset->spacing = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "margin") == 0)
                raytmxState->tileset->margin = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "tilecount") == 0)
                raytmxState->tileset->tileCount = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "columns") == 0)
                raytmxState->tileset->columns = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "objectalignment") == 0) {
                if (strcmp(hoxmlContext->value, "unspecified") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_UNSPECIFIED;
                else if (strcmp(hoxmlContext->value, "topleft") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_TOP_LEFT;
                else if (strcmp(hoxmlContext->value, "top") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_TOP;
                else if (strcmp(hoxmlContext->value, "topright") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_TOP_RIGHT;
                else if (strcmp(hoxmlContext->value, "left") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_LEFT;
                else if (strcmp(hoxmlContext->value, "center") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_CENTER;
                else if (strcmp(hoxmlContext->value, "right") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_RIGHT;
                else if (strcmp(hoxmlContext->value, "bottomleft") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM_LEFT;
                else if (strcmp(hoxmlContext->value, "bottom") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM;
                else if (strcmp(hoxmlContext->value, "bottomright") == 0)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM_RIGHT;
            }
        } /* raytmState->tileset != NULL */
    } /* strcmp(hoxmlContext->tag, "tileset") == 0 */
    else if (strcmp(hoxmlContext->tag, "tileoffset") == 0) {
        if (raytmxState->tileset != NULL) {
            if (strcmp(hoxmlContext->attribute, "x") == 0)
                raytmxState->tileset->tileOffsetX = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "y") == 0)
                raytmxState->tileset->tileOffsetY = atoi(hoxmlContext->value);
        }
    } /* strcmp(hoxmlContext->tag, "tileoffset") == 0 */
    else if (strcmp(hoxmlContext->tag, "image") == 0) {
        if (raytmxState->image != NULL) {
            if (strcmp(hoxmlContext->attribute, "source") == 0) {
                raytmxState->image->source = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->image->source, hoxmlContext->value);
                RaytmxCachedTextureNode* cachedTexture = LoadCachedTexture(raytmxState, hoxmlContext->value);
                if (cachedTexture != NULL)
                     raytmxState->image->texture = cachedTexture->texture;
            } else if (strcmp(hoxmlContext->attribute, "trans") == 0) {
                raytmxState->image->trans = GetColorFromHexString(hoxmlContext->value);
                raytmxState->image->hasTrans = true;
            } else if (strcmp(hoxmlContext->attribute, "width") == 0)
                raytmxState->image->width = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "height") == 0)
                raytmxState->image->height = atoi(hoxmlContext->value);
        }
    } /* strcmp(hoxmlContext->tag, "image") == 0 */
    else if (strcmp(hoxmlContext->tag, "tile") == 0) {
        if (raytmxState->tilesetTile != NULL) { /* If the <tile> corresponds to a tileset tile */
            if (strcmp(hoxmlContext->attribute, "id") == 0)
                raytmxState->tilesetTile->id = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "type") == 0 || strcmp(hoxmlContext->attribute, "class") == 0)
                raytmxState->tilesetTile->id = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "x") == 0)
                raytmxState->tilesetTile->x = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "y") == 0)
                raytmxState->tilesetTile->y = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "width") == 0)
                raytmxState->tilesetTile->width = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "height") == 0)
                raytmxState->tilesetTile->height = atoi(hoxmlContext->value);
        } else { /* If the <tile> corresponds to a layer tile */
            if (strcmp(hoxmlContext->attribute, "gid") == 0)
                AddTileLayerTile(raytmxState, atoi(hoxmlContext->value));
        }
    } /* strcmp(hoxmlContext->tag, "tile") == 0 */
    else if (strcmp(hoxmlContext->tag, "frame") == 0) {
        if (raytmxState->animationFrame != NULL) {
            if (strcmp(hoxmlContext->attribute, "tileid") == 0)
                raytmxState->animationFrame->id = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "duration") == 0)
                raytmxState->animationFrame->duration = (float)atoi(hoxmlContext->value) / 1000.0f;
        }
    } /* strcmp(hoxmlContext->tag, "frame") == 0 */
    else if (strcmp(hoxmlContext->tag, "layer") == 0) {
        if (raytmxState->tileLayer != NULL) {
            /* Check for attributes specific to <layer> layers */
            if (strcmp(hoxmlContext->attribute, "width") == 0)
                raytmxState->tileLayer->width = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "height") == 0)
                raytmxState->tileLayer->height = atoi(hoxmlContext->value);
        }
    } /* strcmp(hoxmlContext->tag, "layer") == 0) */
    else if (strcmp(hoxmlContext->tag, "data") == 0) {
        if (raytmxState->tileLayer != NULL) { /* If this <data> applies to a <layer> */
            if (strcmp(hoxmlContext->attribute, "encoding") == 0) {
                raytmxState->tileLayer->encoding = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->tileLayer->encoding, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "compression") == 0) {
                raytmxState->tileLayer->compression =
                    (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->tileLayer->compression, hoxmlContext->value);
            }
        } else if (raytmxState->image != NULL) { /* If this <data> applies to an <image> */
            /* TODO (?): The TMX map format documentation says an <image> can contain a <data> element but doesn't */
            /* provide any more information than that. Tiled doesn't seem to have a feature for this either. */
        }
    } /* strcmp(hoxmlContext->tag, "data") == 0 */
    else if (strcmp(hoxmlContext->tag, "objectgroup") == 0) {
        if (raytmxState->objectGroup != NULL) {
            /* Check for attributes specific to <objectgroup> layers */
            if (strcmp(hoxmlContext->attribute, "color") == 0) {
                raytmxState->objectGroup->color = GetColorFromHexString(hoxmlContext->value);
                raytmxState->objectGroup->hasColor = true;
            } /* else if (strcmp(hoxmlContext->attribute, "width") == 0)
                raytmxState->objectGroup->width = atoi(hoxmlContext->value); */ /* "Meaningless" according to docs. */
            /* else if (strcmp(hoxmlContext->attribute, "height") == 0)
                raytmxState->objectGroup->height = atoi(hoxmlContext->value); */ /* "Meaningless" according to docs. */
           else if (strcmp(hoxmlContext->attribute, "draworder") == 0) {
                if (strcmp(hoxmlContext->value, "index") == 0)
                    raytmxState->objectGroup->drawOrder = OBJECT_GROUP_DRAW_ORDER_INDEX;
                else if (strcmp(hoxmlContext->value, "topdown") == 0)
                    raytmxState->objectGroup->drawOrder = OBJECT_GROUP_DRAW_ORDER_TOP_DOWN;
           }
        }
    } /* strcmp(hoxmlContext->tag, "objectgroup") == 0 */
    else if (strcmp(hoxmlContext->tag, "object") == 0) {
        if (raytmxState->object != NULL) {
            if (strcmp(hoxmlContext->attribute, "id") == 0)
                raytmxState->object->id = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "name") == 0) {
                raytmxState->object->name = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->object->name, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "type") == 0) {
                raytmxState->object->typeString = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->object->typeString, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "x") == 0)
                raytmxState->object->x = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "y") == 0)
                raytmxState->object->y = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "width") == 0)
                raytmxState->object->width = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "height") == 0)
                raytmxState->object->height = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "rotation") == 0)
                raytmxState->object->rotation = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "gid") == 0) {
                raytmxState->object->gid = atoi(hoxmlContext->value);
                /* The presence of a 'gid' attribute also indicates the object's type is that of a tile */
                raytmxState->object->type = OBJECT_TYPE_TILE;
            } else if (strcmp(hoxmlContext->attribute, "visible") == 0)
                raytmxState->object->visible = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "template") == 0) {
                raytmxState->object->templateString =
                    (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->object->templateString, hoxmlContext->value);
            }
        }
    } /* strcmp(hoxmlContext->tag, "object") == 0 */
    else if (strcmp(hoxmlContext->tag, "polygon") == 0 || strcmp(hoxmlContext->tag, "polyline") == 0) {
        /* <polygon> and <polyline>, children of <object>, both have just one attribute: 'points' */
        if (raytmxState->object != NULL && strcmp(hoxmlContext->attribute, "points") == 0) {
            if (raytmxState->object->points != NULL) { /* If there's already an array of points */
                TraceLog(LOG_WARNING, "RAYTMX: object \"%s\", has multiple 'points' attributes; points listed in any "
                    "latter 'points' attributes will be dropped", raytmxState->object->name);
                return;
            }
            /* The 'points' attribute's value is a string containing all points of the poly(gon|line) in the form */
            /* "0,0 31.25,-0.75 49.5,-16.5 93.25,-17.25" where the points are [0, 0], [31.25, -0.75], etc. and these */
            /* points are relative to the object's position (its 'x' and 'y' attributes) */
            char x[32], y[32], *iterator, *terminator, *comma;
            iterator = hoxmlContext->value;
            RaytmxPolyPointNode *pointsRoot = NULL, *pointsTail = NULL;
            uint32_t pointsLength = 0;
            Vector2 vertexSum; /* Specific to polygons, the centroid is calculated requiring a sum of all vertices */
            vertexSum.x = 0;
            vertexSum.y = 0;
            while (iterator != NULL) {
                terminator = strstr(iterator, " "); /* Use this pointer to mark the end of the current point */
                if (terminator == NULL) { /* There are no space delimiters this far into the value string */
                    /* This is the end of the value string so we'll use a regular null terminator instead */
                    terminator = iterator + strlen(iterator);
                }
                comma = strstr(iterator, ","); /* Use this pointer to mark the location of the comma between X and Y */
                if (comma == NULL) {
                    /* The string is malformed. Cannot continue. */
                    TraceLog(LOG_WARNING, "RAYTMX: The 'points' attribute on line %d has a malformed value; at least "
                        "one point is lost as a result", hoxmlContext->line);
                    break;
                }
                /* 'iterator' is pointing to the first digit, 'comma' is pointing to the comma, and 'terminator' is */
                /* pointing to either the terminating space or a typical null terminator */
                StringCopyN(x, iterator, comma - iterator); /* Copy from 'iterator' up to but excluding 'comma' */
                x[comma - iterator] = '\0';
                iterator = comma + 1; /* Point the iterator right after the ',' where Y begins */
                StringCopyN(y, iterator, terminator - iterator); /* Copy 'iterator' up to but excluding 'terminator' */
                y[terminator - iterator] = '\0';
                /* Create a linked list node to hold the point and append it to the linked list */
                RaytmxPolyPointNode* node = (RaytmxPolyPointNode*)MemAllocZero(sizeof(RaytmxPolyPointNode));
                /* Note: These values may be negative. A poly(gon|line) object's position is determined by the first */
                /* vertex added leading to the first entry to be "0,0" and all other vertices relative to it. */
                node->point.x = (float)atof(x);
                node->point.y = (float)atof(y);
                vertexSum.x += node->point.x;
                vertexSum.y += node->point.y;
                if (pointsRoot == NULL) /* If this is the first point to add to the list */
                    pointsRoot = node;
                else /* If adding to the tail of the list */
                    pointsTail->next = node;
                pointsTail = node;
                pointsLength += 1;
                iterator = terminator[0] != '\0' ? terminator + 1 : NULL;
            }

            if (pointsRoot != NULL) { /* If a list with at least one node was created from the 'points' attribute */
                /* The first vertex will be duplicated and appended to the end of the list, for drawing purposes, so */
                /* the length of the points list is incremented by one */
                pointsLength += 1;
                bool isPolygon = strcmp(hoxmlContext->tag, "polygon") == 0;
                if (isPolygon) { /* If the object is a polygon, not polyline */
                    /* Polygons will be drawn using raylib's DrawTriangleFan() function in which the first point is */
                    /* the centroid. It must also end with the first, non-centroid point. So, for polygons, the list */
                    /* will have two more points. */
                    pointsLength += 1;
                }
                /* Allocate the array and assign NULL to every index to be safe */
                Vector2* points = (Vector2*)MemAllocZero(sizeof(Vector2) * pointsLength);
                if (isPolygon) { /* If the centroid should be included as a vertex */
                    /* Finish calculating the centroid by averaging the sum of the vertices keeping in mind that */
                    /* 'pointsLength' is equal to N + 2 */
                    points[0].x = vertexSum.x / (pointsLength - 2);
                    points[0].y = vertexSum.y / (pointsLength - 2);
                }
                /* Copy the points as Vector2s into the array and free the nodes while we're at it */
                RaytmxPolyPointNode* iteratorNode = pointsRoot;
                uint32_t i = isPolygon ? 1 : 0; /* Skip over the first element, the centroid, for polygons only */
                while (iteratorNode != NULL) {
                    points[i] = iteratorNode->point;
                    RaytmxPolyPointNode* parent = iteratorNode;
                    iteratorNode = iteratorNode->next;
                    i += 1;
                    MemFree(parent);
                }
                /* End the list with the first point. Both polygons and polylines use this when drawing. */
                points[pointsLength - 1].x = points[isPolygon ? 1 : 0].x;
                points[pointsLength - 1].y = points[isPolygon ? 1 : 0].y;
                /* TODO: sort the vertices into counter-clockwise order as DrawTriangleFan() requires it */
                /* Add the points array to the element it applies to */
                raytmxState->object->points = points;
                raytmxState->object->pointsLength = pointsLength;
                raytmxState->object->drawPoints = (Vector2*)MemAllocZero(sizeof(Vector2) * pointsLength);
            }
        } /* raytmxState->object != NULL && strcmp(hoxmlContext->attribute, "points") == 0 */
    } /* strcmp(hoxmlContext->tag, "polygon") == 0 || strcmp(hoxmlContext->tag, "polyline") == 0 */
    else if (strcmp(hoxmlContext->tag, "text") == 0) {
        if (raytmxState->object != NULL && raytmxState->object->text != NULL) {
            if (strcmp(hoxmlContext->attribute, "fontfamily") == 0) {
                raytmxState->object->text->fontFamily =
                    (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->object->text->fontFamily, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "pixelsize") == 0)
                raytmxState->object->text->pixelSize = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "wrap") == 0)
                raytmxState->object->text->wrap = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "color") == 0)
                raytmxState->object->text->color = GetColorFromHexString(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "bold") == 0)
                raytmxState->object->text->bold = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "italic") == 0)
                raytmxState->object->text->italic = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "underline") == 0)
                raytmxState->object->text->underline = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "strikeout") == 0)
                raytmxState->object->text->strikeOut = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "kerning") == 0)
                raytmxState->object->text->kerning = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "halign") == 0) {
                if (strcmp(hoxmlContext->value, "left") == 0)
                    raytmxState->object->text->halign = HORIZONTAL_ALIGNMENT_LEFT;
                else if (strcmp(hoxmlContext->value, "center") == 0)
                    raytmxState->object->text->halign = HORIZONTAL_ALIGNMENT_CENTER;
                else if (strcmp(hoxmlContext->value, "right") == 0)
                    raytmxState->object->text->halign = HORIZONTAL_ALIGNMENT_RIGHT;
                else if (strcmp(hoxmlContext->value, "justify") == 0)
                    raytmxState->object->text->halign = HORIZONTAL_ALIGNMENT_JUSTIFY;
            } else if (strcmp(hoxmlContext->attribute, "valign") == 0) {
                if (strcmp(hoxmlContext->value, "top") == 0)
                    raytmxState->object->text->valign = VERTICAL_ALIGNMENT_TOP;
                else if (strcmp(hoxmlContext->value, "center") == 0)
                    raytmxState->object->text->valign = VERTICAL_ALIGNMENT_CENTER;
                else if (strcmp(hoxmlContext->value, "bottom") == 0)
                    raytmxState->object->text->valign = VERTICAL_ALIGNMENT_BOTTOM;
            }
        } /* raytmxState->object != NULL && raytmxState->object->text != NULL */
    } /* strcmp(hoxmlContext->tag, "text") == 0 */
    else if (strcmp(hoxmlContext->tag, "imagelayer") == 0) {
        if (raytmxState->imageLayer != NULL) {
            /* Check for attributes specific to <imagelayer> layers */
            if (strcmp(hoxmlContext->attribute, "repeatx") == 0)
                raytmxState->imageLayer->repeatX = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "repeaty") == 0)
                raytmxState->imageLayer->repeatY = atoi(hoxmlContext->value) != 0 ? true : false;
        }
    } /* strcmp(hoxmlContext->tag, "imagelayer") == 0 */

    if (strcmp(hoxmlContext->tag, "layer") == 0 || strcmp(hoxmlContext->tag, "objectgroup") == 0 ||
            strcmp(hoxmlContext->tag, "imagelayer") == 0 || strcmp(hoxmlContext->tag, "group") == 0) {
        if (raytmxState->layer != NULL) {
            /* Check for attributes common to all layer types */
            if (strcmp(hoxmlContext->attribute, "id") == 0)
                raytmxState->layer->id = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "name") == 0) {
                raytmxState->layer->name = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->layer->name, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "class") == 0) {
                raytmxState->layer->classString = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->value) + 1);
                StringCopy(raytmxState->layer->classString, hoxmlContext->value);
            } else if (strcmp(hoxmlContext->attribute, "opacity") == 0)
                raytmxState->layer->opacity = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "visible") == 0)
                raytmxState->layer->visible = atoi(hoxmlContext->value) != 0 ? true : false;
            else if (strcmp(hoxmlContext->attribute, "tintcolor") == 0) {
                raytmxState->layer->tintColor = GetColorFromHexString(hoxmlContext->value);
                raytmxState->layer->hasTintColor = true;
            } else if (strcmp(hoxmlContext->attribute, "offsetx") == 0)
                raytmxState->layer->offsetX = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "offsety") == 0)
                raytmxState->layer->offsetY = atoi(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "parallaxx") == 0)
                raytmxState->layer->parallaxX = atof(hoxmlContext->value);
            else if (strcmp(hoxmlContext->attribute, "parallaxy") == 0)
                raytmxState->layer->parallaxY = atof(hoxmlContext->value);
        }
    } /* strcmp(hoxmlContext->tag, "layer") == 0 || strcmp(hoxmlContext->tag, "objectgroup") == 0 || */
      /* strcmp(hoxmlContext->tag, "imagelayer") == 0 || strcmp(hoxmlContext->tag, "group") == 0 */
}

void HandleElementEnd(RaytmxState* raytmxState, hoxml_context_t* hoxmlContext) {
    if (raytmxState == NULL || hoxmlContext == NULL)
        return;

    /* If the element is one of the layer types which share some common attributes that may need default strings */
    if (strcmp(hoxmlContext->tag, "layer") == 0 || strcmp(hoxmlContext->tag, "objectgroup") == 0 ||
            strcmp(hoxmlContext->tag, "imagelayer") == 0 || strcmp(hoxmlContext->tag, "group") == 0) {
        TmxLayer* layer = raytmxState->layer;
        if (layer == NULL && raytmxState->groupNode != NULL)
            layer = &raytmxState->groupNode->layer;

        if (layer != NULL) {
            /* Apply default values for the attribute(s) that aren't covered by a simple memset(x, 0, sizeof(x)) */
            if (layer->name == NULL) { /* If this layer didn't have a 'name' attribute */
                /* The default value for 'name' is "" (an empty string) */
                layer->name = (char*)MemAlloc(1);
                layer->name[0] = '\0';
            }
            if (layer->classString == NULL) { /* If this layer didn't have a 'class' attribute */
                /* The default value for 'class' is "" (an empty string) */
                layer->classString = (char*)MemAlloc(1);
                layer->classString[0] = '\0';
            }
        }
    } /* strcmp(hoxmlContext->tag, "layer") == 0 || strcmp(hoxmlContext->tag, "objectgroup") == 0 || */
      /* strcmp(hoxmlContext->tag, "imagelayer") == 0 || strcmp(hoxmlContext->tag, "group") == 0 */

    if (strcmp(hoxmlContext->tag, "properties") == 0) {
        if (raytmxState->propertiesRoot == NULL)
            return;
        /* TMX allows nested properties (e.g. <properties><properties><property/></properties></properties>) but */
        /* that is unsupported. Depth is tracked such that all properties, included nested ones, are appended to one */
        /* array when the outermost element ends. */
        raytmxState->propertiesDepth -= 1;
        if (raytmxState->propertiesDepth > 0) /* If the outermost <properties> has not yet ended */
            return;
        /* Allocate the array and assign NULL to every index to be safe */
        TmxProperty* properties = (TmxProperty*)MemAllocZero(sizeof(TmxProperty) * raytmxState->propertiesLength);
        /* Copy the TmxProperty pointers into the array and free the nodes while we're at it */
        RaytmxPropertyNode* iterator = raytmxState->propertiesRoot;
        for (uint32_t i = 0; i < raytmxState->propertiesLength; i++) {
            properties[i] = iterator->property;
            RaytmxPropertyNode* parent = iterator;
            iterator = iterator->next;
            MemFree(parent);
        }
        /* Add the properties array to the element it applies to */
        /* A <property>, or rather its parent <properties>, can be within 10+ other elements. The order of the checks */
        /* here is slightly important as the most-nested elements must come first in cases where one may contain */
        /* another (e.g. an <object> within an <objectgroup> layer). */
        if (raytmxState->tilesetTile != NULL && raytmxState->tilesetTile->properties == NULL) {
            raytmxState->tilesetTile->properties = properties;
            raytmxState->tilesetTile->propertiesLength = raytmxState->propertiesLength;
        }
        /* else if (raytmxState->wangColor != NULL && raytmxState->wangColor->properties == NULL) {
            raytmxState->wangColor->properties = properties;
            raytmxState->wangColor->propertiesLength = raytmxState->propertiesLength;
        }
        else if (raytmxState->wangSet != NULL && raytmxState->wangSet->properties == NULL) {
            raytmxState->wangSet->properties = properties;
            raytmxState->wangSet->propertiesLength = raytmxState->propertiesLength;
        } */ /* TODO: Wang sets. Low priority. */
        else if (raytmxState->tileset != NULL && raytmxState->tileset->properties == NULL) {
            raytmxState->tileset->properties = properties;
            raytmxState->tileset->propertiesLength = raytmxState->propertiesLength;
        } else if (raytmxState->object != NULL && raytmxState->object->properties == NULL) {
            raytmxState->object->properties = properties;
            raytmxState->object->propertiesLength = raytmxState->propertiesLength;
        } else if (raytmxState->layer != NULL && raytmxState->layer->properties == NULL) {
            raytmxState->layer->properties = properties;
            raytmxState->layer->propertiesLength = raytmxState->propertiesLength;
        } else {
            raytmxState->mapProperties = properties;
            raytmxState->mapPropertiesLength = raytmxState->propertiesLength;
        }
        /* Clean up the state object */
        raytmxState->propertiesRoot = NULL;
        raytmxState->propertiesTail = NULL;
        raytmxState->propertiesLength = 0;
    } /* strcmp(hoxmlContext->tag, "properties") == 0 */
    else if (strcmp(hoxmlContext->tag, "property") == 0) {
        if (raytmxState->property != NULL) {
            /* Apply default values for the attribute(s) that aren't covered by a simple memset(x, 0, sizeof(x)) */
            /* Properties are cast and assigned to type-specific variables at the end of the element due to the order */
            /* of the property's attributes not being guaranteed, so we wait until we have all the information */
            switch (raytmxState->property->type) {
            case PROPERTY_TYPE_STRING:
            default: /* The default type of a property is 'string' */
                if (raytmxState->property->stringValue == NULL) {
                    if (hoxmlContext->content != NULL) { /* If Tiled opted to put the value in the element's content */
                        /* From the documentation: "When a string property contains newlines, the current version of */
                        /* Tiled will write out the value as characters contained inside the property element rather */
                        /* than as the value attribute." */
                        raytmxState->property->stringValue =
                            (char*)MemAlloc((unsigned int)strlen(hoxmlContext->content) + 1);
                        StringCopy(raytmxState->property->stringValue, hoxmlContext->content);
                    } else { /* If the string's value was neither provided as an attribute nor content */
                        /* The default value for 'string' is an empty string */
                        raytmxState->property->stringValue = (char*)MemAlloc(1);
                        raytmxState->property->stringValue[0] = '\0';
                    }
                } break;
            case PROPERTY_TYPE_INT:
            case PROPERTY_TYPE_OBJECT:
                /* The default value for 'int' and 'object' is 0 */
                raytmxState->property->intValue = atoi(raytmxState->property->stringValue); /* Defaults to 0 */
                break;
            case PROPERTY_TYPE_FLOAT:
                /* The default value for 'float' is 0 */
                raytmxState->property->floatValue = (float)atof(raytmxState->property->stringValue); /* Defaults to 0 */
                break;
            case PROPERTY_TYPE_BOOL:
                /* The default value for 'bool' is false */
                if (raytmxState->property->stringValue == NULL ||
                        strcmp(raytmxState->property->stringValue, "true") != 0)
                    raytmxState->property->boolValue = false;
                else
                    raytmxState->property->boolValue = true;
                break;
            case PROPERTY_TYPE_COLOR:
                /* The default value for 'color' is the color #00000000 */
                if (raytmxState->property->stringValue == NULL)
                    raytmxState->property->colorValue = BLANK; /* #defined by raylib as { 0, 0, 0, 0 } */
                else
                    raytmxState->property->colorValue = GetColorFromHexString(raytmxState->property->stringValue);
                break;
            case PROPERTY_TYPE_FILE:
                /* The default value for 'file' is "." */
                if (raytmxState->property->stringValue == NULL) {
                    raytmxState->property->stringValue = (char*)MemAlloc(2);
                    raytmxState->property->stringValue[0] = '.';
                    raytmxState->property->stringValue[1] = '\0';
                } break;
            } /* switch (raytmxState->property->type) */

            /* If the type was neither 'string' nor 'file' and 'stringValue' is set */
            if (raytmxState->property->type != PROPERTY_TYPE_STRING &&
                    raytmxState->property->type != PROPERTY_TYPE_FILE && raytmxState->property->stringValue != NULL) {
                /* Properties of types other than 'string' and 'file' are placed in 'stringValue' temporarily. Now */
                /* that they have been cast and assigned appropriately, 'stringValue' can be freed. */
                MemFree(raytmxState->property->stringValue);
                raytmxState->property->stringValue = NULL;
            }
        }
        raytmxState->property = NULL;
    } /* strcmp(hoxmlContext->tag, "property") == 0 */
    else if (strcmp(hoxmlContext->tag, "tileset") == 0) {
        if (raytmxState->tileset != NULL) {
            /* Apply default values for the attribute(s) that aren't covered by a simple memset(x, 0, sizeof(x)) */
            if (raytmxState->tileset->name == NULL) { /* If this <tileset> didn't have a 'name' attribute */
                /* The default value for 'name' is "" (an empty string) */
                raytmxState->tileset->name = (char*)MemAlloc(1);
                raytmxState->tileset->name[0] = '\0';
            }
            if (raytmxState->tileset->classString == NULL) { /* If this <tileset> didn't have a 'class' attribute */
                /* The default value for 'class' is "" (an empty string) */
                raytmxState->tileset->classString = (char*)MemAlloc(1);
                raytmxState->tileset->classString[0] = '\0';
            }
            if (raytmxState->tileset->objectAlignment == OBJECT_ALIGNMENT_UNSPECIFIED) {
                /* There are default object alignments for orthogonal and isometric modes */
                if (raytmxState->mapOrientation == ORIENTATION_ORTHOGONAL)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM_LEFT;
                else if (raytmxState->mapOrientation == ORIENTATION_ISOMETRIC)
                    raytmxState->tileset->objectAlignment = OBJECT_ALIGNMENT_BOTTOM;
            }

            if (raytmxState->tilesetTilesRoot != NULL) {
                /* Allocate the array and zeroize every index as initialization */
                TmxTilesetTile* tiles = (TmxTilesetTile*)MemAllocZero(sizeof(TmxTilesetTile) *
                    raytmxState->tilesetTilesLength);
                /* Copy the TmxTilesetTile pointers into the array and free the nodes while we're at it */
                RaytmxTilesetTileNode* iterator = raytmxState->tilesetTilesRoot;
                for (uint32_t i = 0; i < raytmxState->tilesetTilesLength; i++) {
                    tiles[i] = iterator->tile;
                    RaytmxTilesetTileNode* parent = iterator;
                    iterator = iterator->next;
                    MemFree(parent);
                }
                /* Add the tiles array to the tileset */
                raytmxState->tileset->tiles = tiles;
                raytmxState->tileset->tilesLength = raytmxState->tilesetTilesLength;
                /* Clean up the state object */
                raytmxState->tilesetTilesRoot = NULL;
                raytmxState->tilesetTilesTail = NULL;
                raytmxState->tilesetTilesLength = 0;
            }
        }
        raytmxState->tileset = NULL;
    } /* strcmp(hoxmlContext->tag, "tileset") == 0 */
    else if (strcmp(hoxmlContext->tag, "image") == 0)
        raytmxState->image = NULL;
    else if (strcmp(hoxmlContext->tag, "animation") == 0) {
        if (raytmxState->tilesetTile != NULL && raytmxState->tilesetTile->hasAnimation) {
            if (raytmxState->animationFramesRoot == NULL)
                return;
            /* Allocate the array and zeroize every index as initialization */
            TmxAnimationFrame* frames = (TmxAnimationFrame*)MemAllocZero(sizeof(TmxAnimationFrame) *
                raytmxState->animationFramesLength);
            /* Copy the TmxAnimationFrame pointers into the array and free the nodes while we're at it */
            RaytmxAnimationFrameNode* iterator = raytmxState->animationFramesRoot;
            for (uint32_t i = 0; i < raytmxState->animationFramesLength; i++) {
                frames[i] = iterator->frame;
                RaytmxAnimationFrameNode* parent = iterator;
                iterator = iterator->next;
                MemFree(parent);
            }
            /* Add the frames array to the tile's animation */
            raytmxState->tilesetTile->animation.frames = frames;
            raytmxState->tilesetTile->animation.framesLength = raytmxState->animationFramesLength;
            /* Clean up the state object */
            raytmxState->animationFramesRoot = NULL;
            raytmxState->animationFramesTail = NULL;
            raytmxState->animationFramesLength = 0;
        }
    } /* strcmp(hoxmlContext->tag, "animation") == 0 */
    else if (strcmp(hoxmlContext->tag, "frame") == 0)
        raytmxState->animationFrame = NULL;
    else if (strcmp(hoxmlContext->tag, "layer") == 0) {
        if (raytmxState->tileLayer != NULL) {
            /* If there were 1+ <tile>s within this <layer> but this <layer> already has tiles (from a <data>?) */
            if (raytmxState->layerTilesRoot != NULL && raytmxState->tileLayer->tiles != NULL) {
                TraceLog(LOG_WARNING, "RAYTMX: layer \"%s\" has more than one source of tile data - the latter tiles "
                    "for this layer will be dropped", raytmxState->layer->name);
                /* Free the nodes and tiles therein */
                RaytmxTileLayerTileNode* iterator = raytmxState->layerTilesRoot;
                while (iterator != NULL) {
                    RaytmxTileLayerTileNode* parent = iterator;
                    iterator = iterator->next;
                    MemFree(parent);
                }
            } else {
                /* Allocate the array and zeroize every index as initialization */
                uint32_t* tiles = (uint32_t*)MemAllocZero(sizeof(uint32_t) * raytmxState->layerTilesLength);
                /* Copy the GID into the array and free the nodes while we're at it */
                RaytmxTileLayerTileNode* iterator = raytmxState->layerTilesRoot;
                for (uint32_t i = 0; i < raytmxState->layerTilesLength; i++) {
                    tiles[i] = iterator->gid;
                    RaytmxTileLayerTileNode* parent = iterator;
                    iterator = iterator->next;
                    MemFree(parent);
                }
                /* Add the tiles array to the tile layer */
                raytmxState->tileLayer->tiles = tiles;
                raytmxState->tileLayer->tilesLength = raytmxState->layerTilesLength;
            }
            /* Clean up the state object */
            raytmxState->layerTilesRoot = NULL;
            raytmxState->layerTilesTail = NULL;
            raytmxState->layerTilesLength = 0;
        }
        raytmxState->tileLayer = NULL;
        raytmxState->layer = NULL;
    } /* strcmp(hoxmlContext->tag, "layer") == 0 */
    else if (strcmp(hoxmlContext->tag, "tile") == 0) {
        if (raytmxState->tilesetTile != NULL) {
            /* Apply default values for the attribute(s) that aren't covered by a simple memset(x, 0, sizeof(x)) */
            if (raytmxState->tilesetTile->hasImage) {
                /* The 'width' and 'height' attributes default to the tile's image's width and height, respectively */
                if (raytmxState->tilesetTile->width == 0)
                    raytmxState->tilesetTile->width = raytmxState->tilesetTile->image.width;
                if (raytmxState->tilesetTile->height == 0)
                    raytmxState->tilesetTile->height = raytmxState->tilesetTile->image.height;
            }
            raytmxState->tilesetTile = NULL;
        }
    } /* strcmp(hoxmlContext->tag, "tile") == 0 */
    else if (strcmp(hoxmlContext->tag, "data") == 0) {
        if (raytmxState->image != NULL) {
            /* TODO (?): The TMX map format documentation says an <image> can contain a <data> element but doesn't */
            /* provide any more information than that. Tiled doesn't seem to have a feature for this either. */
        } else if (raytmxState->tileLayer != NULL && raytmxState->tileLayer->tiles != NULL) {
            TraceLog(LOG_WARNING, "RAYTMX: layer \"%s\" has more than one source of tile data - the latter tiles for "
                "this layer will be dropped", raytmxState->layer->name);
        } else if (raytmxState->tileLayer != NULL && raytmxState->tileLayer->encoding != NULL) {
            RaytmxTileLayerTileNode* tiles = NULL;
            if (strcmp(raytmxState->tileLayer->encoding, "base64") == 0) {
                /* The layer's data is a series of unsigned, 32-bit integers encoded as a Base64 string. But, XML */
                /* considers everything between <data> and </data> to be content meaning there is probably some */
                /* whitespace on both ends of the content we need to ignore. So, find the actual start and stop: */
                char *encodedStart = hoxmlContext->content, *encodedEnd;
                while (isspace(*encodedStart))
                    encodedStart++;
                encodedEnd = encodedStart + strlen(encodedStart) - 1;
                while (encodedEnd > encodedStart && isspace(*encodedEnd))
                    encodedEnd--;

                /* With the string of encoded Base64 data trimmed, decode it */
                int decodedLength;
                unsigned char* decoded = DecodeDataBase64((const unsigned char*)encodedStart, &decodedLength);
                if (decoded != NULL) {
                    if (raytmxState->tileLayer->compression == NULL) { /* If the Base64-encoded data is uncompressed */
                        /* Iterate through N bytes ('decodedLength') with every four bytes being a single GID */
                        /* resulting in N / 4 tiles */
                        uint32_t* iterator = (uint32_t*)decoded;
                        for (int i = 0; i < decodedLength / 4; i++) {
                            AddTileLayerTile(raytmxState, *iterator);
                            iterator += 1; /* Point to the next unsigned, 32-bit integer in the decoded data */
                        }
                    } else { /* If the Base-64encoded data is also compressed */
                        if (strcmp(raytmxState->tileLayer->compression, "gzip") == 0 ||
                                strcmp(raytmxState->tileLayer->compression, "zlib") == 0) {
                            unsigned char* postHeaderDecoded = NULL;
                            if (strcmp(raytmxState->tileLayer->compression, "gzip") == 0) {
                                /* The first two bytes of a GZIP header are expected to be a magic number, 0x1F8B, */
                                /* identifying the format and the third is expected to indicate the compression */
                                /* method where 0x08 is DEFLATE. */
                                /* If these values are found, decompression can continue */
                                if (decoded[0] == 0x1F && decoded[1] == 0x8B && decoded[2] == 0x08) {
                                    /* Skip past the GZIP header. The header is typically ten bytes. The bytes not */
                                    /* checked are unimportant things like a timestamp and OS ID. Additional optional */
                                    /* headers are possible but not used by Tiled so they are assumed to be missing. */
                                    postHeaderDecoded = decoded + 10;
                                } else { /* If the GZIP header doesn't match a decompressable one */
                                    TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" uses GZIP compression but the stream's "
                                        "header doesn't indicate DEFLATE compression", raytmxState->layer->name);
                                }
                            } else /* if (strcmp(raytmxState->tileLayer->compression, "zlib") == 0) */ {
                                /* The first byte of a ZLIB header is expected to be 0x78 where the 8 indicates the */
                                /* DEFLATE compression method and the 7 is "compression info" that indicates a 32K */
                                /* LZ77 window size and, in practice, cannot be anything else. */
                                /* If these values are found, decompression can continue */
                                if (decoded[0] == 0x78) {
                                    /* Skip past the ZLIB header. The header is two bytes. */
                                    postHeaderDecoded = decoded + 2;
                                } else { /* If the ZLIB header doesn't match a decompressable one */
                                    TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" uses ZLIB compression but the stream's "
                                        "header doesn't indicate DEFLATE compression", raytmxState->layer->name);
                                }
                            }

                            if (postHeaderDecoded != NULL) {
                                /* "zlib" and "gzip" both use the DEFLATE algorithm and raylib provides a */
                                /* decompression function when it's built with SUPPORT_COMPRESSION_API (default) */
                                int decompressedLength;
                                unsigned char* decompressed = DecompressData(postHeaderDecoded, decodedLength,
                                    &decompressedLength);
                                if (decompressed != NULL && decompressedLength > 0) {
                                    uint32_t* iterator = (uint32_t*)decompressed;
                                    for (int i = 0; i < decompressedLength / 4; i++) {
                                        AddTileLayerTile(raytmxState, *iterator);
                                        iterator += 1; /* Point to the next unsigned integer in the decompressed data */
                                    }
                                    MemFree(decompressed); /* Free the memory allocated by DecompressData() */
                                } else { /* raylib wasn't built with compression or allocation failed */
                                    TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" compressed with \"%s\" cannot be parsed "
                                        "because DEFLATE decompression failed - either raylib was not built with "
                                        "SUPPORT_COMPRESSION_API or memory allocation failed", raytmxState->layer->name,
                                        raytmxState->tileLayer->compression);
                                }
                            }
                        } else {
                            TraceLog(LOG_ERROR, "RAYTMX: Layer \"%s\" cannot be parsed because the compression method "
                                "\"%s\" is unsupported", raytmxState->layer->name, raytmxState->tileLayer->compression);
                        }
                    }
                    MemFree(decoded); /* Free the memory allocated by DecodeDataBase64() */
                } else {
                    TraceLog(LOG_ERROR, "RAYTMX: Unable to decode Base64 data for layer \"%s\"",
                        raytmxState->layer->name);
                }
            } /* strcmp(raytmxState->tileLayer->encoding, "base64") == 0 */
            else if (strcmp(raytmxState->tileLayer->encoding, "csv") == 0) {
                /* The Comma-Separated Value (CSV) list herein is a series of Global IDs (GIDs) of tiles in the form */
                /* "31,32,33" where 31, 32, and 33 are GIDs */
                char valueAsString[16]; /* Needs to fit all digits of a single value - should be more than enough */
                char* iterator = hoxmlContext->content;
                while (*iterator != '\0') { /* While not pointing to the end of the end of the string */
                    memset(valueAsString, '\0', 16); /* Reset the value-as-a-string buffer with all zeroes */
                    /* Copy each character into the buffer until either a comma or the terminator is reached */
                    for (int i = 0; *iterator != ',' && *iterator != '\0'; i++) {
                        memcpy(valueAsString + i, iterator, 1);
                        iterator++;
                    }
                    if (*iterator == ',') /* If iteration was paused by a comma */
                        iterator++; /* Advance one character so the comma isn't parsed with the value next iteration */
                    AddTileLayerTile(raytmxState, atoi(valueAsString)); /* Read the value as an integer GID */
                }
            } /* strcmp(raytmxState->tileLayer->encoding, "csv") == 0 */

            if (tiles != NULL) { /* If there was no error in parsing the data and there's a linked list of tiles */
                /* Allocate the array and assign 0 to every index to be safe */
                uint32_t* tiles = (uint32_t*)MemAlloc(sizeof(uint32_t) * raytmxState->layerTilesLength);
                memset(tiles, 0, sizeof(uint32_t) * raytmxState->layerTilesLength);
                /* Copy the GIDs into the array and free the nodes while we're at it */
                RaytmxTileLayerTileNode* layerTilesIterator = raytmxState->layerTilesRoot;
                for (uint32_t i = 0; i < raytmxState->layerTilesLength; i++) {
                    tiles[i] = layerTilesIterator->gid;
                    RaytmxTileLayerTileNode* layerTilesTemp = layerTilesIterator;
                    layerTilesIterator = layerTilesIterator->next;
                    MemFree(layerTilesTemp);
                }
                /* Add the tiles array to the element it applies to */
                raytmxState->tileLayer->tiles = tiles;
                raytmxState->tileLayer->tilesLength = raytmxState->layerTilesLength;
                /* Clean up the state object */
                raytmxState->layerTilesRoot = NULL;
                raytmxState->layerTilesTail = NULL;
                raytmxState->layerTilesLength = 0;
            }
        } /* raytmxState->tileLayer != NULL && raytmxState->tileLayer->encoding != NULL */
    } /* strcmp(hoxmlContext->tag, "data") == 0 */
    else if (strcmp(hoxmlContext->tag, "objectgroup") == 0) {
        if (raytmxState->objectGroup != NULL) {
            if (raytmxState->objectsRoot == NULL)
                return;
            /* Allocate the arrays and zeroize every index as initialization */
            TmxObject* objects = (TmxObject*)MemAllocZero(sizeof(TmxObject) * raytmxState->objectsLength);
            uint32_t* ySortedObjects = (uint32_t*)MemAllocZero(sizeof(uint32_t) * raytmxState->objectsLength);
            /* Create a contiguous array of TmxObjects, create a sorted linked list of indexes within that array of */
            /* TmxObjects (sorted by ascending y-coordinate), and free the object linked list */
            RaytmxObjectNode *objectsIterator = raytmxState->objectsRoot, *objectsTemp;
            RaytmxObjectSortingNode *sortingRoot = NULL, *sortingIterator, *sortingTemp, *newSortingNode;
            for (uint32_t i = 0; objectsIterator != NULL; i++) {
                objects[i] = objectsIterator->object;
                /* Add a new node into the sorted list */
                newSortingNode = (RaytmxObjectSortingNode*)MemAllocZero(sizeof(RaytmxObjectSortingNode));
                newSortingNode->y = objects[i].y;
                newSortingNode->index = i;
                if (sortingRoot == NULL) /* If this is the first node */
                    sortingRoot = newSortingNode;
                else if (sortingRoot->y >= newSortingNode->y) { /* If replacing the root node */
                    newSortingNode->next = sortingRoot;
                    sortingRoot = newSortingNode;
                } else { /* If inserting at some location after the root */
                    sortingIterator = sortingRoot;
                    while (sortingIterator->next != NULL && newSortingNode->y > sortingIterator->next->y)
                        sortingIterator = sortingIterator->next;
                    newSortingNode->next = sortingIterator->next;
                    sortingIterator->next = newSortingNode;
                }
                /* Free the object node */
                objectsTemp = objectsIterator;
                objectsIterator = objectsIterator->next;
                MemFree(objectsTemp);
            }
            /* Create a contiguous array from the sorted linked list such that index 0 of this array points to the */
            /* TmxObject (via its index in 'objects') with the lowest (visually, highest) y-coordinate */
            sortingIterator = sortingRoot;
            for (uint32_t i = 0; sortingIterator != NULL; i++) {
                ySortedObjects[i] = sortingIterator->index;
                sortingTemp = sortingIterator;
                sortingIterator = sortingIterator->next;
                MemFree(sortingTemp);
            }
            /* Add the objects and ySortedObjects array to the object layer */
            raytmxState->objectGroup->objects = objects;
            raytmxState->objectGroup->objectsLength = raytmxState->objectsLength;
            raytmxState->objectGroup->ySortedObjects = ySortedObjects;
            /* Clean up the state object */
            raytmxState->objectsRoot = NULL;
            raytmxState->objectsTail = NULL;
            raytmxState->objectsLength = 0;
        }
        raytmxState->objectGroup = NULL;
        raytmxState->layer = NULL;
    } else if (strcmp(hoxmlContext->tag, "object") == 0) {
        if (raytmxState->object != NULL) {
            /* Apply default values for the attribute(s) that aren't covered by a simple memset(x, 0, sizeof(x)) */
            if (raytmxState->object->name == NULL) { /* If this <object> didn't have a 'name' attribute */
                /* The default value for 'name' is "" (an empty string) */
                raytmxState->object->name = (char*)MemAlloc(1);
                raytmxState->object->name[0] = '\0';
            }
            if (raytmxState->object->typeString == NULL) { /* If this <object> didn't have a 'type' attribute */
                /* The default value for 'type' is "" (an empty string) */
                raytmxState->object->typeString = (char*)MemAlloc(1);
                raytmxState->object->typeString[0] = '\0';
            }

            if (raytmxState->object->templateString != NULL) {
                /* During editing, objects can be created from templates with pre-defined values and/or <properties>, */
                /* a concept similar to copying and pasting or instancing. These templates are saved in external TX */
                /* files that the 'template' attribute points to. Try to load it. */
                RaytmxCachedTemplateNode* cachedTemplate = LoadCachedTemplate(raytmxState,
                    raytmxState->object->templateString);
                if (cachedTemplate != NULL) {
                    RaytmxObjectTemplate objectTemplate = cachedTemplate->objectTemplate;
                    /* The template's <object> will contain some subset of values and/or <properties> that apply to */
                    /* the instanced <object> referencing it. For example, if the template's <object> has a 'name' */
                    /* attribute with the value "cactus" then the instanced <object> will also have a 'name' */
                    /* attribute. "cactus" is a default value that will be used if the instanced <object> does not */
                    /* define one of its own. The template's <object> needs to be checked for non-default values and */
                    /* <properties> and they need to be applied to the instanced <object> where none exist. */
                    if (objectTemplate.object.name != NULL && raytmxState->object->name == NULL) {
                        raytmxState->object->name =
                            (char*)MemAllocZero((unsigned int)strlen(objectTemplate.object.name) + 1);
                        StringCopy(raytmxState->object->name, objectTemplate.object.name);
                    }
                    if (objectTemplate.object.typeString != NULL && raytmxState->object->typeString != NULL) {
                        raytmxState->object->typeString =
                            (char*)MemAllocZero((unsigned int)strlen(objectTemplate.object.typeString) + 1);
                        StringCopy(raytmxState->object->typeString, objectTemplate.object.typeString);
                    }
                    if (objectTemplate.object.x != 0.0 && raytmxState->object->x == 0.0)
                        raytmxState->object->x = objectTemplate.object.x;
                    if (objectTemplate.object.y != 0.0 && raytmxState->object->y == 0.0)
                        raytmxState->object->y = objectTemplate.object.y;
                    if (objectTemplate.object.width != 0.0 && raytmxState->object->width == 0.0)
                        raytmxState->object->width = objectTemplate.object.width;
                    if (objectTemplate.object.height != 0.0 && raytmxState->object->height == 0.0)
                        raytmxState->object->height = objectTemplate.object.height;
                    if (objectTemplate.object.rotation != 0.0 && raytmxState->object->rotation == 0.0)
                        raytmxState->object->rotation = objectTemplate.object.rotation;
                    if (objectTemplate.object.gid != 0 && raytmxState->object->gid == 0)
                        raytmxState->object->gid = objectTemplate.object.gid;
                    if (!objectTemplate.object.visible && raytmxState->object->visible)
                        raytmxState->object->visible = false;
                    /* TMX documentation doesn't specify some particulars of templates, namely if it is possible for */
                    /* a template to be type-specific. In other words, can a template be a polygon? If so, are points */
                    /* merged or replaced? Based on Tiled's behavior, templates do not have type-specific values or, */
                    /* at least, they are ignored if they exist. However, <properties> are definitely merged. */
                    /* If the template's <object> has <properties> */
                    if (objectTemplate.object.properties != NULL) {
                        /* There are two cases here: the instanced <object> already has properties, or it doesn't */
                        if (raytmxState->object->properties == NULL) { /* If the instance doesn't have <properties> */
                            /* This is the easy case. Just copy the pointer to the existing array. */
                            raytmxState->object->properties = objectTemplate.object.properties;
                            raytmxState->object->propertiesLength = objectTemplate.object.propertiesLength;
                        } else {
                            /* The two <properties> need to be merged keeping in mind that they may, or probably, */
                            /* have overlapping entries. (When both have properties of the same name, the instanced */
                            /* <object>'s takes priority.) Create a linked list from which to create this new, merged */
                            /* properties array. */
                            RaytmxPropertyNode *propertiesRoot = NULL, *propertiesTail = NULL, *node;
                            uint32_t propertiesLength = 0;
                            /* Add the properties from the instanced <object> */
                            for (uint32_t i = 0; i < raytmxState->object->propertiesLength; i++) {
                                node = (RaytmxPropertyNode*)MemAllocZero(sizeof(RaytmxPropertyNode));
                                node->property = raytmxState->object->properties[i];
                                if (propertiesRoot == NULL)
                                    propertiesRoot = node;
                                else
                                    propertiesTail->next = node;
                                propertiesTail = node;
                                propertiesLength += 1;
                            }
                            /* Add the properties from the template's <object> if they do not already exist */
                            for (uint32_t i = 0; i < objectTemplate.object.propertiesLength; i++) {
                                RaytmxPropertyNode* propertiesIterator = propertiesRoot;
                                bool isNew = true;
                                while (propertiesIterator != NULL) {
                                    if (strcmp(objectTemplate.object.properties[i].name,
                                            propertiesIterator->property.name) == 0) {
                                        isNew = false;
                                        break;
                                    }
                                    propertiesIterator = propertiesIterator->next;
                                }
                                if (isNew) {
                                    node = (RaytmxPropertyNode*)MemAllocZero(sizeof(RaytmxPropertyNode));
                                    node->property = objectTemplate.object.properties[i];
                                    if (propertiesRoot == NULL)
                                        propertiesRoot = node;
                                    else
                                        propertiesTail->next = node;
                                    propertiesTail = node;
                                    propertiesLength += 1;
                                }
                            }
                            /* Free the separate array that was previously allocated */
                            MemFree(raytmxState->object->properties);
                            /* Allocate a new array to be populated with the merged properties */
                            raytmxState->object->properties =
                                (TmxProperty*)MemAllocZero(sizeof(TmxProperty) * propertiesLength);
                            raytmxState->object->propertiesLength = propertiesLength;
                            /* Copy the TmxProperty entires into the array and free the nodes while we're at it */
                            RaytmxPropertyNode* propertiesIterator = propertiesRoot;
                            for (uint32_t i = 0; propertiesIterator != NULL; i++) {
                                raytmxState->object->properties[i] = propertiesIterator->property;
                                RaytmxPropertyNode* propertiesTemp = propertiesIterator;
                                propertiesIterator = propertiesIterator->next;
                                MemFree(propertiesTemp);
                            }
                        }
                    }
                } else
                    TraceLog(LOG_WARNING, "RAYTMX: Unable to apply template to object ID %u", raytmxState->object->id);
            }

            /* Calculate the Axis-Aligned Bounding Box (AABB) of the object. This can vary by object type. Due to the */
            /* possibility of 'width' and 'height' being derived from a template, these must be calculated after a */
            /* template is applied to the object. */
            switch (raytmxState->object->type) {
            case OBJECT_TYPE_RECTANGLE:
            case OBJECT_TYPE_ELLIPSE:
            case OBJECT_TYPE_TEXT:
                raytmxState->object->aabb.x = (float)raytmxState->object->x;
                raytmxState->object->aabb.y = (float)raytmxState->object->y;
                raytmxState->object->aabb.width = (float)raytmxState->object->width;
                raytmxState->object->aabb.height = (float)raytmxState->object->height;
            break;
            case OBJECT_TYPE_POINT:
            {
                raytmxState->object->aabb.x = (float)raytmxState->object->x;
                raytmxState->object->aabb.y = (float)raytmxState->object->y;
                raytmxState->object->aabb.width = 0.0f;
                raytmxState->object->aabb.height = 0.0f;
            }
            break;
            case OBJECT_TYPE_POLYGON:
            case OBJECT_TYPE_POLYLINE:
            {
                float minX = INFINITY, maxX = -INFINITY, minY = INFINITY, maxY = -INFINITY;
                for (uint32_t i = 1; i < raytmxState->object->pointsLength; i++) {
                    Vector2 point = raytmxState->object->points[i];
                    if (point.x < minX)
                        minX = point.x;
                    if (point.x > maxX)
                        maxX = point.x;
                    if (point.y < minY)
                        minY = point.y;
                    if (point.y > maxY)
                        maxY = point.y;
                }
                /* Note: Poly(gon|line) objects' vertices are stored with relative positions whereas AABBs use */
                /* absolute values. The object's X and Y values must be added. */
                raytmxState->object->aabb.x = minX + (float)raytmxState->object->x;
                raytmxState->object->aabb.y = minY + (float)raytmxState->object->y;
                raytmxState->object->aabb.width = maxX - minX;
                raytmxState->object->aabb.height = maxY - minY;
            }
            break;
            case OBJECT_TYPE_TILE:
                /* The tile object type can have varying sizes, depending on the tile. While most will have the width */
                /* and height defined in the top-level <map>, a "collection of images" tileset will have tiles with */
                /* arbitrary dimensions. For that reason, an AABB cannot be calculated. */
            break;
            }
        }
        raytmxState->object = NULL;
    } /* strcmp(hoxmlContext->tag, "object") == 0 */
    else if (strcmp(hoxmlContext->tag, "text") == 0) {
        /* Apply default values for the attribute(s) that aren't covered by a simple memset(x, 0, sizeof(x)) */
        if (raytmxState->object != NULL && raytmxState->object->text != NULL) {
            TmxObject* object = raytmxState->object;
            TmxText* objectText = object->text;
            if (hoxmlContext->content != NULL) { /* If the element had content e.g. <text>Content here</text> */
                objectText->content = (char*)MemAllocZero((unsigned int)strlen(hoxmlContext->content) + 1);
                StringCopy(objectText->content, hoxmlContext->content);
            }

            if (objectText->fontFamily == NULL) { /* If this <text> didn't have a 'fontfamily' attribute */
                /* The default value for 'fontfamily' is "sans-serif" */
                objectText->fontFamily = (char*)MemAllocZero((unsigned int)strlen("sans-serif") + 1);
                StringCopy(objectText->fontFamily, "sans-serif");
            }

            if (objectText->content != NULL) { /* If there's text to be drawn */
                RaytmxTextLineNode *linesRoot = NULL, *linesTail = NULL;
                uint32_t linesLength = 0;

                /* There's some aligning and allocating to be done in order to draw the text. This isn't something */
                /* that should be done per-draw and, given the text is static, can be done ahead of time. */
                Font font = GetFontDefault();
                float spacing = 1.0f * (objectText->kerning ? 1.0f : 0.0f);

                unsigned int bufferLength = (unsigned int)strlen(objectText->content) + 1;
                /* This buffer will hold hold subsets of the content while iterating through it. The string in this */
                /* buffer may exceed the bounds. */
                char* testingBuffer = (char*)MemAllocZero(bufferLength);
                /* This one will hold the last known good string whose graphical text would fit within the bounds */
                char* validBuffer = (char*)MemAllocZero(bufferLength);
                /* This one will hold space-delimited subsets of the above */
                char* delimtedBuffer = (char*)MemAllocZero(bufferLength);
                bool isDelimited = false;

                char *start = objectText->content, *end = start, *validEnd = start, *delimitedEnd = start;
                /* While the 'end' iterator hasn't reached the end of the content AND further lines will fit within */
                /* the Y bounds of the object */
                while (*end != '\0' &&
                        object->y + (objectText->pixelSize * (linesLength + 1)) <= object->y + object->height) {
                    end++;
                    int length = (int)(end - start);
                    if (length <= 0) /* Quick error check for a case that is hopefully impossible in practice */
                        continue;

                    StringCopyN(testingBuffer, start, length);
                    /* Remove any trailing whitespace by iterating backwards and adding a terminator in place of any */
                    /* whitespace character */
                    for (char* i = testingBuffer + length - 1; i > testingBuffer && isspace(*i); i--)
                        *i = '\0';

                    /* Measure the dimensions of the now-widest text */
                    Vector2 textSize = MeasureTextEx(font, testingBuffer, (float)objectText->pixelSize, spacing);
                    /* If this text still fits within the width of the object (the bounds) */
                    if (textSize.x <= object->width) {
                        /* This string is still valid. Remember the string and where it ends. Iteration may return to */
                        /* last known good position. */
                        validEnd = end;
                        StringCopy(validBuffer, testingBuffer);
                        /* Note: Because the testing buffer is stripped of trailing whitespace, so too is this one */

                        /* If the end of the current string is whitespace but the last character wasn't */
                        if (isspace(*end) && !isspace(*(end - 1))) {
                            /* Taking "Hello, from TMX!" as an example, this would be triggered by the first space */
                            /* with "Hello," being the string to remember. In the event that the would-be text is too */
                            /* wide, meaning "Hello, from" is too long to fit width-wise, then this delimted "Hello," */
                            /* will be the line and iteration will return to "from." */
                            isDelimited = true;
                            delimitedEnd = end;
                            StringCopy(delimtedBuffer, validBuffer);
                        }
                    }

                    /* If it's time to create a visual line of text from the string either because: 1) the string has */
                    /* become too wide to fit in the bounds of the object, or 2) the end of the content was reached */
                    if (textSize.x > object->width || *end == '\0') {
                        char* sourceBuffer;
                        if (*end == '\0' || !isDelimited) {
                            /* The valid buffer is used in cases where the would-be text is simply the remainder of */
                            /* the content or the line does not have a clear separator (whitespace), like an extra */
                            /* long word that continues to the next line */
                            sourceBuffer = validBuffer;
                            start = validEnd;
                        } else {
                            /* The delimited buffer is used in cases where one or more words were identified using */
                            /* space between them as a delimiter */
                            sourceBuffer = delimtedBuffer;
                            start = delimitedEnd;
                            /* Skip over any whitespace at the delimiter */
                            while (isspace(*start) && *start != '\0')
                                start++;
                        }

                        /* The 'start' pointer is pointing to the first character of the next line. Point 'end' */
                        /* to the same place and continue from there so nothing is skipped. Note: 'end' will be */
                        /* incremented at the start of the next loop. */
                        end = start;

                        TmxTextLine line;
                        line.content = (char*)MemAllocZero((unsigned int)strlen(sourceBuffer) + 1);
                        StringCopy(line.content, sourceBuffer);
                        line.font = font;
                        line.spacing = spacing;
                        /* Note: The number of lines is not yet known but needs to be for Y positioning */

                        RaytmxTextLineNode* node = (RaytmxTextLineNode*)MemAllocZero(sizeof(RaytmxTextLineNode));
                        node->line = line;
                        if (linesRoot == NULL)
                            linesRoot = node;
                        else
                            linesTail->next = node;
                        linesTail = node;
                        linesLength += 1;

                        /* Reset variables */
                        memset(testingBuffer, '\0', bufferLength);
                        memset(validBuffer, '\0', bufferLength);
                        memset(delimtedBuffer, '\0', bufferLength);
                        isDelimited = false;

                        if (!objectText->wrap) { /* If word wrapping is disabled */
                            /* It's unclear why this would be done but, having hit the end of what can be displayed */
                            /* on a single line, no more text can be appended */
                            break;
                        }
                    } /* textSize.x > object->width || *end == '\0' */
                } /* *end != '\0' && */
                  /* object->y + (objectText->pixelSize * (linesLength + 1)) <= object->y + object->height */

                MemFree(testingBuffer);
                MemFree(validBuffer);
                MemFree(delimtedBuffer);

                if (linesRoot != NULL) {
                    /* Allocate the array and zero out every value as initialization */
                    TmxTextLine* lines = (TmxTextLine*)MemAllocZero(sizeof(TmxTextLine) * linesLength);
                    /* Copy the TmxTextLines into the array and free the nodes while we're at it */
                    RaytmxTextLineNode* iterator = linesRoot;
                    for (uint32_t i = 0; i < linesLength; i++) {
                        lines[i] = iterator->line;
                        Vector2 textSize = MeasureTextEx(font, lines[i].content,
                            (float)objectText->pixelSize, lines[i].spacing);

                        /* Horizontal alignment */
                        if (objectText->halign == HORIZONTAL_ALIGNMENT_RIGHT)
                            lines[i].position.x = (float)(object->x + object->width) - textSize.x;
                        else if (objectText->halign == HORIZONTAL_ALIGNMENT_CENTER)
                            lines[i].position.x = (float)(object->x + (object->width / 2.0)) - (textSize.x / 2.0f);
                        else if (objectText->halign == HORIZONTAL_ALIGNMENT_JUSTIFY) {
                            /* Horizontally justified text extends from the left bound to the right bound. Typically, */
                            /* this is done by adding space betweens words or, where there is only one word, adding */
                            /* space between letters. All additional space is distributed evenly. However, the method */
                            /* here is a hybrid: because control over spacing between words is coarse, spacing */
                            /* between letters is added on top of it. */
                            lines[i].position.x = (float)object->x; /* Place the text on the left bound */
                            /* Count the number of spaces between words */
                            uint32_t numSpaces = 0;
                            for (uint32_t j = 0; lines[i].content[j] != '\0'; j++) {
                                if (isspace(lines[i].content[j]))
                                    numSpaces++;
                            }
                            size_t length = strlen(lines[i].content);
                            if (numSpaces > 0) { /* If there's more than one word in the line */
                                /* Measure the dimensions of a single space using this line's configuration */
                                Vector2 originalTextSize = textSize;
                                textSize = MeasureTextEx(font, " ", (float)objectText->pixelSize, lines[i].spacing);
                                /* Calculate the number of new spaces to add between words, per existing space */
                                float idealNumAdditionalSpaces =
                                    ((float)object->width - originalTextSize.x) / textSize.x;
                                uint32_t numSpacesToAddPer =
                                    (uint32_t)floor((idealNumAdditionalSpaces - (float)numSpaces) / (float)numSpaces);
                                /* Create a new string with the additional space */
                                size_t justifiedLength = length + (numSpacesToAddPer * numSpaces);
                                char* justifiedContent = (char*)MemAllocZero((unsigned int)justifiedLength + 1);
                                uint32_t sourceIndex = 0, destinationIndex = 0;
                                while (lines[i].content[sourceIndex] != '\0') {
                                    justifiedContent[destinationIndex++] = lines[i].content[sourceIndex];
                                    /* If the current character is whitespace but the next one is not */
                                    if (sourceIndex < length && !isspace(lines[i].content[sourceIndex]) &&
                                            isspace(lines[i].content[sourceIndex + 1])) {
                                        /* Add 'numSpacesToAddPer' spaces to the justified string */
                                        for (uint32_t j = 0; j < numSpacesToAddPer; j++)
                                            justifiedContent[destinationIndex++] = ' ';
                                    }
                                    sourceIndex++;
                                }
                                /* Free the original content buffer and replace it with the justified one */
                                MemFree(lines[i].content);
                                lines[i].content = justifiedContent;
                                length = justifiedLength;
                            }
                            /* Calculate a spacing, between each letter, with which the drawn text will span the full */
                            /* width of the bounds */
                            textSize = MeasureTextEx(font, lines[i].content, (float)objectText->pixelSize, 0.0f);
                            lines[i].spacing = (float)((object->width - textSize.x) / (double)(length - 1));
                        } /* objectText->halign == HORIZONTAL_ALIGNMENT_JUSTIFY */
                        else /* if (objectText->halign == HORIZONTAL_ALIGNMENT_LEFT) */
                            lines[i].position.x = (float)object->x;

                        /* Vertical alignment */
                        if (objectText->valign == VERTICAL_ALIGNMENT_BOTTOM) {
                            lines[i].position.y = (float)(object->y + object->height) -
                                (float)(objectText->pixelSize * (i + 1));
                        } else if (objectText->valign == VERTICAL_ALIGNMENT_CENTER) {
                            float totalLineHeight = (float)(objectText->pixelSize * linesLength); /* All N lines */
                            lines[i].position.y = (float)object->y + /* Top of the <object>'s bounds */
                                ((float)object->height / 2.0f) + /* Vertical center of the <object>'s bounds */
                                (totalLineHeight / 2.0f) - /* Bottom of the centered lines' bounds */
                                (float)(objectText->pixelSize * (i + 1));
                        } else /* if (objectText->valign == VERTICAL_ALIGNMENT_TOP) */
                            lines[i].position.y = (float)object->y + (float)(objectText->pixelSize * i);

                        RaytmxTextLineNode* parent = iterator;
                        iterator = iterator->next;
                        MemFree(parent);
                    }
                    /* Add the lines array to the text object */
                    objectText->lines = lines;
                    objectText->linesLength = linesLength;
                }
            } /* objectText->content != NULL */
        }
    } /* strcmp(hoxmlContext->tag, "text") == 0 */
    else if (strcmp(hoxmlContext->tag, "imagelayer") == 0) {
        raytmxState->imageLayer = NULL;
        raytmxState->layer = NULL;
    } else if (strcmp(hoxmlContext->tag, "group") == 0) {
        /* <group>s can be nested so we must return to processing its parent, if it exists */
        if (raytmxState->groupNode != NULL)
            raytmxState->groupNode = raytmxState->groupNode->parent; /* Will be null when returning to the root map */
    }
}

void FreeStateLayers(RaytmxLayerNode* layers) {
    RaytmxLayerNode *layersIterator = layers, *layersTemp;
    while (layersIterator != NULL) {
        /* Group layers may have children, forming a tree-like structure. Free the children. */
        FreeStateLayers(layersIterator->childrenRoot);
        /* Iterate to the next node and free this one */
        layersTemp = layersIterator;
        layersIterator = layersIterator->next;
        MemFree(layersTemp);
    }
}

void FreeState(RaytmxState* raytmxState) {
    if (raytmxState == NULL)
        return;

    /* Clear the caches. These allow for quick lookups of previously-loaded textures and object templates. They */
    /* aren't needed once loading is complete. */
    RaytmxCachedTextureNode *cachedTextureIterator = raytmxState->texturesRoot, *cachedTextureTemp;
    while (cachedTextureIterator != NULL) {
        cachedTextureTemp = cachedTextureIterator;
        cachedTextureIterator = cachedTextureIterator->next;
        if (cachedTextureTemp->fileName != NULL) /* Just in case. Should always be set. */
            MemFree(cachedTextureTemp->fileName);
        MemFree(cachedTextureTemp);
    }
    raytmxState->texturesRoot = NULL;
    RaytmxCachedTemplateNode *cachedTemplateIterator = raytmxState->templatesRoot, *cachedTemplateTemp;
    while (cachedTemplateIterator != NULL) {
        cachedTemplateTemp = cachedTemplateIterator;
        cachedTemplateIterator = cachedTemplateIterator->next;
        FreeObject(cachedTemplateTemp->objectTemplate.object);
        if (cachedTemplateTemp->fileName != NULL) /* Just in case. Should always be set. */
            MemFree(cachedTemplateTemp->fileName);
        MemFree(cachedTemplateTemp);
    }
    raytmxState->templatesRoot = NULL;

    raytmxState->property = NULL;
    raytmxState->tileset = NULL;
    raytmxState->image = NULL;
    raytmxState->tilesetTile = NULL;
    raytmxState->animationFrame = NULL;
    /* raytmxState->wangSet = NULL; */ /* TODO:  Wang sets. Low priority. */
    /* raytmxState->wangColor = NULL; */ /* TODO:  Wang sets. Low priority. */
    raytmxState->layer = NULL;
    raytmxState->tileLayer = NULL;
    raytmxState->objectGroup = NULL;
    raytmxState->imageLayer = NULL;
    raytmxState->object = NULL;

    /* Free each property in the linked list of properties */
    RaytmxPropertyNode *propertiesIterator = raytmxState->propertiesRoot, *propertiesTemp;
    while (propertiesIterator != NULL) {
        propertiesTemp = propertiesIterator;
        propertiesIterator = propertiesIterator->next;
        MemFree(propertiesTemp);
    }
    /* Zeroize this linked list's properties */
    raytmxState->propertiesRoot = NULL;
    raytmxState->propertiesTail = NULL;
    raytmxState->propertiesLength = 0;

    /* Free each node in the linked list of tilesets */
    RaytmxTilesetNode *tilesetsIterator = raytmxState->tilesetsRoot, *tilesetsTemp;
    while (tilesetsIterator != NULL) {
        tilesetsTemp = tilesetsIterator;
        tilesetsIterator = tilesetsIterator->next;
        MemFree(tilesetsTemp);
    }
    /* Zeroize this linked list's properties */
    raytmxState->tilesetsRoot = NULL;
    raytmxState->tilesetsTail = NULL;
    raytmxState->tilesetsLength = 0;

    /* Free each node in the linked list of tileset tiles */
    RaytmxTilesetTileNode *tilesetTilesIterator = raytmxState->tilesetTilesRoot, *tilesetTilesTemp;
    while (tilesetTilesIterator != NULL) {
        tilesetTilesTemp = tilesetTilesIterator;
        tilesetTilesIterator = tilesetTilesIterator->next;
        MemFree(tilesetTilesTemp);
    }
    /* Zeroize this linked list's properties */
    raytmxState->tilesetTilesRoot = NULL;
    raytmxState->tilesetTilesTail = NULL;
    raytmxState->tilesetTilesLength = 0;

    /* Free each node in the linked list of animation frames */
    RaytmxAnimationFrameNode *animationFramesIterator = raytmxState->animationFramesRoot, *animationFramesTemp;
    while (animationFramesIterator != NULL) {
        animationFramesTemp = animationFramesIterator;
        animationFramesIterator = animationFramesIterator->next;
        MemFree(animationFramesTemp);
    }
    /* Zeroize this linked list's properties */
    raytmxState->animationFramesRoot = NULL;
    raytmxState->animationFramesTail = NULL;
    raytmxState->animationFramesLength = 0;

    /* Layers may be groups. The resulting collection isn't a link list as much as it is a tree. So freeing layers is */
    /* best done with a recursive approach. */
    FreeStateLayers(raytmxState->layersRoot);
    /* Zeroize this linked list's properties */
    raytmxState->layersRoot = NULL;
    raytmxState->layersTail = NULL;
    raytmxState->layersLength = 0;

    /* Free each node in the linked list of layer tiles */
    RaytmxTileLayerTileNode *layerTilesIterator = raytmxState->layerTilesRoot, *layerTilesTemp;
    while (layerTilesIterator != NULL) {
        layerTilesTemp = layerTilesIterator;
        layerTilesIterator = layerTilesIterator->next;
        MemFree(layerTilesTemp);
    }
    /* Zeroize this linked list's properties */
    raytmxState->layerTilesRoot = NULL;
    raytmxState->layerTilesTail = NULL;
    raytmxState->layerTilesLength = 0;

    /* Free each node in the linked list of objects */
    RaytmxObjectNode *objectsIterator = raytmxState->objectsRoot, *objectsTemp;
    while (objectsIterator != NULL) {
        objectsTemp = objectsIterator;
        objectsIterator = objectsIterator->next;
        MemFree(objectsTemp);
    }
    /* Zeroize this linked list's properties */
    raytmxState->objectsRoot = NULL;
    raytmxState->objectsTail = NULL;
    raytmxState->objectsLength = 0;
}

void inline FreeString(char* str) {
    if (str != NULL)
        MemFree(str);
}

void FreeTileset(TmxTileset tileset) {
    FreeString(tileset.source);
    FreeString(tileset.name);
    FreeString(tileset.classString);
    if (tileset.hasImage) {
        FreeString(tileset.image.source);
        UnloadTexture(tileset.image.texture);
    }
    if (tileset.properties != NULL) {
        for (uint32_t i = 0; i < tileset.propertiesLength; i++)
            FreeProperty(tileset.properties[i]);
        MemFree(tileset.properties);
    }
    for (uint32_t i = 0; i < tileset.tilesLength; i++) {
        TmxTilesetTile tile = tileset.tiles[i];
        if (tile.hasImage) {
            FreeString(tile.image.source);
            UnloadTexture(tile.image.texture);
            if (tile.properties != NULL) {
                for (uint32_t j = 0; j < tile.propertiesLength; j++)
                    FreeProperty(tile.properties[j]);
                MemFree(tile.properties);
            }
        }
        if (tile.hasAnimation && tile.animation.frames != NULL)
            MemFree(tile.animation.frames);
    }
}

void FreeProperty(TmxProperty property) {
    FreeString(property.name);
    FreeString(property.stringValue);
}

void FreeLayer(TmxLayer layer) {
    FreeString(layer.name);
    FreeString(layer.classString);
    if (layer.properties != NULL) {
        for (uint32_t i = 0; i < layer.propertiesLength; i++)
            FreeProperty(layer.properties[i]);
        MemFree(layer.properties);
    }
    switch (layer.type) {
    case LAYER_TYPE_TILE_LAYER:
        FreeString(layer.exact.tileLayer.encoding);
        FreeString(layer.exact.tileLayer.compression);
        MemFree(layer.exact.tileLayer.tiles);
    break;
    case LAYER_TYPE_OBJECT_GROUP:
        for (uint32_t j = 0; j < layer.exact.objectGroup.objectsLength; j++)
            FreeObject(layer.exact.objectGroup.objects[j]);
        MemFree(layer.exact.objectGroup.objects);
    break;
    case LAYER_TYPE_IMAGE_LAYER:
        if (layer.exact.imageLayer.hasImage)
            UnloadTexture(layer.exact.imageLayer.image.texture);
    break;
    case LAYER_TYPE_GROUP: break; /* Nothing to do for this case but compilers like to complain */
    }
    /* <group> layers are expected to have child layers, or child <group>s, so recursively free them too */
    for (uint32_t i = 0; i < layer.layersLength; i++)
        FreeLayer(layer.layers[i]);
}

void FreeObject(TmxObject object) {
    FreeString(object.name);
    FreeString(object.typeString);
    FreeString(object.templateString);
    if (object.points != NULL)
        MemFree(object.points);
    if (object.text != NULL) {
        if (object.text->lines != NULL) {
            for (uint32_t j = 0; j < object.text->linesLength; j++)
                FreeString(object.text->lines[j].content);
            MemFree(object.text->lines);
        } /* object.text->lines != NULL */
        MemFree(object.text);
    } /* object.text != NULL */
}

#define SIGN(x) (x < 0 ? -1 : +1)

/**
 * Helper function that keeps an integer within the given range.
 * Note: A function named Clamp() exists in raylib but uses floats, hence Clampi().
 * 
 * @param value The preferred value that will be used as long as it is between the minimum and maximum values.
 * @param minimum The smallest acceptable value, inclusive.
 * @param maximum The largest acceptable value, inclusive.
 * @return The preferred value if within the range or the nearest acceptable value.
 */
int Clampi(int value, int minimum, int maximum) {
    if (value < minimum)
        return minimum;
    else if (value > maximum)
        return maximum;
    return value;
}

/**
 * Scary-looking helper function that does something kind of simple: iterates through the tiles of the given tile layer
 * overlapping with the given screen rectangle, one tile per call. This function returns true while iteration is still
 * ongoing and false when done. This allows the function to be used e.g. "while (IterateTileLayer( { ...} ))". Details
 * of the current tile are returned to the caller with output parameters. Iteration is done row-by-row.
 *
 * @param map A loaded map model containing the given tile layer.
 * @param layer The tile layer within the given map whose tiles will be iterated.
 * @param screenRect A rectangle representing the screen. This could also be considered a search area.
 * @param rawGid Optional output. The Global ID (GID) with possible flip flags. Pass NULL if not wanted.
 * @param tile Optional output. Metadata of the current tile. Pass NULL if not wanted.
 * @param tileRect Optional output. The destination rectangle, in pixels, of the current tile. Pass NULL if not wanted.
 * @return True if the next tile is being provided via the output parameters, or false if iteration is done.
 */
bool IterateTileLayer(const TmxMap* map, const TmxTileLayer* layer, Rectangle screenRect, uint32_t* rawGid,
        TmxTile* tile, Rectangle* tileRect) {
    /* Static variables whose values will persist between calls. These are needed to initialize and iterate. */
    static const TmxTileLayer* currentLayer = NULL; /* Tile layer being iterated */
    static int fromX = 0; /* Initial X position, tile not pixel, that row-by-row iteration begins at */
    static int fromY = 0; /* Initial Y position, tile not pixel, that row-by-row iteration begins at */
    static int toX = 0; /* Final X position, tile not pixel, that iteration ends at */
    static int toY = 0; /* Final Y position, tile not pixel, that iteration ends at */
    static int currentX = 0; /* Current tile X position (column) within the iteration */
    static int currentY = 0; /* Current tile Y position (row) within the iteration */

    if (map == NULL || map->width == 0 || map->height == 0 || map->tileWidth == 0 || map->tileHeight == 0 ||
            layer == NULL || layer->tilesLength == 0)
        return false;

    if (currentLayer != layer) { /* If the layer has changed (i.e. iteration should initialize) */
        currentLayer = layer; /* Remember this layer */
        switch (map->renderOrder) {
        case RENDER_ORDER_RIGHT_DOWN:
            /* Start at the top-left, iterate right, then iterate down, ending at the bottom-right. */
            /* In other words, this is the order in which English is read. */
            fromX = (int)screenRect.x / (int)map->tileWidth;
            fromY = (int)screenRect.y / (int)map->tileHeight;
            toX = (int)(screenRect.x + screenRect.width) / (int)map->tileWidth;
            toY = (int)(screenRect.y + screenRect.height) / (int)map->tileHeight;
        break;
        case RENDER_ORDER_RIGHT_UP:
            /* Start at the bottom-left, iterate right, then iterate up, ending at the top-right */
            fromX = (int)screenRect.x / (int)map->tileWidth;
            fromY = (int)(screenRect.y + screenRect.height) / (int)map->tileHeight;
            toX = (int)(screenRect.x + screenRect.width) / (int)map->tileWidth;
            toY = (int)screenRect.y / (int)map->tileHeight;
        break;
        case RENDER_ORDER_LEFT_DOWN:
            /* Start at the top-right, iterate left, then iterate down, ending at the bottom-left */
            fromX = (int)(screenRect.x + screenRect.width) / (int)map->tileWidth;
            fromY = (int)screenRect.y / (int)map->tileHeight;
            toX = (int)screenRect.x / (int)map->tileWidth;
            toY = (int)(screenRect.y + screenRect.height) / (int)map->tileHeight;
        break;
        case RENDER_ORDER_LEFT_UP:
            /* Start at the bottom-right, iterate left, then iterate up, ending at the top-left */
            fromX = (int)(screenRect.x + screenRect.width) / (int)map->tileWidth;
            fromY = (int)(screenRect.y + screenRect.height) / (int)map->tileHeight;
            toX = (int)screenRect.x / (int)map->tileWidth;
            toY = (int)screenRect.y / (int)map->tileHeight;
        break;
        } /* switch (map->renderOrder) */
        /* Restrain the the tile positions to those within the map in case of rounding mistakes */
        fromX = Clampi(fromX, 0, (int)map->width - 1);
        fromY = Clampi(fromY, 0, (int)map->height - 1);
        toX = Clampi(toX, 0, (int)map->width - 1);
        toY = Clampi(toY, 0, (int)map->height - 1);
        /* Begin iteration from both "from" tile positions */
        currentX = fromX;
        currentY = fromY;
    } else if (currentX == toX) { /* If the end of the current row was reached */
        /* Rendering is done row-by-row. This row is done so move to the next one. */
        currentX = fromX;
        currentY += SIGN(toY - fromY); /* Either +1 or -1 */
    } else { /* If still iterating through the current row */
        /* Move to the right or left by one tile */
        currentX += SIGN(toX - fromX); /* Either +1 or -1 */
    }

    /* If iteration has gone beyond the final row */
    if ((toY - fromY > 0 && currentY > toY) || (toY - fromY < 0 && currentY < toY)) {
        /* This is the termination condition. Zero all values and return false so the caller exits its loop. */
        currentLayer = NULL;
        fromX = fromY = toX = toY = currentX = currentY = 0;
        return false;
    }

    /* Calculate the index in the tile layer from knowing the tile's X and Y position (in tiles, not pixels) */
    int index = (currentY * (int)map->width) + currentX;
    if (index < 0 || index >= (int)layer->tilesLength) { /* Bounds check */
        currentLayer = NULL;
        fromX = fromY = toX = toY = currentX = currentY = 0;
        return false;
    }

    /* Get the raw Global ID (GID) of the tile at this position from the layer's list of tiles. This list's order */
    /* matches the map's render order. */
    uint32_t localRawGid = layer->tiles[index];
    if (rawGid != NULL)
        *rawGid = localRawGid; /* Assign the value to he output parameter */
    if (tile != NULL) {
        /* The raw GID may have bit flags on it. They need to be removed in order to get the actual GID value.*/
        uint32_t gid = GetGid(localRawGid, NULL, NULL, NULL, NULL);
        /* Get the tile's metadata from knowing its GID */
        *tile = map->gidsToTiles[gid];
    }
    if (tileRect != NULL) {
        /* Calculate the tile's destination rectangle, in pixels */
        *tileRect = (Rectangle) {
            .x = (float)((uint32_t)currentX * map->tileWidth),
            .y = (float)((uint32_t)currentY * map->tileHeight),
            .width = (float)map->tileWidth,
            .height = (float)map->tileHeight
        };
    }

    return true;
}

void DrawTMXTileLayer(const TmxMap* map, Rectangle screenRect, TmxLayer layer, int posX, int posY, Color tint) {
    if (map == NULL || layer.type != LAYER_TYPE_TILE_LAYER || layer.exact.tileLayer.tilesLength == 0)
        return;

    /* Iterate through each tile that the screen rectangle overlaps with */
    uint32_t rawGid;
    Rectangle tileRect;
    while (IterateTileLayer(/* map: */ map, /* layer: */ &layer.exact.tileLayer, /* screenRect: */ screenRect,
            /* rawGid: */ &rawGid, /* tile: */ NULL, /* tileRect: */ &tileRect)) {
        DrawTMXLayerTile(/* map: */ map, /* screenRect: */ screenRect, /* rawGid: */ rawGid,
                         /* posX: */ posX + (int)tileRect.x, /* posY: */ posY + (int)tileRect.y, /* tint: */ tint);
    }
}

void DrawTextureTile(Texture2D texture, Rectangle source, Rectangle dest, bool flipX, bool flipY, bool flipDiag,
        Color tint) {
    if (texture.id == 0) /* If the texture is invalid */
        return;

    float textureWidth = (float)texture.width;
    float textureHeight = (float)texture.height;

    /* Determine the area within the texture to be drawn */
    /* Note: The coordinates here are in the [0.0, 1.0] range where (0.0, 0.0) is the bottom-left corner of the */
    /* texture, (1.0, 0.0) is the bottom-right, and (1.0, 1.0) is the top-right. In other words, the coordinates are */
    /* a ratio of the dimensions making (0.5, 0.5) the center of the texture regardless of its aspect ratio. */
    Vector2 sourceTopLeft, sourceTopRight, sourceBottomLeft, sourceBottomRight;
    sourceTopLeft.x = source.x / textureWidth;
    sourceTopLeft.y = source.y / textureHeight;
    sourceTopRight.x = (source.x + source.width) / textureWidth;
    sourceTopRight.y = source.y / textureHeight;
    sourceBottomLeft.x = source.x / textureWidth;
    sourceBottomLeft.y = (source.y + source.height) / textureHeight;
    sourceBottomRight.x = (source.x + source.width) / textureWidth;
    sourceBottomRight.y = (source.y + source.height) / textureHeight;
    if (flipDiag) { /* If the tile uses a diagonal flip */
        /* "The diagonal flip should flip the bottom left and top right corners of the tile..." */
        Vector2 temp = sourceBottomLeft;
        sourceBottomLeft = sourceTopRight;
        sourceTopRight = temp;
    }

    /* Determine the area on the screen to be drawn to */
    Vector2 destTopLeft, destTopRight, destBottomLeft, destBottomRight;
    destTopLeft.x = dest.x;
    destTopLeft.y = dest.y;
    destTopRight.x = dest.x + dest.width;
    destTopRight.y = dest.y;
    destBottomLeft.x = dest.x;
    destBottomLeft.y = dest.y + dest.height;
    destBottomRight.x = dest.x + dest.width;
    destBottomRight.y = dest.y + dest.height;

    rlSetTexture(texture.id);
    rlBegin(RL_QUADS);
    {
        rlColor4ub(tint.r, tint.g, tint.b, tint.a);
        rlNormal3f(0.0f, 0.0f, 1.0f); /* Normal vector pointing towards viewer */

        /* Top-left corner of the quad */
        if (flipX && !flipY)
            rlTexCoord2f(sourceTopRight.x, sourceTopRight.y);
        else if (flipY && !flipX)
            rlTexCoord2f(sourceBottomLeft.x, sourceBottomLeft.y);
        else
            rlTexCoord2f(sourceTopLeft.x, sourceTopLeft.y);
        if (flipX && flipY)
            rlVertex2f(destBottomRight.x, destBottomRight.y);
        else
            rlVertex2f(destTopLeft.x, destTopLeft.y);

        /* Bottom-left corner of the quad */
        if (flipX && !flipY)
            rlTexCoord2f(sourceBottomRight.x, sourceBottomRight.y);
        else if (flipY && !flipX)
            rlTexCoord2f(sourceTopLeft.x, sourceTopLeft.y);
        else
            rlTexCoord2f(sourceBottomLeft.x, sourceBottomLeft.y);
        if (flipX && flipY)
            rlVertex2f(destTopRight.x, destTopRight.y);
        else
            rlVertex2f(destBottomLeft.x, destBottomLeft.y);

        /* Bottom-right corner of the quad */
        if (flipX && !flipY)
            rlTexCoord2f(sourceBottomLeft.x, sourceBottomLeft.y);
        else if (flipY && !flipX)
            rlTexCoord2f(sourceTopRight.x, sourceTopRight.y);
        else
            rlTexCoord2f(sourceBottomRight.x, sourceBottomRight.y);
        if (flipX && flipY)
            rlVertex2f(destTopLeft.x, destTopLeft.y);
        else
            rlVertex2f(destBottomRight.x, destBottomRight.y);

        /* Top-right corner of the quad */
        if (flipX && !flipY)
            rlTexCoord2f(sourceTopLeft.x, sourceTopLeft.y);
        else if (flipY && !flipX)
            rlTexCoord2f(sourceBottomRight.x, sourceBottomRight.y);
        else
            rlTexCoord2f(sourceTopRight.x, sourceTopRight.y);
        if (flipX && flipY)
            rlVertex2f(destBottomLeft.x, destBottomLeft.y);
        else
            rlVertex2f(destTopRight.x, destTopRight.y);
    }
    rlEnd();
    rlSetTexture(0);
}

void DrawTMXLayerTile(const TmxMap* map, Rectangle screenRect, uint32_t rawGid, int posX, int posY, Color tint) {
    if (map == NULL || tint.a == 0)
        return;

    bool isFlippedHorizontally, isFlippedVertically, isFlippedDiagonally, isRotatedHexagonal120;
    /* Tile Global IDs (GIDs) can have several bit flags that indicate transforms. This function is used to both get */
    /* those possible transform flags as well as the actual GID value without those bit flags. */
    uint32_t gid = GetGid(rawGid, &isFlippedHorizontally, &isFlippedVertically, &isFlippedDiagonally,
        &isRotatedHexagonal120);
    if (gid >= map->gidsToTilesLength) /* If the GID is outside the range of known GIDs */
        return; /* Do not attempt to draw this time */
    /* With the GID, grab the relevant tile information (texture, animation, etc.) from the global mapping */
    TmxTile tile = map->gidsToTiles[gid];
    if (tile.gid == 0) /* If the GID is not known to exist in any tilesets within the map */
        return; /* Do not attempt to draw this tile */

    if (tile.hasAnimation) {
        /* Animations aren't really tiles. Instead, they contain frames that identify a tile to draw for the duration */
        /* of that frame. */
        /* The 'gid' of an animation tile is assigned with the first GID of the tileset and the frames have local IDs */
        /* within that tileset. The GID of the frame, then, can be calculated by adding them together. */
        gid = tile.gid + tile.animation.frames[tile.frameIndex].id;
        /* Copy any flip flags that may be present in the layer data. */
        gid |= rawGid & (FLIP_FLAG_HORIZONTAL | FLIP_FLAG_VERTICAL | FLIP_FLAG_DIAGONAL | FLIP_FLAG_ROTATE_120);
        /* Draw the tile using the calculated GID of the frame, along with the possible flags. */
        DrawTMXLayerTile(map, screenRect, gid, posX, posY, tint);
    } else {
        /* Determine where the tile will be drawn. raylib's coordinates consider [x, y] to be the top-left corner of */
        /* the rectangle being drawn. The TMX documentation complicates things a bit saying "Larger tiles will extend */
        /* at the top and right (anchored to the bottom left)" meaning that TMX considers [x, y] to be the */
        /* bottom-left corner. The simplest way to reconcile the Y coordinate differences is to substract the */
        /* texture's height at Y + 1. This way, tiles larger than the map's tile height values will be drawn further */
        /* up (negative Y direction). */
        Rectangle destRect;
        destRect.x = posX + tile.offset.x;
        destRect.y = posY + tile.offset.y + map->tileHeight - tile.sourceRect.height;
        destRect.width = tile.sourceRect.width;
        destRect.height = tile.sourceRect.height;

        /* If the screen and destination rectangles are overlapping to any degree (i.e. if the tile is visible) */
        if (CheckCollisionRecs(screenRect, destRect)) {
            DrawTextureTile(/* texture: */ tile.texture, /* source: */ tile.sourceRect, /* dest: */ destRect,
                /* flipX: */ isFlippedHorizontally, /* flipY: */ isFlippedVertically,
                /* flipDiag: */ isFlippedDiagonally, /* tint: */ tint);
        }
    }
}

void DrawTMXObjectTile(const TmxMap* map, Rectangle screenRect, uint32_t rawGid, int posX, int posY, float width,
        float height, Color tint) {
    if (map == NULL || width <= 0 || height <= 0 || tint.a == 0)
        return;

    bool isFlippedHorizontally, isFlippedVertically, isFlippedDiagonally, isRotatedHexagonal120;
    /* Tile Global IDs (GIDs) can have several bit flags that indicate transforms. This function is used to both get */
    /* those possible transform flags as well as the actual GID value without those bit flags. */
    uint32_t gid = GetGid(rawGid, &isFlippedHorizontally, &isFlippedVertically, &isFlippedDiagonally,
        &isRotatedHexagonal120);
    if (gid >= map->gidsToTilesLength) /* If the GID is outside the range of known GIDs */
        return; /* Do not attempt to draw this time */
    /* With the GID, grab the relevant tile information (texture, animation, etc.) from the global mapping */
    TmxTile tile = map->gidsToTiles[gid];
    if (tile.gid == 0) /* If the GID is not known to exist in any tilesets within the map */
        return; /* Do not attempt to draw this time */

    if (tile.hasAnimation) {
        /* Animations aren't really tiles. Instead, they contain frames that identify a tile to draw for the duration */
        /* of that frame. That current tile should be drawn. */
        DrawTMXLayerTile(map, screenRect, tile.gid + tile.animation.frames[tile.frameIndex].id, posX, posY, tint);
    } else {
        /* Determine the area in which to draw, and potentially stretch, the texture. This area matches that of the */
        /* <object>, not the tile size. This also means that the Y coordinate needs consideration because raylib */
        /* considers [x, y] to the be top-left corner of any area but the TMX format considers it the bottom-left. */
        Rectangle destRect;
        destRect.x = posX + tile.offset.x;
        destRect.y = posY + tile.offset.y - height;
        destRect.width = width;
        destRect.height = height;

        /* If the screen and destination rectangles are overlapping to any degree (i.e. if the tile is visible) */
        if (CheckCollisionRecs(screenRect, destRect)) {
            DrawTextureTile(/* texture: */ tile.texture, /* source: */ tile.sourceRect, /* dest: */ destRect,
                /* flipX: */ isFlippedHorizontally, /* flipY: */ isFlippedVertically,
                /* flipDiag: */ isFlippedDiagonally, /* tint: */ tint);
        }
    }
}

void DrawTMXObjectGroup(const TmxMap* map, Rectangle screenRect, TmxLayer layer, int posX, int posY, Color tint) {
    if (map == NULL || layer.type != LAYER_TYPE_OBJECT_GROUP || tint.a == 0)
        return;

    TmxObjectGroup objectGroup = layer.exact.objectGroup;
    for (int32_t i = 0; i < (int32_t)objectGroup.objectsLength; i++) {
        /* Select the object to draw based on the <objectgroup>'s draw order */
        TmxObject object;
        if (objectGroup.drawOrder == OBJECT_GROUP_DRAW_ORDER_INDEX)
            object = objectGroup.objects[i];
        else /* if (objectGroup.drawOrder == OBJECT_GROUP_DRAW_ORDER_TOP_DOWN) */
            object = objectGroup.objects[objectGroup.ySortedObjects[i]];

        if (object.type == OBJECT_TYPE_TILE) { /* If the object is a tile with an abitrary GID and dimensions */
            /* Note: This draw method handles occlusion culling so it doesn't need to be done here */
            DrawTMXObjectTile(map, /* screenRect: */ screenRect, /* rawGid: */ object.gid,
                /* posX: */ posX + (int)object.x, /* posY: */ posY + (int)object.y, /* width: */ (float)object.width,
                /* height: */ (float)object.height, /* color: */ tint);
        } else { /* If the object is any type other than a tile */
            Rectangle offsetAabb = object.aabb;
            offsetAabb.x += posX;
            offsetAabb.y += posY;
            /* If the screen rectangle and the polygon's AABB are overlapping to any degree (i.e. it is visible) */
            if (CheckCollisionRecs(screenRect, offsetAabb)) {
                switch (object.type) {
                case OBJECT_TYPE_RECTANGLE:
                    DrawRectangle(/* posX: */ posX + (int)object.x, /* posY: */ posY + (int)object.y,
                        /* width: */ (int)object.width, /* height: */ (int)object.height,
                        /* color: */ objectGroup.color);
                break;
                case OBJECT_TYPE_ELLIPSE:
                {
                    /* The width and height of the object are used here as the semi major and minor axes */
                    float halfWidth = (float)object.width / 2.0f, halfHeight = (float)object.height / 2.0f;
                    DrawEllipse(/* centerX: */ posX + (int)(object.x + halfWidth),
                        /* centerY: */ posY + (int)(object.y + halfHeight), /* radiusH: */ halfWidth,
                        /* radiusV: */ halfHeight, /* color: */ objectGroup.color);
                }
                break;
                case OBJECT_TYPE_POINT:
                DrawCircle(/* centerX: */ (int)object.x, /* centerY: */ (int)object.y,
                    /* radius: */ (float)map->tileWidth / 4.0f, /* color: */ objectGroup.color);
                break;
                case OBJECT_TYPE_POLYGON:
                case OBJECT_TYPE_POLYLINE:
                    /* Copy the 'points' array to the 'drawPoints' array and apply the drawing position, an offset */
                    /* applied by the layer and/or draw call. The 'drawPoints' array was allocated at the same time */
                    /* as 'points' with the same size. This improves draw call times by reducing memory allocations. */
                    memcpy(object.drawPoints, object.points, sizeof(Vector2) * object.pointsLength);
                    for (uint32_t i = 0; i < object.pointsLength; i++) {
                        /* Polygons' and polyglines' vertices are stored with relative positions. To get the absolute */
                        /* position needed for drawing, just add the object's position and offset. */
                        object.drawPoints[i].x += (float)object.x + (float)posX;
                        object.drawPoints[i].y += (float)object.y + (float)posY;
                    }
                    /* Use the offset points to draw the poly(gon|line) */
                    if (object.type == OBJECT_TYPE_POLYGON) {
                        /* Note: Polygons' first elements are their centroids. DrawTriangleFan() requires this. */
                        /* And, the last element in 'drawPoints' is a duplicate of the first, non-centroid point. */
                        DrawTriangleFan(/* points: */ object.drawPoints, /* pointCount: */ object.pointsLength,
                            /* color: */ objectGroup.color);
                    } else /* if (object.type == OBJECT_TYPE_POLYLINE) */ {
                        /* Note: The last element in 'drawPoints' is a duplicate of the first point */
                        for (uint32_t i = 1; i < object.pointsLength; i++) {
                            DrawLineEx(/* startPos: */ object.drawPoints[i - 1], /* endPos: */ object.drawPoints[i],
                                /* thick: */ TMX_LINE_THICKNESS, /* color: */ objectGroup.color);
                        }
                    }
                break;
                case OBJECT_TYPE_TEXT:
                    for (uint32_t i = 0; i < object.text->linesLength; i++) {
                        Vector2 position = object.text->lines[i].position;
                        position.x += posX;
                        position.y += posY;
                        DrawTextEx(/* font: */ object.text->lines[i].font, /* text: */ object.text->lines[i].content,
                            /* position: */ position, /* fontSize: */ (float)object.text->pixelSize,
                            /* spacing: */ object.text->lines[i].spacing, /* tint: */ object.text->color);
                    }
                break;
                case OBJECT_TYPE_TILE:
                    /* Object tiles are handled in the 'if' case of this 'else' block because the use of an AABB for */
                    /* occlusion culling is not reliable for them */
                break;
                }
            }
        }
    }
}

void DrawTMXImageLayer(const TmxMap* map, Rectangle screenRect, TmxLayer layer, int posX, int posY, Color tint) {
    if (map == NULL || layer.type != LAYER_TYPE_IMAGE_LAYER || !layer.exact.imageLayer.hasImage ||
            layer.exact.imageLayer.image.width == 0 || layer.exact.imageLayer.image.height == 0 || tint.a == 0)
        return;

    TmxImageLayer imageLayer = layer.exact.imageLayer;
    /* Determine where the image of this image layer would be drawn, assuming no repetitions */
    Rectangle imageRect;
    imageRect.x = (float)posX;
    imageRect.y = (float)posY;
    imageRect.width = (float)imageLayer.image.width;
    imageRect.height = (float)imageLayer.image.height;

    if (!imageLayer.repeatX && !imageLayer.repeatY && CheckCollisionRecs(screenRect, imageRect)) /* If visible */
        DrawTexture(/* texture: */ imageLayer.image.texture, /* posX: */ posX, /* posY: */ posY, /* tint: */ tint);
    else if (imageLayer.repeatX || imageLayer.repeatY) { /* If the image might be drawn across a whole axis, or both */
        /* Use integer division to determine the X and Y positions at which a the image would appear if it were */
        /* repeated across the whole axis (i.e. if "Repeat X" and/or "Repeat Y" are enabled) */
        int coefficientX = (int)(screenRect.x - imageRect.x) / (int)imageRect.width;
        int coefficientY = (int)(screenRect.y - imageRect.y) / (int)imageRect.height;
        float x0 = imageRect.x + (imageRect.width * (float)coefficientX); /* Initial X position */
        float y0 = imageRect.y + (imageRect.height * (float)coefficientY); /* Initial Y position */

        if (imageLayer.repeatX) /* If repeating along the X axis */
            imageRect.x = x0; /* Move the image's representative rectangle to that X */
        if (imageLayer.repeatY)
            imageRect.y = y0;
        if (CheckCollisionRecs(screenRect, imageRect)) { /* If the repeating image would be visible in the screen */
            /* Take a step back on each axis that the image repeats on. This ensures we don't leave any empty space */
            /* along the left and/or top edge of the screen. */
            if (imageLayer.repeatX)
                x0 -= imageRect.width;
            if (imageLayer.repeatY)
                y0 -= imageRect.height;

            /* Create some unchanging objects that the draw function will need */
            Rectangle sourceRect; /* Region within the texture to be drawn. We'll use the whole texture. */
            sourceRect.x = 0.0f;
            sourceRect.y = 0.0f;
            sourceRect.width = (float)imageLayer.image.width;
            sourceRect.height = (float)imageLayer.image.height;
            Vector2 origin; /* Reference point used for rotations. We're not rotating so we'll just use all zeroes. */
            origin.x = 0.0f;
            origin.y = 0.0f;

            if (imageLayer.repeatX && imageLayer.repeatY) { /* If repeating on both axes */
                /* Loop over both the X and Y axes to draw an array of repeated images */
                for (float x = x0; x <= screenRect.x + screenRect.width; x += imageRect.width) {
                    for (float y = y0; y <= screenRect.y + screenRect.height; y += imageRect.height) {
                        imageRect.x = x;
                        imageRect.y = y;
                        DrawTexturePro(/* texture: */ imageLayer.image.texture, /* source: */ sourceRect,
                            /* dest: */ imageRect, /* origin: */ origin, /* rotation: */ 0.0f, /* tint: */ tint);
                    }
                }
            } else if (imageLayer.repeatX) { /* If repeating on just the X axis */
                /* Loop over just the X axis to draw a horizontal line of repeated images */
                for (float x = x0; x <= screenRect.x + screenRect.width; x += imageRect.width) {
                    imageRect.x = x;
                    DrawTexturePro(/* texture: */ imageLayer.image.texture, /* source: */ sourceRect,
                        /* dest: */ imageRect, /* origin: */ origin, /* rotation: */ 0.0f, /* tint: */ tint);
                }
            } else if (imageLayer.repeatY) { /* If repeating on just the Y axis */
                /* Loop over just the Y axis to draw a vertical line of repeated images */
                for (float y = y0; y <= screenRect.y + screenRect.height; y += imageRect.height) {
                    imageRect.y = y;
                    DrawTexturePro(/* texture: */ imageLayer.image.texture, /* source: */ sourceRect,
                        /* dest: */ imageRect, /* origin: */ origin, /* rotation: */ 0.0f, /* tint: */ tint);
                }
            }
        }
    }
}

/**
 * Helper function for detecting collisions between a line and a polygon given the line's start and end points and an
 * array and count of the polygon's vertices.
 * 
 * @param startPos One of the two points forming the line.
 * @param endPos The other point forming the line.
 * @param polyPos Position, in pixels, of the polygon. Its points are relative to this position.
 * @param points Array of vertices of the polygon.
 * @param pointCount Number of verticies in the polygon and, therefore, length of the array of vertices.
 * @return True if the line and polygon collide with one another, or false if there is no collision.
 */
bool CheckCollisionLinePoly(Vector2 startPos, Vector2 endPos, Vector2 polyPos, Vector2* points, int pointCount) {
    if (pointCount < 3)
        return false;

    /* Cycle through each edge of the polygon */
    int nextIndex = 0;
    for (int currentIndex = 0; currentIndex < pointCount; currentIndex++) {
        /* Get the next index. If the current index is the last, wrap around. */
        nextIndex = currentIndex + 1;
        if (nextIndex == pointCount)
            nextIndex = 0;
        /* Get the current and next points. These two points form an edge of the polygon. */
        Vector2 currentPoint = points[currentIndex];
        Vector2 nextPoint = points[nextIndex];
        /* Check these edges for collisions. Note: The last parameter is unused, hence zero. */
        if (CheckCollisionLines(/* startPos1: */ startPos, /* endPos1: */ endPos, /* startPos2: */ currentPoint,
                /* endPos: */ nextPoint, /* collisionPoint: */ NULL))
            return true;
    }

    return false;
}

/**
 * Helper function for detecting collisions between two polygons given arrays and counts of their vertices.
 *
 * @param polyPos1 Position, in pixels, of the first polygon. Its points are relative to this position.
 * @param points1 Array of vertices of the first polygon.
 * @param pointCount1 Number of vertices in the first polygon and, therefore, length of the array of vertices.
 * @param polyPos2 Position, in pixels, of the second polygon. Its points are relative to this position.
 * @param points2 Array of vertices of the second polygon.
 * @param pointCount2 Number of vertices in the second polygon and, therefore, length of the array of vertices.
 * @return True if the given polygons collide with one another, or false if there is no collision.
 */
bool CheckCollisionPolyPoly(Vector2 polyPos1, Vector2* points1, int pointCount1, Vector2 polyPos2, Vector2* points2,
        int pointCount2) {
    /* If either vertex array is missing or does not contain enough vertices to define a polygon */
    if (points1 == NULL || pointCount1 < 3 || points2 == NULL || pointCount2 < 3)
        return false;

    /* Cycle through each edge of polygon 1 */
    int nextIndex = 0;
    for (int currentIndex = 0; currentIndex < pointCount1; currentIndex++) {
        /* Get the next index. If the current index is the last, wrap around. */
        nextIndex = currentIndex + 1;
        if (nextIndex == pointCount1)
            nextIndex = 0;
        /* Get the current and next points. These two points form an edge of polygon 1. */
        Vector2 currentPoint = points1[currentIndex];
        currentPoint.x += polyPos1.x;
        currentPoint.y += polyPos1.y;
        Vector2 nextPoint = points1[nextIndex];
        nextPoint.x += polyPos1.x;
        nextPoint.y += polyPos1.y;
        /* Check this edge for collisions against edges of polygon 2 */
        if (CheckCollisionLinePoly(currentPoint, nextPoint, polyPos2, points2, pointCount2))
            return true;
    }

    /* Check if polygon 1 is fully inside polygon 2 */
    if (CheckCollisionPointPoly(/* point: */ points1[0], /* points: */ points2, /* pointCount: */ pointCount2))
        return true;

    /* Check if polygon 2 is fully inside polygon 1 */
    if (CheckCollisionPointPoly(/* point: */ points2[0], /* points: */ points1, /* pointCount: */ pointCount1))
        return true;

    return false;
}

RAYTMX_DEC bool CheckCollisionTMXObjects(TmxObject object1, TmxObject object2) {
    /* Perform a quick collision check on the Axis-Aligned Bounding Boxes (AABB) before more accurate checks */
    if (!CheckCollisionRecs(/* rec1: */ object1.aabb, /* rec2: */ object2.aabb))
        return false; /* The AABBs do not collide so the objects cannot possibly collide */

    switch (object1.type) {
    case OBJECT_TYPE_RECTANGLE: /* Object 1's type */
    case OBJECT_TYPE_ELLIPSE: /* Object 1's type (treated as a rectangle due to difficulty) */
    case OBJECT_TYPE_TEXT: /* Object 1's type */
    case OBJECT_TYPE_TILE: /* Object 1's type */
        switch (object2.type) {
        case OBJECT_TYPE_RECTANGLE: /* Object 2's type */
        case OBJECT_TYPE_ELLIPSE: /* Object 2's type (treated as a rectangle due to difficulty) */
        case OBJECT_TYPE_TEXT: /* Object 2's type */
        case OBJECT_TYPE_TILE: /* Object 2's type */
            /* The objects' shapes, as far as collisions are concerned, are identical to the AABB. We already */
            /* determined the AABBs collide so these objects collide. */
            return true;

        case OBJECT_TYPE_POINT: /* Object 2's type */
            return CheckCollisionPointRec(/* point: */ (Vector2){(float)object2.x, (float)object2.y},
                /* rec: */ object1.aabb);

        case OBJECT_TYPE_POLYGON: /* Object 2's type */
        case OBJECT_TYPE_POLYLINE: /* Object 2's type */
        {
            /* A rectangle is a polygon. Create an array of points to treat it as a polygon keeping in mind polygon */
            /* vertices are relative so the top-left corner is always (0, 0). */
            Vector2 points[4];
            points[0] = (Vector2){0.0f, 0.0f};
            points[1] = (Vector2){(float)object1.width, 0.0f};
            points[2] = (Vector2){(float)object1.width, (float)object1.height};
            points[3] = (Vector2){0.0f, (float)object1.height};
            return CheckCollisionPolyPoly(/* polyPos1: */ (Vector2){(float)object1.x, (float)object1.y},
                /* points1: */ points, /* pointCount: */ 4,
                /* polyPos2: */ (Vector2){(float)object2.x, (float)object2.y}, /* points2: */ object2.points,
                /* pointCount2: */ object2.pointsLength);
        }
        }
    break;

    case OBJECT_TYPE_POINT: /* Object 1's type */
        switch (object2.type) {
        case OBJECT_TYPE_RECTANGLE: /* Object 2's type */
        case OBJECT_TYPE_ELLIPSE: /* Object 2's type (treated as a rectangle due to difficulty) */
        case OBJECT_TYPE_TEXT: /* Object 2's type */
        case OBJECT_TYPE_TILE: /* Object 2's type */
            return CheckCollisionPointRec(/* point: */ (Vector2){(float)object1.x, (float)object2.y},
                /* rec: */ object2.aabb);

        case OBJECT_TYPE_POINT: /* Object 2's type */
            return object1.x == object2.x && object1.y == object2.y;

        case OBJECT_TYPE_POLYGON: /* Object 2's type */
        case OBJECT_TYPE_POLYLINE: /* Object 2's type */
            return CheckCollisionPointPoly((Vector2){(float)object1.x, (float)object1.y}, object2.points,
                object2.pointsLength);
        }
    break;

    case OBJECT_TYPE_POLYGON: /* Object 1's type */
    case OBJECT_TYPE_POLYLINE: /* Object 1's type */
        switch (object2.type) {
        case OBJECT_TYPE_RECTANGLE: /* Object 2's type */
        case OBJECT_TYPE_ELLIPSE: /* Object 2's type (treated as a rectangle due to difficulty) */
        case OBJECT_TYPE_TEXT: /* Object 2's type */
        case OBJECT_TYPE_TILE: /* Object 2's type */
        {
            /* A rectangle is a polygon. Create an array of points to treat it as a polygon keeping in mind polygon */
            /* vertices are relative so the top-left corner is always (0, 0). */
            Vector2 points[4];
            points[0] = (Vector2){0.0f, 0.0f};
            points[1] = (Vector2){(float)object2.width, 0.0f};
            points[2] = (Vector2){(float)object2.width, (float)object2.height};
            points[3] = (Vector2){0.0f, (float)object2.height};
            return CheckCollisionPolyPoly(/* polyPos1: */ (Vector2){(float)object1.x, (float)object1.y},
                /* points1: */ object1.points, /* pointCount1: */ object1.pointsLength,
                /* polyPos2: */ (Vector2){(float)object2.x, (float)object2.y}, /* points2: */ points,
                /* pointCount2: */ 4);
        }

        case OBJECT_TYPE_POINT: /* Object 2's type */
            return CheckCollisionPointPoly(/* point: */ (Vector2){(float)object2.x, (float)object2.y},
                /* points: */ object1.points, /* pointCount: */ object1.pointsLength);

        case OBJECT_TYPE_POLYGON: /* Object 2's type */
        case OBJECT_TYPE_POLYLINE: /* Object 2's type */
            return CheckCollisionPolyPoly(/* polyPos1: */ (Vector2){(float)object1.x, (float)object1.y},
                /* points1: */ object1.points, /* pointCount1: */ object1.pointsLength,
                /* polyPos2: */ (Vector2){(float)object2.x, (float)object2.y}, /* points2: */ object2.points,
                /* pointCount2: */ object2.pointsLength);
        }
    break;
    }

    return false;
}

/**
 * Helper function for applying a translation to a given object by the given deltas.
 *
 * @param object The object whose position(s) should be translated.
 * @param dx The X delta, in pixels, to translate.
 * @param dy The Y delta, in pixels, to translate.
 * @return A translated copy of the given object.
 */
TmxObject TranslateObject(TmxObject object, float dx, float dy) {
    /* Translate the position of the object by the different X and Y deltas */
    object.x += (double)dx;
    object.y += (double)dy;
    /* Translate the Axis-Aligned Bounding Boxes (AABBs) to match */
    object.aabb.x += dx;
    object.aabb.y += dy;
    /* Note: Although polygons and polylines have a series of vertices, they are relative to the object. This means */
    /* nothing more needs to be done. Translating the object's X and Y effectively translates each vertex. Plus, the */
    /* points array is a heap allocation so doing a += on the points modifies them globally. That's bad. */
    return object;
}

/**
 * Helper function for checking for collisions between 1+ tile layers, or groups potentially containing 1+ tile layers,
 * and an object of arbitrary type. These checks use the collision information associated with tiles as object groups.
 *
 * @param map A loaded map model containing the given layers.
 * @param layers An array of select tile layers or group layers to be checked for collisions.
 * @param layersLength Length of the given array of tile layers.
 * @param object A TMX <object> to be checked for a collision.
 * @param outputObject Output parameter assigned with the object in the tile layer that the given object collided with.
 *                     NULL if not wanted.
 * @return True if one of the given tile layers collides with the given object, or false if there is no collision.
 */
bool CheckCollisionTMXTileLayerObject(const TmxMap* map, const TmxLayer* layers, uint32_t layersLength,
        TmxObject object, TmxObject* outputObject) {
    if (map == NULL || layers == NULL || layersLength == 0)
        return false;

    /* Iterate through each layer and check their tiles for collisions with the given object */
    for (uint32_t i = 0; i < layersLength; i++) {
        if (layers[i].type == LAYER_TYPE_TILE_LAYER) { /* If the layer has tiles */
            /* Iterate through each tile that the object's Axis-Aligned Bounding Box (AABB) overlaps with */
            TmxTile tile;
            Rectangle tileRect;
            while (IterateTileLayer(/* map: */ map, /* layer: */ &layers[i].exact.tileLayer,
                    /* screenRect: */ object.aabb, /* rawGid: */ NULL, /* tile: */ &tile, /* tileRect: */ &tileRect)) {
                /* Iterate through each object associated with the tile */
                for (uint32_t j = 0; j < tile.objectGroup.objectsLength; j++) {
                    /* This object, the tile's collision information, has a relative position so this object must be */
                    /* translated to the position of the tile as it would be drawn with the layer */
                    TmxObject positionedObject = TranslateObject(tile.objectGroup.objects[j], tileRect.x, tileRect.y);
                    /* If this tile's object collides with the given object */
                    if (CheckCollisionTMXObjects(positionedObject, object)) {
                        if (outputObject != NULL)
                            *outputObject = positionedObject;
                        return true; /* Found a collision. Exit now to save some CPU cycles. */
                    }
                }
            }
        } else if (layers[i].type == LAYER_TYPE_GROUP) { /* If the layer contains other layers */
            if (CheckCollisionTMXTileLayerObject(map, layers[i].layers, layers[i].layersLength, object, outputObject))
                return true;
        }
    }

    return false;
}

/**
 * Helper function for checking for collisions between an object group and an object of arbitrary type.
 *
 * @param group The object group whose 0+ objects will be checked for collisions.
 * @param object A TMX <object> to be checked for collision.
 * @param outputObject Output parameter assigned with the object in the object group that the given object collided
 *                     with. NULL if not wanted.
 * @return True if an object in the object group collides with the given object, or false if there is no collision.
 */
bool CheckCollisionTMXObjectGroupObject(TmxObjectGroup group, TmxObject object, TmxObject* outputObject) {
    for (size_t i = 0; i < group.objectsLength; i++) {
        if (CheckCollisionTMXObjects(group.objects[i], object)) {
            if (outputObject != NULL)
                *outputObject = group.objects[i];
            return true;
        }
    }

    return false;
}

void TraceLogTMXTilesets(int logLevel, TmxOrientation orientation, TmxTileset* tilesets, uint32_t tilesetsLength,
        int numSpaces) {
    for (uint32_t i = 0; i < tilesetsLength; i++) {
        TmxTileset tileset = tilesets[i];
        if (i == 0)
            TraceLog(logLevel, "tilesets:");
        TraceLog(logLevel, "  \"%s\":", tileset.name);
        TraceLog(logLevel, "    first GID: %u", tileset.firstGid);
        TraceLog(logLevel, "    last GID: %u", tileset.lastGid);
        if (tileset.source != NULL)
            TraceLog(logLevel, "    source: \"%s\"", tileset.source);
        if (tileset.classString[0] != '\0')
            TraceLog(logLevel, "    class: \"%s\"", tileset.classString);
        TraceLog(logLevel, "    tile width: %u", tileset.tileWidth);
        TraceLog(logLevel, "    tile height: %u", tileset.tileHeight);
        if (tileset.spacing != 0)
            TraceLog(logLevel, "    spacing: %u", tileset.spacing);
        if (tileset.margin != 0)
            TraceLog(logLevel, "    margin: %u", tileset.margin);
        TraceLog(logLevel, "    tile count: %u", tileset.tileCount);
        TraceLog(logLevel, "    columns: %u", tileset.columns);
        if (tileset.tileOffsetX != 0)
            TraceLog(logLevel, "    tile offset X: %d", tileset.tileOffsetX);
        if (tileset.tileOffsetY != 0)
            TraceLog(logLevel, "    tile offset Y: %d", tileset.tileOffsetY);
        if ((orientation == ORIENTATION_ORTHOGONAL && tileset.objectAlignment != OBJECT_ALIGNMENT_BOTTOM_LEFT) ||
                (orientation == ORIENTATION_ISOMETRIC && tileset.objectAlignment != OBJECT_ALIGNMENT_BOTTOM)) {
            switch (tileset.objectAlignment) {
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
        if (tileset.hasImage) {
            TraceLog(logLevel, "    image:");
            TraceLog(logLevel, "      source: \"%s\"", tileset.image.source);
            if (tileset.image.hasTrans)
                TraceLog(logLevel, "      trans: 0x%08X", tileset.image.trans);
            if (tileset.image.width != 0)
                TraceLog(logLevel, "      width: %u", tileset.image.width);
            if (tileset.image.height != 0)
                TraceLog(logLevel, "      height: %u", tileset.image.height);
            TraceLog(logLevel, "      texture (ID): %u", tileset.image.texture.id);
        }
        for (uint32_t j = 0; j < tileset.tilesLength; j++) {
            TmxTilesetTile tile = tileset.tiles[j];
            if (j == 0)
                TraceLog(logLevel, "    tiles:");
            TraceLog(logLevel, "      ID: %u", tile.id);
            if (tile.hasAnimation) {
                for (uint32_t i = 0; i < tile.animation.framesLength; i++) {
                    if (i == 0)
                        TraceLog(logLevel, "      frames:");
                    TraceLog(logLevel, "        ID: %u", tile.animation.frames[i].id);
                    TraceLog(logLevel, "          duration: %f", tile.animation.frames[i].duration);
                }
            }
            TraceLogTMXProperties(logLevel, tile.properties, tile.propertiesLength, 8);
            if (tile.hasImage && tile.image.texture.id != tileset.image.texture.id) {
                /* The 'x,' 'y,' 'width,' and 'height' attributes relate to the image so only log them if one exists */
                if (tile.x != 0)
                    TraceLog(logLevel, "      x: %d", tile.x);
                if (tile.y != 0)
                    TraceLog(logLevel, "      y: %d", tile.y);
                if (tile.width != tile.image.width)
                    TraceLog(logLevel, "      width: %u", tile.width);
                if (tile.height != tile.image.height)
                    TraceLog(logLevel, "      height: %u", tile.height);
                TraceLog(logLevel, "      image:");
                TraceLog(logLevel, "        source: \"%s\"", tile.image.source);
                if (tile.image.hasTrans)
                    TraceLog(logLevel, "        trans: 0x%08X", tile.image.trans);
                if (tile.image.width != 0)
                    TraceLog(logLevel, "        width: %u", tile.image.width);
                if (tile.image.height != 0)
                    TraceLog(logLevel, "        height: %u", tile.image.height);
                TraceLog(logLevel, "        texture (ID): %u", tile.image.texture.id);
            }
            if (tile.objectGroup.objectsLength > 0) {
                if (tmxLogFlags & LOG_SKIP_OBJECTS)
                    TraceLog(logLevel, "      skipping %u objects", tile.objectGroup.objectsLength);
                else {
                    TraceLog(logLevel, "      objects:");
                    for (uint32_t k = 0; k < tile.objectGroup.objectsLength; k++)
                        TraceLogTMXObject(logLevel, tile.objectGroup.objects[k], numSpaces + 2);
                }
            }
        }
        TraceLogTMXProperties(logLevel, tileset.properties, tileset.propertiesLength, 2);
    }
}

void TraceLogTMXProperties(int logLevel, TmxProperty* properties, uint32_t propertiesLength, int numSpaces) {
    char padding[16];
    memset(padding, '\0', 16);
    StringCopyN(padding, "                ", numSpaces);

    if (tmxLogFlags & LOG_SKIP_PROPERTIES)
        TraceLog(logLevel, "%sskipped %u properties", padding, propertiesLength);
    else {
        for (uint32_t i = 0; i < propertiesLength; i++) {
            TmxProperty property = properties[i];
            if (i == 0)
                TraceLog(logLevel, "%sproperties:", padding);
            switch (property.type) {
            case PROPERTY_TYPE_STRING:
            case PROPERTY_TYPE_FILE:
                TraceLog(logLevel, "%s  \"%s\": \"%s\"", padding, property.name, property.stringValue);
            break;
            case PROPERTY_TYPE_INT:
            case PROPERTY_TYPE_OBJECT:
                TraceLog(logLevel, "%s  \"%s\": %d", padding, property.name, property.intValue);
            break;
            case PROPERTY_TYPE_FLOAT:
                TraceLog(logLevel, "%s  \"%s\": %f", padding, property.name, property.floatValue);
            break;
            case PROPERTY_TYPE_BOOL:
                TraceLog(logLevel, "%s  \"%s\": %s", padding, property.name, property.boolValue ? "true" : "false");
            break;
            case PROPERTY_TYPE_COLOR:
                TraceLog(logLevel, "%s  \"%s\": 0x%08X", padding, property.name, property.colorValue);
            break;
            }
        }
    }
}

void TraceLogTMXLayers(int logLevel, TmxLayer* layers, uint32_t layersLength, int numSpaces) {
    char padding[16];
    memset(padding, '\0', 16);
    StringCopyN(padding, "                ", numSpaces);

    if (tmxLogFlags & LOG_SKIP_LAYERS)
        TraceLog(logLevel, "%sskipped %u layers", padding, layersLength);
    else {
        uint32_t numTileLayers = 0, numObjectGroups = 0, numImageLayers = 0;
        for (uint32_t i = 0; i < layersLength; i++) {
            TmxLayer layer = layers[i];
            if (i == 0)
                TraceLog(logLevel, "%slayers:", padding);
            /* If, based on the layer type, this layer isn't one that should be skipped */
            if ((layer.type == LAYER_TYPE_GROUP) ||
                    (layer.type == LAYER_TYPE_TILE_LAYER && !(tmxLogFlags & LOG_SKIP_TILE_LAYERS)) ||
                    (layer.type == LAYER_TYPE_OBJECT_GROUP && !(tmxLogFlags & LOG_SKIP_OBJECT_GROUPS)) ||
                    (layer.type == LAYER_TYPE_IMAGE_LAYER && !(tmxLogFlags & LOG_SKIP_IMAGE_LAYERS))) {
                /* Log the attributes of this layer common to all layers */
                TraceLog(logLevel, "%s  \"%s\":", padding, layer.name);
                switch (layer.type) {
                case LAYER_TYPE_TILE_LAYER: TraceLog(logLevel, "%s    type: tile layer", padding); break;
                case LAYER_TYPE_OBJECT_GROUP: TraceLog(logLevel, "%s    type: object layer", padding); break;
                case LAYER_TYPE_IMAGE_LAYER: TraceLog(logLevel, "%s    type: image layer", padding); break;
                case LAYER_TYPE_GROUP: TraceLog(logLevel, "%s    type: group", padding); break;
                }
                if (layer.id > 0)
                    TraceLog(logLevel, "%s    ID: %u", padding, layer.id);
                if (layer.visible == false)
                    TraceLog(logLevel, "%s    visible: false", padding);
                if (layer.classString[0] != '\0')
                    TraceLog(logLevel, "%s    class: \"%s\"", padding, layer.classString);
                if (layer.offsetX != 0)
                    TraceLog(logLevel, "%s    offset X: %d", padding, layer.offsetX);
                if (layer.offsetY != 0)
                    TraceLog(logLevel, "%s    offset Y: %d", padding, layer.offsetY);
                if (layer.parallaxX != 1.0)
                    TraceLog(logLevel, "%s    parallax X: %f", padding, layer.parallaxX);
                if (layer.parallaxY != 1.0)
                    TraceLog(logLevel, "%s    parallax Y: %f", padding, layer.parallaxY);
                if (layer.opacity != 1.0)
                    TraceLog(logLevel, "%s    opacity: %f", padding, layer.opacity);
                if (layer.hasTintColor)
                    TraceLog(logLevel, "%s    tint color: 0x%08X", padding, layer.tintColor);
                TraceLogTMXProperties(logLevel, layer.properties, layer.propertiesLength, numSpaces + 4);
            }

            /* Log attributes specific to the layer's type (tile layer, object layer, image layer, or group) */
            switch (layer.type) {
            case LAYER_TYPE_TILE_LAYER:
                numTileLayers += 1;
                if (tmxLogFlags & LOG_SKIP_TILE_LAYERS)
                    continue;
                if (layer.exact.tileLayer.width != 0)
                    TraceLog(logLevel, "%s    width: %u", padding, layer.exact.tileLayer.width);
                if (layer.exact.tileLayer.height != 0)
                    TraceLog(logLevel, "%s    height: %u", padding, layer.exact.tileLayer.height);
                if (tmxLogFlags & LOG_SKIP_TILES)
                    TraceLog(logLevel, "%s    skipping %u tiles", padding, layer.exact.tileLayer.tilesLength);
                else {
                    for (uint32_t j = 0; j < layer.exact.tileLayer.tilesLength; j++) {
                        if (j == 0)
                            TraceLog(logLevel, "%s    tiles:", padding);
                        TraceLog(logLevel, "%s      GID: %u", padding, layer.exact.tileLayer.tiles[j]);
                    }
                }
                break;
            case LAYER_TYPE_OBJECT_GROUP:
                numObjectGroups += 1;
                if (tmxLogFlags & LOG_SKIP_OBJECT_GROUPS)
                    continue;
                if (layer.exact.objectGroup.hasColor)
                    TraceLog(logLevel, "%s    color: 0x%08X", padding, layer.exact.objectGroup.color);
                /* if (layer.exact.objectGroup.width != 0)
                    TraceLog(logLevel, "%s    width: %u", padding, layer.exact.objectGroup.width); */
                /* if (layer.exact.objectGroup.height != 0)
                    TraceLog(logLevel, "%s    height: %u", padding, layer.exact.objectGroup.height); */
                if (layer.exact.objectGroup.drawOrder != OBJECT_GROUP_DRAW_ORDER_TOP_DOWN) {
                    switch (layer.exact.objectGroup.drawOrder) {
                    case OBJECT_GROUP_DRAW_ORDER_TOP_DOWN:
                    default:
                        TraceLog(logLevel, "%s    draw order: top-down", padding);
                        break;
                    case OBJECT_GROUP_DRAW_ORDER_INDEX:
                        TraceLog(logLevel, "%s    draw order: index", padding);
                        break;
                    }
                }
                if (tmxLogFlags & LOG_SKIP_OBJECTS)
                    TraceLog(logLevel, "%s    skipping %u objects", padding, layer.exact.objectGroup.objectsLength);
                else {
                    for (uint32_t j = 0; j < layer.exact.objectGroup.objectsLength; j++) {
                        TmxObject object = layer.exact.objectGroup.objects[j];
                        if (j == 0)
                            TraceLog(logLevel, "%s    objects:", padding);
                        TraceLogTMXObject(logLevel, object, numSpaces);
                    }
                }
                break;
            case LAYER_TYPE_IMAGE_LAYER:
                numImageLayers += 1;
                if (tmxLogFlags & LOG_SKIP_IMAGE_LAYERS)
                    continue;
                if (layer.exact.imageLayer.repeatX)
                    TraceLog(logLevel, "%s    repeat X: true", padding);
                if (layer.exact.imageLayer.repeatY)
                    TraceLog(logLevel, "%s    repeat Y: true", padding);
                if (layer.exact.imageLayer.hasImage) {
                    TraceLog(logLevel, "%s    image:", padding);
                    TraceLog(logLevel, "%s      source: \"%s\"", padding, layer.exact.imageLayer.image.source);
                    if (layer.exact.imageLayer.image.hasTrans)
                        TraceLog(logLevel, "%s      trans: 0x%08X", padding, layer.exact.imageLayer.image.trans);
                    if (layer.exact.imageLayer.image.width != 0)
                        TraceLog(logLevel, "%s      width: %u", padding, layer.exact.imageLayer.image.width);
                    if (layer.exact.imageLayer.image.height != 0)
                        TraceLog(logLevel, "%s      height: %u", padding, layer.exact.imageLayer.image.height);
                    TraceLog(logLevel, "%s      texture (ID): %u", padding, layer.exact.imageLayer.image.texture.id);
                }
                break;
            case LAYER_TYPE_GROUP:
                TraceLogTMXLayers(logLevel, layer.layers, layer.layersLength, numSpaces + 4);
                break;
            }
        }
        if (tmxLogFlags & LOG_SKIP_TILE_LAYERS && numTileLayers > 0)
            TraceLog(logLevel, "%s  skipped %u tile layers", padding, numTileLayers);
        if (tmxLogFlags & LOG_SKIP_OBJECT_GROUPS && numObjectGroups > 0)
            TraceLog(logLevel, "%s  skipped %u object layers", padding, numObjectGroups);
        if (tmxLogFlags & LOG_SKIP_IMAGE_LAYERS && numImageLayers > 0)
            TraceLog(logLevel, "%s  skipped %u image layers", padding, numImageLayers);
    }
}

void TraceLogTMXObject(int logLevel, TmxObject object, int numSpaces) {
    char padding[16];
    memset(padding, '\0', 16);
    StringCopyN(padding, "                ", numSpaces);

    TraceLog(logLevel, "%s      ID: %u", padding, object.id);
    TraceLog(logLevel, "%s        name: \"%s\"", padding, object.name);
    switch (object.type) {
    case OBJECT_TYPE_RECTANGLE: TraceLog(logLevel, "%s        type: quad", padding); break;
    case OBJECT_TYPE_ELLIPSE: TraceLog(logLevel, "%s        type: ellipse", padding); break;
    case OBJECT_TYPE_POINT: TraceLog(logLevel, "%s        type: point", padding); break;
    case OBJECT_TYPE_POLYGON: TraceLog(logLevel, "%s        type: polygon", padding); break;
    case OBJECT_TYPE_POLYLINE: TraceLog(logLevel, "%s        type: polyline", padding); break;
    case OBJECT_TYPE_TEXT: TraceLog(logLevel, "%s        type: text", padding); break;
    case OBJECT_TYPE_TILE: TraceLog(logLevel, "%s        type: tile", padding); break;
    }
    if (object.typeString[0] != '\0')
        TraceLog(logLevel, "%s        type: \"%s\"", padding, object.typeString);
    if (object.x != 0.0)
        TraceLog(logLevel, "%s        x: %f", padding, object.x);
    if (object.y != 0.0)
        TraceLog(logLevel, "%s        y: %f", padding, object.y);
    if (object.width != 0.0)
        TraceLog(logLevel, "%s        width: %f", padding, object.width);
    if (object.height != 0.0)
        TraceLog(logLevel, "%s        height: %f", padding, object.height);
    if (object.rotation != 0.0)
        TraceLog(logLevel, "%s        rotation: %f", padding, object.rotation);
    if (object.gid > 0)
        TraceLog(logLevel, "%s        GID: %u", padding, object.gid);
    if (!object.visible)
        TraceLog(logLevel, "%s        visible: false", padding);
    if (object.templateString != NULL)
        TraceLog(logLevel, "%s        template: \"%s\"", padding, object.templateString);
    for (uint32_t k = 0; k < object.pointsLength; k++) {
        if (k == 0)
            TraceLog(logLevel, "%s        points:", padding);
        TraceLog(logLevel, "%s          [%f, %f]", padding, object.points[k].x, object.points[k].y);
    }
    TraceLogTMXProperties(logLevel, object.properties, object.propertiesLength, numSpaces + 8);
    if (object.text != NULL) {
        TraceLog(logLevel, "%s        font family: \"%s\"", padding, object.text->fontFamily);
        TraceLog(logLevel, "%s        pixel size: %u", padding, object.text->pixelSize);
        if (object.text->wrap)
            TraceLog(logLevel, "%s        wrap: true", padding);
        TraceLog(logLevel, "%s        color: 0x%08X", padding, object.text->color);
        if (object.text->bold)
            TraceLog(logLevel, "%s        bold: true", padding);
        if (object.text->italic)
            TraceLog(logLevel, "%s        italic: true", padding);
        if (object.text->underline)
            TraceLog(logLevel, "%s        underline: true", padding);
        if (object.text->strikeOut)
            TraceLog(logLevel, "%s        strike out: true", padding);
        if (object.text->kerning)
            TraceLog(logLevel, "%s        kerning: true", padding);
        switch (object.text->halign) {
        case HORIZONTAL_ALIGNMENT_LEFT:
            TraceLog(logLevel, "%s        horizontal align: left", padding);
            break;
        case HORIZONTAL_ALIGNMENT_CENTER:
            TraceLog(logLevel, "%s        horizontal align: center", padding);
            break;
        case HORIZONTAL_ALIGNMENT_RIGHT:
            TraceLog(logLevel, "%s        horizontal align: right", padding);
            break;
        case HORIZONTAL_ALIGNMENT_JUSTIFY:
            TraceLog(logLevel, "%s        horizontal align: justify", padding);
            break;
        }
        switch (object.text->valign) {
        case VERTICAL_ALIGNMENT_TOP:
            TraceLog(logLevel, "%s        vertical align: top", padding);
            break;
        case VERTICAL_ALIGNMENT_CENTER:
            TraceLog(logLevel, "%s        vertical align: center", padding);
            break;
        case VERTICAL_ALIGNMENT_BOTTOM:
            TraceLog(logLevel, "%s        vertical align: bottom", padding);
            break;
        }
        if (object.text->content[0] != '\0')
            TraceLog(logLevel, "%s        content: \"%s\"", padding, object.text->content);
    }
}

TmxProperty* AddProperty(RaytmxState* raytmxState) {
    RaytmxPropertyNode* node = (RaytmxPropertyNode*)MemAllocZero(sizeof(RaytmxPropertyNode));

    if (raytmxState->propertiesRoot == NULL)
        raytmxState->propertiesRoot = node;
    else
        raytmxState->propertiesTail->next = node;
    raytmxState->propertiesTail = node;
    raytmxState->propertiesLength += 1;

    return &node->property;
}

void AddTileLayerTile(RaytmxState* raytmxState, uint32_t gid) {
    RaytmxTileLayerTileNode* node = (RaytmxTileLayerTileNode*)MemAllocZero(sizeof(RaytmxTileLayerTileNode));
    node->gid = gid;

    if (raytmxState->layerTilesRoot == NULL)
        raytmxState->layerTilesRoot = node;
    else
        raytmxState->layerTilesTail->next = node;
    raytmxState->layerTilesTail = node;
    raytmxState->layerTilesLength += 1;
}

TmxTileset* AddTileset(RaytmxState* raytmxState) {
    RaytmxTilesetNode* node = (RaytmxTilesetNode*)MemAllocZero(sizeof(RaytmxTilesetNode));

    if (raytmxState->tilesetsRoot == NULL)
        raytmxState->tilesetsRoot = node;
    else
        raytmxState->tilesetsTail->next = node;
    raytmxState->tilesetsTail = node;
    raytmxState->tilesetsLength += 1;

    return &node->tileset;
}

TmxTilesetTile* AddTilesetTile(RaytmxState* raytmxState) {
    RaytmxTilesetTileNode* node = (RaytmxTilesetTileNode*)MemAllocZero(sizeof(RaytmxTilesetTileNode));

    if (raytmxState->tilesetTilesRoot == NULL)
        raytmxState->tilesetTilesRoot = node;
    else
        raytmxState->tilesetTilesTail->next = node;
    raytmxState->tilesetTilesTail = node;
    raytmxState->tilesetTilesLength += 1;

    return &node->tile;
}

TmxAnimationFrame* AddAnimationFrame(RaytmxState* raytmxState) {
    RaytmxAnimationFrameNode* node = (RaytmxAnimationFrameNode*)MemAllocZero(sizeof(RaytmxAnimationFrameNode));

    if (raytmxState->animationFramesRoot == NULL)
        raytmxState->animationFramesRoot = node;
    else
        raytmxState->animationFramesTail->next = node;
    raytmxState->animationFramesTail = node;
    raytmxState->animationFramesLength += 1;

    return &node->frame;
}

TmxLayer* AddGenericLayer(RaytmxState* raytmxState, bool isGroup) {
    RaytmxLayerNode* node = (RaytmxLayerNode*)MemAllocZero(sizeof(RaytmxLayerNode));
    /* There are some non-zero default values for several layer attributes: */
    node->layer.opacity = 1.0;
    node->layer.visible = true;
    node->layer.parallaxX = 1.0;
    node->layer.parallaxY = 1.0;

    if (raytmxState->groupNode != NULL) { /* If the layer is being appended to a <group> */
        node->parent = raytmxState->groupNode;
        if (raytmxState->groupNode->childrenRoot == NULL) /* If the layer is the first in the group */
            raytmxState->groupNode->childrenRoot = node;
        else
            raytmxState->groupNode->childrenTail->next = node;
        raytmxState->groupNode->childrenTail = node;
        raytmxState->groupNode->childrenLength += 1;
    } else { /* If there is no <group> to append to, meaning the layer belongs to the top-level <map> */
        if (raytmxState->layersRoot == NULL) /* If the layer is the first in the map */
            raytmxState->layersRoot = node;
        else
            raytmxState->layersTail->next = node;
        raytmxState->layersTail = node;
        raytmxState->layersLength += 1;
    }

    if (isGroup)
        raytmxState->groupNode = node;

    return &node->layer;
}

TmxObject* AddObject(RaytmxState* raytmxState) {
    RaytmxObjectNode* node = (RaytmxObjectNode*)MemAllocZero(sizeof(RaytmxObjectNode));
    /* <object> elements have one non-zero default value: */
    node->object.visible = true;

    if (raytmxState->objectsRoot == NULL)
        raytmxState->objectsRoot = node;
    else
        raytmxState->objectsTail->next = node;
    raytmxState->objectsTail = node;
    raytmxState->objectsLength += 1;

    return &node->object;
}

void AppendLayerTo(TmxMap* map, RaytmxLayerNode* groupNode, RaytmxLayerNode* layersRoot, uint32_t layersLength) {
    if (map == NULL || layersRoot == NULL || layersLength == 0)
        return;

    TmxLayer* groupLayer = NULL;
    if (groupNode != NULL)
        groupLayer = &(groupNode->layer);

    /* Allocate the array and zerioze every index as initialization */
    TmxLayer* layers = (TmxLayer*)MemAllocZero(sizeof(TmxLayer) * layersLength);
    /* Copy the TmxLayers into the array */
    RaytmxLayerNode* layersIterator = layersRoot;
    for (uint32_t i = 0; layersIterator != NULL; i++) {
        if (layersIterator->childrenRoot != NULL)
            AppendLayerTo(map, layersIterator, layersIterator->childrenRoot, layersIterator->childrenLength);
        layers[i] = layersIterator->layer;
        layersIterator = layersIterator->next;
    }

    if (groupLayer != NULL) { /* If the list of layers is being appended to a group */
        groupLayer->layers = layers;
        groupLayer->layersLength = layersLength;
    } else { /* If the list of layers is being appended to the top-level map */
        map->layers = layers;
        map->layersLength = layersLength;
    }
}

RaytmxCachedTextureNode* LoadCachedTexture(RaytmxState* raytmxState, const char* fileName) {
    if (raytmxState == NULL || fileName == NULL)
        return NULL;

    /* First try to find an already-loaded texture identified by the file name */
    RaytmxCachedTextureNode* cachedTextureNode = raytmxState->texturesRoot;
    while (cachedTextureNode != NULL) {
        /* If the file name associated with the node matches the given file name */
        if (strcmp(cachedTextureNode->fileName, fileName) == 0)
            return cachedTextureNode;
        cachedTextureNode = cachedTextureNode->next;
    }

    /* Try to load the texture */
    char* fullPath = JoinPath(raytmxState->documentDirectory, fileName);
    Texture2D texture = loadTextureOverride ? loadTextureOverride(fullPath) : LoadTexture(fullPath);
    if (texture.id == 0) { /* If loading the texture failed */
        TraceLog(LOG_ERROR, "RAYTMX: Unable to load texture \"%s\"", fullPath);
        return NULL;
    }

    /* Create a new node in the list of known textures */
    cachedTextureNode = (RaytmxCachedTextureNode*)MemAllocZero(sizeof(RaytmxCachedTextureNode));
    cachedTextureNode->fileName = (char*)MemAllocZero((unsigned int)strlen(fileName) + 1);
    StringCopy(cachedTextureNode->fileName, fileName);
    cachedTextureNode->texture = texture;

    /* Add to the cache */
    if (raytmxState->texturesRoot == NULL)
        raytmxState->texturesRoot = cachedTextureNode;
    else {
        RaytmxCachedTextureNode* cachedTextureIterator = raytmxState->texturesRoot;
        while (cachedTextureIterator->next != NULL)
            cachedTextureIterator = cachedTextureIterator->next;
        cachedTextureIterator->next = cachedTextureNode;
    }

    return cachedTextureNode;
}

RaytmxCachedTemplateNode* LoadCachedTemplate(RaytmxState* raytmxState, const char* fileName) {
    if (raytmxState == NULL || fileName == NULL)
        return NULL;

    /* First try to find an already-loaded template identified by the file name */
    RaytmxCachedTemplateNode* cachedTemplateNode = raytmxState->templatesRoot;
    while (cachedTemplateNode != NULL) {
        /* If the file name associated with the node matches the given file name */
        if (strcmp(cachedTemplateNode->fileName, fileName) == 0)
            return cachedTemplateNode;
        cachedTemplateNode = cachedTemplateNode->next;
    }

    /* Load the template from the external TX file */
    char* fullPath = JoinPath(raytmxState->documentDirectory, fileName);
    RaytmxObjectTemplate objectTemplate = LoadTX(fullPath);
    if (!objectTemplate.isSuccess) { /* If loading the template failed */
        TraceLog(LOG_ERROR, "RAYTMX: Unable to load template \"%s\"", fullPath);
        return NULL;
    }

    /* Create a new node in the list of known templates */
    cachedTemplateNode = (RaytmxCachedTemplateNode*)MemAllocZero(sizeof(RaytmxCachedTemplateNode));
    cachedTemplateNode->fileName = (char*)MemAllocZero((unsigned int)strlen(fileName) + 1);
    StringCopy(cachedTemplateNode->fileName, fileName);
    cachedTemplateNode->objectTemplate = objectTemplate;

    if (objectTemplate.hasTileset) { /* If the template contains a tileset in addition to an object */
        /* In cases where the template's object references a tile (i.e. its 'gid' attribute is set), the template */
        /* will have at most one tileset. Search the state object's list of tilesets and add this one if it's new. */
        RaytmxTilesetNode* tilesetsIterator = raytmxState->tilesetsRoot;
        bool isNew = true;
        while (tilesetsIterator != NULL) {
            /* If the existing tileset has a name, the template's tileset has a name, and they match OR if the */
            /* existing tileset has a source, template's tileset has a source, and they match */
            /* TODO: The comparison of sources without respect to directory should be reviewed as it could result in */
            /* false negatives and duplicate tileset instances */
            if ((tilesetsIterator->tileset.name != NULL && objectTemplate.tileset.name != NULL &&
                    strcmp(tilesetsIterator->tileset.name, objectTemplate.tileset.name) == 0) ||
                    (tilesetsIterator->tileset.source != NULL && objectTemplate.tileset.source != NULL &&
                    strcmp(tilesetsIterator->tileset.source, objectTemplate.tileset.source) == 0)) {
                isNew = false;
                break;
            }
        }
        if (isNew) {
            TmxTileset* tileset = AddTileset(raytmxState);
            *tileset = objectTemplate.tileset;
        }
    }

    /* Add to the cache */
    if (raytmxState->templatesRoot == NULL)
        raytmxState->templatesRoot = cachedTemplateNode;
    else {
        RaytmxCachedTemplateNode* cachedTemplateIterator = raytmxState->templatesRoot;
        while (cachedTemplateIterator->next != NULL)
            cachedTemplateIterator = cachedTemplateIterator->next;
        cachedTemplateIterator->next = cachedTemplateNode;
    }

    return cachedTemplateNode;
}

Color GetColorFromHexString(const char* hex) {
    Color color = BLACK; /* #define'd by raylib as { 0, 0, 0, 255 } */

    size_t length = strlen(hex);
    if (length < 6) /* If the hex string is too short to contain at least R, G, and B components */
        return color;

    /* Hex strings are in the form #AARRGGBB or #RRGGBB meaning alpha is optional. To avoid any special logic for the */
    /* two cases where alpha is or isn't given, parsing will just be done backwards. Here, 'hex' is used as an */
    /* iterator so it will begin at the end of the string. For example, for "#789abc" this begins at 'c'. */
    hex += length - 1;

    char component[] = "\0\0\0"; /* Used to hold the two-digit component (e.g. "55" or "ff") */
    for (size_t i = 0; i < length / 2; i++) { /* Iterate three or four times with four meaning alpha was included */
        component[1] = hex[0]; /* Store the char 'hex' points to. For "#789abc" this is 'c', 'a', or '8'. */
        hex--; /* Point to the previous char in the string. For "#789abc" this now points to 'b', '9' or '7'. */
        component[0] = hex[0]; /* Store the other char of the current component */
        hex--; /* Iterate backwards again in preparation for the next component */
        /* For "#789abc" the 'component' array is now "bc", "9a", or "78" */
        switch (i) {
        case 0: color.b = (unsigned char)strtoul(component, NULL, 16); break; /* e.g. "ff" -> 255 for blue */
        case 1: color.g = (unsigned char)strtoul(component, NULL, 16); break; /* ...for green */
        case 2: color.r = (unsigned char)strtoul(component, NULL, 16); break; /* ...for red */
        case 3: color.a = (unsigned char)strtoul(component, NULL, 16); break; /* ...for alpha, maybe */
        }
    }

    return color;
}

uint32_t GetGid(uint32_t rawGid, bool* isFlippedHorizontally, bool* isFlippedVertically, bool* isFlippedDiagonally,
        bool* isRotatedHexagonal120) {
    /* If the output parameters can be written to, output the status of the flip flags */
    if (isFlippedHorizontally != NULL)
        *isFlippedHorizontally = rawGid & FLIP_FLAG_HORIZONTAL;
    if (isFlippedVertically != NULL)
        *isFlippedVertically = rawGid & FLIP_FLAG_VERTICAL;
    if (isFlippedDiagonally != NULL)
        *isFlippedDiagonally = rawGid & FLIP_FLAG_DIAGONAL;
    if (isRotatedHexagonal120 != NULL)
        *isRotatedHexagonal120 = rawGid & FLIP_FLAG_ROTATE_120;

    /* Return the integer with flip flags removed */
    return rawGid & ~(FLIP_FLAG_HORIZONTAL | FLIP_FLAG_VERTICAL | FLIP_FLAG_DIAGONAL | FLIP_FLAG_ROTATE_120);
}

void* MemAllocZero(unsigned int size) {
    void* buffer = MemAlloc(size); /* Reserve 'size' bytes of memory */
    memset(buffer, 0, size); /* Initialize any values to zero, NULL, false, or an equivalent enum value */
    return buffer;
}

/* "Get directory for a given filePath" */
/* raylib's GetDirectoryPath() doesn't work as described so this is used in its place */
char* GetDirectoryPath2(const char* filePath) {
    static char directoryPath[260]; /* Max path length on Windows, the bottleneck, is 260 characters */
    memset(directoryPath, '\0', 260);
    size_t length = strlen(filePath);
    /* Paths beginning with a Windows drive letter (C:\, D:\, etc.) or beginning with a slash are absolute paths */
    if (length >= 2 && (filePath[1] == ':' || filePath[0] == '\\' || filePath[0] == '/')) { /* If absolute */
        if (IsPathFile(filePath)) /* If filePath points to a file, and we already know it's absolute */
            StringCopy(directoryPath, filePath);
        else { /* If filePath points to a directory, and we already know it's absolute */
            StringCopy(directoryPath, filePath);
            return directoryPath;
        }
    } else /* If filePath is relative */
        StringCopy(directoryPath, JoinPath(GetWorkingDirectory(), filePath));

    /* The goal is to return part of filePath, up to the last slash */
    length = strlen(directoryPath);
    char* iterator = directoryPath + length - 1;
    /* Iterate backwards until a slash is found */
    while (iterator != directoryPath && *iterator != '\0' && *iterator != '\\' && *iterator != '/')
        iterator -= 1;
    *(iterator + 1) = '\0'; /* Place a null terminator after the slash to effectively end the string there */
    return directoryPath;
}

char* JoinPath(const char* prefix, const char* suffix) {
    static char joinedPath[260]; /* Max path length on Windows, the bottleneck, is 260 characters */
    memset(joinedPath, '\0', 260);
    StringCopy(joinedPath, prefix);
    size_t prefixLength = strlen(prefix);
    if ((prefixLength >= 1) && (joinedPath[prefixLength - 1] != '/') && (joinedPath[prefixLength - 1] != '\\'))
#ifdef _WIN32
        joinedPath[prefixLength] = '\\'; /* Append the path with a '\\' separator */
#else
        joinedPath[prefixLength] = '/'; /* Append the path with a '/' separator */
#endif
    const char* suffixStart = suffix;
    size_t suffixLength = strlen(suffix);
    if (suffixLength >= 2 && suffix[0] == '.' && (suffix[1] == '/' || suffix[1] == '\\'))
        suffixStart += 2; /* Skip over the "this directory" part (e.g. "./a.tsx" -> "a.tsx") */
    /* Note: ".." is kept in the joined path intentionally although it is possible to exceed 260 characters. TODO? */
    StringConcatenate(joinedPath, suffixStart);
    return joinedPath;
}

void StringCopy(char* destination, const char* source) {
#if (!defined _MSC_VER || defined _CRT_SECURE_NO_WARNINGS)
    /* This is for build environments where "[M]icro[S]oft [C]ompiler [VER]sion" is not defined, meaning the compiler */
    /* is one other than MSVC, or where MSVC is being used but the security deprecation warning has been disabled. */
    /* MSVC does not consider strcpy() to be secure because it's somewhat unbounded. In practice, it's fine. */
    strcpy(destination, source);
#else
    /* This keeps MSVC happy by providing the supposed size of the destination buffer. However, these buffers are */
    /* dynamically allocated so there's no quick way to get the actual size. The best solution is to use strlen() */
    /* but this makes the function O(2n) vs. the alternative's O(n). */
    strcpy_s(destination, strlen(source) + 1, source); /* + 1 for the terminator */
#endif
}

void StringCopyN(char* destination, const char* source, size_t number) {
#if (!defined _MSC_VER || defined _CRT_SECURE_NO_WARNINGS)
    strncpy(destination, source, number);
#else
    strncpy_s(destination, number + 1, source, number);
#endif
}

void StringConcatenate(char* destination, const char* source) {
#if (!defined _MSC_VER || defined _CRT_SECURE_NO_WARNINGS)
    strcat(destination, source);
#else
    strcat_s(destination, strlen(destination) + strlen(source) + 1, source);
#endif
}

#endif /* RAYTMX_IMPLEMENTATION */

#endif /* RAYTMX_H */

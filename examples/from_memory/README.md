# Load from memory example

This program demonstrates raytmx's ability to load a TMX map from an in-memory string and, optionally, in-memory TSX
tilesets, TX object templates, and images. A preprocessor deftermines if only the TMX document is loaded from memory or
if everything is: SPECIFY_WORKING_DIRECTORY. When _true_, the program only loads the TMX from directory while specifying
a working directory that on-disk files are relativev to. When _false_, the program loads everyhing from memory.
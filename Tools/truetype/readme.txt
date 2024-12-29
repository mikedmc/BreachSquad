MIHAI KHG: it's an old system of ours which supports both pre-rendered bitmap fonts and ttf/otf fonts. You should only use the FontHybrid.cpp/h files

-then fonthybrid.cpp loads the glyphs for a certain pixel size. FreeType returns a monochrome bitmap glyph. You then write that to a texture (in our case we prealloc a fixed size texture, eg 1024x512, then dynamically increase the height if needed for more letters).
-each letter is kept in a map, with information regarding its position into the texture atlas and size stuff

there is a lot of unused code there, like the ability to save the atlas to a texture and load it later from there (not needed, it works very well and fast without caching)

or custom kerning, which some fonts might have, but none of the fonts we actually used had custom kerning
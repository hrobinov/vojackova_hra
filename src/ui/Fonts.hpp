// The game's typeface, shared by the HUD and the Agui widgets.
//
// Titillium Web, the face Factorio's GUI is set in, in its three weights. The
// .ttf files are compiled into the exe (see fastbuild/fbuild.bff; the licence
// is resources/fonts/OFL.txt) and rasterised once per pixel size, so text is
// crisp at every size instead of scaled up from one.

#pragma once

#include <raylib.h>

namespace ui {

enum class Weight { Regular, SemiBold, Bold };

// raylib sizes a font by its whole line, ascender to descender; style.lua's
// font sizes are the em, which for Titillium is a good deal less. The line a
// style.lua font of `size` needs.
int LineHeight(int size);

// The font rasterised at `px` pixels tall. Loaded on first use; needs the window.
const ::Font& FontAt(int px, Weight weight = Weight::Regular);

// Extra px between characters at `px`: none, the face's own spacing is right.
float FontSpacing(int px);

// Drop-in replacements for raylib's DrawText / MeasureText.
void Text(const char* text, int x, int y, int px, ::Color color);
int  TextWidth(const char* text, int px);

// Frees every size loaded so far. Call before CloseWindow().
void UnloadFonts();

}  // namespace ui

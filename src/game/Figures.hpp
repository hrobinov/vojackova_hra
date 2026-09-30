// The soldier and the zombies, seen from above but in the round: models of
// solids (see Relief) -- helmet, shoulders, arms, the rifle; the zombies'
// heads, rags and grasping hands -- worked out once, when first drawn, into
// a picture of their colours and one of which way their surfaces face. A
// shader lights them as they turn, from the game's one light, so the light
// always falls the same way whichever way they face; and each casts a soft
// shadow on the grass.
//
// Each has a second model lying dead, on its back, arms flung out -- the
// soldier's helmet rolled away -- which it falls into.

#pragma once

#include <game/Goban.hpp>

#include <raylib.h>

namespace figures {

// A figure is drawn this many squares across, its arms and rifle reaching
// out of its square.
constexpr float SIZE = 1.95f;

// The soldier and a zombie in the middle of a square `square` pixels
// across, facing `facing`, fallen from 0 (standing) to 1 (lying dead), the
// zombie's body fading from 1 to 0 as it goes.
void DrawSoldier(Vector2 at, float square, Vector2 facing, float fallen);
void DrawZombie(Vector2 at, float square, Vector2 facing, const Goban::Zombie& zombie, float fallen, float fade);

// Works the models out now, rather than the first time they are drawn --
// it takes a moment.
void Load();

// Frees the models' textures and shader, while the window is still open.
void Unload();

}  // namespace figures

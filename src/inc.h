
#pragma once

#include <imgui.h>

#include "stb.h"
#include "linalg.h"
#include "linalgop.h"
#include "linalgex.h"
#include "imgui_debug.h"

#define OWNER
#define ARRAY
#define HASHTABLE

#define DEBUG_BREAK __debugbreak
#define DEBUG_BREAK_IF(x) if (x) { __debugbreak(); } else { }

#define PI 3.1415926535897932384f
#define TWOPI 6.28318530717958647692f
#define HALFPI 1.57079632679489661923132f

#define stepf(f) (f < 1.0f ? 0.0f : 1.0f)

#define stb_arr_find_fastdelete(a,v)  \
   for (int __index = 0; __index < stb_arr_len(a); ++__index) if (a[__index] == v) { stb_arr_fastdelete(a, __index); break; }

using namespace linalg;

struct Game;
struct Team;
struct Group;
struct Unit;
struct UnitData;
struct UnitVisual;
struct Combat;
struct Grid;
struct Cell;
struct Touch;

typedef int TeamIndex;
typedef int GroupIndex;
typedef int MemberIndex;
typedef int UnitIndex;

extern Game game;

static const int InvalidTeamIndex = -1;
static const int InvalidGroupIndex = -1;
static const int InvalidMemberIndex = -1;
static const int InvalidUnitIndex = -1;

#include "settings.h"
#include "game.h"
#include "team.h"
#include "group.h"
#include "unit.h"
#include "grid.h"
#include "touch.h"
#include "combat.h"


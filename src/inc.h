
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

#define PI 3.1415926535897932384f
#define TWOPI 6.28318530717958647692f
#define HALFPI 1.57079632679489661923132f

using namespace linalg;

struct Game;
struct Grid;
struct Unit;
struct UnitData;
struct UnitVisual;
struct Cell;
struct Touch;
struct Group;

typedef int TeamIndex;
typedef int GroupIndex;
typedef int MemberIndex;
typedef int UnitIndex;

static const int InvalidTeamIndex = -1;
static const int InvalidGroupIndex = -1;
static const int InvalidMemberIndex = -1;
static const int InvalidUnitIndex = -1;

#include "unit.h"
#include "game.h"
#include "grid.h"
#include "group.h"
#include "touch.h"


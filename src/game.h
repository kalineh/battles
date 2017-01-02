
#include <imgui.h>

#include "linalg.h"
#include "stb.h"

using namespace linalg;

struct UnitData
{
	float radius;
	float mass;
	float accel;
	float armor;
	float health;
	float fatigue;
	float resolve;
	bool flyer;
};

struct UnitVisual
{
	v4 color;
};

struct Unit
{
	const UnitData* data;

	v3 pos;
	v2 dir;
	v2 vel;

	float health;
	float fatigue;
	float resolve;
};

struct Game
{
	void Init();
	void Release();

	void Render();
	void Update();

	Unit* units;
};
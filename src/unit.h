
#include "inc.h"

struct UnitData
{
	const char* type;
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

typedef int UnitID;

struct Unit
{
	static Unit CreateTestUnit();

	const UnitData* data;
	const UnitVisual* visual;

	v3 pos;
	v2 dir;
	v3 vel;

	float health;
	float fatigue;
	float resolve;
};

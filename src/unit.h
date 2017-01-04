
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

	void AI();
	void Update();

	const UnitData* data;
	const UnitVisual* visual;

	int team;

	v2 pos;
	float height;
	float angle;
	v2 vel;

	float health;
	float fatigue;
	float resolve;

	v2 targetPos;
	float targetAngle;
};

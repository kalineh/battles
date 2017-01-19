
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

struct UnitCombat
{
	float attack;
	float defense;
};

typedef int UnitIndex;
static const int InvalidUnitIndex = 0;

struct Unit
{
	static Unit CreateUnit(UnitData* data, UnitVisual* visual, UnitCombat* combat);
	static Unit CreateUnit(const char* type);

	void AI();
	void Update();

	bool IsValid();
	bool IsAlive();

	void ResolveTouch(Unit* unit);
	void ResolveCombat(Unit* unit);

	const UnitData* data;
	const UnitVisual* visual;
	const UnitCombat* combat;

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

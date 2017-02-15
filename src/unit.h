
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

struct Unit
{
	static Unit CreateUnit(UnitData* data, UnitVisual* visual, UnitCombat* combat);
	static Unit CreateUnit(const char* type);

	void AI();
	void Update();

	bool IsValid();
	bool IsAlive();

	void ResolveTouchFriendly(Unit* unit);
	void ResolveTouchHostile(Unit* unit);

	const UnitData* data;
	const UnitVisual* visual;
	const UnitCombat* combat;

	int team;
	int group;

	v2 pos;
	float height;
	float angle;
	v2 vel;

	float health;
	float fatigue;
	float resolve;
	float attacking;
	float reload;
	float footing;
	float charging;
	float bunching;

	v2 targetPos;
	float targetAngle;
};

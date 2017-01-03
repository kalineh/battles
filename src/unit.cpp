
#include "inc.h"

Unit Unit::CreateTestUnit()
{
	static UnitData data = {
		"TestType",
		1.0f,
		5.0f,
		2.0f,
		10.0f,
		100.0f,
		100.0f,
		100.0f,
		false,
	};

	Unit unit;
	
	unit.data = &data;
	unit.pos = v3zero();
	unit.dir = v2zero();
	unit.vel = v3zero();
	unit.health = data.health;
	unit.fatigue = 0.0f;
	unit.resolve = data.resolve;

	return unit;
}

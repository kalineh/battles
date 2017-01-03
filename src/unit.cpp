
#include "inc.h"

Unit Unit::CreateTestUnit()
{
	static UnitData data = {
		"TestType", // type
		10.0f, // radius
		5.0f, // mass
		2.0f, // accel
		10.0f, // armor
		100.0f, // health
		100.0f, // fatigue
		100.0f, // resolve
		false, // flyer
	};

	static UnitVisual visual = {
		v4rgb1(1,1,1),
	};

	Unit unit;
	
	unit.data = &data;
	unit.visual = &visual;

	unit.pos = v3zero();
	unit.dir = v2zero();
	unit.vel = v3zero();
	unit.health = data.health;
	unit.fatigue = 0.0f;
	unit.resolve = data.resolve;

	return unit;
}

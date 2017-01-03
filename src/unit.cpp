
#include "inc.h"

Unit Unit::CreateTestUnit()
{
	static UnitData data = {
		"TestType", // type
		10.0f, // radius
		5.0f, // mass
		125.0f, // accel
		10.0f, // armor
		100.0f, // health
		100.0f, // fatigue
		100.0f, // resolve
		false, // flyer
	};

	static UnitVisual visuals[2] = {
		{ v4rgb1(1,0,0), },
		{ v4rgb1(0,0,1), },
	};

	Unit unit;

	unit.team = stb_rand() % 2;

	unit.data = &data;
	unit.visual = &visuals[unit.team];

	unit.pos = v2zero();
	unit.height = 0.0f;
	unit.angle = 0.0f;
	unit.vel = v2zero();
	unit.health = data.health;
	unit.fatigue = 0.0f;
	unit.resolve = data.resolve;
	unit.target = v2zero();

	return unit;
}

void Unit::AI()
{
	if ((stb_rand() % 5000) == 0)		
	{
		target = v2new(
			pos.x + (float)(stb_frand() * (float)100.0f),
			pos.y + (float)(stb_frand() * (float)100.0f)
		);
	}
}

void Unit::Update()
{
	const float dt = 1.0f / 60.0f;
	const float friction = 0.25f;
	const float brake = 15.0f;
	const float snap = 1.5f;

	v2 targetOfs = target - pos;
	v2 targetDir = v2unitsafe(targetOfs);

	float targetAngle = v2toangle(targetDir);
	float targetAngleLen = anglediff(angle, targetAngle);
	float targetAngleApproach = fminf(targetAngleLen, 0.5f) / 0.5f;

	angle = angleto(angle, targetAngle, PI * dt * targetAngleApproach);

	float targetLen = v2lensafe(targetOfs);
	float targetApproach = fminf(targetLen, 2.5f) / 2.5f;

	v2 approachDir = v2fromangle(angle) * targetApproach;
	v2 brakeDir = approachDir * -1.0f;

	if (targetLen < snap)
		approachDir = v2zero();

	vel += approachDir * data->accel / data->mass * dt * targetApproach;
	vel += brakeDir * data->accel / data->mass * dt * (1.0f - targetApproach) * brake;
	vel -= vel * friction * data->mass * dt;
	pos += vel * dt;
}

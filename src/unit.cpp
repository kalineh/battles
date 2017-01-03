
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
	//target = v2new(100,100);
	target = v2new(ImGui::GetMousePos().x, ImGui::GetMousePos().y);
	IMGUI_DEBUG(target);
	return;

	if ((stb_rand() % 50) == 0)		
	{
		target = v2new(
			pos.x + (float)(stb_frand() * (float)100.0f),
			pos.y + (float)(stb_frand() * (float)100.0f)
		);
	}
}

void Unit::Update()
{
	// face to
	// move to
	// face to

	const float dt = 1.0f / 60.0f;
	const float rotationRate = 0.5f;
	const float friction = 0.25f;
	const float brake = 0.25f;

	v2 targetOfs = target - pos;
	v2 targetDir = v2unitsafe(targetOfs);

	float targetAngle = v2toangle(targetDir);
	float targetAngleDiff = anglediff(angle, targetAngle);
	float targetAngleApproach = fminf(fabsf(targetAngleDiff), 0.5f) / 0.5f;

	targetAngleApproach = fmaxf(targetAngleApproach, 0.25f);

	angle = angleto(angle, targetAngle, PI * dt * rotationRate * targetAngleApproach);

	float targetLen = v2lensafe(targetOfs);
	float targetApproach = fminf(targetLen, 10.0f) / 10.0f;
	float targetApproachBrake = 1.0f - targetApproach;
	float velLen = v2lensafe(vel);

	targetApproach = fmaxf(targetApproach, 0.25f);

	v2 approachDir = v2fromangle(angle) * targetApproach;
	v2 brakeDir = approachDir * -1.0f;

	if (ImGui::IsMouseDown(0))
	{
		vel += approachDir * data->accel / data->mass * dt * targetApproach;
		vel += brakeDir * data->accel / data->mass * dt * targetApproachBrake * brake * velLen;
		vel -= vel * friction * data->mass * dt;
		pos += vel * dt;
	}
}

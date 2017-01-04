
#include "inc.h"

Unit Unit::CreateTestUnit()
{
	static UnitData data = {
		"TestType", // type
		10.0f, // radius
		5.0f, // mass
		250.0f, // accel
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
	unit.targetPos = v2zero();
	unit.targetAngle = 0.0f;

	return unit;
}

void Unit::AI()
{
	if (ImGui::IsMouseDown(1))
		targetPos = v2new(ImGui::GetMousePos().x, ImGui::GetMousePos().y);

	if ((stb_rand() % 5000) == 0)		
	{
		targetPos = v2new(
			pos.x + (float)(stb_frand() * (float)100.0f),
			pos.y + (float)(stb_frand() * (float)100.0f)
		);
	}
}

void Unit::Update()
{
	const float dt = 1.0f / 60.0f;
	const float rotationRate = 1.5f;
	const float friction = 0.25f;
	const float brake = 0.25f;
	const float arrive = 0.05f;

	v2 targetOfs = targetPos - pos;
	v2 targetDir = v2unitsafe(targetOfs);

	float targetLen = v2lensafe(targetOfs);

	if (targetLen > arrive)
	{
		float targetPosAngle = v2toangle(targetDir);
		float targetPosAngleDiff = anglediff(angle, targetPosAngle);
		float targetPosAngleApproach = fminf(fabsf(targetPosAngleDiff), 0.5f) / 0.5f;

		targetPosAngleApproach = fmaxf(targetPosAngleApproach, 0.25f);

		angle = angleto(angle, targetPosAngle, PI * dt * rotationRate * targetPosAngleApproach);
	}
	else
	{
		float targetAngleDiff = anglediff(angle, targetAngle);
		float targetAngleApproach = fminf(fabsf(targetAngleDiff), 0.5f) / 0.5f;

		targetAngleApproach = fmaxf(targetAngleApproach, 0.25f);

		angle = angleto(angle, targetAngle, PI * dt * rotationRate * targetAngleApproach);
	}

	float targetApproach = fminf(targetLen, 10.0f) / 10.0f;
	float targetApproachBrake = 1.0f - targetApproach;
	float velLen = v2lensafe(vel);

	targetApproach = fmaxf(targetApproach, 0.25f);

	v2 approachDir = v2fromangle(angle) * targetApproach;
	v2 brakeDir = approachDir * -1.0f;

	vel += approachDir * data->accel / data->mass * dt * targetApproach;
	vel += brakeDir * data->accel / data->mass * dt * targetApproachBrake * brake * velLen;
	vel -= vel * friction * data->mass * dt;
	pos += vel * dt;
}

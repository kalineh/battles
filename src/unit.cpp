#include "inc.h"

static UnitData unitDataNull = {
	"Null",
};

static UnitData unitDataTest = {
	"Test", // type
	5.0, // radius
	5.0f, // mass
	250.0f, // accel
	10.0f, // armor
	100.0f, // health
	100.0f, // fatigue
	100.0f, // resolve
	false, // flyer
};

static UnitData unitDataLight = {
	"Light", // type
	5.0f, // radius
	5.0f, // mass
	125.0f, // accel
	10.0f, // armor
	100.0f, // health
	100.0f, // fatigue
	100.0f, // resolve
	false, // flyer
};

static UnitData unitDataHeavy = {
	"Heavy", // type
	10.0f, // radius
	20.0f, // mass
	200.0f, // accel
	10.0f, // armor
	100.0f, // health
	100.0f, // fatigue
	100.0f, // resolve
	false, // flyer
};

static UnitVisual unitVisualNull = {
	v4rgb1(0,0,0),
};

static UnitVisual unitVisualTest = {
	v4rgb1(1,1,1),
};

static UnitVisual unitVisualLight = {
	v4rgb1(0,1,0),
};

static UnitVisual unitVisualHeavy = {
	v4rgb1(0,0,1),
};

static UnitCombat unitCombatNull = {
	0.0f,
	0.0f,
};

static UnitCombat unitCombatTest = {
	1.0f,
	0.0f,
};

static UnitCombat unitCombatLight = {
	5.0f,
	0.5f,
};

static UnitCombat unitCombatHeavy = {
	2.5f,
	1.0f,
};

Unit Unit::CreateUnit(UnitData* data, UnitVisual* visual, UnitCombat* combat)
{
	Unit unit;

	memset(&unit, 0, sizeof(unit));

	unit.team = InvalidTeamIndex;
	unit.group = InvalidGroupIndex;

	unit.data = data;
	unit.visual = visual;
	unit.combat = combat;

	unit.health = data->health;
	unit.resolve = data->resolve;

	return unit;
}

Unit Unit::CreateUnit(const char* type)
{
	if (type != NULL)
	{
		if (strcmp(type, "Test") == 0)
			return CreateUnit(&unitDataTest, &unitVisualTest, &unitCombatTest);
		else if (strcmp(type, "Light") == 0)
			return CreateUnit(&unitDataLight, &unitVisualLight, &unitCombatLight);
		else if (strcmp(type, "Heavy") == 0)
			return CreateUnit(&unitDataHeavy, &unitVisualHeavy, &unitCombatHeavy);
	}

	return CreateUnit(&unitDataNull, &unitVisualNull, &unitCombatNull);
}

void Unit::AI()
{
}

void Unit::Update()
{
	const float dt = 1.0f / 60.0f;
	const float radius = data->radius;

	const float rotationRate = 2.0f;
	const float frictionForce = 0.25f;
	const float brakeForce = 2.5f;
	const float arriveRange = 0.5f * radius;
	const float overshootForce = 0.1f;

	v2 targetOfs = targetPos - pos;
	v2 targetDir = v2unitsafe(targetOfs);
	v2 facingDir = v2fromangle(angle);
	v2 movingDir = v2unitsafe(vel);

	float targetLen = v2lensafe(targetOfs);
	float movingLen = v2lensafe(vel);

	float arriveFactor = 1.0f - fminf(targetLen / arriveRange, 1.0f);
	float travelFactor = 1.0f - arriveFactor;

	float targetPosAngle = v2toangle(targetDir);
	float targetPosAngleDiff = anglediff(angle, targetPosAngle);
	float targetPosAngleApproach = fminf(fabsf(targetPosAngleDiff), 0.5f) / 0.5f;

	targetPosAngleApproach = fmaxf(targetPosAngleApproach, 0.25f);

	angle = angleto(angle, targetPosAngle, PI * dt * rotationRate * targetPosAngleApproach * travelFactor);

	float targetAngleDiff = anglediff(angle, targetAngle);
	float targetAngleApproach = fminf(fabsf(targetAngleDiff), 0.5f) / 0.5f;

	targetAngleApproach = fmaxf(targetAngleApproach, 0.25f);

	angle = angleto(angle, targetAngle, PI * dt * rotationRate * targetAngleApproach * arriveFactor);

	float velAngle = v2toangle(movingDir);
	float targetAngleVelAdjust = anglediff(velAngle, targetPosAngle);
	float targetAngleVelAdjustFixed = fminf(fabsf(targetAngleVelAdjust), PI * 0.25f) * (targetAngleVelAdjust < 0.0f ? -1.0f : 1.0f);

	angle = angleto(angle, targetPosAngle + targetAngleVelAdjustFixed, overshootForce * movingLen * dt);

	float targetApproachRange = 15.0f + movingLen;
	float targetApproach = fminf(targetLen, targetApproachRange) / targetApproachRange;
	float targetApproachBrake = 1.0f - targetApproach;

	targetApproach = fminf(targetApproach, 1.0f);

	vel += targetDir * data->accel * dt * arriveFactor;
	vel += facingDir * data->accel * dt * targetApproach * travelFactor;
	vel -= movingDir * data->accel * dt * targetApproachBrake * brakeForce * arriveFactor;
	vel -= vel * stb_min(frictionForce * data->mass * dt, 1.0f);
	pos += vel * dt;
}

bool Unit::IsValid()
{
	return team != InvalidTeamIndex;
}

bool Unit::IsAlive()
{
	if (health <= 0.0f)
		return false;

	return true;
}

void Unit::ResolveTouch(Unit* unit)
{
	const float dt = 1.0f / 60.0f;
	const float ejectRate = 100.0f;
	const float pushRate = 1.5f;
	const float massExponent = 1.25f;

	v2 dir;
	float len;

	const v2 ofs = unit->pos - pos;
	v2unitlensafe(ofs, &dir, &len);

	const float rads = unit->data->radius + data->radius;
	const float intersect = -(len - rads);
	const float ratio = powf(data->mass / (data->mass + unit->data->mass), massExponent);

	unit->vel += dir * intersect * dt * ejectRate * ratio;

	const v2 transfer = vel * dt * pushRate * ratio;
	const v2 transferAligned = v2projsafe(transfer, dir);

	unit->vel += transferAligned;
	vel -= transferAligned;
}

void Unit::ResolveCombat(Unit* unit)
{
	const float dt = 1.0f / 60.0f;
	float damage = combat->attack;
	damage -= fmaxf(unit->combat->defense, 0.0f);
	health = fmaxf(health - damage * dt, 0.0f);
	unit->vel -= unit->vel * 10.0f * dt;
	targetPos = v2moveto(targetPos, unit->pos, 15.0f * dt);

	// behind = more dmg
	// reduce vel
	// wraparound group
	// retreat
}

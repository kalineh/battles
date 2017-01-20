
#include "inc.h"

int SortFurthestV2FromCentroidWithDisplacementCompare(void* context, const void* lhs, const void* rhs)
{
	Group* group = (Group*)context;
	v2* lhsPos = (v2*)lhs;
	v2* rhsPos = (v2*)rhs;
	v2 centroid = group->CalcCentroid();
	v2 bias = group->displacementAggregate * -1.0f;
	v2 center = centroid + bias;
	v2 lhsOfs = *lhsPos - center;
	v2 rhsOfs = *rhsPos - center;
	float lhsDistSq = v2lensq(lhsOfs);
	float rhsDistSq = v2lensq(rhsOfs);

	if (lhsDistSq < rhsDistSq)
		return -1;
	if (lhsDistSq > rhsDistSq)
		return +1;

	return 0;
}

struct SlotData
{
	v2 pos;
	float dist;
};

void Group::Init(Unit* ARRAY aunits)
{
	formationType = FormationType_None;
	groupPos = v2zero();
	commandPos = v2zero();
	commandAngle = 0.0f;
	disarrayRatio = 0.0f;
	displacementAggregate = v2zero();
	formationRatio = 0.5f;
	formationLoose = 0.0f;
	units = aunits;
	members = NULL;
	stb_arr_setsize(members, 8);
	slots = NULL;
	stb_arr_setsize(slots, 8);
}

void Group::Release()
{
	stb_arr_free(members);
	stb_arr_free(slots);
}

void Group::Update()
{
	const float dt = 1.0f / 60.0f;

	v2 centroid = CalcCentroid();
	v2 toCentroid = centroid - groupPos;
	v2 toCommand = commandPos - groupPos;
	v2 toCommandDir = v2unitsafe(toCommand);

	float movementSpeed = CalcUnitSlowestMovement();
	float disarrayFactor = 1.0f - stb_clamp(disarrayRatio - 1.5f, 0.0f, 1.0f);

	float toCentroidSpeed = movementSpeed * disarrayFactor * 0.01f;
	float toCommandSpeed = movementSpeed * disarrayFactor;

	// pull group toward centroid
	groupPos += toCentroid * dt * toCentroidSpeed;
	groupPos += toCommandDir * dt * toCommandSpeed;

	UpdateFormation();
}

void Group::UpdateFormation()
{
	float largestUnitRadius = CalcUnitLargestRadius();
	int aliveUnitCount = CalcUnitAliveCount();
	int aliveMemberCounter = 0;
	
	stb_arr_setlen(slots, aliveUnitCount);
	for (int i = 0; i < stb_arr_len(slots); ++i)
		slots[i] = InvalidUnitIndex;

	UnitIndex* ARRAY searchPool = NULL;
	stb_arr_setlen(searchPool, aliveUnitCount);
	int searchPoolCursor = 0;

	// collect valid members for slots
	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		searchPool[searchPoolCursor] = unitIndex;
		searchPoolCursor++;
	}

	v2* slotTargetPositions = NULL;
	stb_arr_setlen(slotTargetPositions, stb_arr_len(slots));
	for (int i = 0; i < stb_arr_len(slotTargetPositions); ++i)
	{
		switch (formationType)
		{
		case FormationType_None:
			slotTargetPositions[i] = (units + searchPool[i])->pos;
			break;

		case FormationType_Box:
			slotTargetPositions[i] = FormationPositionBox(i, groupPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			break;

		case FormationType_Wedge:
			slotTargetPositions[i] = FormationPositionWedge(i, groupPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			break;

		case FormationType_Circle:
			slotTargetPositions[i] = FormationPositionCircle(i, groupPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			break;
		}
	}
	qsort_s(slotTargetPositions, stb_arr_len(slotTargetPositions), sizeof(slotTargetPositions[0]), &SortFurthestV2FromCentroidWithDisplacementCompare, (void*)this);

	// find best unit for each slot (nearest)
	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		v2 slotTargetPos = slotTargetPositions[i];
		float bestDistance = FLT_MAX;
		UnitIndex bestIndex = InvalidUnitIndex;
		int bestIndexPoolIndex = -1;

		for (int j = 0; j < stb_arr_len(searchPool); ++j)
		{
			UnitIndex poolUnitIndex = searchPool[j];
			if (poolUnitIndex == InvalidUnitIndex)
				continue;

			Unit* unit = units + poolUnitIndex;

			if (!unit->IsValid())
				continue;
			if (!unit->IsAlive())
				continue;

			v2 ofs = unit->pos - slotTargetPos;
			float lensq = v2lensq(ofs);
			if (lensq < bestDistance)
			{
				bestDistance = lensq;
				bestIndex = poolUnitIndex;
				bestIndexPoolIndex = j;
			}
		}

		slots[i] = bestIndex;
		searchPool[bestIndexPoolIndex] = InvalidUnitIndex;
	}

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];
		if (unitIndex == InvalidUnitIndex)
			continue;

		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		v2 unitTargetPos = slotTargetPositions[aliveMemberCounter];

		unit->targetPos = unitTargetPos;
		unit->targetAngle = commandAngle;

		aliveMemberCounter++;
	}

	float disarray = 0.0f;
	int disarrayCount = 0;

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		v2 unitTargetPos = slotTargetPositions[i];
		v2 unitTargetOfs = unit->pos - unitTargetPos;
		float unitTargetDisarray = v2lensafe(unitTargetOfs) / unit->data->radius;
		disarray += unitTargetDisarray;
		disarrayCount += 1;
	}

	disarrayRatio = disarray / fmaxf((float)disarrayCount, 1.0f);

	const float dt = 1.0f / 60.0f;

	displacementAggregate -= displacementAggregate * 1.5f * dt;

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		v2 unitTargetPos = slotTargetPositions[i];
		v2 unitTargetOfs = unit->pos - unitTargetPos;

		// ignore small displacements
		float lensq = v2lensq(unitTargetOfs);
		float ignore = (unit->data->radius * 1.25f) * (unit->data->radius * 1.25f);
		if (lensq < ignore)
			continue;

		displacementAggregate += unitTargetOfs * 0.5f * dt;
	}

	stb_arr_free(slotTargetPositions);
}

void Group::AddUnit(UnitIndex index)
{
	assert(index != InvalidUnitIndex);
	assert(index >= 0);
	stb_arr_push(members, index);
}

void Group::RemoveUnit(UnitIndex index)
{
	assert(index != InvalidUnitIndex);
	assert(index >= 0);
	stb_arr_fastdelete(members, index);
}

void Group::CommandStop()
{
	v2 centroid = v2zero();
	float count = 0.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		centroid += unit->pos;
		count += 1.0f;
	}

	if (count > 0.0f)
		commandPos = centroid / count;
}

void Group::CommandMoveTo(v2 pos, float angle)
{
	commandPos = pos;
	commandAngle = angle;
}

void Group::CommandTeleportTo(v2 pos, float angle)
{
	groupPos = pos;
	commandPos = pos;
	commandAngle = angle;
	disarrayRatio = 0.0f;
	displacementAggregate = v2zero();

	UpdateFormation();

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (unit->IsValid() && unit->IsAlive())
		{
			unit->pos = unit->targetPos;
			unit->angle = unit->targetAngle;
			unit->vel = v2zero();
		}
	}
}

void Group::CommandFormationNone()
{
	formationType = FormationType_None;
	formationRatio = 0.5f;
	formationLoose = 1.0f;
}

void Group::CommandFormationBox(float ratio, float loose)
{
	formationType = FormationType_Box;
	formationRatio = ratio;
	formationLoose = loose;
}

void Group::CommandFormationWedge()
{
	formationType = FormationType_Wedge;
	formationRatio = 0.5f;
	formationLoose = 1.0f;
}

void Group::CommandFormationCircle(float ratio, float loose)
{
	formationType = FormationType_Circle;
	formationRatio = ratio;
	formationLoose = loose;
}

MemberIndex Group::PositionToMemberIndexBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	v2 localPos = pos - groupCenter;
	v2 totalSize = v2new(
		cellsX * unitRadius * (1.0f + loose),
		cellsY * unitRadius * (1.0f + loose)
	);


	int cellX = (int)(localPos.x / cellsX);
	int cellY = (int)(localPos.y / cellsX);

	cellX = stb_max(cellX, 0);
	cellY = stb_max(cellY, 0);

	cellX = stb_min(cellX, cellsX - 1);
	cellY = stb_min(cellY, cellsY - 1);

	int index = cellX + cellY * cellsX;

	return index;
}

MemberIndex Group::PositionToMemberIndexWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	return 0;
}

v2 Group::FormationPositionBox(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	int cellX = index % cellsX;
	int cellY = index / cellsX;

	v2 totalSize = v2new(
		cellsX * unitRadius * (1.0f + loose),
		cellsY * unitRadius * (1.0f + loose)
	);

	v2 halfSize = totalSize * 0.5f;

	v2 pos = groupCenter - halfSize + v2new(
		totalSize.x / (float)cellsX * (float)cellX,
		totalSize.y / (float)cellsY * (float)cellY
	);

	return pos;
}

v2 Group::FormationPositionWedge(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	// how do we even calculate a wedge

	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	int cellX = index % cellsX;
	int cellY = index / cellsX;

	v2 totalSize = v2new(
		cellsX * unitRadius * (1.0f + loose),
		cellsY * unitRadius * (1.0f + loose)
	);

	v2 halfSize = totalSize * 0.5f;

	v2 pos = groupCenter - halfSize + v2new(
		totalSize.x / (float)cellsX * (float)cellX,
		totalSize.y / (float)cellsY * (float)cellY
	);

	return pos;

	return pos;
}

v2 Group::FormationPositionCircle(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	float step = TWOPI / (float)unitCount;
	float out = loose * unitRadius * unitCount * (1.0f / PI);
	float t = (float)index * step;
	v2 pos = v2new(cosf(t), sinf(t)) * out;
	return groupCenter + pos;
}

v2 Group::CalcCentroid()
{
	v2 sum = v2zero();
	int count = 0;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		sum += unit->pos;
		count += 1;
	}

	v2 centroid = v2zero();
	centroid = sum / fmaxf((float)count, 0.0f);

	return centroid;
}

float Group::CalcUnitSlowestMovement()
{
	float slowestSpeed = -1.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		const float unitSpeed = unit->data->accel / unit->data->mass;
		if (unitSpeed < slowestSpeed || slowestSpeed < 0.0f)
			slowestSpeed = unitSpeed;
	}

	if (slowestSpeed < 0.0f)
		slowestSpeed = 0.0f;

	return slowestSpeed;
}

float Group::CalcUnitLargestRadius()
{
	float radius = 0.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		if (unit->data->radius > radius)
			radius = unit->data->radius;
	}

	return radius;
}

int Group::CalcUnitAliveCount()
{
	int count = 0;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		count++;
	}

	return count;
}

float Group::CalcUnitAverageResolve()
{
	float resolveMax = 0.0f;
	float resolveCurrent = 0.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		resolveMax += unit->data->resolve;
		resolveCurrent += unit->resolve;
	}

	if (resolveMax <= 0.0f)
		return 0.0f;

	return resolveCurrent / resolveMax;
}

float Group::CalcUnitAverageHealth()
{
	float healthMax = 0.0f;
	float healthCurrent = 0.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		healthMax += unit->data->health;
		healthCurrent += unit->health;
	}

	if (healthMax <= 0.0f)
		return 0.0f;

	return healthCurrent / healthMax;
}

MemberIndex Group::FindNearestUnoccupied(MemberIndex queryMemberIndex)
{
	float bestDistance = FLT_MAX;
	MemberIndex bestIndex  = queryMemberIndex;
	int aliveUnitCount = CalcUnitAliveCount();
	int aliveMemberCursor = 0;
	float largestUnitRadius = CalcUnitLargestRadius();

	UnitIndex queryUnitIndex = members[queryMemberIndex];
	Unit* queryUnit = units + queryUnitIndex;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (i != queryMemberIndex)
			if (unit->IsValid() && unit->IsAlive())
				continue;

		switch (formationType)
		{
			case FormationType_None:
				break;

			case FormationType_Box:
			{
				v2 pos = FormationPositionBox(aliveMemberCursor, groupPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
				v2 ofs = queryUnit->pos - pos;
				float lensq = v2lensq(ofs);
				if (lensq < bestDistance)
				{
					bestDistance = lensq;
					bestIndex = i;
				}
				break;
			}

			case FormationType_Wedge:
				break;
		}

		aliveMemberCursor++;
	}

	return bestIndex;
}

UnitIndex Group::FindNearestUnit(v2 pos, UnitIndex* ARRAY source, UnitIndex failureIndex)
{
	float bestDistance = FLT_MAX;
	UnitIndex bestIndex = failureIndex;

	for (int i = 0; i < stb_arr_len(source); ++i)
	{
		UnitIndex unitIndex = source[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		v2 ofs = unit->pos - pos;
		float lensq = v2lensq(ofs);
		if (lensq < bestDistance)
		{
			bestDistance = lensq;
			bestIndex = unitIndex;
		}
	}

	return bestIndex;
}

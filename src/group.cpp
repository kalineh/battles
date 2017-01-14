
#include "inc.h"

void Group::Init(Unit* ARRAY aunits)
{
	formationType = FormationType_None;
	commandPos = v2zero();
	commandAngle = 0.0f;
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
}

void Group::Update()
{
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

	// units can request to other units that are closer? and request them to move?
	// objective is minimizing total movement
	// ie. ask unit, are you closer? can you move
	// ie. if we swap, is less overall distance?

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		v2 slotTargetPos = FormationPositionBox(i, commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);

		float bestDistance = FLT_MAX;
		UnitIndex bestIndex = InvalidUnitIndex;
		int bestIndexPoolIndex = -1;

		for (int j = 0; j < stb_arr_len(searchPool); ++j)
		{
			UnitIndex poolUnitIndex = searchPool[j];
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
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		switch (formationType)
		{
			case FormationType_None:
				unit->targetPos = commandPos;
				unit->targetAngle = commandAngle;
				break;

			case FormationType_Box:
			{
				v2 unitTargetPos = FormationPositionBox(aliveMemberCounter, commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);

				unit->targetPos = unitTargetPos;
				unit->targetAngle = commandAngle;

				break;
			}

			case FormationType_Wedge:
				// TODO
				unit->targetPos = commandPos;
				unit->targetAngle = commandAngle;
				break;
		}

		aliveMemberCounter++;
	}
}

void Group::AddUnit(UnitIndex index)
{
	assert(index != InvalidUnitIndex);
	assert(index > 0);
	stb_arr_push(members, index);
}

void Group::RemoveUnit(UnitIndex index)
{
	assert(index != InvalidUnitIndex);
	assert(index > 0);
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
	commandPos = pos;
	commandAngle = angle;

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
}

Group::MemberIndex Group::PositionToMemberIndexBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
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

Group::MemberIndex Group::PositionToMemberIndexWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
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

	v2 pos = groupCenter + v2new(
		totalSize.x / (float)cellsX * (float)cellX,
		totalSize.y / (float)cellsY * (float)cellY
	);

	return pos;
}

v2 Group::FormationPositionWedge(int memberIndex, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	int cellX = memberIndex % cellsX;
	int cellY = memberIndex / cellsX;

	v2 totalSize = v2new(
		cellsX * unitRadius * (1.0f + loose),
		cellsY * unitRadius * (1.0f + loose)
	);

	v2 pos = groupCenter + v2new(
		totalSize.x / (float)cellsX * (float)cellX,
		totalSize.y / (float)cellsY * (float)cellY
	);

	return pos;
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

Group::MemberIndex Group::FindNearestUnoccupied(MemberIndex queryMemberIndex)
{
	float bestDistance = FLT_MAX;
	MemberIndex bestIndex  = queryMemberIndex;
	int aliveUnitCount = CalcUnitAliveCount();
	int aliveMemberCounter = 0;
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
				v2 pos = FormationPositionBox(aliveMemberCounter, commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
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

		aliveMemberCounter++;
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

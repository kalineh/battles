
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

void Group::Init(Team* ARRAY ateams, Group* ARRAY agroups, Unit* ARRAY aunits)
{
	team = InvalidTeamIndex;
	formationType = FormationType_None;
	commandType = CommandType_None;
	groupPos = v2zero();
	commandPos = v2zero();
	commandAngle = 0.0f;
	commandTargetGroup = InvalidGroupIndex;
	bunchedRatio = 0.0f;
	combatRatio = 0.0f;
	disarrayRatio = 0.0f;
	displacementAggregate = v2zero();
	formationRatio = 0.5f;
	formationLoose = 0.0f;
	teams = ateams;
	groups = agroups;
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

	if (commandType == CommandType_Attack)
	{
		Group* targetGroup = groups + commandTargetGroup;

		const v2 commandSrc = centroid;
		const v2 commandDst = targetGroup->groupPos;
		const float commandLerp = stb_clamp(0.25f + combatRatio * -0.1f + bunchedRatio * -0.1f, 0.0f, 1.0f);
		commandPos = v2lerp(commandSrc, commandDst, commandLerp);

		v2 ofs = commandPos - groupPos;
		float len = v2lensafe(ofs);
		if (len > 2.5f)
		{
			const float src = commandAngle;
			const float dst = v2toangle(ofs) + HALFPI;
			const float t = fmaxf(1.0f - combatRatio * 5.0f, 0.0f) * 1.0f * dt;
			commandAngle = stb_lerp(t, src, dst);
		}
	}

	v2 toCentroid = centroid - groupPos;
	v2 toCommand = commandPos - groupPos;
	v2 toCommandDir = v2unitsafe(toCommand);

	float movementSpeed = CalcUnitSlowestMovement();
	float disarrayFactor = 1.0f - stb_clamp(disarrayRatio - 1.5f, 0.0f, 1.0f);
	float combatFactor = stb_clamp(combatRatio, 0.0f, 1.0f);

	float toCentroidSpeed = movementSpeed * disarrayFactor * 0.02f + movementSpeed * combatFactor * 0.05f;
	float toCommandSpeed = movementSpeed * disarrayFactor;

	groupPos += toCentroid * dt * toCentroidSpeed;
	groupPos += toCommandDir * dt * toCommandSpeed;

	assert(!isnan(groupPos.x));
	assert(!isnan(groupPos.y));

	UpdateFormation();
	UpdateRout();

	float average = CalcUnitAverageHealth();
	float change = healthAggregatePrev - average;
	healthAggregatePrev = average;
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
	float* slotTargetAngles = NULL;
	stb_arr_setlen(slotTargetAngles, stb_arr_len(slots));

	for (int i = 0; i < stb_arr_len(slotTargetPositions); ++i)
	{
		switch (formationType)
		{
		case FormationType_None:
			slotTargetPositions[i] = (units + searchPool[i])->pos;
			slotTargetAngles[i] = 0.0f;
			break;

		case FormationType_Box:
			slotTargetPositions[i] = FormationPositionBox(i, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			slotTargetAngles[i] = FormationAngleBox(i, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			break;

		case FormationType_Wedge:
			slotTargetPositions[i] = FormationPositionWedge(i, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			slotTargetAngles[i] = FormationAngleWedge(i, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			break;

		case FormationType_Circle:
			slotTargetPositions[i] = FormationPositionCircle(i, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
			slotTargetAngles[i] = FormationAngleWedge(i, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
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
		unit->targetAngle = slotTargetAngles[aliveMemberCounter];

		aliveMemberCounter++;
	}

	float disarray = 0.0f;
	int disarrayCount = 0;

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];

		// TODO: how are we getting invalid here?
		// if sorting fails and there are NaN, invalid slot arranging occurs
		assert(unitIndex != InvalidUnitIndex);

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

	float bunched = 0.0f;
	int bunchedCount = 0;

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		bunched += unit->bunching;
		bunchedCount += 1;
	}

	bunchedRatio = bunched / fmaxf((float)bunchedCount, 1.0f);

	float combat = 0.0f;
	int combatCount = 0;

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		combat += unit->attacking;
		combatCount += 1;
	}

	combatRatio = combat / fmaxf((float)combatCount, 1.0f);

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

void Group::UpdateRout()
{
	const float dt = 1.0f / 60.0f;

	float routFactor = 0.0f;
	float resolveFactor = 0.0f;
	int routCount = 0;

	for (int i = 0; i < stb_arr_len(slots); ++i)
	{
		UnitIndex unitIndex = slots[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		routFactor += unit->scared;
		resolveFactor += unit->data->resolve;
		routCount++;
	}

	if (routCount > 0.0f)
	{
		routFactor /= resolveFactor / (float)routCount;
		routFactor /= (float)routCount;
	}

	if (commandType == CommandType_Rout)
	{
		for (int i = 0; i < stb_arr_len(slots); ++i)
		{
			UnitIndex unitIndex = slots[i];
			Unit* unit = units + unitIndex;

			if (!unit->IsValid())
				continue;
			if (!unit->IsAlive())
				continue;

			// TODO: don't really want to touch unit from group
			unit->reload = 0.0f;
		}

		routRatio = fmaxf(routRatio - 0.01f * dt, 0.0f);
		if (routRatio <= 0.0f)
			CommandStop();
		
		return;
	}

	routRatio = fmaxf(routRatio - 0.1f * dt, 0.0f);
	routRatio = fminf(routRatio + 0.5f * routFactor * dt, 1.0f);

	if (routRatio >= 1.0f)
		CommandRout();
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
	stb_arr_find_fastdelete(members, index);
}

void Group::CommandStop()
{
	commandType = CommandType_None;

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

void Group::CommandMoveAttack(GroupIndex group)
{
	commandType = CommandType_Attack;
	commandTargetGroup = group;

	Group* targetGroup = groups + group;
	commandPos = targetGroup->groupPos;
	v2 ofs = commandPos - groupPos;
	float len = v2lensafe(ofs);
	if (len > 5.0f)
		commandAngle = v2toangle(ofs) + HALFPI;
}

void Group::CommandMoveTo(v2 pos, float angle)
{
	commandType = CommandType_Move;
	commandPos = pos;
	commandAngle = angle;
	commandTargetGroup = InvalidGroupIndex;
}

void Group::CommandMoveToInstant(v2 pos, float angle)
{
	commandType = CommandType_None;
	groupPos = pos;
	commandPos = pos;
	commandAngle = angle;
	commandTargetGroup = InvalidGroupIndex;
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

void Group::CommandRout()
{
	commandType = CommandType_Rout;
	commandTargetGroup = InvalidGroupIndex;

	commandPos = groupPos + v2unitsafe(commandPos - groupPos) * -500.0f;
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
	(void)pos;
	(void)groupCenter;
	(void)unitCount;
	(void)unitRadius;
	(void)ratio;
	(void)loose;

	return 0;
}

v2 Group::FormationPositionBox(int index, v2 groupCenter, float groupAngle, int unitCount, float unitRadius, float ratio, float loose)
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
	v2 unitSize = v2new(unitRadius, unitRadius);
	v2 localPos = halfSize * -1.0f + v2new(
		totalSize.x / (float)cellsX * (float)cellX,
		totalSize.y / (float)cellsY * (float)cellY
	) + unitSize;

	v2 rotatedPos = v2rotate(localPos, groupAngle);
	v2 pos = groupCenter + rotatedPos;

	return pos;
}

v2 Group::FormationPositionWedge(int index, v2 groupCenter, float groupAngle, int unitCount, float unitRadius, float ratio, float loose)
{
	(void)groupAngle;
	(void)ratio;

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
}

v2 Group::FormationPositionCircle(int index, v2 groupCenter, float groupAngle, int unitCount, float unitRadius, float ratio, float loose)
{
	(void)groupAngle;
	(void)ratio;

	float step = TWOPI / (float)unitCount;
	float out = loose * unitRadius * unitCount * (1.0f / PI);
	float t = (float)index * step;
	v2 pos = v2new(cosf(t), sinf(t)) * out;
	return groupCenter + pos;
}

float Group::FormationAngleBox(int index, v2 groupCenter, float groupAngle, int unitCount, float unitRadius, float ratio, float loose)
{
	(void)index;
	(void)groupCenter;
	(void)unitCount;
	(void)unitRadius;
	(void)ratio;
	(void)loose;

	return anglewrap0TWOPI(groupAngle - HALFPI);
}

float Group::FormationAngleWedge(int index, v2 groupCenter, float groupAngle, int unitCount, float unitRadius, float ratio, float loose)
{
	(void)index;
	(void)groupCenter;
	(void)unitCount;
	(void)unitRadius;
	(void)ratio;
	(void)loose;

	return groupAngle;
}

float Group::FormationAngleCircle(int index, v2 groupCenter, float groupAngle, int unitCount, float unitRadius, float ratio, float loose)
{
	(void)index;
	(void)groupCenter;
	(void)unitCount;
	(void)unitRadius;
	(void)ratio;
	(void)loose;

	return groupAngle;
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
	centroid = sum / fmaxf((float)count, 1.0f);

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

v2 Group::CalcUnitFightingAggregateOffset()
{
	v2 aggregate = v2zero();

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitIndex unitIndex = members[i];
		Unit* unit = units + unitIndex;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		v2 ofs = unit->targetPos - unit->pos;
		
		aggregate += ofs * unit->attacking;
	}

	return aggregate;
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
				v2 pos = FormationPositionBox(aliveMemberCursor, groupPos, commandAngle, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
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

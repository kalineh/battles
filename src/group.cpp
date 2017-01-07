
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
	float largestUnitRadius = 0.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitID unitID = members[i];
		Unit* unit = units + unitID;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		if (unit->data->radius > largestUnitRadius)
			largestUnitRadius = unit->data->radius;
	}

	int aliveUnitCount = 0;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitID unitID = members[i];
		Unit* unit = units + unitID;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		aliveUnitCount++;
	}

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitID unitID = members[i];
		Unit* unit = units + unitID;

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
				v2 unitTargetPos = MemberIDToPositionBox(i, commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
				MemberID memberID = PositionToMemberIDBox(unit->pos, commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
				MemberID memberIDFront = PositionToMemberIDBox(unit->pos + v2new(0.0f, 1.0f * largestUnitRadius * (1.0f + formationLoose)), commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);
				MemberID memberIDLeft = PositionToMemberIDBox(unit->pos + v2new(-1.0f * largestUnitRadius * (1.0f + formationLoose), 9.0f), commandPos, aliveUnitCount, largestUnitRadius, formationRatio, formationLoose);

				if (i != memberID && i != memberIDFront)
				{
					UnitID unitIDFront = members[memberIDFront];
					Unit* unitFront = units + unitIDFront;
					if (!unitFront->IsValid() || !unitFront->IsAlive())
					{
						//stb_swap((void*)(units + i), (void*)(units + unitIndexFront), sizeof(Unit));
						Unit tmp = *unit;
						*unit = *unitFront;
						*unitFront = tmp;
					}
				}

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
	}
}

void Group::AddUnit(UnitID id)
{
	stb_arr_push(members, id);
}

void Group::RemoveUnit(UnitID id)
{
	stb_arr_fastdelete(members, id);
}

void Group::CommandStop()
{
	v2 centroid = v2zero();
	float count = 0.0f;

	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitID unitID = members[i];
		Unit* unit = units + unitID;

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
		UnitID unitID = members[i];
		Unit* unit = units + unitID;

		if (unit->IsValid() && unit->IsAlive())
		{
			unit->pos = unit->targetPos;
			unit->angle = unit->targetAngle;
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

int Group::PositionToMemberIDBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	v2 localPos = pos - groupCenter;
	v2 totalSize = v2new(
		cellsX * unitRadius * (1.0f + loose),
		cellsY * unitRadius * (1.0f + loose)
	);

	int cellX = localPos.x / cellsX;
	int cellY = localPos.y / cellsX;

	cellX = stb_max(cellX, 0);
	cellY = stb_max(cellY, 0);

	cellX = stb_min(cellX, cellsX - 1);
	cellY = stb_min(cellY, cellsY - 1);

	int index = cellX + cellY * cellsX;

	return index;
}

int Group::PositionToMemberIDWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	// TODO: wedge/triangle calc
	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	v2 localPos = pos - groupCenter;
	v2 totalSize = v2new(
		cellsX * unitRadius * (1.0f + loose),
		cellsY * unitRadius * (1.0f + loose)
	);

	int cellX = localPos.x / cellX;
	int cellY = localPos.y / cellY;

	int index = cellX + cellY * cellsX;

	return index;
}

v2 Group::MemberIDToPositionBox(int index, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
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

v2 Group::MemberIDToPositionWedge(MemberID memberID, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose)
{
	int cellsX = stb_max((int)((float)unitCount * ratio), 1);
	int cellsY = unitCount / cellsX;

	int cellX = memberID % cellsX;
	int cellY = memberID / cellsX;

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

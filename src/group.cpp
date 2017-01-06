
#include "inc.h"

void Group::Init(Unit* ARRAY aunits)
{
	formationType = FormationType_None;
	commandPos = v2zero();
	commandAngle = 0.0f;
	boxRatio = 0.5f;
	boxLoose = 0.0f;
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
			case FormationType_Box:
			{
				int count = stb_arr_len(members);
				int cellsX = (int)((float)count * boxRatio);
				int cellsY = count / cellsX;
				int remainder = count - (cellsX * cellsY);
				int cellX = i % cellsX;
				int cellY = i / cellsX;

				v2 totalSize = v2new(cellsX, cellsY) * 5.0f;
				v2 offset = v2new(cellX, cellY) * largestUnitRadius * (1.0f + boxLoose) - totalSize * 0.5f;
				v2 unitTargetPos = commandPos + offset;

				unit->targetPos = unitTargetPos;
				unit->targetAngle = commandAngle;

				break;
			}
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

void Group::CommandMoveTo(v2 pos, float angle)
{
	commandPos = pos;
	commandAngle = 0.0f;
}

void Group::CommandFormationBox(float ratio, float loose)
{
	formationType = FormationType_Box;
	boxRatio = ratio;
	boxLoose = loose;
}

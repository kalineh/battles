
#include "inc.h"

void Group::Init(Unit* ARRAY aunits)
{
	units = aunits;
	members = NULL;
	stb_arr_setsize(members, 8);
}

void Group::Release()
{
	stb_arr_free(members);
}

void Group::AddUnit(UnitID id)
{
	stb_arr_push(members, id);
}

void Group::RemoveUnit(UnitID id)
{
	stb_arr_fastdelete(members, id);
}

void Group::CommandTo(v2 pos)
{
	for (int i = 0; i < stb_arr_len(members); ++i)
	{
		UnitID unitID = members[i];
		Unit* unit = units + unitID;

		if (!unit->IsValid())
			continue;
		if (!unit->IsAlive())
			continue;

		unit->targetPos = pos;
	}
}

void Group::FormationBox(float ratio, float loose)
{
}

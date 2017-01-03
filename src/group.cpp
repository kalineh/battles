
#include "inc.h"

void Group::Init()
{
	members = NULL;
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
}

void Group::FormationBox(float ratio, float loose)
{
}

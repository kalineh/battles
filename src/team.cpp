
#include "inc.h"

void Team::Init()
{
	teamIndex = 0;
	groups = NULL;
	stb_arr_setsize(groups, 0);
}

void Team::Release()
{
	stb_arr_free(groups);
}

void Team::AddGroup(GroupIndex groupIndex)
{
	assert(groupIndex != InvalidGroupIndex);
	assert(groupIndex >= 0);
	stb_arr_push(groups, groupIndex);
}

void Team::RemoveGroup(GroupIndex groupIndex)
{
	assert(groupIndex != InvalidGroupIndex);
	assert(groupIndex >= 0);
	stb_arr_find_fastdelete(groups, groupIndex);
}

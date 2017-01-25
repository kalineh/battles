
#include "inc.h"

struct Team
{
	void Init();
	void Release();

	void AddGroup(GroupIndex groupIndex);
	void RemoveGroup(GroupIndex groupIndex);

	TeamIndex teamIndex;
	GroupIndex* ARRAY OWNER groups;
};

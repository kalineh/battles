
#include "inc.h"

struct Touch
{
	void Init(Unit* aunits);
	void Release();

	void Clear();
	void Collect(Unit* unit, UnitID* ARRAY candidates);

	struct Entry
	{
		UnitID ids[4];
	};

	Unit* ARRAY units;
	Entry* OWNER ARRAY entries;
};

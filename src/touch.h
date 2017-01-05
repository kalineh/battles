
#include "inc.h"

struct Touch
{
	void Init(Unit* aunits);
	void Release();

	void Clear();

	int Collect(Unit* unit, UnitID* ARRAY candidates);

	struct Entry
	{
		UnitID ids[4];
	};

	Entry* GetEntry(UnitID id);

	Unit* ARRAY units;
	Entry* OWNER ARRAY entries;
};

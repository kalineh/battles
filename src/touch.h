
#include "inc.h"

struct Touch
{
	void Init(Unit* aunits);
	void Release();

	void Clear();

	int Collect(Unit* unit, UnitIndex* ARRAY candidates);

	struct Entry
	{
		UnitIndex indexes[4];
	};

	Entry* GetEntry(UnitIndex id);

	Unit* ARRAY units;
	Entry* OWNER ARRAY entries;
};

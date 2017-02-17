
#include "inc.h"

struct Touch
{
	void Init();
	void Release();

	void Clear();

	int Collect(Unit* unit, UnitIndex* ARRAY candidates);

	struct Entry
	{
		UnitIndex indexes[4];
	};

	Entry* GetEntry(UnitIndex id);

	Entry* OWNER ARRAY entries;
};


#include "inc.h"

void Touch::Init(Unit* aunits)
{
	units = aunits;
	entries = NULL;
	stb_arr_setlen(entries, stb_arr_len(units));
}

void Touch::Release()
{
	stb_arr_free(entries);
}

void Touch::Clear()
{
	for (int i = 0; i < stb_arr_len(entries); ++i)
	{		
		Entry* entry = entries + i;
		memset(entry->ids, 0, sizeof(entry->ids));
	}
}

void Touch::Collect(Unit* unit, UnitID* ARRAY candidates)
{
	v2 srcp = unit->pos;
	float srcr = unit->data->radius;

	for (int i = 0; i < stb_arr_len(candidates); ++i)
	{
		Entry* entry = entry + i;
		UnitID* candidateID = candidates + i;
		Unit* candidate = units + *candidateID;

		v2 dstp = candidate->pos;
		float dstr = candidate->data->radius;
		float lensq = v2lensq(dstp - srcp);
		float rangesq = srcr * srcr + dstr * dstr;

		if (lensq < rangesq)
		{
			for (int j = 0; j < stb_arrcount(entry->ids); ++i)
			{
				if (entry->ids[j] == 0)
				{
					entries->ids[j] = *candidateID;
					break;
				}
			}
		}
	}
}

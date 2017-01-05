
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

int Touch::Collect(Unit* unit, UnitID* ARRAY candidates)
{
	int found = 0;
	v2 srcp = unit->pos;
	float srcr = unit->data->radius;
	UnitID unitID = (int)(unit - units);

	for (int i = 0; i < stb_arr_len(candidates); ++i)
	{
		UnitID* candidateID = candidates + i;
		Unit* candidate = units + *candidateID;

		if (unit == candidate)
			continue;

		v2 dstp = candidate->pos;
		float dstr = candidate->data->radius;
		float lensq = v2lensq(dstp - srcp);
		float rangesq = (srcr + dstr) * (srcr + dstr);

		if (lensq < rangesq)
		{
			Entry* entry = entries + unitID;
			for (int j = 0; j < stb_arrcount(entry->ids); ++j)
			{
				if (entry->ids[j] == 0)
				{
					entry->ids[j] = *candidateID;
					found++;
					break;
				}
			}
		}
	}

	return found;
}

Touch::Entry* Touch::GetEntry(UnitID id)
{
	Entry* entry = entries + id;
	return entry;
}

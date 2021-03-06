
#include "inc.h"

void Touch::Init()
{
	entries = NULL;
	stb_arr_setlen(entries, stb_arr_len(game.units));
	for (int i = 0; i < stb_arr_len(entries); ++i)
	{
		Entry* entry = entries + i;
		for (int j = 0; j < stb_arrcount(entry->indexes); ++j)
			entry->indexes[j] = InvalidUnitIndex;
	}
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
		for (int j = 0; j < stb_arrcount(entry->indexes); ++j)
			entry->indexes[j] = InvalidUnitIndex;
	}
}

int Touch::Collect(Unit* unit, UnitIndex* ARRAY candidates)
{
	int found = 0;
	v2 srcp = unit->pos;
	float srcr = unit->data->radius;
	UnitIndex unitIndex = (int)(unit - game.units);

	for (int i = 0; i < stb_arr_len(candidates); ++i)
	{
		UnitIndex* candidateIndex = candidates + i;
		Unit* candidate = game.units + *candidateIndex;

		if (unit == candidate)
			continue;

		v2 dstp = candidate->pos;
		float dstr = candidate->data->radius;
		float lensq = v2lensq(dstp - srcp);
		float rangesq = (srcr + dstr) * (srcr + dstr);

		if (lensq < rangesq)
		{
			Entry* entry = entries + unitIndex;
			for (int j = 0; j < stb_arrcount(entry->indexes); ++j)
			{
				if (entry->indexes[j] == InvalidUnitIndex)
				{
					entry->indexes[j] = *candidateIndex;
					found++;
					break;
				}
			}
		}
	}

	return found;
}

Touch::Entry* Touch::GetEntry(UnitIndex unitIndex)
{
	Entry* entry = entries + unitIndex;
	return entry;
}

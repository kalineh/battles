
#include "inc.h"

struct Combat
{
	struct Event
	{
		UnitIndex attacker;
		UnitIndex defender;
		float damage;
	};

	void Init();
	void Release();

	void Record(UnitIndex attacker, UnitIndex defender, float damage);
};

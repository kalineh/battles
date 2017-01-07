
#include "inc.h"

struct Group
{
	enum FormationType
	{
		FormationType_None,
		FormationType_Box,
		FormationType_Wedge,
	};

	typedef int MemberID;

	void Init(Unit* ARRAY aunits);
	void Release();

	void Update();
	void UpdateFormation();

	void AddUnit(UnitIndex id);
	void RemoveUnit(UnitIndex id);

	void CommandStop();
	void CommandMoveTo(v2 pos, float angle);
	void CommandTeleportTo(v2 pos, float angle);

	void CommandFormationNone();
	void CommandFormationBox(float ratio, float loose);
	void CommandFormationWedge();

	MemberID PositionToMemberIDBox(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	MemberID PositionToMemberIDWedge(v2 pos, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	v2 MemberIDToPositionBox(MemberID memberID, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);
	v2 MemberIDToPositionWedge(MemberID memberID, v2 groupCenter, int unitCount, float unitRadius, float ratio, float loose);

	Unit* ARRAY units;
	UnitIndex* OWNER ARRAY members;

	FormationType formationType;
	v2 commandPos;
	float commandAngle;
	float formationRatio;
	float formationLoose;
};

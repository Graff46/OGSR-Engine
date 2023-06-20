#include "stdafx.h"
#include "CarPassengers.h"
#include "..\Include/xrRender/Kinematics.h"

void CarPassengers::create(IKinematics* pKinematics)
{
	CInifile* ini = pKinematics->LL_UserData();

	if (ini->section_exist("passengers"))
	{
		CInifile::Sect& section = ini->r_section("passengers");

		for (const auto& item : section.Data)
		{
			u16 idBone = item.first == "root" ? pKinematics->LL_GetBoneRoot() : pKinematics->LL_BoneID(item.first);
			Fmatrix mx = pKinematics->LL_GetTransform(idBone);

			if (item.second.size()) {
				Fvector offsetVec = ini->r_fvector3("passengers", item.first.c_str());
				mx.c.add( offsetVec );
			}
			list.push_back(mx);
		}
	}
}

bool CarPassengers::addPassenger(CGameObject* npc)
{
	for (const Fmatrix& place : list)
	{
		if (!occupiedPlaces.contains(npc)) 
		{
			occupiedPlaces.emplace(npc, &place);
			return true;
		}
	}

	return false;
}

void CarPassengers::removePassenger(CGameObject* npc)
{
	occupiedPlaces.erase(npc);
}

std::map<CGameObject*, const Fmatrix*> CarPassengers::getOccupiedPlaces()
{
	return occupiedPlaces
}

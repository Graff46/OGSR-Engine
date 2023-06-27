#include "stdafx.h"
#include "CarPassengers.h"
#include "..\Include/xrRender/Kinematics.h"

void CarPassengers::create(IKinematics* pKinematics)
{
	/*CInifile* ini = pKinematics->LL_UserData();

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

			list.emplace(mx, false); 
		}
	}*/
}

const Fmatrix* CarPassengers::addPassenger(CAI_Stalker* npc)
{
	/*for (const auto& place : list)
	{
		if ((!place.second) && (!occupiedPlaces.contains(npc)))
		{
			occupiedPlaces.emplace(npc, &place.first);
			//doorId.emplace(npc, car->calcDoorForPlace(npc->XFORM().c));

			list.at(place.first) = true;

			return &place.first;
		}
	}*/
	const Fmatrix mx;
	return &mx;
}

void CarPassengers::removePassenger(CAI_Stalker* npc)
{
	/*if (occupiedPlaces.contains(npc))
	{
		const Fmatrix* mx = occupiedPlaces.at(npc);
		list.at(*mx) = false;
		occupiedPlaces.erase(npc);
	}*/
}

std::map<CAI_Stalker*, const Fmatrix*> CarPassengers::getOccupiedPlaces()
{
	return occupiedPlaces;
}

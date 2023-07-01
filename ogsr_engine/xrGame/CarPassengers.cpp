#include "stdafx.h"
#include "CarPassengers.h"
#include "..\Include/xrRender/Kinematics.h"
#include "Car.h"

CarPassengers::CarPassengers(CCar* obj) 
{
	car = obj;
}

void CarPassengers::create(IKinematics* pKinematics)
{
	CInifile* ini = pKinematics->LL_UserData();

	if (ini->section_exist("passengers"))
	{
		CInifile::Sect& section = ini->r_section("passengers");
		u8 id = 0;
		for (const auto& item : section.Data)
		{
			u16 idBone = item.first == "root" ? pKinematics->LL_GetBoneRoot() : pKinematics->LL_BoneID(item.first);
			Fmatrix mx = pKinematics->LL_GetTransform(idBone);

			if (item.second.size()) {
				Fvector offsetVec = ini->r_fvector3("passengers", item.first.c_str());
				mx.c.add( offsetVec );
			}

			id++;
			list.emplace(id, Place{id, mx, false, 0});
		}
	}
}

const Fmatrix* CarPassengers::addPassenger(CAI_Stalker* npc)
{
	if (occupiedPlaces.contains(npc))
		return nullptr;

	for (auto& [id, place] : list)
	{
		if (!place.occupied)
		{
			//place.setProps(true, car->calcDoorForPlace(place.position()));
			place.exitDoorId = car->calcDoorForPlace(&place.xform.c);
			place.occupied = true;
			occupiedPlaces.emplace(npc, &place);

			return &place.xform;
		}
	}

	return nullptr;
}

void CarPassengers::removePassenger(CAI_Stalker* npc)
{
	if (occupiedPlaces.contains(npc))
	{
		occupiedPlaces.at(npc)->setProps(false);
		occupiedPlaces.erase(npc);
	}
}

xr_unordered_map<CAI_Stalker*, CarPassengers::Place*>* CarPassengers::getOccupiedPlaces()
{
	return &occupiedPlaces;
}

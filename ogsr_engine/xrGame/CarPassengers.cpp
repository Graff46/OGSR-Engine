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

	/*if (ini->section_exist("passengers"))
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
	}*/

	if (ini->section_exist("passengers"))
	{
		u8 id = 0;

		LPCSTR str = ini->r_string("passengers", "places");
		string32 boneName;
		string32 vecStr;
		u8 count = _GetItemCount(str, '|');
		for (u8 i = 0; i < count; i++)
		{
			_GetItem(str, i, boneName, '|');
			_GetItem(str, ++i, vecStr, '|');

			Fvector offset{};
			sscanf(vecStr, "%f,%f,%f", &offset.x, &offset.y, &offset.z); 

			u16 idBone = strcmp(boneName, "root") ? pKinematics->LL_BoneID(boneName) : pKinematics->LL_GetBoneRoot();
			Fmatrix mx = pKinematics->LL_GetTransform(idBone);

			mx.c.sub(offset);

			list.emplace(++id, Place{ id, mx, false, 0 });
		}	
	}
}

const Fmatrix* CarPassengers::addPassenger(CGameObject* npc)
{
	if (occupiedPlaces.contains(npc))
		return nullptr;

	for (auto& [id, place] : list)
	{
		if (!place.occupied)
		{
			place.exitDoorId = car->calcDoorForPlace(&place.xform.c);
			place.occupied = true;
			occupiedPlaces.emplace(npc, &place);

			return &place.xform;
		}
	}

	return nullptr;
}

void CarPassengers::removePassenger(CGameObject* npc)
{
	if (occupiedPlaces.contains(npc))
	{
		occupiedPlaces.at(npc)->occupied = false;
		occupiedPlaces.erase(npc);
	}
}

xr_unordered_map<CGameObject*, CarPassengers::Place*>* CarPassengers::getOccupiedPlaces()
{
	return &occupiedPlaces;
}

const u8 CarPassengers::vacantSits()
{
	return list.size() - occupiedPlaces.size();
}

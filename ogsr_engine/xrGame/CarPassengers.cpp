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
		u8 id = 1;

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

			mx.c.add(offset);

			list.emplace(id, Place{ id, mx, false, 0 });
            id++;
		}
	}
}

const Fmatrix* CarPassengers::addPassenger(CGameObject* npc, u8 seat)
{
	if (occupiedPlaces.contains(npc))
		return nullptr;

	for (auto& [id, place] : list)
	{
		if ( (!place.occupied) && ((seat == u8(-1)) || (seat == place.id)))
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

const u8 CarPassengers::getSeatId(CGameObject* npc)
{
	if (occupiedPlaces.contains(npc))
		return occupiedPlaces.at(npc)->id;
	else if (car->Owner()->ID() == npc->ID())
		return 0;
	else if (car->wpnSeat->getOwnerID() == npc->ID())
		return u8(-1) - 1;

	return u8(-1);
}
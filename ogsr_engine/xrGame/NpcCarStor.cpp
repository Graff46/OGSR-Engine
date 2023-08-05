#include "stdafx.h"
#include "NpcCarStor.h"

xr_unordered_map<ALife::_OBJECT_ID, NpcCarStor::CarStorIsDriver> NpcCarStor::NPCid2CarIdToIsDriver;
bool NpcCarStor::needClear = true;
void NpcCarStor::add(ALife::_OBJECT_ID npcId, ALife::_OBJECT_ID carId, bool isDriver)
{
	NPCid2CarIdToIsDriver.insert_or_assign(npcId, CarStorIsDriver(carId, isDriver));
};

void NpcCarStor::remove(ALife::_OBJECT_ID npcId)
{
	if (NPCid2CarIdToIsDriver.contains(npcId))
		NPCid2CarIdToIsDriver.erase(npcId);
};

bool NpcCarStor::get(ALife::_OBJECT_ID npcId, ALife::_OBJECT_ID& carId, bool& isDriver)
{
	bool exist = false;
	if (exist = NPCid2CarIdToIsDriver.contains(npcId))
	{
		CarStorIsDriver p = NPCid2CarIdToIsDriver.at(npcId);
		carId = p.carID;
		isDriver = p.isDriver;
	}

	return exist;
}

bool NpcCarStor::getFromCarId(ALife::_OBJECT_ID carId, ALife::_OBJECT_ID& npcId, bool& isDriver)
{
	for (const auto& [id, carStor] : NPCid2CarIdToIsDriver)
	{
		if (carStor.carID == carId)
		{
			npcId = id;
			isDriver = carStor.isDriver;

			return true;
		}
	}

	return false;
}

void NpcCarStor::clear()
{
	if (needClear)
		NPCid2CarIdToIsDriver.clear();
}

void NpcCarStor::setFlagClear(bool flag)
{
	needClear = flag;
}
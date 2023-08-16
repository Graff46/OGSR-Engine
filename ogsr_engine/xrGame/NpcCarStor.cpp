#include "stdafx.h"
#include "NpcCarStor.h"
#include <Actor.h>
#include <Car.h>
#include "alife_simulator.h"
#include "alife_object_registry.h"
#include "ai_object_location.h"
#include "game_sv_single.h"

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

const xr_vector<std::pair<ALife::_OBJECT_ID, bool>> NpcCarStor::getFromCarId(ALife::_OBJECT_ID carId)
{
    xr_vector<std::pair<ALife::_OBJECT_ID, bool>> result;
    
	for (const auto& [id, carStor] : NPCid2CarIdToIsDriver)
		if (carStor.carID == carId)
			result.push_back({id, carStor.isDriver});

	return result;
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

void NpcCarStor::replaceLevelNpcOfCar(GameGraph::_GRAPH_ID destGvid, u32 destLvid, Fvector* destPos)
{
	if (Actor()->Holder()) 
	{
		if (CCar* car = smart_cast<CCar*>(Actor()->Holder()))
		{
			if ((car->Owner()) && (!car->OwnerActor()))
				car->Owner()->alife_object()->alife().teleport_object(car->Owner()->ID(), destGvid, destLvid, *destPos);

			if (!car->passengers->getOccupiedPlaces()->empty())
				for (const auto [npc, place] : *car->passengers->getOccupiedPlaces())
					if (npc->ID() != Actor()->ID())
						npc->alife_object()->alife().teleport_object(npc->ID(), destGvid, destLvid, *destPos);
		}
	}
}
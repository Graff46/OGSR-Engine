#pragma once

#include "alife_space.h"
#include "game_graph_space.h"

class NpcCarStor
{
private:
    struct CarStorIsDriver
    {
        ALife::_OBJECT_ID carID;
        bool isDriver;
    };

    static xr_unordered_map<ALife::_OBJECT_ID, CarStorIsDriver> NPCid2CarIdToIsDriver;
    static bool needClear;
public:
    static void add(ALife::_OBJECT_ID npcId, ALife::_OBJECT_ID carId, bool isDriver);
    static void remove(ALife::_OBJECT_ID npcId);
    static bool get(ALife::_OBJECT_ID npcId, ALife::_OBJECT_ID &carId, bool &isDriver);
    static bool getFromCarId(ALife::_OBJECT_ID carId, ALife::_OBJECT_ID &npcId, bool &isDriver);
    static void clear();
    static void setFlagClear(bool flag);
    static void replaceLevelNpcOfCar(GameGraph::_GRAPH_ID destGvid, u32 destLvid, Fvector* destPos);
};
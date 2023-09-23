#pragma once

#include "alife_space.h"
#include "game_graph_space.h"

class NpcCarStor
{
private:
    struct CarStorIsDriver
    {
        ALife::_OBJECT_ID carID;
        u8 seat;
    };

    static xr_unordered_map<ALife::_OBJECT_ID, CarStorIsDriver> NPCid2CarIdToIsDriver;
    static bool needClear;
public:
    static void add(ALife::_OBJECT_ID npcId, ALife::_OBJECT_ID carId, u8 seat);
    static void remove(ALife::_OBJECT_ID npcId);
    static bool get(ALife::_OBJECT_ID npcId, ALife::_OBJECT_ID &carId, u8& seat);
    static const xr_vector<std::pair<ALife::_OBJECT_ID, u8>> getFromCarId(ALife::_OBJECT_ID carId);
    static void clear();
    static void setFlagClear(bool flag);
    static void replaceLevelNpcOfCar(GameGraph::_GRAPH_ID destGvid, u32 destLvid, Fvector* destPos);
};
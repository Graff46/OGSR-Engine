#include "stdafx.h"
#include "data_tail_save.h"
#include "alife_space.h"

shared_str DataTailSave::toSaveData = "";
shared_str DataTailSave::inSaveData = "";

void DataTailSave::setData(LPCSTR newData)
{
	toSaveData = newData;
}

void DataTailSave::save(IWriter &memory_stream)
{
	if (toSaveData.size() == 0) return;

	memory_stream.open_chunk(SAVE_TAIL);
	memory_stream.w_stringZ(toSaveData);
	memory_stream.close_chunk();
}

LPCSTR DataTailSave::load (IReader file_stream)
{
	if (file_stream.find_chunk(SAVE_TAIL))
		file_stream.r_stringZ(inSaveData);

	return inSaveData.c_str();
}
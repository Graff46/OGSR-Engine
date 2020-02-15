#pragma once

class DataTailSave
{
private:

public:
	shared_str static toSaveData;
	shared_str static inSaveData;

	void static setData(LPCSTR newData);
	void static save(IWriter &memory_stream);
	LPCSTR static load(IReader file_stream);
};


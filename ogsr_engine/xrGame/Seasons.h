#pragma once
class Seasons
{
public:
	static LPSTR currentSeason;
	static void swithSeason(LPCSTR newSeason, BOOL needReload, shared_str levelName=nullptr);
	static void load(IReader& stream, shared_str levelName);
	static void save(IWriter& stream);
	static void reset();
};


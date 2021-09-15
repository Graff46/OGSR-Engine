#include "stdafx.h"
#include "Seasons.h"
#include "alife_space.h"
#include "level.h"
#include "..\xrSound\SoundRender_Core.h"

LPSTR Seasons::currentSeason = "default";
void Seasons::swithSeason(LPCSTR newSeason, BOOL needReload)
{
	if (xr_strcmp(newSeason, currentSeason)) {
		string_path tmp, tmp2, tmp3;
		FS_Path* p1 = FS.get_path("$game_textures_ex$");
		FS_Path* p2 = FS.get_path("$level_textures_ex$");
		FS_Path* p3 = FS.get_path("$sound_ex$");
		p1->_set(xr_strconcat(tmp,  "seasons\\", newSeason, "\\textures"));
		p3->_set(xr_strconcat(tmp3, "seasons\\", newSeason, "\\sounds"));
		if ((&Level()) && (Level().bReady))
			p2->_set(xr_strconcat(tmp2, "seasons\\", newSeason, "\\levels\\", Level().name().c_str()));
		else
			p2->_set(xr_strconcat(tmp2, "seasons\\", newSeason, "\\levels"));
		FS.rescan_path(p1->m_Path, TRUE);
		FS.rescan_path(p2->m_Path, TRUE);
		FS.rescan_path(p3->m_Path, TRUE);

		currentSeason = (LPSTR) newSeason;

		if (needReload) {
			Device.m_pRender->DeferredLoad(FALSE);
			Device.m_pRender->ResourcesDeferredUpload(TRUE);
			if (Device.b_is_Ready) Device.Reset();

			//SoundRender->reload();// Нормально не работает

#pragma todo("create reload all sounds from FS")
		}
	}
};

void Seasons::save(IWriter& stream)
{
	stream.open_chunk(SEASON_CHUNK_DATA);
	stream.w_stringZ(currentSeason);
	stream.close_chunk();
};

void Seasons::load(IReader& stream)
{
	R_ASSERT2(stream.find_chunk(SEASON_CHUNK_DATA), "Can't find chunk SEASON_CHUNK_DATA");
		shared_str saveSeason;
		stream.r_stringZ(saveSeason);

		swithSeason(saveSeason.c_str(), FALSE);
		currentSeason = (LPSTR) saveSeason.c_str();	
};
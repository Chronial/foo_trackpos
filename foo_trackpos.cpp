#include "stdafx.h"

#define VERSION "1.0"

DECLARE_COMPONENT_VERSION( "Track Positioner", VERSION,
	"This compoment allows you to place tracks in the playlist after the current playing Track\n"
	"by Chronial (Christian Fersch)\n"
	"\n"
	"Version: " VERSION "\n"
	"Compiled: " __DATE__ " - " __TIME__ )

// Settings

// {87C514D4-776B-4ed2-9CBB-E4AD52C40707}
static const GUID guid_cfgMovePlaylistContext = { 0x87c514d4, 0x776b, 0x4ed2, { 0x9c, 0xbb, 0xe4, 0xad, 0x52, 0xc4, 0x7, 0x7 } };
cfg_bool cfgMovePlaylistContext (guid_cfgMovePlaylistContext, true);

// {53E78C2B-D357-40a5-A070-93CCFEA08217}
static const GUID guid_cfgMovePlaylistExists = { 0x53e78c2b, 0xd357, 0x40a5, { 0xa0, 0x70, 0x93, 0xcc, 0xfe, 0xa0, 0x82, 0x17 } };
cfg_bool cfgMovePlaylistExists (guid_cfgMovePlaylistExists, false);

// {74A30E17-F2C6-465d-94BA-97EADBB352B0}
static const GUID guid_cfgFocus = { 0x74a30e17, 0xf2c6, 0x465d, { 0x94, 0xba, 0x97, 0xea, 0xdb, 0xb3, 0x52, 0xb0 } };
cfg_bool cfgFocus (guid_cfgFocus, true);

// {E5A09D5D-2742-4102-9A37-14EF626552A4}
static const GUID guid_cfgEmptyQueue = { 0xe5a09d5d, 0x2742, 0x4102, { 0x9a, 0x37, 0x14, 0xef, 0x62, 0x65, 0x52, 0xa4 } };
cfg_bool cfgEmptyQueue (guid_cfgEmptyQueue,true);

// {ECD7510C-63C5-465c-822C-4054252FA391}
static const GUID guid_cfgEnqueueOnLock = { 0xecd7510c, 0x63c5, 0x465c, { 0x82, 0x2c, 0x40, 0x54, 0x25, 0x2f, 0xa3, 0x91 } };
cfg_bool cfgEnqueueOnLock (guid_cfgEnqueueOnLock,true);

// {D1EE91FE-CFEF-4425-95DA-485691DD54B3}
static const GUID guid_cfgSetPlaybackOrder = { 0xd1ee91fe, 0xcfef, 0x4425, { 0x95, 0xda, 0x48, 0x56, 0x91, 0xdd, 0x54, 0xb3 } };
cfg_bool cfgSetPlaybackOrder (guid_cfgSetPlaybackOrder, true);

// {8079575C-9728-4b2d-892B-8A0D9F54713B}
static const GUID guid_cfgNoPlayPlace = { 0x8079575c, 0x9728, 0x4b2d, { 0x89, 0x2b, 0x8a, 0xd, 0x9f, 0x54, 0x71, 0x3b } };
cfg_bool cfgNoPlayPlace (guid_cfgNoPlayPlace, true);

char * guidToSource (GUID guid){
	char * out = new char[100];
	sprintf_s(out,100, "{ 0x%x, 0x%x, 0x%x, { 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x, 0x%x } }",
		guid.Data1, guid.Data2, guid.Data3,
		(int)guid.Data4[0], (int)guid.Data4[1], (int)guid.Data4[2], (int)guid.Data4[3],
		(int)guid.Data4[4], (int)guid.Data4[5], (int)guid.Data4[6], (int)guid.Data4[7]);
	return out;
}

bool getCurrentLocation(t_size * playlist, t_size * trackindex){
	static_api_ptr_t<playlist_manager> pm;
	if (!pm->get_playing_item_location(playlist,trackindex)){
		if (!cfgNoPlayPlace)
			return false;
		*trackindex = pm->activeplaylist_get_focus_item();
		if (*trackindex == ~0){
			*trackindex = 0;
		}
		*playlist = pm->get_active_playlist();
		if (*playlist == ~0)
			return false;
	}
	return true;
}

void placeAfterCurrent(const pfc::list_base_const_t<metadb_handle_ptr> & p_data, bool playlistCalled){
	static_api_ptr_t<playlist_manager> pm;
	t_size playingList, playingIndex;
	if (!getCurrentLocation(&playingList,&playingIndex))
		return;
	bool wasQueued = false;
	pfc::list_base_const_t<metadb_handle_ptr> const * insertItems = 0;
	t_size playlistLength = pm->playlist_get_item_count(playingList);
	bit_array_bittable moveMask(playlistLength);
	t_size moveCount = 0;

	pm->playlist_undo_backup(playingList);

	if (playlistCalled && (cfgMovePlaylistContext || cfgMovePlaylistExists)&& (playingList == pm->get_active_playlist())){
		pm->playlist_set_selection_single(playingList,playingIndex,false);
		pm->activeplaylist_get_selection_mask(moveMask);
		moveCount = pm->activeplaylist_get_selection_count(~0);
	} else {
		if (cfgMovePlaylistExists){
			pfc::list_t<metadb_handle_ptr> listContent;
			bit_array_bittable deleteMask(playlistLength);
			pm->playlist_get_all_items(playingList,listContent);
			for (int i=0; i < p_data.get_count(); i++){
				metadb_handle_ptr item = p_data.get_item(i);
				for (int j=0; j < listContent.get_count(); j++){
					if (listContent.get_item(j) == item && j != playingIndex){
						deleteMask.set(j,true);
					}
				}
			}
			pm->playlist_remove_items(playingList,deleteMask);
			getCurrentLocation(&playingList,&playingIndex);
			playlistLength = pm->playlist_get_item_count(playingList);
		}
		insertItems = &p_data;
	}

	if (moveCount > 0){
		t_size * newOrder = new t_size[playlistLength];
		int delta = 0;
		t_size * toInsert = new t_size[moveCount];
		unsigned int n = 0;
		int insertPos = -1;
		for ( int i=0; i < playlistLength; i++ ){
			if (!moveMask.get(i)){
				newOrder[i + delta] = i;
				if (i == playingIndex){
					playingIndex = i + delta; // we need this to focus the track later.
											  // can do this as delta will be <= 0 when we reach playingIndex so i == playingIndex won't be true again
					insertPos = i + delta + 1;
					for (int j = 0; j < n; j++){
						newOrder[insertPos++] = toInsert[j];
					}
					delta += moveCount;
				}
			} else if (insertPos == -1){
				toInsert[n++] = i;
				delta--;
			} else {
				newOrder[insertPos++] = i;
				delta--;
			}
		}
		if(!pm->playlist_reorder_items(playingList,newOrder,playlistLength)){
			if (cfgEnqueueOnLock){
				for ( int i=0; i < playlistLength; i++ ){
					if (moveMask.get(i)){
						metadb_handle_ptr toQueue;
						pm->playlist_get_item_handle(toQueue,playingList,i);
						pm->queue_add_item(toQueue);
					}
				}
			}
			wasQueued = true;
		}
		delete[] newOrder;
		delete[] toInsert;
	}
	if (insertItems != 0 && insertItems->get_count() > 0){
		bit_array_val * selection;
		if (playingList == pm->get_active_playlist()){
			pm->playlist_clear_selection(playingList);
			selection = new bit_array_val(true);
		} else {
			selection = new bit_array_val(false);
		}
		if (pm->playlist_insert_items(playingList,playingIndex+1,*insertItems,*selection) == -1){
			if (cfgEnqueueOnLock){
				for ( int i=0; i < insertItems->get_count(); i++ ){
					pm->queue_add_item(insertItems->get_item(i));
				}
			}
			wasQueued = true;
		}
		delete selection;
	}
	if (cfgFocus && (playingList == pm->get_active_playlist()) && !wasQueued){
		pm->playlist_set_focus_item(playingList,playingIndex+1);
	}
	if (cfgEmptyQueue && !wasQueued)
		pm->queue_flush();
	if (cfgSetPlaybackOrder && !wasQueued){
		static const GUID guid_playbackOrder_default = { 0xbfc61179, 0x49ad, 0x4e95, { 0x8d, 0x60, 0xa2, 0x27, 0x6, 0x48, 0x55, 0x5 } };
		static const GUID guid_playbackOrder_repeatPlaylist = { 0x681cc6ea, 0x60ae, 0x4bf9, { 0x91, 0x3b, 0xbb, 0x5f, 0x4e, 0x86, 0x4f, 0x2a } };
		//static const GUID guid_playbackOrder_repeatTrack = { 0x4bf4b280, 0xbb4, 0x4dd0, { 0x8e, 0x84, 0x37, 0xc3, 0x20, 0x9c, 0x3d, 0xa2 } };
		GUID activeOrder = pm->playback_order_get_guid(pm->playback_order_get_active());
		if (activeOrder != guid_playbackOrder_default && activeOrder != guid_playbackOrder_repeatPlaylist){
			for (int i=0; i < pm->playback_order_get_count(); i++){
				if (pm->playback_order_get_guid(i) == guid_playbackOrder_default){
					pm->playback_order_set_active(i);
				}
			}
		}
	}
}

class my_contextmenu : public contextmenu_item_simple {
	virtual unsigned get_num_items(){
		return 1;
	};
	virtual void get_item_name(unsigned p_index,pfc::string_base & p_out){
		p_out = "Place After Current Playing";
	};
	virtual void get_item_default_path(unsigned p_index,pfc::string_base & p_out){
		p_out = "";
	};
	virtual void context_command(unsigned p_index,const pfc::list_base_const_t<metadb_handle_ptr> & p_data,const GUID& p_caller){
		placeAfterCurrent(p_data,(p_caller == this->caller_playlist));
	};
	virtual GUID get_item_guid(unsigned p_index){
		// {b5889b2b-6d8e-4932-82ed-d20a7a54be5d}
		static const GUID guid_contextAddAfterPlaying = { 0xb5889b2b, 0x6d8e, 0x4932, { 0x82, 0xed, 0xd2, 0xa, 0x7a, 0x54, 0xbe, 0x5d } };
		return guid_contextAddAfterPlaying;
	}
	virtual bool get_item_description(unsigned p_index,pfc::string_base & p_out){
		p_out = "Places the selected song in the playlist after the currently playling song";
		return true;
	}
};
static contextmenu_item_factory_t< my_contextmenu > foo_contextmenu;
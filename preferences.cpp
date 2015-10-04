#include "stdafx.h"
#include "resource.h"

// {87C514D4-776B-4ed2-9CBB-E4AD52C40707}
static const GUID guid_cfgMovePlaylistContext = { 0x87c514d4, 0x776b, 0x4ed2, { 0x9c, 0xbb, 0xe4, 0xad, 0x52, 0xc4, 0x7, 0x7 } };
bool default_cfgMovePlaylistContext = true;
cfg_bool cfgMovePlaylistContext(guid_cfgMovePlaylistContext, default_cfgMovePlaylistContext);

// {53E78C2B-D357-40a5-A070-93CCFEA08217}
static const GUID guid_cfgMovePlaylistExists = { 0x53e78c2b, 0xd357, 0x40a5, { 0xa0, 0x70, 0x93, 0xcc, 0xfe, 0xa0, 0x82, 0x17 } };
bool default_cfgMovePlaylistExists = false;
cfg_bool cfgMovePlaylistExists(guid_cfgMovePlaylistExists, default_cfgMovePlaylistExists);

// {74A30E17-F2C6-465d-94BA-97EADBB352B0}
static const GUID guid_cfgFocus = { 0x74a30e17, 0xf2c6, 0x465d, { 0x94, 0xba, 0x97, 0xea, 0xdb, 0xb3, 0x52, 0xb0 } };
bool default_cfgFocus = true;
cfg_bool cfgFocus(guid_cfgFocus, default_cfgFocus);

// {E5A09D5D-2742-4102-9A37-14EF626552A4}
static const GUID guid_cfgEmptyQueue = { 0xe5a09d5d, 0x2742, 0x4102, { 0x9a, 0x37, 0x14, 0xef, 0x62, 0x65, 0x52, 0xa4 } };
bool default_cfgEmptyQueue = true;
cfg_bool cfgEmptyQueue(guid_cfgEmptyQueue, default_cfgEmptyQueue);

// {ECD7510C-63C5-465c-822C-4054252FA391}
static const GUID guid_cfgEnqueueOnLock = { 0xecd7510c, 0x63c5, 0x465c, { 0x82, 0x2c, 0x40, 0x54, 0x25, 0x2f, 0xa3, 0x91 } };
bool default_cfgEnqueueOnLock = true;
cfg_bool cfgEnqueueOnLock(guid_cfgEnqueueOnLock, default_cfgEnqueueOnLock);

// {D1EE91FE-CFEF-4425-95DA-485691DD54B3}
static const GUID guid_cfgSetPlaybackOrder = { 0xd1ee91fe, 0xcfef, 0x4425, { 0x95, 0xda, 0x48, 0x56, 0x91, 0xdd, 0x54, 0xb3 } };
bool default_cfgSetPlaybackOrder = true;
cfg_bool cfgSetPlaybackOrder(guid_cfgSetPlaybackOrder, default_cfgSetPlaybackOrder);

// {8079575C-9728-4b2d-892B-8A0D9F54713B}
static const GUID guid_cfgNoPlayPlace = { 0x8079575c, 0x9728, 0x4b2d, { 0x89, 0x2b, 0x8a, 0xd, 0x9f, 0x54, 0x71, 0x3b } };
bool default_cfgNoPlayPlace = true;
cfg_bool cfgNoPlayPlace(guid_cfgNoPlayPlace, default_cfgNoPlayPlace);


std::unordered_map<int, std::pair<cfg_bool*, bool>> bool_var_map({
	{ IDC_EMPTY_QUEUE, { &cfgEmptyQueue, default_cfgEmptyQueue } },
	{ IDC_FOCUS, { &cfgFocus, default_cfgFocus } },
	{ IDC_MOVE_PLAYLIST_EXIST, { &cfgMovePlaylistExists, default_cfgMovePlaylistExists } },
	{ IDC_MOVE_PLAYLIST_CONTEXT, { &cfgMovePlaylistContext, default_cfgMovePlaylistContext } },
	{ IDC_LOCK_ENQUEUE, { &cfgEnqueueOnLock, default_cfgEnqueueOnLock } },
	{ IDC_PLAYBACK_ORDER, { &cfgSetPlaybackOrder, default_cfgSetPlaybackOrder } },
	{ IDC_NOPLAY_PLACE, { &cfgNoPlayPlace, default_cfgNoPlayPlace } },
});


class pref_page_window : public CDialogImpl<pref_page_window>, public preferences_page_instance
{
	preferences_page_callback::ptr callback;
public:
	enum { IDD = IDD_CONFIG };

	pref_page_window(preferences_page_callback::ptr callback) : callback(callback) {}

	BEGIN_MSG_MAP(pref_page_window)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_COMMAND(OnCommand)
	END_MSG_MAP()

	t_uint32 get_state(){
		t_uint32 state = preferences_state::resettable;
		if (isDirty())
			state |= preferences_state::changed;
		return state;
	}
	
	void apply(){
		for (auto e : bool_var_map){
			*e.second.first = IsDlgButtonChecked(e.first) != 0;
			callback->on_state_changed();
		}
	};

	bool isDirty(){
		for (auto e : bool_var_map){
			if (*(e.second.first) != (IsDlgButtonChecked(e.first) != 0)){
				return true;
			}
		}
		return false;
	}

	void reset(){
		for (auto e: bool_var_map){
			CheckDlgButton(e.first, e.second.second);
		}
		callback->on_state_changed();
	};

	LRESULT OnInitDialog(CWindow wndFocus, LPARAM lInitParam){
		for (auto e : bool_var_map){
			CheckDlgButton(e.first, *(e.second.first));
		}
		return 0;
	}

	void OnCommand(UINT uNotifyCode, int nID, CWindow wndCtl){
		callback->on_state_changed();
	}
};

class pref_page : public preferences_page_impl<pref_page_window>
{
	const char * get_name(){
		return "Track Positioner";
	}

	GUID get_guid(){
		// {2A1FF559-732D-4dc9-9580-3F257FD3C54F}
		static const GUID guid_preferences = { 0x2a1ff559, 0x732d, 0x4dc9, { 0x95, 0x80, 0x3f, 0x25, 0x7f, 0xd3, 0xc5, 0x4f } };
		return guid_preferences;
	}

	GUID get_parent_guid(){
		return preferences_page::guid_tools;
	}
};

static preferences_page_factory_t<pref_page> g_pref;
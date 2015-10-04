#include "stdafx.h"
#include "resource.h"

extern cfg_bool cfgMovePlaylistContext;
extern cfg_bool cfgMovePlaylistExists;
extern cfg_bool cfgFocus;
extern cfg_bool cfgEmptyQueue;
extern cfg_bool cfgEnqueueOnLock;
extern cfg_bool cfgSetPlaybackOrder;
extern cfg_bool cfgNoPlayPlace;


struct {
   int id;
   cfg_bool *var;
} bool_var_map[] = 
{
	{ IDC_EMPTY_QUEUE, &cfgEmptyQueue },
	{ IDC_FOCUS, &cfgFocus },
	{ IDC_MOVE_PLAYLIST_EXIST, &cfgMovePlaylistExists },
	{ IDC_MOVE_PLAYLIST_CONTEXT, &cfgMovePlaylistContext },
	{ IDC_LOCK_ENQUEUE, &cfgEnqueueOnLock },
	{ IDC_PLAYBACK_ORDER, &cfgSetPlaybackOrder },
	{ IDC_NOPLAY_PLACE, &cfgNoPlayPlace },
};

class pref_page : public preferences_page
{  
   static BOOL CALLBACK dialog_proc( HWND wnd, UINT msg, WPARAM wp, LPARAM lp ){
	   if( msg == WM_INITDIALOG){
		   int n, m;
		   m = tabsize( bool_var_map );
		   // Setup ALL check boxes
		   for ( n = 0; n < m; n++ )
		   {
			   CheckDlgButton( wnd, bool_var_map[n].id, (bool_var_map[n].var->get_value()) ? BST_CHECKED : BST_UNCHECKED ) ;            
		   }
	   } else if ( msg == WM_COMMAND){
		   // Handle all the booleans in bool_var_map
		   if ( wp >> 16 == BN_CLICKED )
		   {
			   int button = wp & 0xffff;
			   int m = tabsize( bool_var_map );
			   int n;

			   for ( n = 0; n < m; n++ )
			   {
				   if ( bool_var_map[n].id == button )
				   {
					   bool val = ( BST_CHECKED == IsDlgButtonChecked( wnd, bool_var_map[n].id ) );
					   (*bool_var_map[n].var) = val;
					   return false;
				   }
			   }
		   }
      }
      return false;
   }

   virtual HWND create(HWND parent){
      return uCreateDialog( IDD_CONFIG, parent, dialog_proc );
   }

   virtual const char * get_name(){
      return "Track Positioner";
   }

   virtual GUID get_guid(){
	   // {2A1FF559-732D-4dc9-9580-3F257FD3C54F}
	   static const GUID guid_preferences = 
	   { 0x2a1ff559, 0x732d, 0x4dc9, { 0x95, 0x80, 0x3f, 0x25, 0x7f, 0xd3, 0xc5, 0x4f } };

	   return guid_preferences;
   }

   virtual GUID get_parent_guid(){
	   return preferences_page::guid_tools;
   }

   //! Queries whether this page supports "reset page" feature.
   virtual bool reset_query(){
	   return false;
   }
   virtual void reset(){
   }
};

static service_factory_single_t<pref_page> g_pref;
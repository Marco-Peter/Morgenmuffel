/**
 * playlist.h
 *
 * Implementation for music sources, which can be chained together
 *
 * 2018 by M. Peter
 *
 *****************************************************************************/

#ifndef __PLAYLIST_H__
#define __PLAYLIST_H__

#include <zephyr/types.h>
#include <wchar.h>

#define PLAYLIST_MAX_NAME_LENGTH 64

/// Protocol type for the item
enum Playlist_ProtocolType_t {
	/// No protocol, should never be used
	PlaylistProtoNone,
	/// FM radio
	PlaylistProtoFm,
	/// DAB radio
	PlaylistProtoDab,
	/// Local file
	PlaylistProtoFile,
	/// Web source (internet radio, media server)
	PlaylistProtoTcp
};

/// Which item is to be selected
enum Playlist_ItemSelection_t {
	/// Select the current item
	Playlist_CurrentItem,
	/// Select the next item in the sequence
	Playlist_NextItem,
	/// Select the previous item in the sequence
	Playlist_PreviousItem
};

int playlistInit(void);
int playlistGetItem(enum Playlist_ItemSelection_t selection);
int playlistSetItem(uint16_t id);
int playlistItemRemove(uint16_t id);

uint16_t playlistGetItemCount(void);

#endif // __PLAYLIST_H__

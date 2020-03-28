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

// Protocol type for the item
typedef enum ProtocolType_t {
	PlaylistProtoNone,
	PlaylistProtoFm,
	PlaylistProtoDab,
	PlaylistProtoFile,
	PlaylistProtoTcp
} ProtocolType_t;

typedef struct PlaylistItemHeader_t {
	/// The identifier of the playlist item
	u16_t id;
	/// The total length of the playlist item including this header
	u16_t len;
	/// Label string including the string delimiter.
	wchar_t label[PLAYLIST_MAX_NAME_LENGTH];
	/// Provides information about the source type
	ProtocolType_t protocolType;
} PlaylistItem_t;

Error_t playlistItemWrite(const PlaylistItemHeader_t *item);
Error_t playlistItemRead(PlaylistItemHeader_t *item);
uint16_t playlistItemReadSize(void);
ProtocolType_t playlistItemReadProtocolType(void);

Error_t playlistItemRemove(void);
Error_t playlistItemGotoIndex(uint16_t index);
Error_t playlistItemGotoId(uint32_t id);

uint16_t playlistGetItemCount(void);

#endif // __PLAYLIST_H__

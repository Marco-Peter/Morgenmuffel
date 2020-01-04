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

#include <stdbool.h>
#include <stdint.h>
#include <wchar.h>

// Protocol type for the item
typedef enum ProtocolType_t {
  PlaylistProtoNone,
  PlaylistProtoFm,
  PlaylistProtoDab,
  PlaylistProtoFile,
  PlaylistProtoTcp
} ProtocolType_t;

// Generic part of a playlist item
typedef struct PlaylistItem_t PlaylistItem_t;

Error_t playlistCreate(const char *fileName);
Error_t playlistOpen(const char *fileName);
Error_t playlistClose(void);
Error_t playlistDestroy(void);

Error_t playlistItemCreate(ProtocolType_t type, const wchar_t *label,
                           const void *payload, size_t payloadLen);
uint16_t playlistItemReadSize(void);
uint32_t playlistItemReadId(void);
ProtocolType_t playlistItemReadProtocolType(void);
uint8_t playlistItemReadLabelLength(void);
uint8_t playlistItemReadLabel(wchar_t *label, uint8_t maxLength);
uint8_t playlistItemReadPayloadLength(void);
uint16_t playlistItemReadPayload(void *payload, uint16_t maxLength);

Error_t playlistItemRemove(void);
Error_t playlistItemGotoIndex(uint16_t index);
Error_t playlistItemGotoId(uint32_t id);

uint16_t playlistGetItemCount(void);

#endif // __PLAYLIST_H__

/**
 * playlist.c
 *
 *  Implementation for music sources, which can be chained together
 *
 * 2017 by M. Peter
 *
 * Organisation of the playlist items on storage:
 * 2 bytes total playlist item length
 * 4 bytes identifier
 * 1 byte protocol type
 * 1 byte label length
 * n bytes label
 * m bytes protocol specific data which is not handled here at all
 *
 *****************************************************************************/

#include "playlist.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "parameters.h"

/**
 * Playlist basic information
 */
struct Playlist_PermanentData {
	/// Currently selected playlist item
	uint16_t currentElement;
};

/**
 * Internal header information of a playlist information
 */
struct Playlist_ItemHeader_t {
	/// Label string including the string delimiter.
	wchar_t label[PLAYLIST_MAX_NAME_LENGTH];
	/// The identifier of the playlist item
	uint16_t id;
	/// Identifier of the next entry
	uint16_t next;
	/// Identifier of the previous entry
	uint16_t previous;
	/// Provides information about the source type
	enum Playlist_ProtocolType_t;
};

struct {
	uint16_t currentElement;
} playlist;

/**
 * Initialisation of the playlist handler
 * 
 * @return An error state, hopefully 0
 */
int playlistInit(void)
{
	int rc;

	struct Playlist_PermanentData data;
	rc = storage_read_parameter(PARAMETERS_BASE_PLAYLIST, &data,
				    sizeof(data));
	if (rc != sizeof(data)) {
		return -EIO;
	}

	playlist.currentElement = data.currentElement;

	return 0;
}

static int readParameter(uint16_t id, void *data, size_t len)
{
}

/**
 * Get the selected playlist item
 * 
 * @param selection	Return the current, the next or previous item.
 * @return The The item belonging to the selected playlist item.
 */
int playlistGetItem(enum Playlist_ItemSelection_t selection)
{
	struct Playlist_ItemHeader_t data;
	int rc;

	rc = storage_read_parameter(playlist.currentElement, &data,
				    sizeof(data));
	if (rc != sizeof(data)) {
		return rc;
	}

	switch (selection) {
	case Playlist_CurrentItem:
		break;

	case Playlist_NextItem:
		rc = storage_read_parameter(data.next, &data, sizeof(data));
		break;

	case Playlist_PreviousItem:
		rc = storage_read_parameter(data.previous, &data, sizeof(data));
		break;
	}
	return data.id;
}

/**
 * Create a new playlist item
 *
 * Organisation of the playlist items on storage:
 * 2 bytes total playlist item length
 * 4 bytes identifier
 * 1 byte protocol type
 * 1 byte label length
 * n bytes label
 * m bytes protocol specific data which is not handled here at all
 *
 * @param	type		The playlist item type
 * @param	label		The label to assign with the item
 * @param	payload		The payload to assign to the item. Set to NULL
 *to prevent data being copied
 * @param	payloadLen	The length of the assigned payload
 * @return	An error state
 *
 *****************************************************************************/
int playlistItemCreate(ProtocolType_t type, const wchar_t *label,
		       const void *payload, size_t payloadLen)
{
	int rc;
	int32_t retVal;

	uint8_t labelLen = wcslen(label) * sizeof(*label);
	uint16_t len = PL_ITEM_LENGTH_HEADER + labelLen + payloadLen;

	/* Just open the target area within the file, without writing anything yet */
	rc = storageInsertBytes(currentList.file, NULL, len);
	if (rc != Error_None) {
		return Error_Memory;
	}

	/* Now write down the data */
	retVal = SPIFFS_write(&spiFileSys, currentList.file, &len, sizeof(len));
	if (retVal != sizeof(len)) {
		return Error_Memory;
	}

	retVal = SPIFFS_write(&spiFileSys, currentList.file,
			      &currentList.nextId, sizeof(currentList.nextId));
	if (retVal != sizeof(currentList.nextId)) {
		return Error_Memory;
	}
	currentList.nextId++;

	retVal = SPIFFS_write(&spiFileSys, currentList.file, &type,
			      sizeof(type));
	if (retVal != sizeof(type)) {
		return Error_Memory;
	}

	retVal = SPIFFS_write(&spiFileSys, currentList.file, &labelLen,
			      sizeof(labelLen));
	if (retVal != sizeof(labelLen)) {
		return Error_Memory;
	}

	retVal = SPIFFS_write(&spiFileSys, currentList.file, label, labelLen);
	if (retVal != labelLen) {
		return Error_Memory;
	}

	retVal = SPIFFS_write(&spiFileSys, currentList.file, payload,
			      payloadLen);
	if (retVal != (int32_t)payloadLen) {
		return Error_Memory;
	}
	return Error_None;
}

/**
 * Get the size of the current playlist item
 *
 * @return The item size of 0 when an error occured
 *
 ******************************************************************************/
uint16_t playlistItemReadSize(void)
{
	int rc;
	uint16_t size;

	rc = playlistItemReadOffset(&size, sizeof(size), 0);
	if (rc == Error_None) {
		return size;
	} else {
		return 0;
	}
}

/**
 * Read the item identifier of the current entry
 *
 *
 * @return	The identifier or 0 when something happened
 *
 ******************************************************************************/
uint32_t playlistItemReadId(void)
{
	int rc;
	uint32_t crc;

	rc = playlistItemReadOffset(&crc, sizeof(crc), PL_ITEM_LENGTH_LEN);
	if (rc == Error_None) {
		return crc;
	} else {
		return 0;
	}
}

/**
 * Read the protocol type of the current entry
 *
 *
 * @return	The protocol type or PlaylistProtoNone when something happened
 *
 ******************************************************************************/
ProtocolType_t playlistItemReadProtocolType(void)
{
	int rc;
	ProtocolType_t protocolType;

	rc = playlistItemReadOffset(&protocolType, sizeof(protocolType),
				    PL_ITEM_LENGTH_LEN + PL_ITEM_LENGTH_ID);
	if (rc == Error_None) {
		return protocolType;
	} else {
		return PlaylistProtoNone;
	}
}

/**
 * Read the label length of the current entry
 *
 *
 * @return	The label length or zero when something happened
 *
 ******************************************************************************/
uint8_t playlistItemReadLabelLength(void)
{
	int rc;
	uint8_t labelLength;

	rc = playlistItemReadOffset(&labelLength, sizeof(labelLength),
				    PL_ITEM_LENGTH_LEN + PL_ITEM_LENGTH_ID +
					    PL_ITEM_LENGTH_PROTO);
	if (rc == Error_None) {
		return labelLength;
	} else {
		return 0;
	}
}

/**
 * Read the label of the current entry
 *
 * @param label		The target location for the read label
 * @param maxLength	The maximum length excluding the terminating /0
 *character, for which the array must be sized too.
 * @return	The label length or 0 when something happened
 *
 ******************************************************************************/
uint8_t playlistItemReadLabel(wchar_t *label, uint8_t maxLength)
{
	int rc;
	uint8_t labelLength;

	labelLength = playlistItemReadLabelLength();
	if (labelLength == 0) {
		return 0;
	}

	if (labelLength > maxLength) {
		labelLength = maxLength;
	}

	rc = playlistItemReadOffset(&label, labelLength * sizeof(wchar_t),
				    PL_ITEM_LENGTH_HEADER);
	if (rc != Error_None) {
		return 0;
	}

	label[labelLength] = L'0';

	return labelLength;
}

/**
 * Read the label length of the current entry
 *
 *
 * @return	The label length or zero when something happened
 *
 ******************************************************************************/
uint8_t playlistItemReadPayloadLength(void)
{
	int rc;
	uint16_t len;
	uint16_t payloadLen;
	uint16_t payloadOffset;
	uint8_t labelLength;

	rc = playlistItemReadOffset(&len, sizeof(len), 0);
	if (rc != Error_None) {
		return 0;
	}

	labelLength = playlistItemReadLabelLength();
	if (labelLength == 0) {
		return 0;
	}

	payloadOffset = PL_ITEM_LENGTH_HEADER + labelLength;
	payloadLen = len - payloadOffset;

	return payloadLen;
}

/**
 * Read the payload of the current entry
 *
 * @param payload	The target location for the read payload
 * @param maxLength	The maximum length to read.
 * @return	The read payload length or 0 when something happened
 *
 ******************************************************************************/
uint16_t playlistItemReadPayload(void *payload, uint16_t maxLength)
{
	int rc;
	uint16_t len;
	uint16_t payloadLen;
	uint16_t payloadOffset;
	uint8_t labelLength;

	rc = playlistItemReadOffset(&len, sizeof(len), 0);
	if (rc != Error_None) {
		return 0;
	}

	labelLength = playlistItemReadLabelLength();
	if (labelLength == 0) {
		return 0;
	}

	payloadOffset = PL_ITEM_LENGTH_HEADER + labelLength;
	payloadLen = len - payloadOffset;

	if (payloadLen > maxLength) {
		payloadLen = maxLength;
	}

	rc = playlistItemReadOffset(&payload, payloadLen, payloadOffset);
	if (rc != Error_None) {
		return 0;
	}

	return payloadLen;
}

/**
 * Remove the currently selected playlist item
 *
 *
 * @return	An error code
 *
 ******************************************************************************/
int playlistItemRemove(void)
{
	uint16_t len;
	int rc;

	len = playlistItemReadSize();
	if (len == 0) {
		return Error_Memory;
	}

	rc = storageRemoveBytes(currentList.file, len);

	return rc;
}

/**
 * Move to the item with the given index
 *
 * @param index Playlist item on the given index
 * @return     Error_None, when successful, an error code otherwise
 *
 ******************************************************************************/
int playlistItemGotoIndex(uint16_t index)
{
	int32_t retVal;

	/* We can not parse backwards, therefore we must start at the beginning
     when moving back */
	if (index < currentList.curItem) {
		retVal = SPIFFS_lseek(&spiFileSys, currentList.file, 0,
				      SEEK_SET);
		if (retVal < 0) {
			return Error_Device;
		}
		currentList.curItem = 0;
	}

	for (currentList.curItem = 0; currentList.curItem < index;
	     ++currentList.curItem) {
		uint16_t size;

		size = playlistItemReadSize();
		if (size == 0) {
			return Error_Memory;
		}

		retVal = SPIFFS_lseek(&spiFileSys, currentList.file, size,
				      SEEK_CUR);
		if (size == 0 || retVal < 0) {
			return Error_Device;
		}
	}

	return Error_None;
}

/**
 * Move to the item with the given ID
 *
 * @param index Playlist item identifier
 * @return     Error_None, when successful, an error code otherwise
 *
 ******************************************************************************/
int playlistItemGotoId(uint32_t id)
{
	for (currentList.curItem = 0;
	     currentList.curItem < currentList.totItems;
	     ++currentList.curItem) {
		uint32_t identifier;
		int rc;

		rc = playlistItemGotoIndex(currentList.curItem);
		if (rc != Error_None) {
			return Error_Memory;
		}

		identifier = playlistItemReadId();
		if (identifier == 0) {
			return Error_Memory;
		}

		if (identifier == id) {
			return Error_None;
		}
	}
	return Error_NotFound;
}

/**
 * Get the number of currently stored items in the playlist
 *
 * @return The number of playlist items on file system
 *
 ******************************************************************************/
uint16_t playlistGetItemCount(void)
{
	return currentList.totItems;
}

/**
 * Read a location within the playlist item and return to the original location
 *
 * @param buf	The target buffer for the read data
 * @param len	The number of bytes to read
 * @param offset	The offset in bytes from the current location to read
 *from.
 * @return		An error code
 *
 ******************************************************************************/
static int playlistItemReadOffset(void *buf, int32_t len, int32_t offset)
{
	int32_t retVal;

	/* We are certainly not at the file end! Or, perhaps... */
	retVal = SPIFFS_eof(&spiFileSys, currentList.file);
	if (retVal != 0) {
		return Error_Memory;
	}

	retVal = SPIFFS_lseek(&spiFileSys, currentList.file, offset, SEEK_CUR);
	if (retVal < 0) {
		return Error_Memory;
	}

	retVal = SPIFFS_read(&spiFileSys, currentList.file, buf, len);
	if (retVal != len) {
		return Error_Memory;
	}

	retVal = SPIFFS_lseek(&spiFileSys, currentList.file, -offset - len,
			      SEEK_CUR);
	if (retVal < 0) {
		return Error_Memory;
	}

	return Error_None;
}

/**
 * Counts the number of items in the playlist and finds the currently largest
 * identifier.
 *
 * This function will leave the file cursor at the beginning of the file.
 *
 * @param itemCount[out]	The number of items within the playlist
 * @return				The error code of the command
 *
 ******************************************************************************/
static int playlistCheckItems(void)
{
	int32_t retVal;

	retVal = SPIFFS_lseek(&spiFileSys, currentList.file, 0, SEEK_SET);
	if (retVal < 0) {
		return Error_Device;
	}

	while (!SPIFFS_eof(&spiFileSys, currentList.file)) {
		uint32_t id;

		currentList.totItems++;

		id = playlistItemReadId();
		if (id >= currentList.nextId) {
			currentList.nextId = id + 1;
		}

		retVal = SPIFFS_lseek(&spiFileSys, currentList.file,
				      playlistItemReadSize(), SEEK_CUR);
		if (retVal < 0) {
			return Error_Device;
		}
	}

	retVal = SPIFFS_lseek(&spiFileSys, currentList.file, 0, SEEK_SET);
	if (retVal < 0) {
		return Error_Device;
	}

	currentList.curItem = 0;

	return Error_None;
}

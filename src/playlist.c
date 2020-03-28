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
#include "error_handler.h"
#include "storage.h"
#include <stdio.h>
#include <string.h>

/* Local Function declarations */
static Error_t playlistCheckItems(void);
static Error_t playlistItemReadOffset(void *buf, int32_t len, int32_t offset);

/**
 * Create a new playlist
 *
 * Any existing file with the same name will be destroyed on storage.
 *
 * @param fileName	The filename of the list to be created.
 * @return Error_None, if ok, Error_Memory otherwise
 *
 ******************************************************************************/
Error_t playlistCreate(const char *fileName)
{
	Error_t err;
	char fullName[SPIFFS_OBJ_NAME_LEN] = PLAYLIST_FILEPREFIX;
	strncat(fullName, fileName,
		SPIFFS_OBJ_NAME_LEN - sizeof(PLAYLIST_FILEPREFIX));

	/* Close a previously opened file */
	err = playlistClose();
	if (err != Error_None) {
		return err;
	}

	// (Re-) Create the file for the playlist
	currentList.file =
		SPIFFS_open(&spiFileSys, fullName,
			    SPIFFS_CREAT | SPIFFS_TRUNC | SPIFFS_RDWR, 0);
	if (currentList.file < 0) {
		debugPrint(ERROR, "Failed to create the file with error %ld",
			   SPIFFS_errno(&spiFileSys));
		return Error_Memory;
	}

	currentList.nextId = 0;
	currentList.curItem = 0;
	currentList.totItems = 0;

	return Error_None;
}

/**
 * Open an existing playlist
 *
 * Any currently opened playlist will be closed first.
 * If the playlist does not exist, it will be created automatically.
 *
 * @param fileName	The filename of the list to be opened.
 * @return Error_None, if ok, Error_Memory otherwise
 *
 ******************************************************************************/
Error_t playlistOpen(const char *fileName)
{
	Error_t err;
	char fullName[SPIFFS_OBJ_NAME_LEN] = PLAYLIST_FILEPREFIX;
	strncat(fullName, fileName,
		SPIFFS_OBJ_NAME_LEN - sizeof(PLAYLIST_FILEPREFIX));

	/* Close a previously opened file */
	err = playlistClose();
	if (err != Error_None) {
		return err;
	}

	// Open (or create) the playlist file
	currentList.file = SPIFFS_open(&spiFileSys, fullName,
				       SPIFFS_CREAT | SPIFFS_RDWR, 0);
	if (currentList.file < 0) {
		debugPrint(ERROR, "Failed to open the file with error %d", err);
		return Error_Memory;
	}

	err = playlistCheckItems();

	return err;
}

/**
 * Close the currently used playlist
 *
 * @return Error_None, if ok, Error_Memory otherwise
 *
 ******************************************************************************/
Error_t playlistClose(void)
{
	// Close a previously opened file
	int16_t err = SPIFFS_close(&spiFileSys, currentList.file);
	if (err < 0) {
		debugPrint(ERROR, "Failed to close the file with error %d",
			   err);
		return Error_Memory;
	}

	currentList.file = 0;
	currentList.nextId = 0;
	currentList.curItem = 0;
	currentList.totItems = 0;

	return Error_None;
}

/**
 * Destroys the currently opened playlist including all its contents
 *
 * @return     The newly created playlist item or zero if an error occured.
 *
 ******************************************************************************/
Error_t playlistDestroy(void)
{
	int16_t err = SPIFFS_fremove(&spiFileSys, currentList.file);
	if (err < 0) {
		debugPrint(ERROR, "Failed do remove the file with error %d",
			   err);
		return Error_Memory;
	}

	currentList.file = 0;
	currentList.nextId = 0;
	currentList.curItem = 0;
	currentList.totItems = 0;

	return Error_None;
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
Error_t playlistItemCreate(ProtocolType_t type, const wchar_t *label,
			   const void *payload, size_t payloadLen)
{
	Error_t err;
	int32_t retVal;

	uint8_t labelLen = wcslen(label) * sizeof(*label);
	uint16_t len = PL_ITEM_LENGTH_HEADER + labelLen + payloadLen;

	/* Just open the target area within the file, without writing anything yet */
	err = storageInsertBytes(currentList.file, NULL, len);
	if (err != Error_None) {
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
	Error_t err;
	uint16_t size;

	err = playlistItemReadOffset(&size, sizeof(size), 0);
	if (err == Error_None) {
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
	Error_t err;
	uint32_t crc;

	err = playlistItemReadOffset(&crc, sizeof(crc), PL_ITEM_LENGTH_LEN);
	if (err == Error_None) {
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
	Error_t err;
	ProtocolType_t protocolType;

	err = playlistItemReadOffset(&protocolType, sizeof(protocolType),
				     PL_ITEM_LENGTH_LEN + PL_ITEM_LENGTH_ID);
	if (err == Error_None) {
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
	Error_t err;
	uint8_t labelLength;

	err = playlistItemReadOffset(&labelLength, sizeof(labelLength),
				     PL_ITEM_LENGTH_LEN + PL_ITEM_LENGTH_ID +
					     PL_ITEM_LENGTH_PROTO);
	if (err == Error_None) {
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
	Error_t err;
	uint8_t labelLength;

	labelLength = playlistItemReadLabelLength();
	if (labelLength == 0) {
		return 0;
	}

	if (labelLength > maxLength) {
		labelLength = maxLength;
	}

	err = playlistItemReadOffset(&label, labelLength * sizeof(wchar_t),
				     PL_ITEM_LENGTH_HEADER);
	if (err != Error_None) {
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
	Error_t err;
	uint16_t len;
	uint16_t payloadLen;
	uint16_t payloadOffset;
	uint8_t labelLength;

	err = playlistItemReadOffset(&len, sizeof(len), 0);
	if (err != Error_None) {
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
	Error_t err;
	uint16_t len;
	uint16_t payloadLen;
	uint16_t payloadOffset;
	uint8_t labelLength;

	err = playlistItemReadOffset(&len, sizeof(len), 0);
	if (err != Error_None) {
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

	err = playlistItemReadOffset(&payload, payloadLen, payloadOffset);
	if (err != Error_None) {
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
Error_t playlistItemRemove(void)
{
	uint16_t len;
	Error_t err;

	len = playlistItemReadSize();
	if (len == 0) {
		return Error_Memory;
	}

	err = storageRemoveBytes(currentList.file, len);

	return err;
}

/**
 * Move to the item with the given index
 *
 * @param index Playlist item on the given index
 * @return     Error_None, when successful, an error code otherwise
 *
 ******************************************************************************/
Error_t playlistItemGotoIndex(uint16_t index)
{
	s32_t retVal;

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
Error_t playlistItemGotoId(uint32_t id)
{
	for (currentList.curItem = 0;
	     currentList.curItem < currentList.totItems;
	     ++currentList.curItem) {
		uint32_t identifier;
		Error_t err;

		err = playlistItemGotoIndex(currentList.curItem);
		if (err != Error_None) {
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
static Error_t playlistItemReadOffset(void *buf, int32_t len, int32_t offset)
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
static Error_t playlistCheckItems(void)
{
	s32_t retVal;

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

/* Main implementation file for display related functions
 *
 *  Created on: 28.05.2017
 *      Author: marco
 */

#include "display_handler.h"
#include <logging/log.h>
#include <wchar.h>
#include <zephyr.h>

LOG_MODULE_REGISTER(display_hander);

#define STACKSIZE_DISPLAYTASK 2560

/// Redraw a display page completely
#define DISPEVENT_REDRAW ((uint32_t)0x01)

/// Refresh all changed text fields
#define DISPEVENT_REFRESH ((uint32_t)0x01)

// Internal functions
static void dispTask(void const *argument);
static int dispWriteTextField(TextField *field);

/**
 * Check the return code variable rc and return it if it is not zero
 *
 *****************************************************************************/
#define checkRc()                                                              \
  if (rc != 0) {                                                               \
    return rc;                                                                 \
  }

// Internal variables

/// Thread handle
K_THREAD_DEFINE(displayThread, STACKSIZE_DISPLAYTASK, dispTask, NULL, NULL,
                NULL, 1, 0, K_NO_WAIT);

/// The currently shown display page
static DisplayPage *_curPage = 0;

/// This mutex protects the current page variable against concurrent accesses.
static K_MUTEX_DEFINE(pageMutex);

/// Signals to the thread, when the page should be redrawn
static struct k_poll_signal displaySignalRedraw =
    K_POLL_SIGNAL_INITIALIZER(displaySignalRedraw);

/// Signals to the thread, when an item should be refreshed
static struct k_poll_signal displaySignalRefresh =
    K_POLL_SIGNAL_INITIALIZER(displaySignalRefresh);

/**
 * Initialise a new text field
 *
 * @param field    Text field to initialise
 * @param text     Text to write into the field
 * @param font     Font to use for rendering
 * @param posX     Text field location X-position
 * @param posY     Text field location Y-position
 * @param fLength  Field length
 * @param align    Text alignment within the field. Allowed values: 'l', 'c',
 *'r'
 * @param inverted Inverted text rendering (black text on white background)
 * @param blinking Text should be blinking (not yet implemented)
 * @param frame    Frame to draw around text field. Combination of values:
 *                 - DISP_FRAME_BOTTOM
 *                 - DISP_FRAME_TOP
 *                 - DISP_FRAME_LEFT
 *                 - DISP_FRAME_RIGHT
 *
 * @return         The initialised textfield instance, which can be used within
 *another function.
 *
 ******************************************************************************/
TextField *dispInitTextField(TextField *field, const wchar_t *text,
                             const Font *font, uint8_t posX, uint8_t posY,
                             uint16_t fLength, char align, bool inverted,
                             bool blinking, uint8_t frame) {
  // determine the text length in pixels
  uint32_t tLength = textLength(text, font);

  field->text = text;
  field->font = font;
  field->posX = posX;
  field->posY = posY;
  field->length = fLength;
  field->blinking = blinking;
  field->inverted = inverted;
  field->frame = frame;
  field->updated = true;

  switch (align) {
  case 'l':
    field->offset = 0;
    field->alignLeft = 1;
    field->alignRight = 0;
    break;
  case 'r':
    field->offset = fLength - tLength;
    field->alignLeft = 0;
    field->alignRight = 1;
    break;
  case 'c':
    field->offset = (fLength - tLength) / 2;
    field->alignLeft = 0;
    field->alignRight = 0;
    break;
  default: // left aligned
    field->offset = 0;
    field->alignLeft = 1;
    field->alignRight = 1;
  }

  // Set the offset to 0 when the text is longer than the frame
  if (tLength > fLength)
    field->offset = 0;

  return field;
}

/**
 * Display a new page
 *
 * @param page	The new page to be displayed, set to NULL for a blank screen
 * @return		The previously displayed page
 *
 ******************************************************************************/
DisplayPage *dispShowPage(DisplayPage *page) {
  DisplayPage *prevPage;
  uint32_t i;

  if (page != NULL) {
    // Force update of all contained text fields
    for (i = 0; i < page->nFields; ++i) {
      page->fields[i].updated = true;
    }
  }

  // Exchange displayed page on display
  k_mutex_lock(&pageMutex, K_FOREVER);
  prevPage = _curPage;
  _curPage = page;
  k_mutex_unlock(&pageMutex);

  // Notify the display task that there is a new page to be displayed
  k_poll_signal_raise(&displaySignalRedraw, 0);

  // Return the previously displayed page
  return prevPage;
}

/**
 * Simple command for dispShowPage which just clears the display.
 *
 * @return		The previously displayed page
 *
 ******************************************************************************/
DisplayPage *dispClearPage(void) { return dispShowPage(NULL); }

/**
 * Define a new text to be displayed, but keep the current settings
 *
 * @param field	The text field to assign the text to.
 * @return		The A pointer to the previous text object.
 *
 ******************************************************************************/
const wchar_t *dispSetText(TextField *field, const wchar_t *text) {
  const wchar_t *oldText = field->text;
  field->text = text;

  // Recalculate the text offset
  uint32_t tLength = textLength(text, field->font);
  if (tLength > field->length) {
    // Text longer than field --> Scrolling text
    field->offset = 0;
  } else if (field->alignLeft == field->alignRight) {
    // Centered text
    field->offset = (field->length - tLength) / 2;
  } else if (field->alignRight) {
    field->offset = field->length - tLength;
  } else {
    field->offset = 0;
  }
  field->updated = true;

  // Notify the display task that there is an updated field to be displayed
  k_poll_signal_raise(&displaySignalRefresh, 0);

  // Return the previously displayed text
  return oldText;
}

/**
 * Set the inversion state of the given text field
 *
 * @param inversion	False: White text on black background
 *					True: Black text on white background
 *
 ******************************************************************************/
void dispSetInversion(TextField *field, uint8_t inversion) {
  // Update the inversion state of the text field
  field->inverted = inversion;
  field->updated = true;

  // Notify the display task that there is an updated field to be displayed
  k_poll_signal_raise(&displaySignalRefresh, 0);
}

/**
 * (Re-)Write a text field on the display
 *
 * @param field	The textfield to be written.
 *
 ******************************************************************************/
static int dispWriteTextField(TextField *field) {
  int rc;
  uint8_t curPage;

  // Take the mutex, so that no other task can interfere

  // Get the actual string length
  uint32_t tLength = textLength(field->text, field->font);

  // When left or right hand frame lines are drawn put the text 2 pixels away
  // from it
  uint16_t fLength = field->length;
  if (field->frame & DISP_FRAME_LEFT)
    fLength -= 2;
  if (field->frame & DISP_FRAME_RIGHT)
    fLength -= 2;

  // We only need to work, if the text field got updated or is larger than the
  // text field. Otherwise leave immediately.
  if ((field->updated == true) || (tLength > fLength)) {
    // We work one page after the other
    for (curPage = 0; curPage < (field->font->size / DISPLAY_PAGESIZE);
         ++curPage) {
      // set the current character pointer onto the text start
      // as well as the current row on the character
      const wchar_t *curChar = field->text;
      uint16_t offset = 0;
      uint8_t posY;
      uint8_t curCol =
          0; // Current column on character (including bearing and advance)
      uint8_t renderedCols = 0; // Number of rendered columns
      uint8_t framePattern = 0;
      uint8_t lastPage =
          (curPage + 1 == (field->font->size / DISPLAY_PAGESIZE));

      if ((curPage == 0) && (field->frame & DISP_FRAME_TOP))
        framePattern |= 0x01;
      else if (lastPage && (field->frame & DISP_FRAME_BOTTOM))
        framePattern |= 0x80;
      else
        framePattern = 0x00;

      // Position the cursor on the display
      posY =
          displaySetPos(field->posX, field->posY + curPage * DISPLAY_PAGESIZE);
      if ((posY < 0) || (posY != (field->posY + curPage * DISPLAY_PAGESIZE))) {
        return -EINVAL;
      }

      // Render the left frame line and the empty space after the frame line
      if (field->frame & DISP_FRAME_LEFT) {
        rc = displayMemset(field->inverted ? 0x00 : 0xFF, 1);
        checkRc();

        rc = displayMemset(field->inverted ? ~framePattern : framePattern, 1);
        checkRc();
      }

      if (tLength <= fLength) {
        // Text length is shorter than the usable field length, depending on the
        // offset it is shifted to the right.

        // calculate the required offset for right and center aligned text
        if (field->alignRight && !field->alignLeft) {
          offset = fLength - tLength;
        } else if (!field->alignLeft && !field->alignRight) {
          offset = (fLength - tLength) / 2;
        }
        rc = displayMemset(field->inverted ? ~framePattern : framePattern,
                           offset - renderedCols);
        checkRc();

        // update the number of rendered columns within the field
        renderedCols += offset;

      } else if (tLength > fLength) {
        // text is longer than the field length, depending on the
        // offset it is shifted to the left.
        // Find out how much of the text already disappeared on the left
        // display border.
        offset = 0;
        const Glyph *curGlyph = getGlyph(*curChar, field->font);
        while (field->offset > offset + curGlyph->desc.advance) {
          offset += curGlyph->desc.advance;
          ++curChar;
          curGlyph = getGlyph(*curChar, field->font);
        }
        // Now we are on the character which must be rendered at least partially
        curCol = field->offset - offset;
      }
      // render as much text as possible within the field borders
      while (curChar != NULL && *curChar != 0 && renderedCols < fLength) {
        uint8_t remCols;

        // Get the glyph to be rendered
        const Glyph *curGlyph = getGlyph(*curChar, field->font);
        // Render the part before the character
        if (curCol < curGlyph->desc.bearingX) {
          // Calculate how much of the current bearing still must be rendered
          remCols = curGlyph->desc.bearingX - curCol;

          // If we are already on the end of the text field, cap the last
          // character
          if ((renderedCols + remCols) > fLength) {
            remCols = fLength - renderedCols;
          }
          rc = displayMemset(field->inverted ? ~framePattern : framePattern,
                             remCols);
          checkRc();

          // update the number of rendered columns
          renderedCols += remCols;
          curCol += remCols;

          // If the field is completely rendered, we are finished.
          if (renderedCols >= fLength) {
            continue;
          }
        } // if(curCol < curGlyph->desc.bearingX)

        // Render the actual character
        if (curCol < (curGlyph->desc.bearingX + curGlyph->desc.width)) {
          // Calculate how much of the current character still must be rendered
          remCols = curGlyph->desc.bearingX + curGlyph->desc.width - curCol;

          // If we are already on the end of the text field, cap the last
          // character
          if ((renderedCols + remCols) > fLength) {
            remCols = fLength - renderedCols;
          }

          // if the glyph is to be rendered inversed, we need to copy and
          // invert the data to a temporary buffer.
          // Also if we apply a frame, we will need to copy the data
          // otherwise we just take the pointer to the data to render
          if (field->inverted || field->frame) {
            uint8_t glyphData[remCols];
            uint8_t curByte;

            for (curByte = 0; curByte < remCols; ++curByte) {
              glyphData[curByte] =
                  (curGlyph->desc
                       .data[(curPage * curGlyph->desc.width) + curCol -
                             curGlyph->desc.bearingX + curByte] |
                   framePattern);
              if (field->inverted) {
                glyphData[curByte] = ~glyphData[curByte];
              }
            } // for
            rc = displayWriteData(glyphData, remCols);

          } else {
            const uint8_t *glyphData;
            glyphData = curGlyph->desc.data + (curPage * curGlyph->desc.width) +
                        curCol - curGlyph->desc.bearingX;
            rc = displayWriteData(glyphData, remCols);
          }
          checkRc();

          // update the number of rendered columns
          renderedCols += remCols;

          // If the field is completely rendered, we are finished.
          if (renderedCols >= fLength) {
            continue;
          }
          curCol += remCols;
        }
        // Render the part after the actual character
        remCols = curGlyph->desc.advance - curCol;

        // If we are already on the end of the text field, cap the last
        // character
        if ((renderedCols + remCols) > fLength) {
          remCols = fLength - renderedCols;
        }
        rc = displayMemset(field->inverted ? ~framePattern : framePattern,
                           remCols);
        checkRc();

        // update the number of rendered columns
        renderedCols += remCols;

        // If the field is completely rendered, we are finished.
        if (renderedCols >= fLength) {
          continue;
        }

        // Go to the next character and set the position within it to zero
        ++curChar;
        curCol = 0;
      } // while(curChar != 0...

      // Clear the part after the characters until the field end
      if (renderedCols < fLength) {
        rc = displayMemset(field->inverted ? ~framePattern : framePattern,
                           fLength - renderedCols);
        checkRc()
      }

      // Render the right frame line and the empty space before the frame line
      if (field->frame & DISP_FRAME_RIGHT) {
        rc = displayMemset(field->inverted ? ~framePattern : framePattern, 1);
        checkRc();

        rc = displayMemset(field->inverted ? 0x00 : 0xFF, 1);
        checkRc();
      }

    } // for(curPage...

    // Increment the offset for the next iteration
    if (tLength > fLength) {
      if (field->offset >= (tLength - fLength)) {
        field->offset = 0;
      } else {
        ++field->offset;
      }
    } else {
      // The text field will not change by itself, so set updated to false.
      field->updated = false;
    }
  } // if((field->updated == true) || (tLength > fLength))

  return 0;
}

/**
 * The actual display task, which refreshes the display all DISP_SCROLL_CYCLE ms
 *
 ******************************************************************************/
static void dispTask(void const *argument) {
  uint32_t rc;

  static struct k_poll_event events[2] = {
      K_POLL_EVENT_STATIC_INITIALIZER(
          K_POLL_TYPE_SIGNAL, K_POLL_MODE_NOTIFY_ONLY, &displaySignalRedraw, 0),
      K_POLL_EVENT_STATIC_INITIALIZER(K_POLL_TYPE_SIGNAL,
                                      K_POLL_MODE_NOTIFY_ONLY,
                                      &displaySignalRefresh, 0)};

  displayInit();
  displayOn();

  for (;;) {
    int curField;

    // Wait for a repaint command or the refresh time
    rc = k_poll(events, 2, K_MSEC(DISP_SCROLL_CYCLE));

    if (events[0].signal->signaled) {
      LOG_INF("Received redraw event");
      // On a redraw event, the complete display needs to be repainted,
      // therefore we clear it first
      displayClear();
      events[0].signal->signaled = 0;
      events[0].state = K_POLL_STATE_NOT_READY;
    }

    if (events[1].signal->signaled) {
      LOG_INF("Received refresh event");
      // No specific action yet on the refresh signal.
      events[1].signal->signaled = 0;
      events[1].state = K_POLL_STATE_NOT_READY;
    }

    // Reserve the display for exclusive use
    k_mutex_lock(&pageMutex, K_FOREVER);

    // Display the current page
    if (_curPage != NULL) {
      for (curField = 0; curField < _curPage->nFields; ++curField) {
        dispWriteTextField(_curPage->fields + curField);
      } // for(curField = 0; curField < _cur...)
    }

    k_mutex_unlock(&pageMutex);

    /* TODO: Move to better place
            displaySetContrast(&dispHandler->disp, brightness > 127 ? 127 :
       brightness); ledSetGroup(&led[ledAlarm1], brightness > 511 ? 255 :
       brightness / 2);
            */

  } // for(;;)
}

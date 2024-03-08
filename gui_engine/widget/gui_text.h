/**
 \internal
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_text.h
  * @brief text widget header file
  * @details text widget header file
  * @author luke_sun@realsil.com.cn
  * @date 2023/10/25
  * @version v1.0
  ***************************************************************************************
    * @attention
  * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
  ***************************************************************************************
 \endinternal
  */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __GUI_TEXT_H__
#define __GUI_TEXT_H__
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <string.h>
#include "guidef.h"
#include "gui_obj.h"

/** @defgroup WIDGET WIDGET
  * @brief
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup WIDGET_Exported_Types WIDGET Exported Types
  * @brief
  * @{
  */

/** @brief  text mode enum */
typedef enum
{
    LEFT = 0,
    CENTER,
    RIGHT,
    MUTI_LEFT,
    MUTI_CENTER,
    MUTI_RIGHT,
    SCROLL_X,
    SCROLL_Y,
} TEXT_MODE;

/** @brief  text encoding format enum */
typedef enum
{
    UTF_8_CHARSET = 0,
    UTF_16_CHARSET = 1,
    UNICODE_ENCODING = 1,
} TEXT_CHARSET;

/** @brief  font type enum */
typedef enum
{
    GUI_FONT_SOURCE_BMP = 0,
    GUI_FONT_SOURCE_TTF = 1,
} FONT_SOUCE_TYPE;

/** @brief  text widget structure */
typedef struct gui_text
{
    gui_obj_t base;
    gui_color_t color;
    uint16_t len;
    uint16_t font_len;
    uint16_t text_offset;
    TEXT_MODE mode;
    TEXT_CHARSET charset;
    FONT_SOUCE_TYPE font_type;
    uint8_t font_height;
    uint8_t checksum;
    gui_animate_t *animate;
    void *content;
    void *data;
    void *path;//!<  address or path
    int16_t offset_x;
    int16_t offset_y;
} gui_text_t;

/** @brief  text line structure */
typedef struct gui_text_line
{
    uint8_t line_char;
    uint8_t line_dx;
} gui_text_line_t;

/** End of WIDGET_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** @defgroup WIDGET_Exported_Constants WIDGET Exported Constants
  * @brief
  * @{
  */


/** End of WIDGET_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup WIDGET_Exported_Macros WIDGET Exported Macros
  * @brief
  * @{
  */


/** End of WIDGET_Exported_Macros
  * @}
  */

/*============================================================================*
 *                         Variables
 *============================================================================*/
/** @defgroup WIDGET_Exported_Variables WIDGET Exported Variables
  * @brief
  * @{
  */


/** End of WIDGET_Exported_Variables
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup WIDGET_Exported_GUI_Functions WIDGET Exported Functions
  * @brief
  * @{
  */

/**
* @brief set the string in a text box widget.
* @note The font size must match the font file!
* @param this the text box widget pointer.
* @param text the text string.
* @param font_type the font's name.
* @param color the text's color.
* @param length the text string's length.
* @param font_size the text string's font size.
* @return void
*/
void gui_text_set(gui_text_t *this, void *text, char *text_type, gui_color_t color,
                  uint16_t length, uint8_t font_size);

/**
 * @brief set animate
 *
 * @param o text widget
 * @param dur durtion. time length of the animate
 * @param repeatCount  0:one shoot -1:endless
 * @param callback happens at every frame
 * @param p callback's parameter
 */
void gui_text_set_animate(void *o, uint32_t dur, int repeatCount, void *callback, void *p);

/**
 * @brief set text mode of this text widget
 * @note if text line count was more than one, it will display on the left even if it was set lft or right
 * @param this the text widget pointer.
 * @param mode there was three mode: LEFT, CENTER and RIGHT.
 */
void gui_text_mode_set(gui_text_t *this, TEXT_MODE mode);

/**
 * @brief move the text widget
 *
 * @param this the text box widget pointer.
 * @param x the X-axis coordinate of the text box.
 * @param y the Y-axis coordinate of the text box.
 */
void gui_text_move(gui_text_t *this, int16_t x, int16_t y);

/**
 * @brief set font size or width and height
 * @note if use freetype, width and height is effective, else height will be applied as font size
 * @param this the text widget pointer.
 * @param height font height or font size.
 * @param width font width(only be effective when freetype was used).
 */
void gui_text_size_set(gui_text_t *this, uint8_t height, uint8_t width);

/**
 * @brief set font type
 * @note The type must match the font size!
 * @param this the text widget pointer
 * @param font_source the addr of .ttf or .bin
 */
void gui_text_type_set(gui_text_t *this, void *font_source);

/**
 * @brief set font encoding
 * @note utf-8 or unicode
 * @param this the text widget pointer
 * @param encoding_type encoding_type
 */
void gui_text_encoding_set(gui_text_t *this, TEXT_CHARSET charset);

/**
 * @brief set text content
 *
 * @param this the text widget pointer
 * @param text the text string.
 * @param length the text string's length
 */
void gui_text_content_set(gui_text_t *this, void *text, uint16_t length);

/**
 * @brief create a text box widget.
 * @note The area of the text box should be larger than that of the string to be shown, otherwise, part of the text will be hidden.
 * @param parent the father widget which the text nested in.
 * @param filename the widget's name.
 * @param x the X-axis coordinate of the text box.
 * @param y the Y-axis coordinate of the text box.
 * @param w the width of the text box.
 * @param h the hight of the text box.
 * @return return the widget object pointer
 *
 */
gui_text_t *gui_text_create(void *parent, const char *name, int16_t x, int16_t y,
                            int16_t w, int16_t h);


/** End of WIDGET_Exported_GUI_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif

/**
 \internal
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_tab.h
  * @brief tab widget header file
  * @details construct and change tab widget
  * @author howie_wang@realsil.com.cn
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
#ifndef __GUI_TAB_H__
#define __GUI_TAB_H__
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <guidef.h>
#include <gui_fb.h>
#include "gui_tabview.h"
#include "gui_matrix.h"

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

/** @brief  tab widget ID structure */
typedef struct
{
    int32_t x;
    int32_t y;
    int32_t z;
} gui_tab_id_t;

/** @brief  tab widget structure */
typedef struct gui_tab
{
    gui_obj_t base;             //!< base structure
    gui_tab_id_t id;
    SLIDE_STYLE style;
    Normal_t normal;
} gui_tab_t;

/** @brief  tab widget stacking structure */
typedef struct _gui_tab_stacking_t
{
    float scale;
    float location;
    uint8_t opacity;
} gui_tab_stacking_t;

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
 * @brief create a tab widget, which should be nested in a tabview.
 * @param parent the father widget it nested in.
 * @param filename this tab widget's name.
 * @param x the X-axis coordinate of the widget.
 * @param x the Y-axis coordinate of the widget.
 * @param w the width of the widget.
 * @param h the hight of the widget.
 * @param idx the X-axis index in a tabview.
 * @param idy the Y-axis index in a tabview.
 * @return return the widget object pointer.
 *
 */
gui_tab_t *gui_tab_create(void *parent, const char *filename, int16_t x, int16_t y,
                          int16_t w, int16_t h, int16_t idx, int16_t idy);

/**
 * @brief set style of this tab
 *
 * @param this tab widget pointer
 * @param style slide style
 */
void gui_tab_set_style(gui_tab_t *this, SLIDE_STYLE style);

/**
 * @brief
 *
 * @param obj
 */
void gui_tab_rotate(gui_obj_t *obj);

/**
 * @brief
 *
 * @param obj
 */
void gui_tab_reduction(gui_obj_t *obj);

/**
 * @brief
 *
 * @param obj
 */
void gui_tab_cube(gui_obj_t *obj);

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

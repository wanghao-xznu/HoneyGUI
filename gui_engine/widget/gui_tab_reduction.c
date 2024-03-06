/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_tab_rotate.c
  * @brief tab widget
  * @details tab widget
  * @author howie_wang@realsil.com.cn
  * @date 2023/10/25
  * @version 1.0
  ***************************************************************************************
    * @attention
  * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
  ***************************************************************************************
  */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <guidef.h>
#include <string.h>
#include <gui_server.h>
#include <gui_obj.h>
#include <tp_algo.h>
#include <kb_algo.h>
#include "gui_tab.h"
#include "gui_curtain.h"
#include "gui_img.h"
#include "gui_cube.h"



/** @defgroup WIDGET WIDGET
  * @{
  */
/*============================================================================*
 *                           Types
 *============================================================================*/
/** @defgroup WIDGET_Exported_Types WIDGET Exported Types
  * @{
  */


/** End of WIDGET_Exported_Types
  * @}
  */

/*============================================================================*
 *                           Constants
 *============================================================================*/
/** @defgroup WIDGET_Exported_Constants WIDGET Exported Constants
  * @{
  */


/** End of WIDGET_Exported_Constants
  * @}
  */

/*============================================================================*
 *                            Macros
 *============================================================================*/
/** @defgroup WIDGET_Exported_Macros WIDGET Exported Macros
  * @{
  */


/** End of WIDGET_Exported_Macros
  * @}
  */
/*============================================================================*
 *                            Variables
 *============================================================================*/
/** @defgroup WIDGET_Exported_Variables WIDGET Exported Variables
  * @{
  */


/** End of WIDGET_Exported_Variables
  * @}
  */

/*============================================================================*
 *                           Private Functions
 *============================================================================*/
/** @defgroup WIDGET_Exported_Functions WIDGET Exported Functions
  * @{
  */

void gui_tab_reduction(gui_obj_t *obj)
{
    gui_tab_t *this = (gui_tab_t *)obj;
    gui_dispdev_t *dc = gui_get_dc();
    gui_tabview_t *parent = (gui_tabview_t *)(obj->parent);
    matrix_translate((this->id.x - parent->cur_id.x) * (int)this->base.w + parent->release_x, \
                     (this->id.y - parent->cur_id.y) * (int)this->base.h + parent->release_y, \
                     obj->matrix);


    int sx = abs((this->id.x - parent->cur_id.x) * (int)this->base.w + parent->release_x);
    sx = sx % this->base.w;
    float s = 1.0f - (float)sx / this->base.w;

    if (s < 0.2f)
    {
        s = 0.2f;
    }
    if (s >= 1.0f)
    {
        s = 1.0f;
    }

    matrix_translate(dc->screen_width / 2, dc->screen_height / 2, obj->matrix);
    matrix_scale(s, s, obj->matrix);
    matrix_translate(-dc->screen_width / 2, -dc->screen_height / 2, obj->matrix);
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */

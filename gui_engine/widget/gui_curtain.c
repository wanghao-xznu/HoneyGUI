/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_curtain.c
  * @brief create a curtain effect widget,which should be nested in a curtainview.
  * @details Slide to extend and retract curtains
  * @author triton_yu@realsil.com.cn
  * @date 2023/10/24
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
#include <gui_curtain.h>
#include <string.h>
#include <gui_server.h>
#include "gui_obj.h"
#include <tp_algo.h>
#include <gui_img.h>


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



static void curtain_prepare(gui_obj_t *obj)
{
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_curtain_t *this = (gui_curtain_t *)obj;
    gui_obj_t *parent = obj->parent;
    gui_curtainview_t *curtainview = (gui_curtainview_t *)parent;
    int8_t cur_idy = 0;
    int8_t cur_idx = 0;
    int8_t idy = 0;
    int8_t idx = 0;
    switch (curtainview->cur_curtain)
    {
    case CURTAIN_UP:
        cur_idy = -1;
        cur_idx = 0;
        break;
    case CURTAIN_DOWN:
        cur_idy = 1;
        cur_idx = 0;
        break;
    case CURTAIN_LEFT:
        cur_idy = 0;
        cur_idx = -1;
        break;
    case CURTAIN_RIGHT:
        cur_idy = 0;
        cur_idx = 1;
        break;
    case CURTAIN_MIDDLE:
        cur_idy = 0;
        cur_idx = 0;
        break;
    default:
        break;
    }
    switch (this->orientation)
    {
    case CURTAIN_UP:
        idy = -1;
        idx = 0;
        break;
    case CURTAIN_DOWN:
        idy = 1;
        idx = 0;
        break;
    case CURTAIN_LEFT:
        idy = 0;
        idx = -1;
        break;
    case CURTAIN_RIGHT:
        idy = 0;
        idx = 1;
        break;
    case CURTAIN_MIDDLE:
        idy = 0;
        idx = 0;
        break;
    default:
        break;
    }

    obj->y = (idy - cur_idy) * (int)gui_get_screen_height() ;

    if ((0 == idy))
    {
        obj->y = 0 - parent->y;
    }
    else
    {
        if (cur_idy == idy)
        {
            obj->y = (idy - cur_idy) * (int)gui_get_screen_height() + (int)gui_get_screen_height() * idy *
                     (1 - this->scope);
        }
    }
    //gui_log("cur_idy:%d,idy:%d, obj->y:%d\n", cur_idy, idy, obj->y);
    {
        obj->x = (idx - cur_idx) * (int)gui_get_screen_width();
    }
    if ((0 == idx))
    {
        obj->x = 0 - parent->x;
    }
    else
    {
        if (cur_idx == idx)
        {
            obj->x = (idx - cur_idx) * (int)gui_get_screen_width() + (int)gui_get_screen_width() * idx *
                     (1 - this->scope);
        }
    }

    obj->y += curtainview->release_y;
    obj->x += curtainview->release_x;
}


void gui_curtain_ctor(gui_curtain_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                      int16_t y,
                      int16_t w, int16_t h, gui_curtain_enum_t orientation, float scope)
{
    gui_obj_ctor(&this->base, parent, filename, x, y, w, h);
    ((gui_obj_t *)this)->obj_prepare = curtain_prepare;
    ((gui_obj_t *)this)->type = CURTAIN;
    if (scope == 0)
    {
        scope = 1.0;
    }
    this->scope = scope;
    if (parent != NULL)
    {
        gui_curtainview_t *parent_ext = (gui_curtainview_t *)parent;
        this->orientation = orientation;

        parent_ext->curtain_cnt++;
        switch (orientation)
        {
        case CURTAIN_UP:
            parent_ext->orientations.up = true;
            parent_ext->scopeup = this->scope;
            GET_BASE(this)->y = -(int)gui_get_screen_height();
            break;
        case CURTAIN_DOWN:
            parent_ext->orientations.down = true;
            parent_ext->scopedown = this->scope;
            GET_BASE(this)->y = (int)gui_get_screen_height();
            break;
        case CURTAIN_LEFT:
            parent_ext->orientations.left = true;
            parent_ext->scopeleft = this->scope;
            GET_BASE(this)->x = -(int)gui_get_screen_width();
            break;
        case CURTAIN_RIGHT:
            parent_ext->orientations.right = true;
            parent_ext->scoperight = this->scope;
            GET_BASE(this)->x = (int)gui_get_screen_width();
            break;
        default:
            break;
        }

    }
}
/*============================================================================*
 *                           Public Functions
 *============================================================================*/

gui_curtain_t *gui_curtain_create(void *parent, const char *filename, int16_t x, int16_t y,
                                  int16_t w, int16_t h, gui_curtain_enum_t orientation, float scope)
{
#define _paramgui_curtain_create_ this, parent, filename, x, y, w, h, orientation, scope
    GUI_NEW(gui_curtain_t, gui_curtain_ctor, _paramgui_curtain_create_)
}


/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */





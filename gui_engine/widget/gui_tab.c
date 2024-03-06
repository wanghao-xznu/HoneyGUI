/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_tab.c
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
static void tab_prepare_fade(gui_obj_t *obj);

static void tab_prepare(gui_obj_t *obj)
{
    gui_tab_t *this = (gui_tab_t *)obj;
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    kb_info_t *kb = kb_get_info();
    gui_tabview_t *parent = (gui_tabview_t *)(obj->parent);

    if (this->style == CLASSIC)
    {
        matrix_translate((this->id.x - parent->cur_id.x) * (int)this->base.w + parent->release_x, \
                         (this->id.y - parent->cur_id.y) * (int)this->base.h + parent->release_y, \
                         obj->matrix);
    }
    else if (this->style == TAB_CUBE)
    {
        gui_tab_cube(obj);
    }
    else if (this->style == TAB_ROTATE)
    {
        gui_tab_rotate(obj);
    }
    else if (this->style == REDUCTION)
    {
        gui_tab_reduction(obj);
    }
    else if (this->style == FADE)
    {
        tab_prepare_fade(obj);
    }
    else if (this->style == REDUCTION_FADE)
    {
        float s;

        int sx = abs((this->id.x - parent->cur_id.x) * (int)this->base.w + parent->release_x);
        sx = sx % this->base.w;
        s = 1.0f - (float)sx / this->base.w;

        if (s < 0.2f)
        {
            s = 0.2f;
        }
        if (s >= 1.0f)
        {
            s = 1.0f;
        }

        tab_prepare_fade(obj);
    }


    if ((kb->type == KB_SHORT) && (obj->event_dsc_cnt > 0))
    {
        gui_obj_event_set(obj, GUI_EVENT_KB_SHORT_CLICKED);
    }
}

static void tab_root_img_fade(gui_obj_t *object, float xx, float yy)
{
    gui_list_t *node = NULL;
    gui_list_t *tmp = NULL;
    float x = xx * 0.6f + 0.4f;
    float y = yy * 0.6f + 0.4f;

    gui_list_for_each_safe(node, tmp, &object->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if ((!(obj->parent->type == CURTAIN &&
               ((gui_curtain_t *)obj->parent)->orientation != CURTAIN_MIDDLE)) &&
            (!(obj->parent->parent->type == CURTAIN &&
               ((gui_curtain_t *)obj->parent->parent)->orientation != CURTAIN_MIDDLE)))
        {
            switch (obj->type)
            {
            case IMAGE_FROM_MEM:
                {
                    gui_img_t *img = (void *)obj;
                    gui_img_set_opacity(img, x * UINT8_MAX);
                    break;
                }
            case VG_LITE_CUBE:
                {
                    gui_cube_t *cube = (void *)obj;
                    gui_cube_set_opacity(cube, CUBE_SIDE_ALL, x * UINT8_MAX);
                    break;
                }
            case CANVAS:
                {
                    break;
                }

            default:
                break;
            }
        }
        tab_root_img_fade(obj, xx, yy);
    }
}

#define TAB_ASIDE(id, cur) (abs(id - cur) <= 1)
static void tab_prepare_fade(gui_obj_t *obj)
{
    touch_info_t *tp = tp_get_info();
    gui_tab_t *tab = (gui_tab_t *)obj;
    gui_tab_t *this = (gui_tab_t *)obj;
    gui_tabview_t *parent = (gui_tabview_t *)(obj->parent);
    /**
     * @note tp->deltaX >0 means slide right, the left tab shows
     *       tp->deltaX <0 means slide left, the right tab shows
     *       the left tab's dx -320~0
     *       the right tab's dx 320~0
     */
    if (((tp->type == TOUCH_HOLD_X) || (tp->type == TOUCH_LEFT_SLIDE) ||
         (tp->type == TOUCH_RIGHT_SLIDE) || (tp->type == TOUCH_INVALIDE))  &&
        TAB_ASIDE(tab->id.x, parent->cur_id.x) && tab->id.y == 0)
    {
        // gui_log("TOUCH_HOLD_X: %d, TOUCH_LEFT_SLIDE: %d, TOUCH_RIGHT_SLIDE: %d, TOUCH_INVALIDE: %d, %d\n", \
        // (tp->type == TOUCH_HOLD_X), (tp->type == TOUCH_LEFT_SLIDE), (tp->type == TOUCH_RIGHT_SLIDE),(tp->type == TOUCH_INVALIDE),\
        // tab->id.x - parent->cur_id.x );

        // current tab decline, aside tab enhance.
        float fade_dir = (tab->id.x == parent->cur_id.x) ? -1.f : 1.f;
        // slide right delta > 0, slide left delta < 0
        float slide_dir = (tp->deltaX > 0) ? 1.f : (tp->deltaX < 0) ? -1.f : 0;
        // slide right dx > 0, slide left dx < 0
        float fade_percent = 1.f + (fade_dir * slide_dir * 0/*todo*/) / this->base.w;

        tab_root_img_fade(obj, fade_percent, fade_percent);
    }
}

static void gui_tab_ctor(gui_tab_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                         int16_t y,
                         int16_t w, int16_t h, int16_t idx, int16_t idy)
{

    gui_obj_ctor(&this->base, parent, filename, x, y, w, h);

    GET_BASE(this)->obj_prepare = tab_prepare;
    GET_BASE(this)->type = TAB;

    gui_tabview_t *parent_ext = (gui_tabview_t *)parent;
    this->style = parent_ext->style;
    this->id.x = idx;
    this->id.y = idy;
    this->id.z = idy;

    this->normal.x = 0;
    this->normal.y = 0;
    this->normal.z = -1.0f;

    parent_ext->tab_cnt++;
    if (idx > 0)
    {
        parent_ext->tab_cnt_right++;
    }
    else if (idx < 0)
    {
        parent_ext->tab_cnt_left--;
    }
    if (idy > 0)
    {
        parent_ext->tab_cnt_down++;
    }
    else if (idy < 0)
    {
        parent_ext->tab_cnt_up--;
    }

}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

gui_tab_t *gui_tab_create(void *parent, const char *name, int16_t x, int16_t y,
                          int16_t w, int16_t h, int16_t idx, int16_t idy)
{
    if (w == 0)
    {
        w = (int)gui_get_screen_width();
    }
    if (h == 0)
    {
        h = (int)gui_get_screen_height();
    }

    gui_tab_t *this = gui_malloc(sizeof(gui_tab_t));
    GUI_ASSERT(this != NULL);
    memset(this, 0x00, sizeof(gui_tab_t));

    gui_tab_ctor(this, (gui_obj_t *)parent, name, x, y, w, h, idx, idy);

    gui_list_init(&(GET_BASE(this)->child_list));
    if ((GET_BASE(this)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(this)->parent)->child_list),
                               &(GET_BASE(this)->brother_list));
    }
    GET_BASE(this)->create_done = true;
    return this;
}

void gui_tab_set_style(gui_tab_t *this, SLIDE_STYLE style)
{
    this->style = style;
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */

/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_card.c
  * @brief 3d card container
  * @details slide to switch card
  * @author howie_wang@realsil.com.cn
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
#include <string.h>
#include <gui_server.h>
#include <gui_obj.h>
#include <tp_algo.h>
#include "gui_card.h"




/** @defgroup WIDGET WIDGET
  * @{
  */
/*============================================================================*
 *                           Types
 *============================================================================*/
/** @defgroup WIDGET_Exported_Types WIDGET Exported Types
  * @{
  */

/** @brief  ... */
typedef enum
{
    EXAMPLE_SUCCESS,                //!< ...
    EXAMPLE_INVALID_STATE,          //!< ...
} T_EXAMPLE;

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
static void hide_child(gui_obj_t *object)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &object->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        obj->not_show = true;
        hide_child(obj);
    }
}
static void show_child(gui_obj_t *object)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &object->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        obj->not_show = false;
        hide_child(obj);
    }
}

static void tab_prepare(gui_obj_t *obj)
{
    gui_card_t *this = (gui_card_t *)obj;
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_cardview_t *parent = (gui_cardview_t *)(obj->parent);


    float h = gui_get_screen_height();//int32_t i = this->id.z - parent->cur_id.y + 1; //parent->cur_id.y


    //gui_cardview_t *cv = this->base.parent;


    int32_t card_dy = parent->release_y;

    if (parent->mute == true)
    {
        card_dy = 0;
    }



    int32_t location = this->base.h * this->id + card_dy;

    int32_t location_0 = h / 2;
    int32_t location_1 = h / 2 + 20;
    int32_t location_2 = h / 2 + 40;

    float scale_0 = 1.0f;
    float scale_1 = 0.9f;

    if (location < location_0)
    {
        //todo
    }
    else if (location < location_1)
    {
        float scale = (scale_0 - scale_1) * (location - location_0) / (location_1 - location_0);
        //todo
    }
    else
    {
        //todo
    }


    if (location >= h)
    {
        hide_child(obj);
    }
    else
    {
        show_child(obj);
    }



}



static void gui_card_ctor(gui_card_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                          int16_t y,
                          int16_t w, int16_t h, int16_t idx, int16_t idy)
{

    gui_obj_ctor(&this->base, parent, filename, x, y, w, h);

    GET_BASE(this)->obj_prepare = tab_prepare;
    GET_BASE(this)->type = CARD;
    if (parent->type == CARDVIEW)
    {
        gui_cardview_t *parent_ext = (gui_cardview_t *)parent;
        if (idy >= 0)
        {
            parent_ext->tab_cnt_down++;
        }
    }

    this->id = idy;


}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

gui_card_t *gui_card_create(void *parent, const char *name, int16_t x, int16_t y,
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

    gui_card_t *this = gui_malloc(sizeof(gui_card_t));
    GUI_ASSERT(this != NULL);
    memset(this, 0x00, sizeof(gui_card_t));

    gui_card_ctor(this, (gui_obj_t *)parent, name, x, y, w, h, idx, idy);

    gui_list_init(&(GET_BASE(this)->child_list));
    if ((GET_BASE(this)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(this)->parent)->child_list),
                               &(GET_BASE(this)->brother_list));
    }
    GET_BASE(this)->create_done = true;
    return this;
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */










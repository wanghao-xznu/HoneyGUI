/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_tabview.c
  * @brief Extended in the cross direction
  * @details switch the currently displayed screen by sliding
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
#include <gui_tabview.h>
#include <gui_matrix.h>


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


#include "gui_tab.h"

static void input_prepare(gui_obj_t *obj)
{
    touch_info_t *tp = tp_get_info();
    GUI_UNUSED(tp);
    gui_obj_skip_all_up_hold(obj);
    gui_obj_skip_all_down_hold(obj);
    gui_obj_skip_all_left_hold(obj);
    gui_obj_skip_all_right_hold(obj);
    obj->skip_tp_left_hold = false;
    obj->skip_tp_right_hold = false;
    obj->skip_tp_up_hold = false;
    obj->skip_tp_down_hold = false;
}

static void tabview_prepare(gui_obj_t *obj)
{
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_tabview_t *tabview = (gui_tabview_t *)obj;
    gui_tabview_t *this = (gui_tabview_t *)obj;

    if (tabview->jump.jump_flag)
    {
        tabview->cur_id.y = tabview->jump.jump_id.y;
        tabview->cur_id.x = tabview->jump.jump_id.x;
        tabview->jump.jump_flag = false;
        return;
    }


    if (tabview->cur_id.x != 0)
    {
        if (tp->type == TOUCH_HOLD_Y || tp->type == TOUCH_DOWN_SLIDE || tp->type == TOUCH_UP_SLIDE)
        {
            return;
        }
    }
    if (tabview->cur_id.y != 0)
    {
        if (tp->type == TOUCH_HOLD_X || tp->type == TOUCH_LEFT_SLIDE || tp->type == TOUCH_RIGHT_SLIDE)
        {
            return;
        }
    }
    switch (tp->type)
    {
    case TOUCH_HOLD_X:
        if ((obj->skip_tp_left_hold) && (tp->deltaX  < 0))
        {
            break;
        }
        if ((obj->skip_tp_right_hold) && (tp->deltaX  > 0))
        {
            break;
        }
        if (tabview->loop)
        {
            this->release_x = tp->deltaX;

            {
                //left edge
                if (!tabview->loop_status && tabview->cur_id.x ==  -tabview->tab_cnt_left)
                {
                    tabview->loop_status = 1;
                    gui_list_t *node = NULL;
                    gui_list_for_each(node, &obj->child_list)
                    {
                        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
                        if (GUI_TYPE(gui_tab_t, obj)->id.x == tabview->tab_cnt_right)
                        {
                            GUI_TYPE(gui_tab_t, obj)->id.x = -tabview->tab_cnt_left - 1;
                        }
                    }
                }
                if (tabview->loop_status && tabview->cur_id.x !=  -tabview->tab_cnt_left)
                {
                    gui_list_t *node = NULL;
                    gui_list_for_each(node, &obj->child_list)
                    {
                        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
                        if (GUI_TYPE(gui_tab_t, obj)->id.x == -tabview->tab_cnt_left - 1)
                        {
                            GUI_TYPE(gui_tab_t, obj)->id.x = tabview->tab_cnt_right;
                        }
                        tabview->loop_status = 0;
                    }
                }
            }

            {
                //right edge
                if (!tabview->loop_status2 && tabview->cur_id.x ==  tabview->tab_cnt_right)
                {
                    tabview->loop_status2 = 1;
                    gui_list_t *node = NULL;
                    gui_list_for_each(node, &obj->child_list)
                    {
                        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
                        if (GUI_TYPE(gui_tab_t, obj)->id.x == -tabview->tab_cnt_left)
                        {
                            GUI_TYPE(gui_tab_t, obj)->id.x = tabview->tab_cnt_right + 1;
                        }
                    }
                }
                if (tabview->loop_status2 && tabview->cur_id.x !=  tabview->tab_cnt_right)
                {
                    gui_list_t *node = NULL;
                    gui_list_for_each(node, &obj->child_list)
                    {
                        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
                        if (GUI_TYPE(gui_tab_t, obj)->id.x == tabview->tab_cnt_right + 1)
                        {
                            GUI_TYPE(gui_tab_t, obj)->id.x = -tabview->tab_cnt_left;
                        }
                        tabview->loop_status2 = 0;
                    }
                }
            }
        }
        else
        {
            if (tabview->tab_cnt_left == 0 && tabview->tab_cnt_right == 0)
            {
                break;
            }

            this->release_x = tp->deltaX;
            if (tabview->cur_id.x == 0 && tabview->tab_cnt_right == 0)
            {
                if (this->release_x < 0)
                {
                    break;
                }
            }
            if (tabview->cur_id.x == 0 && tabview->tab_cnt_left == 0)
            {
                if (this->release_x > 0)
                {
                    this->release_x = 0;
                    break;
                }
            }
            break;
        }


    case TOUCH_HOLD_Y:
        if ((obj->skip_tp_up_hold) && (tp->deltaY  < 0))
        {
            break;
        }
        if ((obj->skip_tp_down_hold) && (tp->deltaY  > 0))
        {
            break;
        }
        if ((obj->skip_tp_up_hold) && (obj->skip_tp_down_hold))
        {
            break;
        }
        if (tabview->tab_cnt_down == 0 && tabview->tab_cnt_up == 0)
        {
            break;
        }
        gui_obj_event_set(obj, GUI_EVENT_8);
        this->release_y = tp->deltaY;
        if (tabview->cur_id.y == 0 && tabview->tab_cnt_down == 0)
        {
            if (this->release_y < 0)
            {
                break;
            }
        }
        if (tabview->cur_id.y == 0 && tabview->tab_cnt_up == 0)
        {
            if (this->release_y > 0)
            {
                this->release_y = 0;
                break;
            }
        }
        break;
    case TOUCH_LEFT_SLIDE:
        gui_log("[TV]TOUCH_LEFT_SLIDE\n");
        if (tabview->tab_cnt_right == 0 && tabview->cur_id.x == 0)
        {
            break;
        }
        //when current tab is the end,come back to current tab if sliding.
        if (tabview->cur_id.x == tabview->tab_cnt_right)
        {
            if (tabview->loop)
            {
                if (tabview->loop_status2 && tabview->cur_id.x ==  tabview->tab_cnt_right)
                {
                    gui_list_t *node = NULL;
                    gui_list_for_each(node, &obj->child_list)
                    {
                        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
                        if (GUI_TYPE(gui_tab_t, obj)->id.x == tabview->tab_cnt_right + 1)
                        {
                            GUI_TYPE(gui_tab_t, obj)->id.x = -tabview->tab_cnt_left;
                        }
                        tabview->loop_status2 = 0;
                    }
                }
                gui_tabview_jump_tab(tabview, 0, 0);
                if (tabview->jump.jump_flag)
                {
                    tabview->cur_id.y = tabview->jump.jump_id.y;
                    tabview->cur_id.x = tabview->jump.jump_id.x;
                    tabview->jump.jump_flag = false;
                    this->release_x = 0;
                    return;
                }
            }
        }
        else
        {
            tabview->cur_id.x = tabview->cur_id.x + 1;
            this->release_x = this->release_x + tabview->base.w;
        }


        break;
    case TOUCH_RIGHT_SLIDE:
        gui_log("[TV]TOUCH_RIGHT_SLIDE\n");
        //when current tab is the end,come back to current tab if sliding.
        if (tabview->cur_id.x == tabview->tab_cnt_left)
        {
            if (tabview->loop)
            {
                if (tabview->loop_status && tabview->cur_id.x ==  -tabview->tab_cnt_left)
                {
                    gui_list_t *node = NULL;
                    gui_list_for_each(node, &obj->child_list)
                    {
                        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
                        if (GUI_TYPE(gui_tab_t, obj)->id.x == -tabview->tab_cnt_left - 1)
                        {
                            GUI_TYPE(gui_tab_t, obj)->id.x = tabview->tab_cnt_right;
                        }
                        tabview->loop_status = 0;
                    }
                }
                gui_tabview_jump_tab(tabview, tabview->tab_cnt_right, 0);
                if (tabview->jump.jump_flag)
                {
                    tabview->cur_id.y = tabview->jump.jump_id.y;
                    tabview->cur_id.x = tabview->jump.jump_id.x;
                    tabview->jump.jump_flag = false;
                    this->release_x = 0;
                    return;
                }
            }

        }
        else
        {
            if (tabview->tab_cnt_left == 0 && tabview->cur_id.x == 0)
            {
                break;
            }
            tabview->cur_id.x = tabview->cur_id.x - 1;
            this->release_x = this->release_x - tabview->base.w;
        }


        break;
    case TOUCH_DOWN_SLIDE:
        gui_log("[TV]TOUCH_DOWN_SLIDE\n");
        if (tabview->tab_cnt_up == 0 && tabview->cur_id.y == 0)
        {
            break;
        }
        //when current tab is the end,come back to current tab if sliding.
        if (tabview->cur_id.y == tabview->tab_cnt_up)
        {
            return;
        }
        tabview->cur_id.y = tabview->cur_id.y - 1;
        this->release_y = this->release_y - tabview->base.h;
        gui_obj_event_set(obj, GUI_EVENT_1);
        break;
    case TOUCH_UP_SLIDE:
        gui_log("[TV]TOUCH_UP_SLIDE\n");
        if (tabview->tab_cnt_down == 0 && tabview->cur_id.y == 0)
        {
            break;
        }
        //when current tab is the end,come back to current tab if sliding.
        if (tabview->cur_id.y == tabview->tab_cnt_down)
        {
            return;
        }
        tabview->cur_id.y = tabview->cur_id.y + 1;
        this->release_y = this->release_y + tabview->base.h;
        gui_obj_event_set(obj, GUI_EVENT_2);
        break;
    case TOUCH_ORIGIN_FROM_X:
    case TOUCH_ORIGIN_FROM_Y:
        break;
    default:
        break;
    }

    if (this->release_x >= GUI_FRAME_STEP)
    {
        this->release_x -= GUI_FRAME_STEP;
    }
    else if (this->release_x <= -GUI_FRAME_STEP)
    {
        this->release_x += GUI_FRAME_STEP;
    }
    else
    {
        this->release_x = 0;
    }

    if (this->release_y >= GUI_FRAME_STEP)
    {
        this->release_y -= GUI_FRAME_STEP;
    }
    else if (this->release_y <= -GUI_FRAME_STEP)
    {
        this->release_y += GUI_FRAME_STEP;
    }
    else
    {
        this->release_y = 0;
    }

    uint8_t last = this->checksum;
    this->checksum = 0;
    this->checksum = gui_checksum(0, (uint8_t *)this, sizeof(gui_tabview_t));

    if (last != this->checksum)
    {
        gui_fb_change();
    }
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
void gui_tabview_set_style(gui_tabview_t *this, SLIDE_STYLE style)
{
    this->style = style;
}


gui_tabview_t *gui_tabview_create(void *parent, const char *filename, int16_t x, int16_t y,
                                  int16_t w, int16_t h)
{
    gui_tabview_t *this = gui_malloc(sizeof(gui_tabview_t));
    memset(this, 0, sizeof(gui_tabview_t));
    if (w == 0)
    {
        w = (int)gui_get_screen_width();
    }
    if (h == 0)
    {
        h = (int)gui_get_screen_height();
    }
    gui_obj_ctor(&this->base, parent, filename, x, y, w, h);
    GET_BASE(this)->obj_input_prepare = input_prepare;
    GET_BASE(this)->obj_prepare = tabview_prepare;
    GET_BASE(this)->type = TABVIEW;
    gui_list_init(&(GET_BASE(this)->child_list));
    if ((GET_BASE(this)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(this)->parent)->child_list), &(GET_BASE(this)->brother_list));
    }
    GET_BASE(this)->create_done = true;
    return this;
}


void gui_tabview_jump_tab(gui_tabview_t *parent_tabview, int8_t idx, int8_t idy)
{
    parent_tabview->jump.jump_flag = true;
    parent_tabview->jump.jump_id.x = idx;
    parent_tabview->jump.jump_id.y = idy;
}
void gui_tabview_loop(gui_tabview_t *tabview, bool loop_or_not)
{
    tabview->loop = loop_or_not;
}

void gui_tabview_tp_disable(gui_tabview_t *tabview, bool disable_tp)
{
    tabview->tp_disable = disable_tp;
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */









/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_curtainview.c
  * @brief curtain effect container widget, which can nest curtains.
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
#include "gui_tabview.h"
#include "gui_tab.h"
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
static gui_curtain_t *get_child(gui_obj_t *object, gui_curtain_enum_t orientation)
{
    gui_list_t *node = NULL;
    gui_list_t *tmp = NULL;
    gui_list_for_each_safe(node, tmp, &object->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (obj->type == CURTAIN && GUI_TYPE(gui_curtain_t, obj)->orientation == orientation)
        {
            return (void *)obj;
        }

    }
    return NULL;
}

static void input_prepare(gui_obj_t *obj)
{
    touch_info_t *tp = tp_get_info();
    gui_curtainview_t *this = (gui_curtainview_t *)obj;
    GUI_UNUSED(tp);

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == false)
    {
        return;
    }
    switch (this->cur_curtain)
    {
    case CURTAIN_MIDDLE:
        {
            if (this->has_up_curtain == true)
            {
                gui_obj_skip_all_down_hold(obj);
                obj->skip_tp_down_hold = false;
            }
            if (this->has_down_curtain == true)
            {
                gui_obj_skip_all_up_hold(obj);
                obj->skip_tp_up_hold = false;
            }
            if (this->has_left_curtain == true)
            {
                gui_obj_skip_all_right_hold(obj);
                obj->skip_tp_right_hold = false;
            }
            if (this->has_right_curtain == true)
            {
                gui_obj_skip_all_left_hold(obj);
                obj->skip_tp_left_hold = false;
            }
            break;
        }
    case CURTAIN_UP:
        {
            break;
        }
    case CURTAIN_DOWN:
        {
            break;
        }
    case CURTAIN_LEFT:
        {
            gui_obj_skip_all_left_hold(obj);
            gui_obj_skip_all_right_hold(obj);
            break;
        }
    case CURTAIN_RIGHT:
        {
            break;
        }

    default:
        break;
    }

}

static void curtainview_prepare(gui_obj_t *obj)
{
    gui_curtainview_t *this = (gui_curtainview_t *)obj;
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_curtainview_t *ext = (gui_curtainview_t *)obj;
    int frame_step = GUI_FRAME_STEP;
    if (frame_step / 10 >= 1)
    {
        frame_step = frame_step / 10;
    }
    gui_list_t *node = NULL;
    gui_list_t *tmp = NULL;
    gui_curtain_t *c_middle = NULL;
    GUI_UNUSED(c_middle);
    gui_curtain_t *c_up = NULL;
    gui_curtain_t *c_down = NULL;
    gui_curtain_t *c_left = NULL;
    gui_curtain_t *c_right = NULL;
    if (tp->released)
    {
        ext->release_flag = true;
    }
    if (tp->pressed)
    {
        ext->release_flag = false;
    }


    gui_list_for_each_safe(node, tmp, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (obj->type == CURTAIN)
        {
            gui_curtain_t *c = (void *)obj;
            switch (c->orientation)
            {
            case CURTAIN_MIDDLE:
                c_middle = c;
                break;
            case CURTAIN_UP:
                c_up = c;
                break;
            case CURTAIN_DOWN:
                c_down = c;
                break;
            case CURTAIN_LEFT:
                c_left = c;
                break;
            case CURTAIN_RIGHT:
                c_right = c;
                break;
            default:
                break;
            }
        }
    }
    if (ext->cur_curtain == CURTAIN_MIDDLE)
    {
        if (ext->init_flag)
        {
            if (c_up)
            {
                GET_BASE(c_up)->not_show = true;
            }
            if (c_down)
            {
                GET_BASE(c_down)->not_show = true;
            }
            if (c_left)
            {
                GET_BASE(c_left)->not_show = true;
            }
            if (c_right)
            {
                GET_BASE(c_right)->not_show = true;
            }
        }
        ext->init_flag = true;
    }


    switch (ext->cur_curtain)
    {
    case CURTAIN_MIDDLE:
        {
            obj->cover = false;
            if ((tp->type == TOUCH_HOLD_Y) || (tp->type == TOUCH_ORIGIN_FROM_Y) ||
                (tp->type == TOUCH_DOWN_SLIDE) || (tp->type == TOUCH_UP_SLIDE) || ext->down_flag)
            {
                if (!ext->down_flag)
                {
                    this->release_y = tp->deltaY;
                }
                if (c_up)
                {
                    GET_BASE(c_up)->not_show = false;
                }
                if (c_down)
                {
                    GET_BASE(c_down)->not_show = false;
                }
                if (this->release_y > 0 && ext->orientations.up == false)
                {
                    this->release_y = 0;
                    if (c_up)
                    {
                        GET_BASE(c_up)->not_show = true;
                    }
                    if (c_down)
                    {
                        GET_BASE(c_down)->not_show = true;
                    }
                }
                if (this->release_y < 0 && ext->orientations.down == false)
                {
                    this->release_y = 0;
                    if (c_up)
                    {
                        GET_BASE(c_up)->not_show = true;
                    }
                    if (c_down)
                    {
                        GET_BASE(c_down)->not_show = true;
                    }
                }
                if (this->release_y < 0 && ext->release_flag)
                {
                    ext->down_flag = true;
                    ext->spring_value -= GUI_FRAME_STEP;
                    this->release_y += ext->spring_value;
                    gui_curtain_t *child =  get_child(obj, CURTAIN_DOWN);
                    float scope = 1;
                    if (child)
                    {
                        scope = child->scope;
                    }

                    if (this->release_y <= -(int)gui_get_screen_height()*scope)
                    {
                        ext->down_flag = false;
                        ext->cur_curtain = CURTAIN_DOWN;
                        if (this->done_cb != NULL)
                        {
                            this->done_cb(this);
                        }
                        this->release_y = 0;
                        ext->spring_value = 0;
                        ext->release_flag = false;
                    }

                }
                else if (this->release_y > 0 && ext->release_flag)
                {
                    ext->down_flag = true;
                    ext->spring_value += GUI_FRAME_STEP;
                    this->release_y += ext->spring_value;
                    gui_curtain_t *child =  get_child(obj, CURTAIN_UP);
                    float scope = 1;
                    if (child)
                    {
                        scope = child->scope;
                    }
                    if (this->release_y >= (int)gui_get_screen_height()*scope)
                    {
                        ext->down_flag = false;
                        ext->cur_curtain = CURTAIN_UP;
                        this->release_y = 0;
                        ext->spring_value = 0;
                        ext->release_flag = false;
                    }
                }
            }
            else if ((tp->type == TOUCH_HOLD_X) || ext->left_flag)
            {
                if (tp->deltaX >= 2 || tp->deltaX <= -2)
                {
                    if (c_left)
                    {
                        GET_BASE(c_left)->not_show = false;
                    }
                    if (c_right)
                    {
                        GET_BASE(c_right)->not_show = false;
                    }
                }


                if (!ext->left_flag)
                {
                    this->release_x = tp->deltaX;
                }
                if (this->release_x < 0 && ext->orientations.right == false)
                {
                    this->release_x = 0;
                    if (c_left)
                    {
                        GET_BASE(c_left)->not_show = true;
                    }
                    if (c_right)
                    {
                        GET_BASE(c_right)->not_show = true;
                    }
                }
                if (this->release_x > 0 && ext->orientations.left == false)
                {
                    this->release_x = 0;
                    if (c_left)
                    {
                        GET_BASE(c_left)->not_show = true;
                    }
                    if (c_right)
                    {
                        GET_BASE(c_right)->not_show = true;
                    }
                }
                if (this->release_x < -(int)gui_get_screen_width() * ext->scoperight * 0.5f)
                {
                    ext->cur_curtain = CURTAIN_RIGHT;
                    this->release_x = 0;
                }
                else if (this->release_x > (int)gui_get_screen_width() * ext->scopeleft * 0.5f)
                {
                    ext->left_flag = true;
                    ext->spring_value += GUI_FRAME_STEP;
                    this->release_x += ext->spring_value;
                    if (this->release_x >= (int)gui_get_screen_width()* ext->scopeleft)
                    {
                        ext->left_flag = false;
                        ext->cur_curtain = CURTAIN_LEFT;
                        this->release_x = 0;
                        ext->spring_value = 0;
                    }
                }
            }
            else
            {
                this->release_x = 0;
                this->release_y = 0;
            }
            break;
        }
    case CURTAIN_UP:
        {
            if (c_up)
            {
                GET_BASE(c_up)->not_show = false;
            }
            if ((tp->type == TOUCH_HOLD_Y))
            {
                if (tp->deltaY < 0)
                {
                    this->release_y = tp->deltaY;
                }
                //if (this->release_y < -(int)gui_get_screen_height() * ext->scopeup * 0.5f)
                //{
                //    ext->cur_curtain = CURTAIN_MIDDLE;gui_log("CURTAIN_MIDDLE:%d\n",this->release_y );
                //    this->release_y = 0;
                //}
            }
            else if (tp->type == TOUCH_DOWN_SLIDE)
            {
                ext->cur_curtain = CURTAIN_MIDDLE;
                this->release_y = 0;
            }
            else
            {
                this->release_y = 0;
            }
            break;
        }
    case CURTAIN_DOWN:
        {
            if (c_down)
            {
                GET_BASE(c_down)->not_show = false;
            }

            if ((tp->type == TOUCH_HOLD_Y))
            {
                if (tp->deltaY > 0)
                {
                    this->release_y = tp->deltaY;
                }
            }
            else if (tp->type == TOUCH_DOWN_SLIDE)
            {
                ext->cur_curtain = CURTAIN_MIDDLE;
                if (this->done_cb != NULL)
                {
                    this->done_cb(this);
                }
                this->release_y = 0;
                gui_fb_change();
            }
            else
            {
                this->release_y = 0;
            }

            break;
        }
    case CURTAIN_LEFT:
        {
            if (c_left)
            {
                GET_BASE(c_left)->not_show = false;
            }
            if ((tp->type == TOUCH_HOLD_X))
            {
                if (tp->deltaX < 0)
                {
                    this->release_x = tp->deltaX;
                }
            }
            else if (tp->type == TOUCH_LEFT_SLIDE)
            {
                ext->cur_curtain = CURTAIN_MIDDLE;
                this->release_x = 0;
            }
            else
            {
                this->release_x = 0;
            }
            break;
        }
    case CURTAIN_RIGHT:
        {
            if (c_right)
            {
                GET_BASE(c_right)->not_show = false;
            }
            if ((tp->type == TOUCH_HOLD_X))
            {
                if (tp->deltaX > 0)
                {
                    this->release_x = tp->deltaX;
                }
            }
            else if (tp->type == TOUCH_RIGHT_SLIDE)
            {
                ext->cur_curtain = CURTAIN_MIDDLE;
                this->release_x = 0;
            }
            else
            {
                this->release_x = 0;
            }
            break;
        }

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
    this->checksum = gui_checksum(0, (uint8_t *)this, sizeof(gui_curtainview_t));

    if (last != this->checksum)
    {
        gui_fb_change();
    }

}
static void gui_curtainview_ctor(gui_curtainview_t *this, gui_obj_t *parent, const char *filename,
                                 int16_t x,
                                 int16_t y, int16_t w, int16_t h)
{
    gui_obj_ctor(&this->base, parent, filename, x, y, w, h);
    ((gui_obj_t *)this)->obj_prepare = curtainview_prepare;
    ((gui_obj_t *)this)->type = CURTAINVIEW;
    ((gui_obj_t *)this)->obj_input_prepare = input_prepare;
    this->cur_curtain = CURTAIN_MIDDLE;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_curtainview_set_done_cb(gui_curtainview_t *this, void (*cb)(gui_curtainview_t *this))
{
    this->done_cb = cb;
}
gui_curtainview_t *gui_curtainview_create(void *parent, const char *filename, int16_t x,
                                          int16_t y,
                                          int16_t w, int16_t h)
{
#define _paramgui_curtainview_create_ this, parent, filename, x, y, w, h
    GUI_NEW(gui_curtainview_t, gui_curtainview_ctor, _paramgui_curtainview_create_)
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */









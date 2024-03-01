/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_switch.c
  * @brief on or off status. Click to change.
  * @details listen to on and off gesture
  * @author triton_yu@realsil.com.cn
  * @date 2023/11/8
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
#include <gui_switch.h>
#include <string.h>
#include <gui_server.h>
#include <gui_obj.h>
#include <gui_win.h>
#include "tp_algo.h"


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
void gui_switch_change_switch(gui_switch_t *sw)
{
    gui_imgconfig_t config =
    {
        .src_mode = sw->src_mode,
        .addr = NULL,
        .x = 0,
        .y = 0,
    };
    //sw->off->base.not_show = (sw->ifon);
    //sw->on->base.not_show = !(sw->ifon);
    if (sw->ifon)
    {
        config.addr = sw->on_pic_addr;
    }
    else
    {
        config.addr = sw->off_pic_addr;
    }
    //gui_app_exec(gui_current_app());
    gui_img_set_config(sw->switch_picture, &config);
}
static void sw_turn_on(gui_switch_t *this)
{
    this->ifon = true;
    {

        gui_switch_change_switch(this);

        if (this->ifon)
        {
            gui_obj_event_set((void *)this, GUI_EVENT_1);
        }
        else if (!this->ifon)
        {
            gui_obj_event_set((void *)this, GUI_EVENT_2);
        }

    }
}
static void sw_turn_off(gui_switch_t *this)
{
    this->ifon = false;
    {

        gui_switch_change_switch(this);

        if (this->ifon)
        {
            gui_obj_event_set((void *)this, GUI_EVENT_1);
        }
        else if (!this->ifon)
        {
            gui_obj_event_set((void *)this, GUI_EVENT_2);
        }

    }
}
void gui_switch_turn_on(gui_switch_t *this)
{
    sw_turn_on(this);
}
void gui_switch_turn_off(gui_switch_t *this)
{
    sw_turn_off(this);
}
void gui_switch_is_on(gui_switch_t *this)
{
    this->ifon = true;
    gui_switch_change_switch(this);
}
void gui_switch_is_off(gui_switch_t *this)
{
    this->ifon = false;
    gui_switch_change_switch(this);
}
/*static void sw_turn_off2(gui_switch_t *this)
{
    this->ifon = false;
    {
        gui_switch_t *sw = (gui_switch_t *)this;
        gui_switch_change_switch(this);
        if (sw->cb_on.link_cb && sw->ifon)
        {
            gui_msg_t msg;
            msg.type = GUI_SRV_CB;
            msg.cb = sw->cb_on.link_cb;
            msg.u.payload = sw->cb_on.param;
            send_msg_to_gui_server(&msg);
        }
        else if(sw->cb_off.link_cb && !sw->ifon)
        {
            gui_msg_t msg;
            msg.type = GUI_SRV_CB;
            msg.cb = sw->cb_off.link_cb;
            msg.u.payload = sw->cb_off.param;
            send_msg_to_gui_server(&msg);
        }

    }
}*/

static void (onOn)(gui_switch_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_1, parameter);
}
static void (onOff)(gui_switch_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_2, parameter);
}




/*============================================================================*
 *                           Public Functions
 *============================================================================*/


static void gui_switch_hl(gui_switch_t *sw)
{
    gui_imgconfig_t config =
    {
        .src_mode = sw->src_mode,
        .addr = NULL,
        .x = 0,
        .y = 0,
    };
    //sw->off->base.not_show = (sw->ifon);
    //sw->on->base.not_show = !(sw->ifon);
    if (sw->ifon)
    {
        config.addr = sw->on_hl_pic_addr;
    }
    else
    {
        config.addr = sw->off_hl_pic_addr;
    }
    //gui_app_exec(gui_current_app());
    gui_img_set_config(sw->switch_picture, &config);

}

static void gui_switch_hl_back(gui_switch_t *sw)
{
    gui_imgconfig_t config =
    {
        .src_mode = sw->src_mode,
        .addr = NULL,
        .x = 0,
        .y = 0,
    };
    //sw->off->base.not_show = (sw->ifon);
    //sw->on->base.not_show = !(sw->ifon);
    if (sw->ifon)
    {
        config.addr = sw->on_pic_addr;
    }
    else
    {
        config.addr = sw->off_pic_addr;
    }
    //gui_app_exec(gui_current_app());
    gui_img_set_config(sw->switch_picture, &config);
}

static void gui_switch_long_touch(gui_switch_t *sw)
{
    gui_imgconfig_t config =
    {
        .src_mode = sw->src_mode,
        .addr = sw->long_touch_state_pic_addr,
        .x = 0,
        .y = 0,
    };
    gui_img_set_config(sw->switch_picture, &config);
}

static void gui_switch_long_touch_back(gui_switch_t *sw)
{
    gui_imgconfig_t config =
    {
        .src_mode = sw->src_mode,
        .addr = NULL,
        .x = 0,
        .y = 0,
    };
    if (sw->ifon)
    {
        config.addr = sw->on_pic_addr;
    }
    else
    {
        config.addr = sw->off_pic_addr;
    }
    gui_img_set_config(sw->switch_picture, &config);
}

static void gui_switch_hl_long_touch(gui_switch_t *sw)
{
    gui_imgconfig_t config =
    {
        .src_mode = sw->src_mode,
        .addr = sw->long_touch_state_hl_pic_addr,
        .x = 0,
        .y = 0,
    };
    gui_img_set_config(sw->switch_picture, &config);
}

void gui_tree_disable_widget_gesture_by_type(gui_obj_t *obj, int type)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        //gui_log("obj name = %s, name =%s\n", obj->name, name);
        if (obj->type == type)
        {
            gui_obj_event_set(obj, (gui_event_t)0);

        }
        gui_tree_disable_widget_gesture_by_type(obj, type);
    }
}
static void switch_prepare(gui_obj_t *obj)
{
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {
        if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
        {
            gui_switch_t *b = (void *)obj;
            switch (tp->type)
            {
            case TOUCH_SHORT:
                {
                    // gui_log("Touch_short,%d,%d,%d,%f,%d\n", tp->x, (obj->ax + obj->tx), obj->dx, obj->sx, obj->tx);
                    // // if ((tp->x >= (obj->ax + obj->tx) && tp->x <= ((obj->ax + obj->tx) + obj->w)) &&
                    // //     (tp->y >= (obj->ay + obj->ty) && tp->y <= ((obj->ay + obj->ty) + obj->h)))
                    // {
                    //     gui_log("%d\n", __LINE__);
                    //     gui_switch_t *sw = (gui_switch_t *)obj;
                    //     sw->ifon = !(sw->ifon);
                    //     gui_switch_change_switch(sw);//gui_log("switch_prepare3\n");
                    //     if (sw->ifon)
                    //     {
                    //         //gui_log("switch_prepare4\n");
                    //         gui_obj_event_set(obj, GUI_EVENT_1);
                    //     }
                    //     else if (!sw->ifon)
                    //     {
                    //         //gui_log("switch_prepare5\n");
                    //         gui_obj_event_set(obj, GUI_EVENT_2);
                    //     }
                    //     gui_tree_disable_widget_gesture_by_type(&(gui_current_app()->screen), WINDOW);
                    // }

                }
                break;
            case TOUCH_UP_SLIDE:
                {

                }
                break;
            case TOUCH_DOWN_SLIDE:
                {

                }
                break;
            case TOUCH_LEFT_SLIDE:
                {

                }
                break;
            case TOUCH_RIGHT_SLIDE:
                {

                }
                break;
            case TOUCH_LONG:
                {
                    if (b->long_flag == false)
                    {

                        //if (b->long_click_cb)
                        {
                            if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                            {
                                b->long_flag = true;
                                if (b->long_touch_enable)
                                {
                                    gui_log("TOUCH_LONG b->long_touch_state = %d\n", b->long_touch_state);
                                    b->long_touch_state = !b->long_touch_state;
                                    gui_obj_event_set(obj, GUI_EVENT_TOUCH_LONG);
                                    gui_switch_hl_back(b);
                                    if (b->long_touch_state)
                                    {
                                        gui_switch_long_touch(b);
                                    }
                                    else
                                    {
                                        gui_switch_long_touch_back(b);
                                    }

                                }
                                //gui_send_callback_p_to_server(b->long_click_cb, b->long_click_cb_p);
                            }

                        }
                    }
                }
                break;

            default:
                break;
            }


            {
                //gui_log("switch_prepare: tp->type = %d\n", tp->type);
                if (tp->pressed)
                {

                    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                    {

                        //gui_send_callback_p_to_server(b->press_cb, b->press_cb_p);

                        //gui_log("%d\n", __LINE__);
                        //gui_obj_event_set(obj, GUI_EVENT_TOUCH_PRESSED);  //gui_log("%d\n", __LINE__);

                        gui_log("pressed\n");
                        gui_obj_event_set(obj, GUI_EVENT_TOUCH_PRESSED);  ////gui_log("%d\n", __LINE__);

                        b->long_flag = false;
                        b->press_flag = true;
                        if (!b->long_touch_state)
                        {
                            gui_switch_hl(b);
                        }
                        else
                        {
                            gui_switch_hl_long_touch(b);
                        }


                    }
                }
                // for case: grid->switch. avoid pic switch stuck
                gui_obj_t *parent = b->base.parent;
                if (!strcmp(parent->name, "grid") || parent->type == PAGELIST)
                {
                    if (b->press_flag == true && (tp->type == TOUCH_HOLD_Y || tp->type == TOUCH_ORIGIN_FROM_Y))
                    {
                        gui_log("pressed TOUCH_HOLD_Y\n");
                        if (!b->long_touch_state)
                        {
                            gui_switch_hl_back(b);
                        }
                    }
                }
                if (b->release_flag)
                {
                    if (tp->type != TOUCH_HOLD_Y && tp->type != TOUCH_ORIGIN_FROM_Y)
                    {
                        if (b->long_touch_state)
                        {
                            gui_switch_long_touch(b);
                        }
                        else if (!b->long_touch_state && !b->long_flag
                                 || !b->long_touch_enable)
                        {
                            if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                            {
                                gui_switch_t *sw = (gui_switch_t *)obj;
                                sw->ifon = !(sw->ifon);
                                gui_switch_change_switch(sw);//gui_log("switch_prepare3\n");
                                if (sw->ifon)
                                {
                                    //gui_log("switch_prepare4\n");
                                    gui_obj_event_set(obj, GUI_EVENT_1);
                                }
                                else if (!sw->ifon)
                                {
                                    //gui_log("switch_prepare5\n");
                                    gui_obj_event_set(obj, GUI_EVENT_2);
                                }
                                gui_tree_disable_widget_gesture_by_type(&(gui_current_app()->screen), WINDOW);
                            }

                        }
                    }
                    b->press_flag = false;
                    b->release_flag = false;
                    b->long_flag = false;
                }
                if (tp->released && b->press_flag)
                {
                    b->release_flag = true;
                }
                if (b->touch_disable)
                {
                    gui_obj_event_set(obj, (gui_event_t)0);
                }
            }

        }
    }
}
void gui_switch_ctor(gui_switch_t *this, gui_obj_t *parent,
                     int16_t x,
                     int16_t y, int16_t w, int16_t h, void *off_pic, void *on_pic)
{
    gui_obj_ctor(&(this->base), parent, "switch", x, y, w, h);

    this->base.type = CLICKSWITCH;
    GET_BASE(this)->obj_prepare = switch_prepare;
    this->turn_off = sw_turn_off;
    this->turn_on = sw_turn_on;
    this->onOn = onOn;
    this->onOff = onOff;
    this->off_pic_addr = off_pic;
    this->on_pic_addr = on_pic;
    this->off_hl_pic_addr = this->off_pic_addr;
    this->on_hl_pic_addr = this->on_pic_addr;
    this->long_touch_state_pic_addr = this->off_pic_addr;
    this->long_touch_state_hl_pic_addr = this->off_pic_addr;
}
static gui_switch_t *gui_switch_create_core(void *parent, int16_t x, int16_t y,
                                            int16_t w, int16_t h, void *off_pic, void *on_pic, IMG_SOURCE_MODE_TYPE src_mode)
{
    //gui_log("gui_switch_create:%p,%d,%d,%d,%d,%p,%p\n", parent, x, y, w, h, off_pic, on_pic);
    gui_switch_t *this = gui_malloc(sizeof(gui_switch_t));
    memset(this, 0, sizeof(gui_switch_t));
    this->src_mode = src_mode;
    gui_switch_ctor(this, parent, x, y, w, h, off_pic, on_pic);
    gui_list_init(&(((gui_obj_t *)this)->child_list));
    if ((((gui_obj_t *)this)->parent) != ((void *)0))
    { gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }

    if (src_mode == IMG_SRC_FILESYS)
    {
        this->switch_picture = (void *)gui_img_create_from_fs(this, off_pic, 0, 0);
    }
    else if (src_mode == IMG_SRC_MEMADDR)
    {
        this->switch_picture = (void *)gui_img_create_from_mem(this, "switch_picture", off_pic, 0, 0, 0, 0);
    }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}

gui_switch_t *gui_switch_create(void *parent, int16_t x, int16_t y,
                                int16_t w, int16_t h, void *off_pic, void *on_pic)
{
    return gui_switch_create_core(parent, x, y, w, h, off_pic, on_pic, IMG_SRC_MEMADDR);
}

gui_switch_t *gui_switch_create_from_fs(void *parent, int16_t x, int16_t y,
                                        int16_t w, int16_t h, void *off_pic, void *on_pic)
{
    return gui_switch_create_core(parent, x, y, w, h, off_pic, on_pic, IMG_SRC_FILESYS);
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */






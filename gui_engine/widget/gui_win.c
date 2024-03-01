/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_win.c
  * @brief Window widget
  * @details A rectangular container widget which can listen to many gestures
  * @author triton_yu@realsil.com.cn
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
#include <gui_win.h>
#include <string.h>
#include <tp_algo.h>
#include "gui_obj.h"
#include <kb_algo.h>


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


static void win_prepare(gui_obj_t *obj)
{
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    kb_info_t *kb = kb_get_info();
    if (kb->pressed == true)
    {
        gui_obj_event_set(obj, GUI_EVENT_KB_DOWN_PRESSED);
    }

    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
    {
        gui_win_t *b = (void *)obj;
        switch (tp->type)
        {
        case TOUCH_SHORT:
            {
                gui_obj_event_set(obj, GUI_EVENT_TOUCH_CLICKED);
            }
            break;
        case TOUCH_UP_SLIDE:
            {
                gui_obj_event_set(obj, GUI_EVENT_3);
            }
            break;
        case TOUCH_DOWN_SLIDE:
            {
                gui_obj_event_set(obj, GUI_EVENT_4);
            }
            break;
        case TOUCH_LEFT_SLIDE:
            {
                gui_obj_event_set(obj, GUI_EVENT_1);
            }
            break;
        case TOUCH_RIGHT_SLIDE:
            {
                gui_obj_event_set(obj, GUI_EVENT_2);
            }
            break;
        case TOUCH_LONG:
            {
                b->long_flag = true;
                gui_obj_event_set(obj, GUI_EVENT_TOUCH_LONG);
            }
            break;

        default:
            break;
        }
        if (tp->pressed)
        {
            gui_obj_event_set(obj, GUI_EVENT_TOUCH_PRESSED);
            b->long_flag = false;
            b->press_flag = true;
        }
        if (b->release_flag)
        {
            b->press_flag = false;
            b->release_flag = false;
            gui_obj_event_set(obj, GUI_EVENT_TOUCH_RELEASED);
            b->long_flag = false;
        }
        if (tp->released && b->press_flag)
        {
            b->release_flag = true;
        }
    }
    gui_win_t *ob = (void *)obj;
    if (ob->animate && ob->animate->animate)
    {
        size_t frame_count = ob->animate->dur * (1000 / 15) / (1000);
        ob->animate->callback(ob->animate->p);
        ob->animate->current_frame++;

        if (ob->animate->current_frame > frame_count)
        {
            if (ob->animate->repeatCount == 0)
            {
                ob->animate->animate = false;
            }
            else if (ob->animate->repeatCount < 0)
            {
                ob->animate->current_frame = 0;
            }
            else if (ob->animate->repeatCount > 0)
            {
                ob->animate->current_repeat_count++;
                if (ob->animate->current_repeat_count >= ob->animate->repeatCount)
                {
                    ob->animate->animate = false;
                }
                else
                {
                    ob->animate->current_frame = 0;
                }
            }
        }
        ob->animate->progress_percent = ((float)(ob->animate->current_frame)) / ((float)(
                                                                                     frame_count));
    }
}


static void gui_win_ctor(gui_win_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                         int16_t y, int16_t w, int16_t h)
{
    gui_obj_ctor(&this->base, parent, filename, x, y, w, h);
    GET_BASE(this)->obj_prepare = win_prepare;
    GET_BASE(this)->type = WINDOW;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_win_set_animate(gui_win_t *o, uint32_t dur, int repeatCount, void *callback, void *p)
{
    gui_animate_t *animate = ((gui_win_t *)o)->animate;
    if (!(animate))
    {
        animate = gui_malloc(sizeof(gui_animate_t));
    }
    memset((animate), 0, sizeof(gui_animate_t));
    animate->animate = true;
    animate->dur = dur;
    animate->callback = (void (*)(void *))callback;
    animate->repeatCount = repeatCount;
    animate->p = p;
    ((gui_win_t *)o)->animate = animate;
}

void gui_win_onLeft(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_1, parameter);
}
void gui_win_onRight(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_2, parameter);
}
void gui_win_onUp(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_3, parameter);
}
void gui_win_onDown(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_4, parameter);
}
void gui_win_onPress(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_PRESSED, parameter);
}
void gui_win_onRelease(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_RELEASED, parameter);
}
void gui_win_onLong(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_LONG, parameter);
}
void gui_win_onClick(gui_win_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_CLICKED, parameter);
}

gui_win_t *gui_win_create(void *parent, const char *filename, int16_t x, int16_t y,
                          int16_t w, int16_t h)
{
#define _GUI_NEW_gui_win_create_param this, parent, filename, x, y, w, h
    GUI_NEW(gui_win_t, gui_win_ctor, _GUI_NEW_gui_win_create_param)
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */







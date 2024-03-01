/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_page.c
  * @brief page widget
  * @details page widget
  * @author howie_wang@realsil.com.cn
  * @date 2023/11/07
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
#include <gui_page.h>
#include <string.h>
#include <gui_server.h>
#include <gui_img.h>
#include "gui_obj.h"
#include <gui_curtain.h>
#include <tp_algo.h>

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
/** @defgroup SUBMOUDLE_Exported_Macros SUBMOUDLE Exported Macros
  * @{
  */


/** End of SUBMOUDLE_Exported_Macros
  * @}
  */

/*============================================================================*
 *                            Variables
 *============================================================================*/
/** @defgroup SUBMOUDLE_Exported_Variables SUBMOUDLE Exported Variables
  * @{
  */


/** End of SUBMOUDLE_Exported_Variables
  * @}
  */

/*============================================================================*
 *                           Private Functions
 *============================================================================*/
/** @defgroup WIDGET_Exported_Functions WIDGET Exported Functions
  * @{
  */
static void deal_img_in_root(gui_obj_t *object, int ayend, int *out)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &object->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (!obj->not_show)
        {
            /*
            obj->ax = obj->x + obj->parent->ax;
            obj->ay = obj->y + obj->parent->ay;
            if (ayend < obj->ay + obj->h)
            {
                ayend = obj->ay + obj->h;
            }
            todo
            */
            if (*out < ayend)
            {
                *out = ayend;
            }
        }
        deal_img_in_root(obj, ayend, out);
    }
}

static void gui_page_add_scroll_bar(gui_page_t *this, void *bar_pic)
{
    this->src_mode = IMG_SRC_MEMADDR;
    this->scroll_bar = gui_img_create_from_mem(this->base.parent, "scroll_bar", bar_pic, 0, 0, 0, 0);
    gui_img_get_height(this->scroll_bar);
    this->scroll_bar->base.x = this->base.w - 3 - this->scroll_bar->base.w;
}
static void gui_page_add_scroll_bar_from_fs(gui_page_t *this, void *bar_pic)
{
    this->src_mode = IMG_SRC_FILESYS;
    this->scroll_bar = gui_img_create_from_fs(this->base.parent, bar_pic, 0, 0);
    gui_img_get_height(this->scroll_bar);
    this->scroll_bar->base.x = this->base.w - 3 - this->scroll_bar->base.w;
}
static void set_offset(gui_page_t *this, int offset)
{
    this->yold = offset;
    this->base.y = offset;
}

static int get_offset(gui_page_t *this)
{
    return this->base.y;
}

_gui_api_page_t gui_page_api =
{
    .set_offset = set_offset,
    .get_offset = get_offset,
    .gui_page_add_scroll_bar = gui_page_add_scroll_bar,
    .gui_page_add_scroll_bar_from_fs = gui_page_add_scroll_bar_from_fs,
};

/*============================================================================*
 *                           Public Functions
 *============================================================================*/
static void (obj_update_att)(struct _gui_obj_t *o)
{
    gui_page_t *obj = (void *)o;
    if (obj->animate && obj->animate->animate)
    {
        size_t frame_count = obj->animate->dur * (1000 / 15) / (1000);
        obj->animate->callback(obj->animate->p);
        obj->animate->current_frame++;

        if (obj->animate->current_frame > frame_count)
        {
            if (obj->animate->repeatCount == 0)
            {
                obj->animate->animate = false;
            }
            else if (obj->animate->repeatCount < 0)
            {
                obj->animate->current_frame = 0;
            }
            else if (obj->animate->repeatCount > 0)
            {
                obj->animate->current_repeat_count++;
                if (obj->animate->current_repeat_count >= obj->animate->repeatCount)
                {
                    obj->animate->animate = false;
                }
                else
                {
                    obj->animate->current_frame = 0;
                }
            }
        }
        obj->animate->progress_percent = ((float)(obj->animate->current_frame)) / ((float)(
                                                                                       frame_count));

    }
}
void gui_page_set_animate(gui_page_t *o, uint32_t dur, int repeatCount, void *callback, void *p)
{
    gui_animate_t *animate = ((gui_page_t *)o)->animate;
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
    ((gui_page_t *)o)->animate = animate;
}
static void center_alignment(gui_page_t *page)
{
    if (page->align_hight)
    {
        gui_obj_t *obj = GET_BASE(page);
        int alien = _UI_ABS(obj->y) % page->align_hight;
        if (alien > page->align_hight / 2)
        {
            obj->y = obj->y - page->align_hight + alien;
        }
        else
        {
            obj->y = obj->y + alien;
        }
    }


}
void page_update(gui_obj_t *obj)
{
    obj_update_att(obj);
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    int ay = 0;
    deal_img_in_root(obj, obj->y + obj->h, &ay);
    obj->h = ay - obj->y;



    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
    {
        if ((tp->x > ((gui_page_t *)obj)->start_x) && (tp->x < ((gui_page_t *)obj)->start_x + obj->w))
        {
            if ((tp->y > ((gui_page_t *)obj)->start_y) && (tp->y < ((gui_page_t *)obj)->start_y + obj->h))
            {
                if ((tp->type == TOUCH_HOLD_Y))
                {
                    // gui_log("obj->y:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-gui_get_screen_height()));
                    obj->y = tp->deltaY + ((gui_page_t *)obj)->yold;
                    if (obj->y > ((gui_page_t *)obj)->start_y)
                    {
                        obj->y = ((gui_page_t *)obj)->start_y;

                    }
                    else if (obj->y < (((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height())) &&
                             obj->y != 0)
                    {
                        // gui_log("obj->yyyy:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-(int)gui_get_screen_height()));
                        obj->y = ((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height());
                    }
                    if (GUI_TYPE(gui_page_t, obj)->count == 5)
                    {
                        GUI_TYPE(gui_page_t, obj)->count = 0;
                    }
                    GUI_TYPE(gui_page_t, obj)->count++;

                    if (GUI_TYPE(gui_page_t, obj)->count == 1)
                    {
                        GUI_TYPE(gui_page_t, obj)->y_last = obj->y;
                    }
                    if (GUI_TYPE(gui_page_t, obj)->count == 5)
                    {
                        //gui_log("((gui_page_t *)obj)->yold:%d,%d,%d\n",obj->y-GUI_TYPE(gui_page_t, obj)->y_last, obj->y, GUI_TYPE(gui_page_t, obj)->y_last);
                        if (GUI_TYPE(gui_page_t, obj)->y_last != 0)
                        {
                            GUI_TYPE(gui_page_t, obj)->speed = obj->y - GUI_TYPE(gui_page_t, obj)->y_last;
                        }
                        if (GUI_TYPE(gui_page_t, obj)->speed > 40)
                        {
                            GUI_TYPE(gui_page_t, obj)->speed = 40;
                        }
                        if (GUI_TYPE(gui_page_t, obj)->speed < -40)
                        {
                            GUI_TYPE(gui_page_t, obj)->speed = -40;
                        }
                        //GUI_TYPE(gui_page_t, obj)->y_last = 0;
                    }
                    gui_obj_event_set(obj, GUI_EVENT_8);

                }
                else if (tp->released && GUI_TYPE(gui_page_t, obj)->speed != 0)
                {
                    GUI_TYPE(gui_page_t, obj)->release = true;
                    GUI_TYPE(gui_page_t, obj)->count = 0;
                    GUI_TYPE(gui_page_t, obj)->y_last = 0;
                }
                else if (tp->released && GUI_TYPE(gui_page_t, obj)->speed == 0)
                {
                    gui_obj_event_set(obj, GUI_EVENT_7);
                    center_alignment((void *)obj);

                }
                else if (tp->pressed)
                {
                    GUI_TYPE(gui_page_t, obj)->release = false;
                    GUI_TYPE(gui_page_t, obj)->count = 0;
                    GUI_TYPE(gui_page_t, obj)->speed = 0;
                    GUI_TYPE(gui_page_t, obj)->y_last = 0;
                }
                else
                {
                    if (GUI_TYPE(gui_page_t, obj)->release)
                    {
                        if (GUI_TYPE(gui_page_t, obj)->speed == 0)
                        {
                            GUI_TYPE(gui_page_t, obj)->release = false;
                            center_alignment((void *)obj);
                        }
                        //gui_log("%d,%d\n",obj->y, GUI_TYPE(gui_page_t, obj)->speed);

                        if (GUI_TYPE(gui_page_t, obj)->speed > 0)
                        {
                            obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                            GUI_TYPE(gui_page_t, obj)->speed -= 1;
                        }
                        else if (GUI_TYPE(gui_page_t, obj)->speed < 0)
                        {
                            obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                            GUI_TYPE(gui_page_t, obj)->speed += 1;
                        }
                        if (GUI_TYPE(gui_page_t, obj)->speed == 0)
                        {
                            gui_obj_event_set(obj, GUI_EVENT_7);
                            center_alignment((void *)obj);
                        }
                        else
                        {
                            gui_obj_event_set(obj, GUI_EVENT_8);
                        }


                    }
                    if (obj->y > ((gui_page_t *)obj)->start_y)
                    {
                        obj->y = ((gui_page_t *)obj)->start_y;
                        GUI_TYPE(gui_page_t, obj)->release = false;
                        gui_obj_event_set(obj, GUI_EVENT_7);
                        center_alignment((void *)obj);
                    }
                    else if (obj->y < (((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height())) &&
                             obj->y != 0)
                    {
                        GUI_TYPE(gui_page_t, obj)->release = false;
                        // gui_log("obj->yyyy:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-(int)gui_get_screen_height()));
                        obj->y = ((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height());
                        gui_obj_event_set(obj, GUI_EVENT_7);
                        center_alignment((void *)obj);
                    }
                    ((gui_page_t *)obj)->yold = obj->y;
                }
            }
        }


        if (((gui_page_t *)obj)->scroll_bar)
        {
            // ((gui_page_t *)obj)->scroll_bar->base.y = ((((gui_page_t *)obj)->start_y - obj->y) *
            //                                            gui_get_screen_height() / obj->h);
        }
        // gui_log("obj->y:%d,%d, %d\n", obj->y, obj->ay, obj->parent->ay);
        if (obj->y == ((gui_page_t *)obj)->start_y)
        {
            obj->cover = false;
        }
        else
        {
            obj->cover = true;
        }
    }



}
static void page_update_rebound(gui_obj_t *obj)
{
    obj_update_att(obj);
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    int ay = 0;
    deal_img_in_root(obj, obj->y + obj->h, &ay);
    obj->h = ay - obj->y;



    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
    {
        if ((tp->x > ((gui_page_t *)obj)->start_x) && (tp->x < ((gui_page_t *)obj)->start_x + obj->w))
        {
            //if ((tp->y > ((gui_page_t *)obj)->start_y) && (tp->y < ((gui_page_t *)obj)->start_y + obj->h))
            {
                if ((tp->type == TOUCH_HOLD_Y))
                {
                    // gui_log("obj->y:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-gui_get_screen_height()));
                    if (tp->deltaY + tp->y < 0)
                    {
                        tp->deltaY = -tp->y;
                    }
                    obj->y = tp->deltaY + ((gui_page_t *)obj)->yold;
                    if (obj->y > ((gui_page_t *)obj)->start_y)
                    {
                        // obj->y = ((gui_page_t *)obj)->start_y;
                        GUI_TYPE(gui_page_t, obj)->status = 1;
                    }
                    else if (obj->y < (((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height())) &&
                             obj->y != 0)
                    {
                        // gui_log("obj->yyyy:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-(int)gui_get_screen_height()));
                        // obj->y = ((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height());
                        GUI_TYPE(gui_page_t, obj)->status = 2;
                    }
                    if (GUI_TYPE(gui_page_t, obj)->count == 5)
                    {
                        GUI_TYPE(gui_page_t, obj)->count = 0;
                    }
                    GUI_TYPE(gui_page_t, obj)->count++;

                    if (GUI_TYPE(gui_page_t, obj)->count == 1)
                    {
                        GUI_TYPE(gui_page_t, obj)->y_last = obj->y;
                    }
                    if (GUI_TYPE(gui_page_t, obj)->count == 5)
                    {
                        //gui_log("((gui_page_t *)obj)->yold:%d,%d,%d\n",obj->y-GUI_TYPE(gui_page_t, obj)->y_last, obj->y, GUI_TYPE(gui_page_t, obj)->y_last);
                        if (GUI_TYPE(gui_page_t, obj)->y_last != 0)
                        {
                            GUI_TYPE(gui_page_t, obj)->speed = obj->y - GUI_TYPE(gui_page_t, obj)->y_last;
                        }
                        if (GUI_TYPE(gui_page_t, obj)->speed > 40)
                        {
                            GUI_TYPE(gui_page_t, obj)->speed = 40;
                        }
                        if (GUI_TYPE(gui_page_t, obj)->speed < -40)
                        {
                            GUI_TYPE(gui_page_t, obj)->speed = -40;
                        }
                        //GUI_TYPE(gui_page_t, obj)->y_last = 0;
                    }
                    gui_obj_event_set(obj, GUI_EVENT_8);
                    if (GUI_TYPE(gui_page_t, obj)->status == 1)
                    {
                        GUI_TYPE(gui_page_t, obj)->speed = -40;
                    }
                    else if (GUI_TYPE(gui_page_t, obj)->status == 2)
                    {
                        GUI_TYPE(gui_page_t, obj)->speed = 40;
                    }


                }
                else if (tp->released && GUI_TYPE(gui_page_t, obj)->speed != 0)
                {
                    GUI_TYPE(gui_page_t, obj)->release = true;
                    GUI_TYPE(gui_page_t, obj)->count = 0;
                    GUI_TYPE(gui_page_t, obj)->y_last = 0;
                }
                else if (tp->released && GUI_TYPE(gui_page_t, obj)->speed == 0)
                {
                    gui_obj_event_set(obj, GUI_EVENT_7);
                    center_alignment((void *)obj);
                }
                else if (tp->pressed)
                {
                    GUI_TYPE(gui_page_t, obj)->release = false;
                    GUI_TYPE(gui_page_t, obj)->count = 0;
                    GUI_TYPE(gui_page_t, obj)->speed = 0;
                    GUI_TYPE(gui_page_t, obj)->y_last = 0;
                    GUI_TYPE(gui_page_t, obj)->status = 0;
                }
                else
                {
                    switch (GUI_TYPE(gui_page_t, obj)->status)
                    {
                    case 0:
                        {
                            if (GUI_TYPE(gui_page_t, obj)->release)
                            {
                                if (GUI_TYPE(gui_page_t, obj)->speed == 0)
                                {
                                    GUI_TYPE(gui_page_t, obj)->release = false;
                                    center_alignment((void *)obj);
                                }
                                //gui_log("%d,%d\n",obj->y, GUI_TYPE(gui_page_t, obj)->speed);

                                if (GUI_TYPE(gui_page_t, obj)->speed > 0)
                                {
                                    obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                                    GUI_TYPE(gui_page_t, obj)->speed -= 1;
                                }
                                else if (GUI_TYPE(gui_page_t, obj)->speed < 0)
                                {
                                    obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                                    GUI_TYPE(gui_page_t, obj)->speed += 1;
                                }
                                if (GUI_TYPE(gui_page_t, obj)->speed == 0)
                                {
                                    gui_obj_event_set(obj, GUI_EVENT_7);
                                    center_alignment((void *)obj);
                                }
                                else
                                {
                                    gui_obj_event_set(obj, GUI_EVENT_8);
                                }



                            }
                            if (obj->y > ((gui_page_t *)obj)->start_y)
                            {
                                obj->y = ((gui_page_t *)obj)->start_y;
                                GUI_TYPE(gui_page_t, obj)->release = false;
                                gui_obj_event_set(obj, GUI_EVENT_7);
                                center_alignment((void *)obj);
                            }
                            else if (obj->y < (((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height())) &&
                                     obj->y != 0)
                            {
                                GUI_TYPE(gui_page_t, obj)->release = false;
                                // gui_log("obj->yyyy:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-(int)gui_get_screen_height()));
                                obj->y = ((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height());
                                gui_obj_event_set(obj, GUI_EVENT_7);
                                center_alignment((void *)obj);
                            }
                        }
                        break;
                    case 1:
                        {
                            if (GUI_TYPE(gui_page_t, obj)->release)
                            {
                                if (GUI_TYPE(gui_page_t, obj)->speed == 0)
                                {
                                    GUI_TYPE(gui_page_t, obj)->release = false;
                                    center_alignment((void *)obj);
                                }


                                if (GUI_TYPE(gui_page_t, obj)->speed > 0)
                                {
                                    obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                                    GUI_TYPE(gui_page_t, obj)->speed -= 1;
                                }
                                else if (GUI_TYPE(gui_page_t, obj)->speed < 0)
                                {
                                    obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                                    GUI_TYPE(gui_page_t, obj)->speed += 1;
                                }

                                gui_obj_event_set(obj, GUI_EVENT_8);


                            }
                            if (obj->y < ((gui_page_t *)obj)->start_y)
                            {
                                obj->y = ((gui_page_t *)obj)->start_y;
                                GUI_TYPE(gui_page_t, obj)->release = false;
                                gui_obj_event_set(obj, GUI_EVENT_7);

                            }
                            else if (obj->y < (((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height())) &&
                                     obj->y != 0)
                            {
                                GUI_TYPE(gui_page_t, obj)->release = false;
                                // gui_log("obj->yyyy:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-(int)gui_get_screen_height()));
                                obj->y = ((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height());

                            }
                        }
                        break;
                    case 2:
                        {
                            if (GUI_TYPE(gui_page_t, obj)->release)
                            {
                                if (GUI_TYPE(gui_page_t, obj)->speed == 0)
                                {
                                    GUI_TYPE(gui_page_t, obj)->release = false;
                                    center_alignment((void *)obj);
                                }


                                if (GUI_TYPE(gui_page_t, obj)->speed > 0)
                                {
                                    obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                                    GUI_TYPE(gui_page_t, obj)->speed -= 1;
                                }
                                else if (GUI_TYPE(gui_page_t, obj)->speed < 0)
                                {
                                    obj->y += GUI_TYPE(gui_page_t, obj)->speed;
                                    GUI_TYPE(gui_page_t, obj)->speed += 1;
                                }

                                gui_obj_event_set(obj, GUI_EVENT_8);


                            }
                            if (obj->y > ((gui_page_t *)obj)->start_y)
                            {
                                obj->y = ((gui_page_t *)obj)->start_y;
                                GUI_TYPE(gui_page_t, obj)->release = false;

                            }
                            else if (obj->y > (((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height())) &&
                                     obj->y != 0)
                            {
                                GUI_TYPE(gui_page_t, obj)->release = false;
                                // gui_log("obj->yyyy:%d,%d",obj->y, ((gui_page_t *)obj)->start_y-(obj->h-(int)gui_get_screen_height()));
                                obj->y = ((gui_page_t *)obj)->start_y - (obj->h - (int)gui_get_screen_height());
                                gui_obj_event_set(obj, GUI_EVENT_7);


                            }
                        }
                        break;
                    default:
                        break;
                    }

                    ((gui_page_t *)obj)->yold = obj->y;
                }
            }
        }

        if (obj->y == ((gui_page_t *)obj)->start_y)
        {
            obj->cover = false;
        }
        else
        {
            obj->cover = true;
        }
    }



}

void gui_page_ctor(gui_page_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                   int16_t y, int16_t w, int16_t h)
{
    gui_obj_ctor(&this->base, parent, filename, x, y, w, h
                );
    GET_BASE(this)->type = PAGE;
    GET_BASE(this)->obj_prepare = page_update;
    this->base.type = PAGE;
    this->start_x = x;
    this->start_y = y;
}

gui_page_t *gui_page_create(void *parent, const char *filename, int16_t x, int16_t y,
                            int16_t w, int16_t h)
{
#define _GUI_NEW_gui_page_create_param this, parent, filename, x, y, w, h
    GUI_NEW(gui_page_t, gui_page_ctor, _GUI_NEW_gui_page_create_param)
}
void gui_page_rebound(gui_page_t *this, bool rebound)
{
    if (rebound)
    {
        GET_BASE(this)->obj_prepare = page_update_rebound;
    }
    else
    {
        GET_BASE(this)->obj_prepare = page_update;
    }


}
void gui_page_center_alignment(gui_page_t *page, int align_hight)
{
    page->align_hight = align_hight;
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */

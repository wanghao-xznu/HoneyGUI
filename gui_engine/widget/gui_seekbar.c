/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_seekbar.c
  * @brief seekbar widget
  * @details seekbar widget
  * @author luke_sun@realsil.com.cn
  * @date 2023/11/06
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
#include <gui_seekbar.h>
#include <string.h>
#include <gui_server.h>
#include <gui_obj.h>
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

/** @brief  ... */

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
static void (obj_update_att)(struct _gui_obj_t *o);
static void seekbar_preapre(gui_obj_t *obj)
{
    obj_update_att(obj);
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_seekbar_t *circle = (gui_seekbar_t *)obj;//gui_log("obj:%s,%p\n",obj->name, circle->animate);


    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {

        if (tp->type == TOUCH_HOLD_Y)
        {
            if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
            {
                int pro = tp->y + tp->deltaY;//todo
                if (pro <= 0) { pro = 1; }
                if (pro >= obj->h) { pro = obj->h; }
                if (GET_BASE(circle->base.c)->type == IMAGE_FROM_MEM)
                {
                    pro = pro * (circle->base.max - 2) / obj->h; //gui_log("pro:%d\n",pro);
                }

                gui_progressbar_set_progress((void *)circle, (circle->base.max - 2) - pro);
            }
        }


    }

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {
        if (tp->type != 271)
        {
            ////gui_log("type2:%d,%d\n", tp->type, tp->released);
        }
        gui_seekbar_t *b = (void *)obj;
        switch (tp->type)
        {
        case TOUCH_SHORT:
            {
                //////gui_log("%s\n", "TOUCH_SHORT");
                //
                bool callback = false;
                if (!callback) { callback = false; }
                for (uint32_t i = 0; i < obj->event_dsc_cnt; i++)
                {
                    gui_event_dsc_t *event_dsc = obj->event_dsc + i;
                    if (event_dsc->filter == GUI_EVENT_TOUCH_CLICKED)
                    {
                        callback = true;
                    }
                } ////gui_log("%d\n", __LINE__);
                //if (callback)
                {
                    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                    {
                        ////gui_log("%d\n", __LINE__);
                        gui_obj_event_set(obj, GUI_EVENT_TOUCH_CLICKED);
                    }
                }
            }
            break;
        case TOUCH_LONG:
            {

            }
            break;

        default:
            break;
        }


        {
            if (tp->pressed)
            {
                if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                {
                    b->press_flag = true;
                    gui_obj_event_set(obj, GUI_EVENT_1);  ////gui_log("%d\n", __LINE__);
                    obj->cover = true;
                }
            }

            if (tp->released && b->press_flag)
            {

                {


                    {
                        b->press_flag = false;
                        gui_obj_event_set(obj, GUI_EVENT_2);
                        obj->cover = false;
                    }  ////gui_log("%d\n", __LINE__);

                }
            }
            if (b->press_flag)
            {
                gui_obj_event_set(obj, GUI_EVENT_3);
            }


        }
    }


}


static bool judge_point_in_ring(uint16_t cx, uint16_t cy, uint16_t r_in, uint16_t r_out,
                                uint16_t point_x, uint16_t point_y)
{
    return true;
}
#include "math.h"
static float get_gegree_to_center(uint16_t cx, uint16_t cy, uint16_t point_x, uint16_t point_y)
{
    float x = point_x - cx;
    float y = point_y - cy;
    if (x >= 0 && y < 0)
    {
        //gui_log("(111111111111)");
        return atan2f(x, -y);
    }
    else if (x > 0 && y >= 0)
    {
        //gui_log("(2222222222222)");
        return M_PI - atan2f(x, y);
    }
    else if (x <= 0 && y > 0)
    {
        //gui_log("(33333333333)");
        return M_PI + atan2f(-x, y);
    }
    else if (x < 0 && y <= 0)
    {
        //gui_log("(444444444444)");
        return 1.5f * M_PI + atan2f(-y, -x);
    }
    return 0.0f;



}
static void seekbar_preapre_arc(gui_obj_t *obj)
{
    obj_update_att(obj);
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_seekbar_t *circle = (gui_seekbar_t *)obj;

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {

        if (tp->type == TOUCH_HOLD_X || tp->type == TOUCH_HOLD_Y || tp->pressed)
        {
            if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
            {
                if (judge_point_in_ring(circle->arcx + 0, circle->arcy + 0, circle->arc_r,
                                        circle->arc_w + circle->arc_r, tp->x + tp->deltaX, tp->y + tp->deltaY))
                {
                    //gui_log("1");
                    float pro = get_gegree_to_center(circle->arcx + 0, circle->arcy + 0, tp->x + tp->deltaX,
                                                     tp->y + tp->deltaY);
                    float progress = 0;
                    GUI_UNUSED(progress);
                    // gui_log("degree:%f\n",pro*180.0f/M_PI);
                    float start = circle->arc_start * M_PI / 180.f;
                    float end = circle->arc_end * M_PI / 180.f;
                    if (start > end)
                    {
                        start = start - 2 * M_PI;
                        if (pro > end)
                        {
                            pro = pro - 2 * M_PI;
                        }
                        if (pro > end || pro < start)
                        {
                            return;
                        }
                        else
                        {
                            progress = (pro - start) / (end - start);
                        }
                    }
                    else
                    {

                        if (pro > end || pro < start)
                        {
                            return;
                        }
                        else
                        {
                            progress = (pro - start) / (end - start);
                        }
                    }
                    //gui_log("start:%f, end:%f, pro:%f, progress:%f\n", start, end, pro, progress);
                    gui_progressbar_set_percentage((void *)circle, progress);
                }
            }
        }


    }

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {
        if (tp->type != 271)
        {
            ////gui_log("type2:%d,%d\n", tp->type, tp->released);
        }
        gui_seekbar_t *b = (void *)obj;
        switch (tp->type)
        {
        case TOUCH_SHORT:
            {
                //////gui_log("%s\n", "TOUCH_SHORT");
                //
                bool callback = false;
                if (!callback) { callback = false; }
                for (uint32_t i = 0; i < obj->event_dsc_cnt; i++)
                {
                    gui_event_dsc_t *event_dsc = obj->event_dsc + i;
                    if (event_dsc->filter == GUI_EVENT_TOUCH_CLICKED)
                    {
                        callback = true;
                    }
                }

                {
                    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                    {
                        ////gui_log("%d\n", __LINE__);
                        gui_obj_event_set(obj, GUI_EVENT_TOUCH_CLICKED);
                    }
                }
            }
            break;
        case TOUCH_LONG:
            {

            }
            break;

        default:
            break;
        }


        {
            if (tp->pressed)
            {
                if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                {
                    b->press_flag = true;

                    gui_obj_event_set(obj, GUI_EVENT_1);  ////gui_log("%d\n", __LINE__);
                    obj->cover = true;
                }
            }

            if (tp->released && b->press_flag)
            {
                b->press_flag = false;
                gui_obj_event_set(obj, GUI_EVENT_2);
                obj->cover = false;
            }
            if (b->press_flag)
            {
                gui_obj_event_set(obj, GUI_EVENT_3);
            }


        }
    }


}
static void seekbar_h_preapre(gui_obj_t *obj)
{
    obj_update_att(obj);
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_seekbar_t *circle = (gui_seekbar_t *)obj;

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {

        if (tp->type == TOUCH_HOLD_X || tp->type == TOUCH_HOLD_Y || tp->pressed)
        {
            if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
            {
                int pro = tp->x + tp->deltaX - 0;
                if (pro <= 0) { pro = 1; }
                if (pro >= obj->w) { pro = obj->w; }
                if (GET_BASE(circle->base.c)->type == IMAGE_FROM_MEM)
                {
                    pro = pro * (circle->base.max - 2) / obj->w;
                    gui_log("pro:%d\n", pro);
                }
                gui_progressbar_set_progress((void *)circle, pro);
            }
        }
    }

    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == true)
    {
        if (tp->type != 271)
        {
            ////gui_log("type2:%d,%d\n", tp->type, tp->released);
        }
        gui_seekbar_t *b = (void *)obj;
        switch (tp->type)
        {
        case TOUCH_SHORT:
            {
                //////gui_log("%s\n", "TOUCH_SHORT");
                //
                bool callback = false;
                if (!callback) { callback = false; }
                for (uint32_t i = 0; i < obj->event_dsc_cnt; i++)
                {
                    gui_event_dsc_t *event_dsc = obj->event_dsc + i;
                    if (event_dsc->filter == GUI_EVENT_TOUCH_CLICKED)
                    {
                        callback = true;
                    }
                } ////gui_log("%d\n", __LINE__);

                if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                {
                    ////gui_log("%d\n", __LINE__);
                    gui_obj_event_set(obj, GUI_EVENT_TOUCH_CLICKED);
                }
            }
            break;
        case TOUCH_LONG:
            {

            }
            break;

        default:
            break;
        }


        {
            if (tp->pressed)
            {
                if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                {
                    b->press_flag = true;
                    gui_obj_event_set(obj, GUI_EVENT_1);  ////gui_log("%d\n", __LINE__);
                    obj->cover = true;
                }
            }

            if (tp->released && b->press_flag)
            {
                b->press_flag = false;
                gui_obj_event_set(obj, GUI_EVENT_2);
                obj->cover = false;
            }
            if (b->press_flag)
            {
                gui_obj_event_set(obj, GUI_EVENT_3);
            }


        }
    }



}
static void (obj_update_att)(struct _gui_obj_t *o)
{
    gui_seekbar_t *obj = (void *)o;

    if (obj->animate && obj->animate->animate)
    {
        size_t frame_count = obj->animate->dur * (1000 / 25) / (1000);
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


void gui_seekbar_ctor_img_v(gui_seekbar_t *this, gui_obj_t *parent, void *picture, int16_t x,
                            int16_t y)
{
    extern void gui_progressbar_v_img_ctor(gui_progressbar_t *this, gui_obj_t *parent, void *picture,
                                           int16_t x,
                                           int16_t y);
    gui_progressbar_v_img_ctor(&(this->base), parent, picture, x, y);
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_preapre;
    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}
void gui_seekbar_ctor_img_h(gui_seekbar_t *this, gui_obj_t *parent, void *picture, int16_t x,
                            int16_t y)
{
    extern void gui_progressbar_h_img_ctor(gui_progressbar_t *this, gui_obj_t *parent, void *picture,
                                           int16_t x,
                                           int16_t y);
    gui_progressbar_h_img_ctor(&(this->base), parent, picture, x, y);
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_h_preapre;
    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}
void gui_seekbar_ctor_movie_h(gui_seekbar_t *this, gui_obj_t *parent, void  **picture_array,
                              uint16_t array_length, int16_t x,
                              int16_t y)
{
    extern void gui_progressbar_movie_ctor(gui_progressbar_t *this, gui_obj_t *parent,
                                           void  **picture_array, uint16_t array_number,
                                           int16_t x,
                                           int16_t y);
    gui_progressbar_movie_ctor(&(this->base), parent, picture_array, array_length, x, y);
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_h_preapre;
    uint16_t w = gui_img_get_width((void *)this->base.c);
    uint16_t h = gui_img_get_height((void *)this->base.c);
    GET_BASE(this)->w = w;
    GET_BASE(this)->h = h;

    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}
void gui_seekbar_ctor_movie_v(gui_seekbar_t *this, gui_obj_t *parent, void  **picture_array,
                              uint16_t array_length, int16_t x,
                              int16_t y)
{
    extern void gui_progressbar_movie_ctor(gui_progressbar_t *this, gui_obj_t *parent,
                                           void  **picture_array, uint16_t array_number,
                                           int16_t x,
                                           int16_t y);
    gui_progressbar_movie_ctor(&(this->base), parent, picture_array, array_length, x, y);
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_preapre;
    uint16_t w = gui_img_get_width((void *)this->base.c);
    uint16_t h = gui_img_get_height((void *)this->base.c);
    GET_BASE(this)->w = w;
    GET_BASE(this)->h = h;

    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}
void gui_seekbar_ctor_movie_arc(gui_seekbar_t *this, gui_obj_t *parent, void  **picture_array,
                                uint16_t array_length, int16_t x,
                                int16_t y, uint16_t arc_x, uint16_t arc_y, uint16_t arc_r, uint16_t arc_w, float arc_start,
                                float arc_end)
{
    extern void gui_progressbar_movie_ctor(gui_progressbar_t *this, gui_obj_t *parent,
                                           void  **picture_array, uint16_t array_number,
                                           int16_t x,
                                           int16_t y);
    gui_progressbar_movie_ctor(&(this->base), parent, picture_array, array_length, x, y);
    this->arc_end = arc_end;
    this->arc_r = arc_r;
    this->arc_start = arc_start;
    this->arc_w = arc_w;
    this->arcx = arc_x;
    this->arcy = arc_y;
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_preapre_arc;
    uint16_t w = gui_img_get_width((void *)this->base.c);
    uint16_t h = gui_img_get_height((void *)this->base.c);
    GET_BASE(this)->w = w;
    GET_BASE(this)->h = h;

    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}
void gui_seekbar_ctor(gui_seekbar_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                      int16_t y, int16_t w, int16_t h)
{
    this->base.ctor = gui_progressbar_v_ctor;
    this->base.ctor(&(this->base), parent, filename, x, y, w, h);
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_preapre;
    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}
void gui_seekbar_h_ctor(gui_seekbar_t *this, gui_obj_t *parent, const char *filename, int16_t x,
                        int16_t y, int16_t w, int16_t h)
{
    this->base.ctor = gui_progressbar_ctor;
    this->base.ctor(&(this->base), parent, filename, x, y, w, h);
    //this->slider.slider_circle = gui_circle_create(this, "slider", this->base.get_progress(&(this->base)),
    //this->base.base.y+this->base.max_rectangle->base.base.base.h/2, this->base.max_rectangle->base.base.base.h/2, 0x0f0f);
    this->base.base.type = SEEKBAR;
    GET_BASE(this)->obj_prepare = seekbar_h_preapre;
    //gui_get_render_api_table()[GET_BASE(this)->type].obj_update_att = seekbar_update_att;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/


#if 0
gui_seekbar_t *gui_seekbar_create(void *parent, const char *filename, int16_t x, int16_t y,
                                  int16_t w, int16_t h)
{
//#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_ctor, _param_gui_seekbar_t)

    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_ctor(this, parent, filename, x, y, w, h);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
#endif
gui_seekbar_t *gui_seekbar_create_img_v(void *parent, const char *filename, int16_t x, int16_t y
                                       )
{
//#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_ctor, _param_gui_seekbar_t)

    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_ctor_img_v(this, parent, (void *)filename, x, y);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
gui_seekbar_t *gui_seekbar_create_img_h(void *parent, const char *filename, int16_t x, int16_t y
                                       )
{
//#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_ctor, _param_gui_seekbar_t)

    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_ctor_img_h(this, parent, (void *)filename, x, y);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
gui_seekbar_t *gui_seekbar_create_movie_h(void *parent, void  **picture_array,
                                          uint16_t array_length, int16_t x, int16_t y)
{
//#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_ctor, _param_gui_seekbar_t)

    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_ctor_movie_h(this, parent, picture_array, array_length, x, y);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
gui_seekbar_t *gui_seekbar_create_movie_v(void *parent, void  **picture_array,
                                          uint16_t array_length, int16_t x, int16_t y)
{
//#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_ctor, _param_gui_seekbar_t)

    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_ctor_movie_v(this, parent, picture_array, array_length, x, y);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
gui_seekbar_t *gui_seekbar_create_movie_arc(void *parent, void  **picture_array,
                                            uint16_t array_length, int16_t x, int16_t y, uint16_t arc_x, uint16_t arc_y, uint16_t arc_r,
                                            uint16_t arc_w, float arc_start, float arc_end)
{
//#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_ctor, _param_gui_seekbar_t)

    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_ctor_movie_arc(this, parent, picture_array, array_length, x, y,
                               arc_x, arc_y,  arc_r,  arc_w, arc_start, arc_end);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
#if 0
gui_seekbar_t *gui_seekbar_h_create(void *parent, const char *filename, int16_t x, int16_t y,
                                    int16_t w, int16_t h)
{
///#define _param_gui_seekbar_t this, parent, filename, x,y,w,h
//GUI_NEW(gui_seekbar_t, gui_seekbar_h_ctor, _param_gui_seekbar_t)
    gui_seekbar_t *this = gui_malloc(sizeof(gui_seekbar_t));
    memset(this, 0, sizeof(gui_seekbar_t));
    gui_seekbar_h_ctor(this, parent, filename, x, y, w, h);
//gui_list_init(&(((gui_obj_t *)this)->child_list));
//if ((((gui_obj_t *)this)->parent) != ((void *)0))
//{ gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
#endif

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */









/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file  gui_button.c
  * @brief button widget
  * @details click to trigger
  * @author triton_yu@realsil.com.cn
  * @date 2023/10/17
  * @version 1.0
  ***************************************************************************************
    * @attention
  * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
  ***************************************************************************************
  */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "gui_button.h"
#include "gui_obj.h"
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

static void (onPress)(gui_button_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_PRESSED, parameter);
}
static void (onRelease)(gui_button_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_RELEASED, parameter);
}
static void (onLong)(gui_button_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_LONG, parameter);
}
static void (onClick)(gui_button_t *b, void *callback, void *parameter)
{
    gui_obj_add_event_cb(b, (gui_event_cb_t)callback, GUI_EVENT_TOUCH_CLICKED, parameter);
}
static void (obj_update_att)(struct _gui_obj_t *o);
static void button_prepare(gui_obj_t *obj)
{
    obj_update_att(obj);
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();



    if (gui_obj_in_rect(obj, 0, 0, gui_get_screen_width(), gui_get_screen_height()) == false)
    {
        return;
    }
    {
        gui_button_t *b = (void *)obj;
        switch (tp->type)
        {
        case TOUCH_SHORT:
            {
                if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                {
                    gui_obj_event_set(obj, GUI_EVENT_TOUCH_CLICKED);
                }
            }
            break;
        case TOUCH_LONG:
            {
                if (b->long_flag == false)
                {
                    if (gui_point_in_obj_rect(obj, tp->x, tp->y) == true)
                    {
                        b->long_flag = true;
                        gui_obj_event_set(obj, GUI_EVENT_TOUCH_LONG);
                    }

                }
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

                    if (b->on_pic_addr && b->style == 0)
                    {
                        gui_imgconfig_t config =
                        {
                            .src_mode = b->src_mode,
                            .addr = b->on_pic_addr,
                            .x = b->img->base.x,
                            .y = b->img->base.y,
                        };
                        gui_img_set_config(b->img, &config);
                    }
                    //gui_log("%d\n", __LINE__);
                    gui_obj_event_set(obj, GUI_EVENT_TOUCH_PRESSED);  ////gui_log("%d\n", __LINE__);
                    b->long_flag = false;
                    b->press_flag = true;
                }
            }

            if (b->release_flag)
            {

                {
                    b->press_flag = false;
                    b->release_flag = false;
                    if (b->off_pic_addr && b->style == 0)
                    {
                        gui_imgconfig_t config =
                        {
                            .src_mode = b->src_mode,
                            .addr = b->off_pic_addr,
                            .x = b->img->base.x,
                            .y = b->img->base.y,
                        };
                        gui_img_set_config(b->img, &config);
                    }
                    {
                        gui_obj_event_set(obj, GUI_EVENT_TOUCH_RELEASED);
                    }
                    b->long_flag = false;
                }
            }
            if (tp->released && b->press_flag)
            {
                b->release_flag = true;
            }
        }

    }
}
static void (obj_update_att)(struct _gui_obj_t *o)
{
    gui_button_t *obj = (void *)o;
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
static void event_cb()
{

}
static void gui_button_ctor(
    gui_button_t *this,
    gui_obj_t *parent,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    void *background_pic,
    void *highlight_pic,
    char *text
)
{
    //for base class
    gui_obj_t *base = (gui_obj_t *)this;
    gui_obj_ctor(base, parent, "DEFAULT", x, y, w, h);

    //for root class
    gui_obj_t *root = (gui_obj_t *)this;
    root->type = BUTTON;
    root->obj_prepare = button_prepare;

    // for self
    this->off_pic_addr = background_pic;
    this->on_pic_addr = highlight_pic;
    gui_obj_add_event_cb(this, (gui_event_cb_t)event_cb, GUI_EVENT_TOUCH_LONG, 0);
    gui_obj_add_event_cb(this, (gui_event_cb_t)event_cb, GUI_EVENT_TOUCH_PRESSED, 0);
}


/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_button_text_move(gui_button_t *this, int16_t text_x, int16_t text_y)
{
    if (this->text)
    {
        this->text->base.x = text_x;
        this->text->base.y = text_y;
    }
}
void gui_button_text_color(gui_button_t *this, gui_color_t color)
{
    if (this->text)
    {
        this->text->color = color;
    }
}
void gui_button_img_move(gui_button_t *this, int16_t img_x, int16_t img_y)
{
    if (this->img)
    {
        this->img->base.x = img_x;
        this->img->base.y = img_y;
    }
}

void gui_button_click(gui_button_t *this, gui_event_cb_t event_cb)
{
    gui_obj_add_event_cb(this, event_cb, GUI_EVENT_TOUCH_CLICKED, NULL);
}
void gui_button_press(gui_button_t *this, gui_event_cb_t event_cb, void *parameter)
{
    gui_obj_add_event_cb(this, event_cb, GUI_EVENT_TOUCH_PRESSED, parameter);
}
void gui_button_long(gui_button_t *this, gui_event_cb_t event_cb, void *parameter)
{
    gui_obj_add_event_cb(this, event_cb, GUI_EVENT_TOUCH_LONG, parameter);
}
void gui_button_release(gui_button_t *this, gui_event_cb_t event_cb, void *parameter)
{
    gui_obj_add_event_cb(this, event_cb, GUI_EVENT_TOUCH_RELEASED, parameter);
}

void gui_button_set_animate(gui_button_t *o, uint32_t dur, int repeatCount, void *callback, void *p)
{
    gui_animate_t *animate = ((gui_button_t *)o)->animate;
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
    ((gui_button_t *)o)->animate = animate;
}
gui_api_button_t gui_button_api =
{
    .onClick = onClick,
    .onLong = onLong,
    .onPress = onPress,
    .onRelease = onRelease
};
static gui_button_t *gui_button_create_core(
    void *parent,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    void *background_pic,
    void *highlight_pic,
    char *text,
    char image_type,
    int count,
    IMG_SOURCE_MODE_TYPE src_mode
)
{
    GUI_ASSERT(parent != NULL);
    gui_button_t *button = gui_malloc(sizeof(gui_button_t));
    GUI_ASSERT(button != NULL);
    memset(button, 0, sizeof(gui_button_t));

    button->src_mode = src_mode;
    gui_button_ctor(button, parent, x, y, w, h, background_pic, highlight_pic, text);
    gui_list_init(&(GET_BASE(button)->child_list));
    if ((GET_BASE(button)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(button)->parent)->child_list),
                               &(GET_BASE(button)->brother_list));
    }
    if (background_pic)
    {
        switch (image_type)
        {
        case 0:
            if (src_mode == IMG_SRC_FILESYS)
            {
                button->img = (void *)gui_img_create_from_fs(button, background_pic, 0, 0);
            }
            else if (src_mode == IMG_SRC_MEMADDR)
            {
                button->img = (void *)gui_img_create_from_mem(button, "icon_img", background_pic, 0, 0, 0, 0);
            }
            break;
        case 1:
            if (src_mode == IMG_SRC_FILESYS)
            {
                button->img = (void *)gui_img_create_from_fs((void *)button, ((void **)background_pic)[0], 0, 0);
            }
            else if (src_mode == IMG_SRC_MEMADDR)
            {
                button->img = (void *)gui_img_create_from_mem((void *)button, "g", ((void **)background_pic)[0], 0,
                                                              0,
                                                              0, 0);
            }
            break;
        case 2:
            //button->img = (void *)gui_svg_create_from_mem((void *)button, background_pic, count, 0, 0, w, h);
            break;
        default:
            break;
        }
    }
    if (text)
    {
        gui_img_get_height(button->img);

        button->text = gui_text_create(button, "icon_text", 0, button->img->draw_img.img_h + 40,
                                       gui_get_screen_width(), h);
        gui_text_set(button->text, text, "rtk_font_mem", gui_rgb(UINT8_MAX, UINT8_MAX, UINT8_MAX),
                     strlen(text), 32);
        gui_text_mode_set(button->text, LEFT);
    }

    GET_BASE(button)->create_done = true;
    return button;
}


gui_button_t *gui_button_create(
    void *parent,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    void *background_pic,
    void *highlight_pic,
    char *text,
    char image_type,
    int count
)
{
    return gui_button_create_core(parent, x, y, w, h, background_pic, highlight_pic, text, image_type,
                                  count, IMG_SRC_MEMADDR);
}

gui_button_t *gui_button_create_from_fs(
    void *parent,
    int16_t x,
    int16_t y,
    int16_t w,
    int16_t h,
    void *background_pic,
    void *highlight_pic,
    char *text,
    char image_type,
    int count
)
{
    return gui_button_create_core(parent, x, y, w, h, background_pic, highlight_pic, text, image_type,
                                  count, IMG_SRC_FILESYS);
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */



/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_obj.c
  * @brief  object widget
  * @details create a object
  * @author luke_sun@realsil.com.cn
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
#include <gui_obj.h>
#include <gui_matrix.h>
#include <string.h>
#include <string.h>
#include <stdio.h>



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
void gui_obj_ctor(gui_obj_t *this, gui_obj_t *parent, const char *name, int16_t x, int16_t y,
                  int16_t w, int16_t h)
{
    GUI_ASSERT(parent != NULL);

    this->parent = parent;
    if (!name)
    {
        name = "_default_widget";
    }

    this->name = name;

    this->x = x;
    this->y = y;
    if (w == 0)
    {
        w = (int)gui_get_screen_width();
    }
    if (h == 0)
    {
        h = (int)gui_get_screen_height();
    }
    this->w = w;
    this->h = h;

    this->opacity_value = UINT8_MAX;
    this->matrix = gui_malloc(sizeof(struct gui_matrix));
    matrix_identity(this->matrix);

}
void gui_obj_dtor(gui_obj_t *this)
{
    volatile int a;
    a = this->x;
    return;
}

uint8_t gui_checksum(uint8_t seed, uint8_t *data, uint8_t len)
{
    uint8_t crc = seed;
    for (uint32_t i = 0; i < len; i++)
    {
        crc += *(data++);
    }
    return crc;
}

static void gui_obj_destory_cb(gui_obj_t *obj)
{
    gui_log("do obj %s free(destory), line = %d\n", obj->name, __LINE__);
    if (obj->obj_destory)
    {
        obj->obj_destory(obj);
    }
}

static void gui_tree_child_free(gui_obj_t *object)
{
    gui_list_t *node = NULL;
    gui_list_t *tmp = NULL;
    gui_list_for_each_safe(node, tmp, &object->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        gui_tree_child_free(obj);
        gui_obj_destory_cb(obj);
        if (obj->event_dsc != NULL)
        {
            gui_free(obj->event_dsc);
        }
        gui_free(obj);
    }
}

static void gui_tree_child_show(gui_obj_t *obj, bool enable)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        obj->not_show = !enable;
        gui_tree_child_show(obj, enable);
    }
}

/*============================================================================*
 *                           Private Functions
 *============================================================================*/
/** @defgroup WIDGET_Exported_Functions WIDGET Exported Functions
  * @{
  */
/*============================================================================*
*                           Public Functions
*============================================================================*/
void gui_obj_add_event_cb(void *obj, gui_event_cb_t event_cb, gui_event_t filter,
                          void *user_data)
{
    gui_obj_t *object = (gui_obj_t *)obj;
    object->event_dsc_cnt++;
    object->event_dsc = gui_realloc(object->event_dsc, sizeof(gui_event_dsc_t) * object->event_dsc_cnt);

    gui_event_dsc_t *event_dsc = object->event_dsc + object->event_dsc_cnt - 1;
    event_dsc->event_cb = event_cb;
    event_dsc->filter = filter;
    event_dsc->user_data = user_data;
    event_dsc->event_code = GUI_EVENT_INVALIDE;

}
void gui_obj_event_set(gui_obj_t *obj, gui_event_t event_code)
{
    for (uint32_t i = 0; i < obj->event_dsc_cnt; i++)
    {
        gui_event_dsc_t *event_dsc = obj->event_dsc + i;
        event_dsc->event_code = event_code;
    }
}

void gui_tree_print(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        gui_log(" %s \n", obj->name);
        gui_tree_print(obj);
    }
}

void gui_tree_free(void *obj)
{
    gui_obj_t *object = (gui_obj_t *)obj;
    gui_tree_child_free(object);
    if (object->parent)
    {
        gui_list_remove(&object->brother_list);
        gui_obj_destory_cb(obj);
        if (object->event_dsc != NULL)
        {
            gui_free(object->event_dsc);
        }
        gui_free(obj);
    }
    else
    {
        gui_log("obj %s parent equal NULL!\n", object->name);
        gui_list_init(&object->child_list);
    }
}

void gui_tree_show(gui_obj_t *obj, bool enable)
{
    gui_tree_child_show(obj, enable);
    GET_BASE(obj)->not_show = !enable;
}



gui_obj_t *gui_tree_get_root(gui_obj_t *obj)
{
    gui_obj_t *child = obj;
    while (true)
    {
        if (child->parent == NULL)
        {
            gui_log("gui_tree_get_root = %s\n", child->name);
            return child;
        }
        else
        {
            child = child->parent;
        }
    }
}


void gui_obj_show(void *obj, bool show_info)
{
    // gui_log("%p",obj);
    if (obj != NULL)
    {
        GET_BASE(obj)->not_show = !show_info;
    }
}



gui_obj_t *gui_get_child_handle(gui_obj_t *obj, obj_type_t child_type)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (obj->type == child_type)
        {
            return obj;
        }
    }
    return NULL;
}
void gui_tree_get_cover(gui_obj_t *obj, obj_type_t target, bool *rst)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *obj = gui_list_entry(node, gui_obj_t, brother_list);
        if (obj->type == target)
        {
            if (!(*rst))
            {
                *rst =  obj->cover;
            }
        }
        gui_tree_get_cover(obj, target, rst);
    }
}

void gui_obj_get_area(gui_obj_t *obj, int16_t *x, int16_t *y, int16_t *w, int16_t *h)
{
    gui_point_t p[4] =
    {
        {0, 0, 1},
        {obj->w - 1, 0, 1},
        {0, obj->h - 1, 1},
        {obj->w - 1, obj->h - 1, 1}
    };

    float x_min = 0.0f;
    float x_max = 0.0f;
    float y_min = 0.0f;
    float y_max = 0.0f;



    matrix_multiply_point(obj->matrix, p);
    x_min = p->p[0];
    x_max = p->p[0];
    y_min = p->p[1];
    y_max = p->p[1];

    matrix_multiply_point(obj->matrix, p + 1);
    matrix_multiply_point(obj->matrix, p + 2);
    matrix_multiply_point(obj->matrix, p + 3);


    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[0] < x_min)
        {
            x_min = point->p[0];
        }
    }

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[0] > x_max)
        {
            x_max = point->p[0];
        }
    }

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[1] < y_min)
        {
            y_min = point->p[1];
        }
    }

    for (uint32_t i = 1; i < 3; i++)
    {
        gui_point_t *point = p + i;
        if (point->p[1] > y_max)
        {
            y_max = point->p[1];
        }
    }
    *x = (int16_t)x_min;
    *y = (int16_t)y_min;
    *w = (int16_t)(x_max - x_min) + 1;
    *h = (int16_t)(y_max - y_min) + 1;
}

bool gui_obj_in_rect(gui_obj_t *obj, int16_t x, int16_t y, int16_t w, int16_t h)
{
    int16_t m_x, m_y, m_w, m_h;//caculate by obj matrix
    gui_obj_get_area(obj, &m_x, &m_y, &m_w, &m_h);

    if ((m_x >= x) && (m_x + m_w <= x + w) && (m_y >= y) && (m_y + m_h <= y + h))
    {
        return true;
    }

    return false;
}

bool gui_point_in_obj_rect(gui_obj_t *obj, int16_t x, int16_t y)
{
    int16_t m_x, m_y, m_w, m_h;//caculate by obj matrix
    gui_obj_get_area(obj, &m_x, &m_y, &m_w, &m_h);

    if ((m_x < x) && ((m_x + m_w) > x) && (m_y < y) && ((m_y + m_h) > y))
    {
        return true;
    }

    return false;
}

void gui_obj_skip_all_parent_left_hold(gui_obj_t *obj)
{
    for (gui_obj_t *o = obj; o->parent != NULL; o = o->parent)
    {
        o->skip_tp_left_hold = true;
    }
}

void gui_obj_skip_all_child_left_hold(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *o = gui_list_entry(node, gui_obj_t, brother_list);
        o->skip_tp_left_hold = true;
        gui_obj_skip_all_child_left_hold(o);
    }
}

void gui_obj_skip_all_left_hold(gui_obj_t *obj)
{
    gui_obj_t *o = obj;
    while (o->parent != NULL)
    {
        o = o->parent;
    }
    gui_obj_skip_all_child_left_hold(o);
}

void gui_obj_skip_all_parent_right_hold(gui_obj_t *obj)
{
    for (gui_obj_t *o = obj; o->parent != NULL; o = o->parent)
    {
        o->skip_tp_right_hold = true;
    }
}

void gui_obj_skip_all_child_right_hold(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *o = gui_list_entry(node, gui_obj_t, brother_list);
        o->skip_tp_right_hold = true;
        gui_obj_skip_all_child_right_hold(o);
    }
}

void gui_obj_skip_all_right_hold(gui_obj_t *obj)
{
    gui_obj_t *o = obj;
    while (o->parent != NULL)
    {
        o = o->parent;
    }
    gui_obj_skip_all_child_right_hold(o);
}

void gui_obj_skip_all_parent_up_hold(gui_obj_t *obj)
{
    for (gui_obj_t *o = obj; o->parent != NULL; o = o->parent)
    {
        o->skip_tp_up_hold = true;
    }
}

void gui_obj_skip_all_child_up_hold(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *o = gui_list_entry(node, gui_obj_t, brother_list);
        o->skip_tp_up_hold = true;
        gui_obj_skip_all_child_up_hold(o);
    }
}

void gui_obj_skip_all_up_hold(gui_obj_t *obj)
{
    gui_obj_t *o = obj;
    while (o->parent != NULL)
    {
        o = o->parent;
    }
    gui_obj_skip_all_child_up_hold(o);
}

void gui_obj_skip_all_parent_down_hold(gui_obj_t *obj)
{
    for (gui_obj_t *o = obj; o->parent != NULL; o = o->parent)
    {
        o->skip_tp_down_hold = true;
    }
}

void gui_obj_skip_all_child_down_hold(gui_obj_t *obj)
{
    gui_list_t *node = NULL;
    gui_list_for_each(node, &obj->child_list)
    {
        gui_obj_t *o = gui_list_entry(node, gui_obj_t, brother_list);
        o->skip_tp_down_hold = true;
        gui_obj_skip_all_child_down_hold(o);
    }
}

void gui_obj_skip_all_down_hold(gui_obj_t *obj)
{
    gui_obj_t *o = obj;
    while (o->parent != NULL)
    {
        o = o->parent;
    }
    gui_obj_skip_all_child_down_hold(o);
}


gui_color_t gui_rgba(unsigned char red, unsigned char green, unsigned char blue,
                     unsigned char opacity)
{
    gui_color_t color;
    // Use longer initialization to suppress warning.
    color.color.rgba.r = red;
    color.color.rgba.g = green;
    color.color.rgba.b = blue ;
    color.color.rgba.a = opacity ;
    return color;
}

gui_color_t gui_rgb(unsigned char red, unsigned char green, unsigned char blue)
{
    gui_color_t color;
    // Use longer initialization to suppress warning.
    color.color.rgba.r = red;
    color.color.rgba.g = green;
    color.color.rgba.b = blue ;
    color.color.rgba.a = UINT8_MAX ;
    return color;
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */







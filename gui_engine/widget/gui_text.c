/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_text.c
  * @brief text widget
  * @details text widget
  * @author luke_sun@realsil.com.cn
  * @date 2023/10/25
  * @version v1.0
  ***************************************************************************************
    * @attention
  * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
  ***************************************************************************************
  */

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include "gui_text.h"
#include "gui_matrix.h"
#include "font_mem.h"
#include "font_stb.h"

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
static void gui_text_font_load(gui_text_t *text, gui_rect_t *rect)
{
    switch (text->font_type)
    {
    case GUI_FONT_SOURCE_BMP:
        gui_font_mem_load(text, rect);
        break;
    case GUI_FONT_SOURCE_TTF:
        gui_font_stb_load(text, rect);
        break;
    default:
        break;
    }
}
static void gui_text_font_draw(gui_text_t *text, gui_rect_t *rect)
{
    switch (text->font_type)
    {
    case GUI_FONT_SOURCE_BMP:
        gui_font_mem_draw(text, rect);
        break;
    case GUI_FONT_SOURCE_TTF:
        gui_font_stb_draw(text, rect);
        break;
    default:
        break;
    }
}
static void gui_text_font_unload(gui_text_t *text)
{
    switch (text->font_type)
    {
    case GUI_FONT_SOURCE_BMP:
        gui_font_mem_unload(text);
        break;
    case GUI_FONT_SOURCE_TTF:
        gui_font_stb_unload(text);
        break;
    default:
        break;
    }
}

static void (obj_update_att)(struct _gui_obj_t *this)
{
    gui_text_t *obj = (void *)this;

    if (obj->animate && obj->animate->animate)
    {
        size_t frame_count = obj->animate->dur * 30 / (1000);
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
        obj->animate->progress_percent = ((float)(obj->animate->current_frame)) /
                                         ((float)(frame_count));
    }
}

static void gui_text_prepare(gui_obj_t *obj)
{
    gui_text_t *this = (void *)obj;


    obj_update_att(obj);
    gui_point_t point = {0, 0, 1};
    matrix_multiply_point(obj->matrix, &point);
    this->offset_x = point.p[0];
    this->offset_y = point.p[1];

    uint8_t last = this->checksum;
    this->checksum = 0;
    this->checksum = gui_checksum(0, (uint8_t *)this, sizeof(gui_text_t));

    if (last != this->checksum)
    {
        gui_fb_change();
    }
}

static void gui_text_draw(gui_obj_t *obj)
{
    gui_text_t *text = (gui_text_t *)obj;
    if (text->len == 0)
    {
        return;
    }
    struct gui_dispdev *dc = gui_get_dc();

    gui_rect_t draw_rect = {0};
    draw_rect.x1 = text->offset_x;
    draw_rect.y1 = text->offset_y;
    draw_rect.x2 = draw_rect.x1 + obj->w;
    draw_rect.y2 = draw_rect.y1 + obj->h;

    if (draw_rect.y1 >= (dc->section_count + 1)*dc->fb_height)
    {
        return;
    }

    if (draw_rect.y2 < (dc->section_count)*dc->fb_height)
    {
        return;
    }

    if (draw_rect.y1 >= dc->section_count * dc->fb_height && \
        draw_rect.y1 < (dc->section_count + 1)*dc->fb_height)
    {
        gui_text_font_load(text, &draw_rect);
    }
    gui_text_font_draw(text, &draw_rect);
    if (draw_rect.y2 >= dc->section_count * dc->fb_height && \
        draw_rect.y2 < (dc->section_count + 1)*dc->fb_height)
    {
        gui_text_font_unload(text);
    }
}

static void gui_text_end(gui_obj_t *obj)
{

}

void gui_text_ctor(gui_text_t *this, gui_obj_t *parent, const char *name, int16_t x,
                   int16_t y, int16_t w, int16_t h)
{
    //for base class
    gui_obj_t *base = (gui_obj_t *)this;
    gui_obj_ctor(base, parent, name, x, y, w, h);

    //for root class
    gui_obj_t *root = (gui_obj_t *)this;
    root->type = TEXTBOX;
    root->obj_prepare = gui_text_prepare;
    root->obj_draw = gui_text_draw;
    root->obj_end = gui_text_end;

    //for self
    this->mode = LEFT;
}

/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_text_set(gui_text_t *this, void *text, char *text_type, gui_color_t color,
                  uint16_t length, uint8_t font_size)
{
    if (strncmp(text_type, "rtk_font_mem", strlen("rtk_font_mem")) == 0)
    {
        this->font_type = GUI_FONT_SOURCE_BMP;
    }
    else if (strncmp(text_type, "rtk_font_stb", strlen("rtk_font_stb")) == 0)
    {
        this->font_type = GUI_FONT_SOURCE_TTF;
    }
    this->content = (uint8_t *)text;
    this->color = color;
    this->len = length;
    this->font_height = font_size;
    this->text_offset = 0;
    gui_fb_change();
}

void gui_text_set_animate(void *o, uint32_t dur, int repeatCount, void *callback, void *p)
{
    gui_animate_t *animate = ((gui_text_t *)o)->animate;
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
    ((gui_text_t *)o)->animate = animate;
}

void gui_text_mode_set(gui_text_t *this, TEXT_MODE mode)
{
    this->mode = mode;
}

// void gui_text_scale(gui_text_t *this, float scale_x, float scale_y)
// {
//     if (scale_x > 0 && scale_y > 0)
//     {
//         img->scale_x = scale_x;
//         img->scale_y = scale_y;
//     }
// }

void gui_text_move(gui_text_t *this, int16_t x, int16_t y)
{
    this->base.x = x ;
    this->base.y = y ;
}

void gui_text_size_set(gui_text_t *this, uint8_t height, uint8_t width)
{
    this->font_height = height;
}

void gui_text_type_set(gui_text_t *this, void *font_source)
{
    this->path = font_source;
}

void gui_text_encoding_set(gui_text_t *this, TEXT_CHARSET charset)
{
    this->charset = charset;
}

void gui_text_content_set(gui_text_t *this, void *text, uint16_t length)
{
    this->content = (uint8_t *)text;
    this->len = length;
    gui_fb_change();
}

gui_text_t *gui_text_create(void *parent, const char *name, int16_t x, int16_t y,
                            int16_t w, int16_t h)
{
    GUI_ASSERT(parent != NULL);
    if (name == NULL)
    {
        name = "DEFAULT_TEXT";
    }
    gui_text_t *text = gui_malloc(sizeof(gui_text_t));
    GUI_ASSERT(text != NULL);
    memset(text, 0, sizeof(gui_text_t));

    gui_text_ctor(text, parent, name, x, y, w, h);
    gui_list_init(&(GET_BASE(text)->child_list));
    if ((GET_BASE(text)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(text)->parent)->child_list), &(GET_BASE(text)->brother_list));
    }
    GET_BASE(text)->create_done = true;
    return text;
}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */

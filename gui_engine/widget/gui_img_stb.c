/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_img_stb.c
  * @brief stb image widget
  * @details stb image widget is used to show image which is not binary file on the screen
  * @author howie_wang@realsil.com.cn
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
#include <gui_img.h>
#include <string.h>
#include <gui_obj.h>
#include <draw_img.h>
#include <tp_algo.h>
#include "acc_engine.h"
#include <gui_img_stb.h>
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"


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
static void gui_image_stb_blit(draw_img_t *image, struct gui_dispdev *dc,
                               gui_rect_t *rect)
{
    gui_stb_img_t *buf_image = (gui_stb_img_t *)((gui_obj_t *)image - 1);
    int num_components = 0;
    uint8_t *stb_data = NULL;
    int stb_length = 0;
    stb_data = (uint8_t *)(image->data) + sizeof(struct gui_rgb_data_head);
    stb_length = ((struct gui_rgb_data_head *)(image->data))->w;
    switch (((struct gui_rgb_data_head *)(image->data))->type)
    {
    case BMP:
    case JPEG:
    case PNG:
        {
            if (dc->section_count == 0)
            {
                if (buf_image->src_changed)
                {
                    if (buf_image->buffer != 0)
                    {
                        gui_free(buf_image->buffer);
                        buf_image->buffer = NULL;
                    }
                    int source_w = 0;
                    int source_h = 0;
                    buf_image->buffer = stbi_load_from_memory(stb_data, stb_length, &source_w, &source_h,
                                                              &num_components, 0);
                    image->img_w = source_w;
                    image->img_h = source_h;
                }
            }
            int image_x = rect->x1;
            int image_y = rect->y1;
            int image_w = image->img_w;
            int image_h = image->img_h;
            int x_start = _UI_MAX(image_x, 0);
            int x_end = _UI_MIN(image_x + image_w, dc->fb_width);
            int y_start = _UI_MAX(dc->section.y1, image_y);
            int y_end = _UI_MIN(dc->section.y2, image_y + image_h);
            if ((x_start >= x_end) || (y_start >= y_end))
            {
                return;
            }
            uint32_t read_x_off = (uint32_t)(buf_image->buffer) - _UI_MIN(image_x, 0) * num_components;
            gui_dispdev_t *dc = gui_get_dc();
            uint8_t dc_bytes_per_pixel = dc->bit_depth >> 3;
            if (dc_bytes_per_pixel == 4)
            {
                uint32_t *writebuf = (uint32_t *)dc->frame_buf;
                uint8_t *pixel;
                for (uint32_t i = y_start; i < y_end; i++)
                {
                    int write_off = (i - dc->section.y1) * dc->fb_width ;
                    uint32_t read_off = read_x_off + ((i - image_y) * image_w) * num_components -
                                        num_components * x_start;
                    for (uint32_t j = x_start; j < x_end; j++)
                    {
                        pixel = (uint8_t *)(read_off + j * num_components);
                        if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0)
                        {
                            writebuf[write_off + j] = (pixel[0] << 16) + (pixel[1] << 8) + (pixel[2] << 0);
                        }
                    }
                }
            }
            else if (dc_bytes_per_pixel == 2)
            {
                uint16_t *writebuf = (uint16_t *)dc->frame_buf;
                uint8_t *pixel;
                for (uint32_t i = y_start; i < y_end; i++)
                {
                    int write_off = (i - dc->section.y1) * dc->fb_width ;
                    uint32_t read_off = read_x_off + ((i - image_y) * image_w) * num_components -
                                        num_components * x_start;
                    for (uint32_t j = x_start; j < x_end; j++)
                    {
                        pixel = (uint8_t *)(read_off + j * num_components);
                        if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0)
                        {
#if 1
                            uint16_t r = pixel[0] * 0x1f / 0xff << 11;
                            uint16_t g = pixel[1] * 0x3f / 0xff << 5;
                            uint16_t b = pixel[2] * 0x1f / 0xff;
                            writebuf[write_off + j] = r + g + b; //R
#else
                            uint16_t r = pixel[0] * 0x1f / 0xff << 3;
                            uint16_t g = pixel[1] * 0x3f / 0xff;
                            uint16_t b = pixel[2] * 0x1f / 0xff << 8;
                            uint16_t g1 = g >> 3;
                            uint16_t g2 = (g & 0x38) << 13;
                            writebuf[write_off + j] = r + g1 + g2 + b; //R
#endif
                        }
                    }
                }
            }
        }
        break;
    case GIF:
        {
            if (dc->section_count == 0)
            {
                if (buf_image->src_changed)
                {
                    if (buf_image->buffer != 0)
                    {
                        gui_free(buf_image->buffer);
                        buf_image->buffer = NULL;
                    }
                    int *delays = NULL;
                    delays = (int *)malloc(sizeof(int) * buf_image->gif_info->total_frame);
                    int source_w = 0;
                    int source_h = 0;
                    int frame = 0;
                    buf_image->buffer = stbi_load_gif_from_memory(stb_data, stb_length, &delays, &source_w, &source_h,
                                                                  &frame, &num_components, 3);
                    image->img_w = source_w;
                    image->img_h = source_h;
                    buf_image->gif_info->total_frame = frame;
                    buf_image->gif_info->delay_ms = (uint32_t *)delays;
                    for (int i = 0; i < buf_image->gif_info->total_frame; i++)
                    {
                        buf_image->gif_info->delay_ms_sum += delays[i];
                    }
                    buf_image->src_changed = false;
                }
            }
            int image_x = rect->x1;
            int image_y = rect->y1;
            int image_w = image->img_w;
            int image_h = image->img_h;
            int x_start = _UI_MAX(image_x, 0);
            int x_end = _UI_MIN(image_x + image_w, dc->fb_width);
            int y_start = _UI_MAX(dc->section.y1, image_y);
            int y_end = _UI_MIN(dc->section.y2, image_y + image_h);
            if ((x_start >= x_end) || (y_start >= y_end))
            {
                return;
            }
            num_components = 3;
            uint32_t read_x_off = (uint32_t)((uint8_t *)buf_image->buffer + image->img_h * image->img_w *
                                             buf_image->gif_info->current_frame * 3) - _UI_MIN(image_x, 0) * num_components;
            gui_dispdev_t *dc = gui_get_dc();
            uint8_t dc_bytes_per_pixel = dc->bit_depth >> 3;
            if (dc_bytes_per_pixel == 4)
            {
                uint32_t *writebuf = (uint32_t *)dc->frame_buf;
                uint8_t *pixel;
                for (uint32_t i = y_start; i < y_end; i++)
                {
                    int write_off = (i - dc->section.y1) * dc->fb_width ;
                    uint32_t read_off = read_x_off + ((i - image_y) * image_w) * num_components -
                                        num_components * x_start;
                    for (uint32_t j = x_start; j < x_end; j++)
                    {
                        pixel = (uint8_t *)(read_off + j * num_components);
                        if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0)
                        {
                            writebuf[write_off + j] = (pixel[0] << 16) + (pixel[1] << 8) + (pixel[2] << 0);
                        }
                    }
                }
            }
            else if (dc_bytes_per_pixel == 2)
            {
                uint16_t *writebuf = (uint16_t *)dc->frame_buf;
                uint8_t *pixel;
                for (uint32_t i = y_start; i < y_end; i++)
                {
                    int write_off = (i - dc->section.y1) * dc->fb_width ;
                    uint32_t read_off = read_x_off + ((i - image_y) * image_w) * num_components -
                                        num_components * x_start;
                    for (uint32_t j = x_start; j < x_end; j++)
                    {
                        pixel = (uint8_t *)(read_off + j * num_components);
                        if (pixel[0] != 0 || pixel[1] != 0 || pixel[2] != 0)
                        {
#if 1
                            uint16_t r = pixel[0] * 0x1f / 0xff << 11;
                            uint16_t g = pixel[1] * 0x3f / 0xff << 5;
                            uint16_t b = pixel[2] * 0x1f / 0xff;
                            writebuf[write_off + j] = r + g + b; //R
#else
                            uint16_t r = pixel[0] * 0x1f / 0xff << 3;
                            uint16_t g = pixel[1] * 0x3f / 0xff;
                            uint16_t b = pixel[2] * 0x1f / 0xff << 8;
                            uint16_t g1 = g >> 3;
                            uint16_t g2 = (g & 0x38) << 13;
                            writebuf[write_off + j] = r + g1 + g2 + b; //R
#endif
                        }
                    }
                }
            }
        }
        break;
    default:
        break;
    }
}
static void stb_image_prepare(gui_obj_t *obj)
{
    GUI_ASSERT(obj != NULL);
}
static void stb_image_draw_cb(gui_obj_t *obj)
{
    GUI_ASSERT(obj != NULL);
    gui_stb_img_t *img = (gui_stb_img_t *)obj;
    struct gui_dispdev *dc = gui_get_dc();
    draw_img_t *draw_img = &img->base.draw_img;

    gui_rect_t draw_rect = {0};

    draw_rect.x1 = draw_img->img_x;
    draw_rect.y1 = draw_img->img_y;
    draw_rect.x2 = draw_rect.x1 + obj->w;
    draw_rect.y2 = draw_rect.y1 + obj->h;
    if (((struct gui_rgb_data_head *)(img->base.draw_img.data))->type == GIF)
    {
        if (img->gif_info->delay_ms_sum)
        {
            uint32_t cur_time_ms = gui_ms_get();
            uint8_t index = (cur_time_ms - img->gif_info->init_time_ms) / img->gif_info->delay_ms_sum;
            int32_t delay_cur = (cur_time_ms - img->gif_info->init_time_ms) % img->gif_info->delay_ms_sum;
            if (index >= img->gif_info->cycle_index)
            {
                return;
            }
            uint8_t i = 0;
            while (delay_cur >= 0)
            {
                delay_cur -= img->gif_info->delay_ms[i++];
            }
            img->gif_info->current_frame = --i;
        }
    }
    gui_image_stb_blit(draw_img, dc, &draw_rect);
}
static void stb_image_end(gui_obj_t *obj)
{
    GUI_ASSERT(obj != NULL);
}
static void gui_stbimg_from_mem_ctor(gui_stb_img_t *this, gui_obj_t *parent, const char *name,
                                     void *addr, uint32_t size, uint8_t type, int16_t x, int16_t y)
{
    //for base class
    gui_img_t *img = (gui_img_t *)this;
    draw_img_t *draw_img = &img->draw_img;
    draw_img->data = addr;
    draw_img->opacity_value = 255;
    draw_img->blend_mode = IMG_FILTER_BLACK;
    draw_img->matrix = gui_malloc(sizeof(struct gui_matrix));
    draw_img->inverse = gui_malloc(sizeof(struct gui_matrix));
    draw_img->opacity_value = UINT8_MAX;

    //for root class
    gui_obj_t *root = (gui_obj_t *)this;
    gui_obj_ctor(root, parent, name, x, y, 0, 0);
    root->type = IMAGE_FROM_MEM;
    root->obj_prepare = stb_image_prepare;
    root->obj_draw = stb_image_draw_cb;
    root->obj_end = stb_image_end;

    //for self
    void *img_buf = gui_malloc(size + sizeof(struct gui_rgb_data_head));
    memcpy(((char *)img_buf + sizeof(struct gui_rgb_data_head)), addr, size);
    struct gui_rgb_data_head head;
    memset(&head, 0x0, sizeof(head));
    head.w = size;
    head.type = type;
    memcpy(img_buf, &head, sizeof(head));
    draw_img->data = img_buf;
    this->src_changed = true;
    if (type == GIF)
    {
        gui_gif_info_t *gif = gui_malloc(sizeof(gui_gif_info_t));
        gif->init_time_ms =  gui_ms_get();
        this->gif_info = gif;
        this->gif_info->total_frame = 1;
        this->gif_info->cycle_index = 0xFFFFFFFF;
    }
}
/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_stbimg_set_attribute(gui_stb_img_t *this, void *addr, uint32_t size,
                              uint8_t type, int16_t x, int16_t y)
{
    GUI_ASSERT(this != NULL);
    this->src_changed = true;
    if (((struct gui_rgb_data_head *)(this->base.draw_img.data))->type == GIF)
    {
        if (type == GIF)
        {
            this->gif_info->init_time_ms =  gui_ms_get();
        }
        else
        {
            gui_free(this->gif_info);
        }
    }
    else
    {
        if (type == GIF)
        {
            gui_gif_info_t *gif = gui_malloc(sizeof(gui_gif_info_t));
            gif->init_time_ms =  gui_ms_get();
            this->gif_info = gif;
            this->gif_info->total_frame = 1;
            this->gif_info->cycle_index = 0xFFFFFFFF;
        }
    }
    gui_free(this->base.draw_img.data);
    void *img_buf = gui_malloc(size + sizeof(struct gui_rgb_data_head));
    memcpy(((char *)img_buf + sizeof(struct gui_rgb_data_head)), addr, size);
    struct gui_rgb_data_head head;
    memset(&head, 0x0, sizeof(head));
    head.w = size;
    head.type = type ;
    GUI_ASSERT(addr != NULL);
    memcpy(img_buf, &head, sizeof(head));
    this->base.draw_img.data = img_buf;
}
gui_stb_img_t *gui_stbimg_create_from_mem(void *parent,  const char *name,
                                          void *addr, uint32_t size, uint8_t type, int16_t x,
                                          int16_t y)
{
    GUI_ASSERT(parent != NULL);
    if (name == NULL)
    {
        name = "DEFAULT_STB_IMAGE";
    }
    gui_stb_img_t *img = gui_malloc(sizeof(gui_stb_img_t));
    GUI_ASSERT(img != NULL);
    memset(img, 0x00, sizeof(gui_stb_img_t));
    gui_stbimg_from_mem_ctor(img, (gui_obj_t *)parent, name, addr, size, type, x, y);
    gui_list_init(&(GET_BASE(img)->child_list));
    if ((GET_BASE(img)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(img)->parent)->child_list), &(GET_BASE(img)->brother_list));
    }
    GET_BASE(img)->create_done = true;
    return img;
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */

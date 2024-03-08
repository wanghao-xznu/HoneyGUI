/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_progressbar.c
  * @brief progressbar widget
 * @details progressbar widget
  * @author luke_sun@realsil.com.cn
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
#include <gui_progressbar.h>
#include <string.h>
#include <gui_server.h>
#include <gui_obj.h>
#include <gui_img.h>


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

static void set_progress(gui_progressbar_t *this, size_t progress)
{
    this->per = ((float)progress) / ((float)this->max);
    if (GET_BASE(this->c)->type == IMAGE_SCOPE)
    {
        if (this->color == 1)
        {
            GUI_TYPE(gui_img_scope_t, this->c)->scope_x2 = progress;
        }
        else
        {
            GUI_TYPE(gui_img_scope_t, this->c)->scope_y1 = (1.0f - this->per) * this->max;
        }
    }
    else if (GET_BASE(this->c)->type == IMAGE_FROM_MEM)
    {
        size_t p = progress;
        if (p > this->max - 2)
        {
            p = this->max - 2;
        }

        GUI_TYPE(gui_img_t, this->c)->draw_img.data = ((void **)(this->color_hl))[p];
    }


}
static size_t get_progress(gui_progressbar_t *this)
{
    return (size_t)(this->per  * ((float)this->max));
}
static void set_percentage(gui_progressbar_t *this, float percentage)
{
    this->per = percentage;
    if (GET_BASE(this->c)->type == IMAGE_SCOPE)
    {
        if (this->color == 1)
        {
            GUI_TYPE(gui_img_scope_t, this->c)->scope_x2 = percentage * this->max;
        }
        else
        {
            GUI_TYPE(gui_img_scope_t, this->c)->scope_y1 = (1.0f - percentage) * this->max;
        }
    }
    else if (GET_BASE(this->c)->type == IMAGE_FROM_MEM)
    {
        size_t p = percentage * this->max;
        if (p > this->max - 2)
        {
            p = this->max - 2;
        }

        GUI_TYPE(gui_img_t, this->c)->draw_img.data = ((void **)(this->color_hl))[p];
    }
}
static float get_percentage(gui_progressbar_t *this)
{
    return this->per;
}
static size_t get_max(gui_progressbar_t *this)
{
    return this->max;
}

static void gui_progressbar_movie_ctor_core(gui_progressbar_t *this, gui_obj_t *parent,
                                            void  **picture_array,
                                            uint16_t array_number,
                                            int16_t x,
                                            int16_t y, IMG_SOURCE_MODE_TYPE src_mode)
{
    gui_obj_ctor(&(this->base), parent, "progressbar_movie", x, y, 0, 0);
    this->per = 0.3f;
    this->base.type = PROGRESSBAR;
    this->max = array_number - 1;
    gui_list_init(&(((gui_obj_t *)this)->child_list));
    if ((((gui_obj_t *)this)->parent) != ((void *)0))
    { gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }

    // gui_log("here %d %d %s\n", __LINE__, src_mode, picture_array[0]);
    this->src_mode = src_mode;
    if (src_mode == IMG_SRC_FILESYS)
    {
        this->c = (void *)gui_img_create_from_fs(this, picture_array[0], 0, 0);
    }
    else if (src_mode == IMG_SRC_MEMADDR)
    {
        this->c = (void *)gui_img_create_from_mem(this, "pro", picture_array[0], 0, 0, 0, 0);
    }
    this->color_hl = (uint32_t)picture_array;
}
/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_progressbar_set_progress(gui_progressbar_t *this, size_t progress)
{
    set_progress(this, progress);
}
size_t gui_progressbar_get_progress(gui_progressbar_t *this)
{
    return get_progress(this);
}
void gui_progressbar_set_percentage(gui_progressbar_t *this, float percentage)
{
    set_percentage(this, percentage);
}
float gui_progressbar_get_percentage(gui_progressbar_t *this)
{
    return get_percentage(this);
}
size_t gui_progressbar_get_max(gui_progressbar_t *this)
{
    return get_max(this);
}


void gui_progressbar_ctor(gui_progressbar_t *this, gui_obj_t *parent, const char *filename,
                          int16_t x,
                          int16_t y, int16_t w, int16_t h)
{

    gui_obj_ctor(&(this->base), parent, filename, x, y, w, h);

    this->color = 0xffffffff;
    this->per = 0.3f;
    this->base.type = PROGRESSBAR;
    this->max = w;
    gui_list_init(&(((gui_obj_t *)this)->child_list));
    if ((((gui_obj_t *)this)->parent) != ((void *)0))
    {
        gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list),
                               &(((gui_obj_t *)this)->brother_list));
    }
}
void gui_progressbar_h_img_ctor(gui_progressbar_t *this, gui_obj_t *parent, void *picture,
                                int16_t x,
                                int16_t y)
{

    gui_obj_ctor(&(this->base), parent, "h_img_pro", x, y, 0, 0);
    this->per = 0.3f;
    this->base.type = PROGRESSBAR;
    this->max = 0;
    this->color = 1;
    gui_list_init(&(((gui_obj_t *)this)->child_list));
    if ((((gui_obj_t *)this)->parent) != ((void *)0))
    { gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    this->c = (void *)gui_img_scope_create(this, picture, 0, 0);

    int w = gui_img_get_width((void *)this->c);
    this->max = w;
    GET_BASE(this)->w = w;
    GET_BASE(this)->h = gui_img_get_height((void *)this->c);
    GUI_TYPE(gui_img_scope_t, this->c)->scope_x2 = this->max * this->per;


}
void gui_progressbar_v_img_ctor(gui_progressbar_t *this, gui_obj_t *parent, void *picture,
                                int16_t x,
                                int16_t y)
{

    gui_obj_ctor(&(this->base), parent, "h_img_pro", x, y, 0, 0);
    this->per = 0.3f;
    this->base.type = PROGRESSBAR;
    this->max = 0;
    gui_list_init(&(((gui_obj_t *)this)->child_list));
    if ((((gui_obj_t *)this)->parent) != ((void *)0))
    { gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list), &(((gui_obj_t *)this)->brother_list)); }
    this->c = (void *)gui_img_scope_create(this, picture, 0, 0);

    int w = gui_img_get_height((void *)this->c);
    this->max = w;
    GET_BASE(this)->w = gui_img_get_width((void *)this->c);
    GET_BASE(this)->h = w;
    GUI_TYPE(gui_img_scope_t, this->c)->scope_y1 = this->max * (1.0f - this->per);


}

gui_progressbar_t *gui_progressbar_img_h_create(void *parent, void *picture, int16_t x, int16_t y)
{
    gui_progressbar_t *this = gui_malloc(sizeof(gui_progressbar_t));
    memset(this, 0, sizeof(gui_progressbar_t));
    gui_progressbar_h_img_ctor(this, parent, picture, x, y);

    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
gui_progressbar_t *gui_progressbar_img_v_create(void *parent, void *picture, int16_t x, int16_t y)
{
    gui_progressbar_t *this = gui_malloc(sizeof(gui_progressbar_t));
    memset(this, 0, sizeof(gui_progressbar_t));
    gui_progressbar_v_img_ctor(this, parent, picture, x, y);

    ((gui_obj_t *)this)->create_done = 1;
    return this;
}
void gui_progressbar_v_ctor(gui_progressbar_t *this, gui_obj_t *parent, const char *filename,
                            int16_t x,
                            int16_t y, int16_t w, int16_t h)
{

    gui_obj_ctor(&(this->base), parent, filename, x, y, w, h);

    this->color = 0xffffffff;
    this->per = 0.3f;
    this->base.type = PROGRESSBAR;
    this->max = h;
    gui_list_init(&(((gui_obj_t *)this)->child_list));
    if ((((gui_obj_t *)this)->parent) != ((void *)0))
    {
        gui_list_insert_before(&((((gui_obj_t *)this)->parent)->child_list),
                               &(((gui_obj_t *)this)->brother_list));
    }

}
void gui_progressbar_movie_ctor(gui_progressbar_t *this, gui_obj_t *parent, void  **picture_array,
                                uint16_t array_number,
                                int16_t x,
                                int16_t y)
{
    gui_progressbar_movie_ctor_core(this, parent, picture_array, array_number, x, y, IMG_SRC_MEMADDR);
}
void gui_progressbar_movie_ctor_fs(gui_progressbar_t *this, gui_obj_t *parent,
                                   void  **picture_array,
                                   uint16_t array_number,
                                   int16_t x,
                                   int16_t y)
{
    gui_progressbar_movie_ctor_core(this, parent, picture_array, array_number, x, y, IMG_SRC_FILESYS);
}
gui_progressbar_t *gui_progressbar_movie_create(void *parent, void  **picture_array,
                                                uint16_t array_length, int16_t x, int16_t y)
{
    gui_progressbar_t *this = gui_malloc(sizeof(gui_progressbar_t));
    memset(this, 0, sizeof(gui_progressbar_t));
    gui_progressbar_movie_ctor_core(this, parent, picture_array, array_length, x, y, IMG_SRC_MEMADDR);

    ((gui_obj_t *)this)->create_done = 1;
    return this;
}

gui_progressbar_t *gui_progressbar_movie_create_from_fs(void *parent, void  **picture_array,
                                                        uint16_t array_length, int16_t x, int16_t y)
{
    gui_progressbar_t *this = gui_malloc(sizeof(gui_progressbar_t));
    memset(this, 0, sizeof(gui_progressbar_t));
    gui_progressbar_movie_ctor_core(this, parent, picture_array, array_length, x, y, IMG_SRC_FILESYS);
    ((gui_obj_t *)this)->create_done = 1;
    return this;
}


/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */







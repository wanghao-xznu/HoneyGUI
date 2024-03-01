/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_perspective.c
  * @brief perspective widget
  * @details create 3D widget
  * @author howie_wang@realsil.com.cn
  * @date 2023/10/20
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
#include <string.h>
#include <gui_obj.h>
#include <tp_algo.h>
#include <draw_img.h>
#include <gui_matrix.h>
#include "gui_perspective.h"
#include "acc_engine.h"
#include <math.h>

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


/**
  * @brief  ...
  * @note   ...
  * @param[IN]  param1 ...
  * @param[IN]  param2 ...
  * @return ...
  * @retval ...
  */



static void scale_3d(Vertex_t *vertex, float scale)
{
    // Scale cube vertex coordinates to proper size.
    //
    vertex->x *= scale;
    vertex->y *= scale;
    vertex->z *= scale;
}



static void prepare(gui_obj_t *obj)
{
    struct gui_matrix rotate_3D;
    Vertex_t rv0, rv1, rv2, rv3;
    Vertex_t tv0, tv1, tv2, tv3;
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();

    gui_perspective_t *this = (gui_perspective_t *)obj;
    for (int i = 0; i < 6; i++)
    {
        gui_image_load_scale(&this->img[i]);
        matrix_identity(this->img[i].matrix);
    }

    float w = 300;
    float h = 300;

    float d = w * fix_sin(60);

    Vertex_t v0 = {-w / 2 + 10, -h / 2, d};
    Vertex_t v1 = {w  / 2 - 10, -h / 2, d};
    Vertex_t v2 = {w  / 2 - 10, h  / 2, d};
    Vertex_t v3 = {-w / 2 + 10, h  / 2, d};

    scale_3d(&v0, 1.0f);
    scale_3d(&v1, 1.0f);
    scale_3d(&v2, 1.0f);
    scale_3d(&v3, 1.0f);

    for (uint32_t i = 0; i < 6; i++)
    {
        this->ry[i] = i * 60;
    }
    switch (tp->type)
    {
    case TOUCH_HOLD_X:
        this->release_x = tp->deltaX;

        break;
    default:
        break;
    }

    for (uint32_t i = 0; i < 6; i++)
    {
        this->ry[i] += this->release_x;
        this->temp[i] = this->ry[i];
    }

    for (uint32_t i = 0; i < 6 - 1; i++)
    {
        for (uint32_t j = 0; j < 6 - i - 1; j++)
        {
            if (fix_cos(this->temp[j]) > fix_cos(this->temp[j + 1]))
            {
                float t = this->temp[j];
                this->temp[j] = this->temp[j + 1];
                this->temp[j + 1] = t;
            }
        }
    }

    for (uint32_t i = 0; i < 6; i++)
    {

        matrix_compute_rotate(0, this->ry[i], 0, &rotate_3D);

        matrix_transfrom_rotate(&rotate_3D, &v0, &tv0, 0, 0, 0);
        matrix_transfrom_rotate(&rotate_3D, &v1, &tv1, 0, 0, 0);
        matrix_transfrom_rotate(&rotate_3D, &v2, &tv2, 0, 0, 0);
        matrix_transfrom_rotate(&rotate_3D, &v3, &tv3, 0, 0, 0);

        matrix_compute_rotate(0, 0, 0, &rotate_3D);//set here for x-axis

        float xoff = (float)dc->screen_width / 2;
        float yoff = (float)dc->screen_height / 2 + 100;
        float zoff = -2 * d;

        matrix_transfrom_rotate(&rotate_3D, &tv0, &rv0, xoff, yoff, zoff);
        matrix_transfrom_rotate(&rotate_3D, &tv1, &rv1, xoff, yoff, zoff);
        matrix_transfrom_rotate(&rotate_3D, &tv2, &rv2, xoff, yoff, zoff);
        matrix_transfrom_rotate(&rotate_3D, &tv3, &rv3, xoff, yoff, zoff);

        Vertex_t p = {(float)(dc->screen_width) / 2, 0, 2 * d};
        matrix_transfrom_blit(this->img[i].img_w, this->img[i].img_h, &p, &rv0, &rv1, &rv2, &rv3,
                              this->img[i].matrix);

        memcpy(this->img[i].inverse, this->img[i].matrix, sizeof(struct gui_matrix));
        matrix_inverse(this->img[i].inverse);
        gui_image_new_area(&this->img[i]);
    }

    uint8_t last = this->checksum;
    this->checksum = this->release_x;

    if (last != this->checksum)
    {
        gui_fb_change();
    }

}

extern void gui_acc_blit(draw_img_t *image, struct gui_dispdev *dc, gui_rect_t *rect);
static void draw_cb(gui_obj_t *obj)
{
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = tp_get_info();
    gui_perspective_t *this = (gui_perspective_t *)obj;



    for (uint32_t j = 0; j < 6; j++)
    {
        for (uint32_t i = 0; i < 6; i++)
        {
            if (this->temp[j] == this->ry[i])
            {
                draw_img_t *draw_img = &this->img[i];

                gui_rect_t rect = {0};
                rect.x1 = draw_img->img_x;
                rect.y1 = draw_img->img_y;
                rect.x2 = rect.x1 + draw_img->target_w - 1;
                rect.y2 = rect.y1 + draw_img->target_h - 1;

                gui_acc_blit(draw_img, dc, &rect);
            }
        }
    }

}

static void end(gui_obj_t *obj)
{

}
static void destory(gui_obj_t *obj)
{
    // gui_log("perspective %s \n", __FUNCTION__);
    for (int i = 0; i < 6; i++)
    {
        draw_img_t *draw_img = &((gui_perspective_t *)obj)->img[i];
        gui_free(draw_img->inverse);
        gui_free(draw_img->matrix);
        if (draw_img->src_mode == IMG_SRC_FILESYS)
        {
#ifdef _WIN32
            // free path transforming memory on win
            gui_free(draw_img->data);
#endif
        }
    }
}
extern char *gui_img_filepath_transforming(void *addr);
static void gui_perspective_ctor(gui_perspective_t *this, gui_obj_t *parent, const char *name,
                                 gui_perspective_imgfile_t *img_file,
                                 int16_t x,
                                 int16_t y, int16_t w, int16_t h)
{
    //for base class
    gui_obj_t *base = (gui_obj_t *)this;
    gui_obj_ctor(base, parent, name, x, y, w, h);

    //for root class
    gui_obj_t *root = (gui_obj_t *)this;
    root->type = VG_LITE_CUBE;
    root->obj_prepare = prepare;
    root->obj_draw = draw_cb;
    root->obj_end = end;
    root->obj_destory = destory;

    //for self
    void **array = (void **)img_file->img_path;
    for (int i = 0; i < 6; i++)
    {
        draw_img_t *draw_img = &this->img[i];
        draw_img->src_mode = img_file->src_mode[i];
        if (img_file->src_mode[i] == IMG_SRC_FILESYS)
        {
            char *path = array[i];
#ifdef _WIN32
            path = gui_img_filepath_transforming(array[i]);
#endif
            draw_img->data = path;
        }
        else if (img_file->src_mode[i] == IMG_SRC_MEMADDR)
        {
            draw_img->data = array[i];
        }
        draw_img->opacity_value = UINT8_MAX;
        draw_img->blend_mode = IMG_SRC_OVER_MODE;
        draw_img->matrix = gui_malloc(sizeof(struct gui_matrix));
        draw_img->inverse = gui_malloc(sizeof(struct gui_matrix));
        this->temp[i] = i * 30;
        this->ry[i] = i * 30;
    }
}
/*============================================================================*
 *                           Public Functions
 *============================================================================*/
void gui_perspective_set_mode(gui_perspective_t *perspective, uint8_t img_index,
                              BLEND_MODE_TYPE mode)
{
    GUI_ASSERT(perspective != NULL);
    draw_img_t *draw_img = &perspective->img[img_index];
    draw_img->blend_mode = mode;
}

void gui_perspective_set_img(gui_perspective_t *perspective, gui_perspective_imgfile_t *img_file)
{
    gui_perspective_t *this = perspective;

    GUI_ASSERT(this != NULL);
    for (int i = 0; i < 6; i++)
    {
        draw_img_t *draw_img = &this->img[i];
        // reset file data
        if (draw_img->src_mode == IMG_SRC_FILESYS)
        {
#ifdef _WIN32
            gui_free(draw_img->data);
#endif
            draw_img->data = NULL;
        }

        // set new images
        draw_img->src_mode = img_file->src_mode[i];
        if (img_file->src_mode[i] == IMG_SRC_FILESYS)
        {
            void *path = (void *)img_file->img_path[i];
#ifdef _WIN32
            path = gui_img_filepath_transforming(path);
#endif
            draw_img->data = path;
        }
        else if (img_file->src_mode[i] == IMG_SRC_MEMADDR)
        {
            draw_img->data = img_file->data_addr[i];
        }
    }
}

gui_perspective_t *gui_perspective_create(void *parent,  const char *name,
                                          gui_perspective_imgfile_t *img_file,
                                          int16_t x,
                                          int16_t y)
{
    GUI_ASSERT(parent != NULL);
    if (name == NULL)
    {
        name = "DEFAULT_PERS";
    }
    gui_perspective_t *this = gui_malloc(sizeof(gui_perspective_t));
    GUI_ASSERT(this != NULL);
    memset(this, 0x00, sizeof(gui_perspective_t));

    gui_perspective_ctor(this, (gui_obj_t *)parent, name, img_file, x, y, 0, 0);

    gui_list_init(&(GET_BASE(this)->child_list));
    if ((GET_BASE(this)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(this)->parent)->child_list),
                               &(GET_BASE(this)->brother_list));
    }
    GET_BASE(this)->create_done = true;
    return this;
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */







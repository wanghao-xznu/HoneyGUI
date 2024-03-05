/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_cube.c
  * @brief cube widget
  * @details cube widget
  * @author howie_wang@realsil.com.cn
  * @date 2023/10/19
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
#include <draw_img.h>
#include <gui_matrix.h>
#include "gui_cube.h"
#include "tp_algo.h"
#include <math.h>
#include "acc_engine.h"

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
#define CUBE_JUDEG_FULL_RANK(m) if(full_rank(m->matrix))  //!< JUDGE IF THIS IS FULL RANK

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
static void scale_cube(Vertex_t *vertex, float scale)
{
    // Scale cube vertex coordinates to proper size.
    //
    vertex->x *= scale;
    vertex->y *= scale;
    vertex->z *= scale;
}

static void compute_rotate(float rx, float ry, float rz, struct gui_matrix *rotate)
{
    // Rotation angles rx, ry, rz (degree) for axis X, Y, Z
    // Compute 3D rotation matrix base on rotation angle rx, ry, rz about axis X, Y, Z.
    //
    rotate->m[0][0] = cos(RAD(rz)) * cos(RAD(ry));
    rotate->m[0][1] = cos(RAD(rz)) * sin(RAD(ry)) * sin(RAD(rx)) - sin(RAD(rz)) * cos(RAD(rx));
    rotate->m[0][2] = cos(RAD(rz)) * sin(RAD(ry)) * cos(RAD(rx)) + sin(RAD(rz)) * sin(RAD(rx));
    rotate->m[1][0] = sin(RAD(rz)) * cos(RAD(ry));
    rotate->m[1][1] = sin(RAD(rz)) * sin(RAD(ry)) * sin(RAD(rx)) + cos(RAD(rz)) * cos(RAD(rx));
    rotate->m[1][2] = sin(RAD(rz)) * sin(RAD(ry)) * cos(RAD(rx)) - cos(RAD(rz)) * sin(RAD(rx));
    rotate->m[2][0] = -sin(RAD(ry));
    rotate->m[2][1] = cos(RAD(ry)) * sin(RAD(rx));
    rotate->m[2][2] = cos(RAD(ry)) * cos(RAD(rx));
}

static void transfrom_rotate(struct gui_matrix *rotate, Vertex_t *vertex, Vertex_t *rc, float tx,
                             float ty)
{
    // Compute the new cube vertex coordinates transformed by the rotation matrix.
    //
    rc->x = rotate->m[0][0] * vertex->x + rotate->m[0][1] * vertex->y + rotate->m[0][2] * vertex->z;
    rc->y = rotate->m[1][0] * vertex->x + rotate->m[1][1] * vertex->y + rotate->m[1][2] * vertex->z;
    rc->z = rotate->m[2][0] * vertex->x + rotate->m[2][1] * vertex->y + rotate->m[2][2] * vertex->z;

    // Translate the vertex in XY plane.
    //
    rc->x += tx;
    rc->y += ty;
}

static void transfrom_normalZ(struct gui_matrix *rotate, Normal_t *nVec, float *nZ)
{
    // Compute the new normal Z coordinate transformed by the rotation matrix.
    //
    *nZ = rotate->m[2][0] * nVec->x + rotate->m[2][1] * nVec->y + rotate->m[2][2] * nVec->z;
}

// From 6.5.2 of OpenVG1.1 Spec: An affine transformation maps a point (x, y) into the point
// (x*sx + y*shx + tx, x*shy + y*sy + ty) using homogeneous matrix multiplication.
// To map a rectangle image (0,0),(w,0),(w,h),(0,h) to a parallelogram (x0,y0),(x1,y1),(x2,y2),(x3,y3).
// We get the following equations:
//
//     x0 = tx;
//     y0 = ty;
//     x1 = w*sx + tx;
//     y1 = w*shy + ty;
//     x3 = h*shx + tx;
//     y3 = h*sy + ty;
//
// So the homogeneous matrix sx, sy, shx, shy, tx, ty can be easily solved from above equations.
//
static void transfrom_blit(float w, float h, Vertex_t *v0, Vertex_t *v1, Vertex_t *v2, Vertex_t *v3,
                           struct gui_matrix *matrix)
{
    float sx, sy, shx, shy, tx, ty;

    // Compute 3x3 image transform matrix to map a rectangle image (w,h) to
    // a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise.
    //
    sx = (v1->x - v0->x) / w;
    sy = (v3->y - v0->y) / h;
    shx = (v3->x - v0->x) / h;
    shy = (v1->y - v0->y) / w;
    tx = v0->x;
    ty = v0->y;

    // Set the Blit transformation matrix
    matrix->m[0][0] = sx;
    matrix->m[0][1] = shx;
    matrix->m[0][2] = tx;
    matrix->m[1][0] = shy;
    matrix->m[1][1] = sy;
    matrix->m[1][2] = ty;
    matrix->m[2][0] = 0.0;
    matrix->m[2][1] = 0.0;
    matrix->m[2][2] = 1.0;
}


static bool full_rank(struct gui_matrix *m)
{
    for (int i = 0; i < 3; i++)
    {
        //gui_log("%f, %f, %f, ", m->m[0][i], m->m[1][i], m->m[2][i]);
        //gui_log("%f,%f,%f,", m->m[i][0], m->m[i][1], m->m[i][2]);
        if (m->m[i][0] == 0 && m->m[i][1] == 0 && m->m[i][2] == 0)
        {
            //gui_log("\n");
            return false;
        }
        //
        if ((m->m[0][i] * 10000.0f < 1000.0f && m->m[0][i] * 10000.0f > -1000.0f) &&
            (m->m[1][i] * 10000.0f < 1000.0f && m->m[1][i] * 10000.0f > -1000.0f) &&
            (m->m[2][i] * 10000.0f < 1000.0f && m->m[2][i] * 10000.0f > -1000.0f))
        {
            //gui_log("\n");
            return false;
        }
    } //gui_log("\n");
    return true;
}

static void cube_prepare(gui_obj_t *obj)
{
    struct gui_matrix rotate_3D;
    struct gui_matrix matrix;
    Vertex_t rv0, rv1, rv2, rv3, rv4, rv5, rv6, rv7;
    Vertex_t cube_v0 = {-1.0, -1.0, -1.0};
    Vertex_t cube_v1 = {1.0, -1.0, -1.0};
    Vertex_t cube_v2 = {1.0, 1.0, -1.0};
    Vertex_t cube_v3 = {-1.0, 1.0, -1.0};
    Vertex_t cube_v4 = {-1.0, -1.0, 1.0};
    Vertex_t cube_v5 = {1.0, -1.0, 1.0};
    Vertex_t cube_v6 = {1.0, 1.0, 1.0};
    Vertex_t cube_v7 = {-1.0, 1.0, 1.0};
    Normal_t normal0321 = {0.0, 0.0, -1.0};
    Normal_t normal4567 = {0.0, 0.0, 1.0};
    Normal_t normal1265 = {1.0, 0.0, 0.0};
    Normal_t normal0473 = {-1.0, 0.0, 0.0};
    Normal_t normal2376 = {0.0, 1.0, 0.0};
    Normal_t normal0154 = {0.0, -1.0, 0.0};

    gui_cube_t *this = (gui_cube_t *)obj;
    gui_dispdev_t *dc = gui_get_dc();
    touch_info_t *tp = (touch_info_t *)tp_get_info();
    gui_obj_t *root = (gui_obj_t *)obj;
    GUI_UNUSED(root);
    // Scale the cube to proper size
    float cbsize = this->cbsize;
    // Translate the cube to the center of framebuffer.
    float xoff = (this->c_x - dc->screen_width / 2) + dc->screen_width / 2;
    float yoff = (this->c_y - dc->screen_height / 2) + dc->screen_height / 2;

    scale_cube(&cube_v0, cbsize);
    scale_cube(&cube_v1, cbsize);
    scale_cube(&cube_v2, cbsize);
    scale_cube(&cube_v3, cbsize);
    scale_cube(&cube_v4, cbsize);
    scale_cube(&cube_v5, cbsize);
    scale_cube(&cube_v6, cbsize);
    scale_cube(&cube_v7, cbsize);


    // Rotation angles (degree) for axis X, Y, Z
    compute_rotate(this->xrot, this->yrot, this->zrot, &rotate_3D);

    if (this->auto_flag == true)
    {
        gui_fb_change();
        this->yrot = this->yrot + this->step_degree_y;
    }

    // Compute the new cube vertex coordinates transformed by the rotation matrix.
    transfrom_rotate(&rotate_3D, &cube_v0, &rv0, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v1, &rv1, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v2, &rv2, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v3, &rv3, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v4, &rv4, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v5, &rv5, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v6, &rv6, xoff, yoff);
    transfrom_rotate(&rotate_3D, &cube_v7, &rv7, xoff, yoff);

    // Compute the surface normal direction to determine the front/back face.
    transfrom_normalZ(&rotate_3D, &normal0321, &this->nz0321);
    transfrom_normalZ(&rotate_3D, &normal4567, &this->nz4567);
    transfrom_normalZ(&rotate_3D, &normal1265, &this->nz5126);
    transfrom_normalZ(&rotate_3D, &normal0473, &this->nz0473);
    transfrom_normalZ(&rotate_3D, &normal2376, &this->nz7623);
    transfrom_normalZ(&rotate_3D, &normal0154, &this->nz0154);

    draw_img_t *front = &this->draw_img_front;
    draw_img_t *back = &this->draw_img_back;
    draw_img_t *up = &this->draw_img_up;
    draw_img_t *down = &this->draw_img_down;
    draw_img_t *left = &this->draw_img_left;
    draw_img_t *right = &this->draw_img_right;

    gui_image_load_scale(front);
    gui_image_load_scale(back);
    gui_image_load_scale(up);
    gui_image_load_scale(down);
    gui_image_load_scale(left);
    gui_image_load_scale(right);


    if (this->nz0321 > 0.0f)
    {
        transfrom_blit(front->img_w, front->img_h, &rv0, &rv3, &rv2, &rv1, &matrix);

        gui_matrix_t tmp;
        memcpy(&tmp, obj->matrix, sizeof(struct gui_matrix));
        matrix_multiply(&tmp, &matrix);

        memcpy(front->matrix, &tmp, sizeof(struct gui_matrix));
        memcpy(front->inverse, &tmp, sizeof(struct gui_matrix));
        matrix_inverse(front->inverse);
        gui_image_new_area(front);
    }

    if (this->nz4567 > 0.0f)
    {
        transfrom_blit(back->img_w, back->img_h, &rv4, &rv5, &rv6, &rv7, &matrix);
        gui_matrix_t tmp;
        memcpy(&tmp, obj->matrix, sizeof(struct gui_matrix));
        matrix_multiply(&tmp, &matrix);
        memcpy(back->matrix, &tmp, sizeof(struct gui_matrix));
        memcpy(back->inverse, &tmp, sizeof(struct gui_matrix));
        matrix_inverse(back->inverse);
        gui_image_new_area(back);
    }

    if (this->nz5126 > 0.0f)
    {
        transfrom_blit(up->img_w, up->img_h, &rv5, &rv1, &rv2, &rv6, &matrix);
        gui_matrix_t tmp;
        memcpy(&tmp, obj->matrix, sizeof(struct gui_matrix));
        matrix_multiply(&tmp, &matrix);
        memcpy(up->matrix, &tmp, sizeof(struct gui_matrix));
        memcpy(up->inverse, &tmp, sizeof(struct gui_matrix));
        matrix_inverse(up->inverse);
        gui_image_new_area(up);
    }

    if (this->nz0473 > 0.0f)
    {
        transfrom_blit(down->img_w, down->img_h, &rv0, &rv4, &rv7, &rv3, &matrix);
        gui_matrix_t tmp;
        memcpy(&tmp, obj->matrix, sizeof(struct gui_matrix));
        matrix_multiply(&tmp, &matrix);
        memcpy(down->matrix, &tmp, sizeof(struct gui_matrix));
        memcpy(down->inverse, &tmp, sizeof(struct gui_matrix));
        matrix_inverse(down->inverse);
        gui_image_new_area(down);
    }

    if (this->nz7623 > 0.0f)
    {
        transfrom_blit(left->img_w, left->img_h, &rv7, &rv6, &rv2, &rv3, &matrix);
        gui_matrix_t tmp;
        memcpy(&tmp, obj->matrix, sizeof(struct gui_matrix));
        matrix_multiply(&tmp, &matrix);
        memcpy(left->matrix, &tmp, sizeof(struct gui_matrix));
        memcpy(left->inverse, &tmp, sizeof(struct gui_matrix));
        matrix_inverse(left->inverse);
        gui_image_new_area(left);
    }

    if (this->nz0154 > 0.0f)
    {
        transfrom_blit(right->img_w, right->img_h, &rv0, &rv1, &rv5, &rv4, &matrix);
        gui_matrix_t tmp;
        memcpy(&tmp, obj->matrix, sizeof(struct gui_matrix));
        matrix_multiply(&tmp, &matrix);
        memcpy(right->matrix, &tmp, sizeof(struct gui_matrix));
        memcpy(right->inverse, &tmp, sizeof(struct gui_matrix));
        matrix_inverse(right->inverse);
        gui_image_new_area(right);
    }
}

extern void gui_acc_blit(draw_img_t *image, struct gui_dispdev *dc, gui_rect_t *rect);
static void cube_draw_cb(gui_obj_t *obj)
{
    gui_dispdev_t *dc = gui_get_dc();
    gui_cube_t *this = (gui_cube_t *)obj;

    draw_img_t *front = &this->draw_img_front;
    draw_img_t *back = &this->draw_img_back;
    draw_img_t *up = &this->draw_img_up;
    draw_img_t *down = &this->draw_img_down;
    draw_img_t *left = &this->draw_img_left;
    draw_img_t *right = &this->draw_img_right;

    gui_rect_t draw_rect = {0};

    if (this->nz0321 > 0.0f)
    {
        draw_rect.x1 = this->draw_img_front.img_x;
        draw_rect.y1 = this->draw_img_front.img_y;
        draw_rect.x2 = draw_rect.x1 + this->draw_img_front.target_w - 1;
        draw_rect.y2 = draw_rect.y1 + this->draw_img_front.target_h - 1;
        CUBE_JUDEG_FULL_RANK(front)
        gui_acc_blit(front, dc, &draw_rect);

    }

    if (this->nz4567 > 0.0f)
    {
        draw_rect.x1 = this->draw_img_back.img_x;
        draw_rect.y1 = this->draw_img_back.img_y;
        draw_rect.x2 = draw_rect.x1 + this->draw_img_back.target_w - 1;
        draw_rect.y2 = draw_rect.y1 + this->draw_img_back.target_h - 1;
        CUBE_JUDEG_FULL_RANK(back)
        gui_acc_blit(back, dc, &draw_rect);
    }

    if (this->nz5126 > 0.0f)
    {
        draw_rect.x1 = this->draw_img_up.img_x;
        draw_rect.y1 = this->draw_img_up.img_y;
        draw_rect.x2 = draw_rect.x1 + this->draw_img_up.target_w - 1;
        draw_rect.y2 = draw_rect.y1 + this->draw_img_up.target_h - 1;
        CUBE_JUDEG_FULL_RANK(up)
        gui_acc_blit(up, dc, &draw_rect);
    }

    if (this->nz0473 > 0.0f)
    {
        draw_rect.x1 = this->draw_img_down.img_x;
        draw_rect.y1 = this->draw_img_down.img_y;
        draw_rect.x2 = draw_rect.x1 + this->draw_img_down.target_w - 1;
        draw_rect.y2 = draw_rect.y1 + this->draw_img_down.target_h - 1;
        CUBE_JUDEG_FULL_RANK(down)
        gui_acc_blit(down, dc, &draw_rect);
    }

    if (this->nz7623 > 0.0f)
    {
        draw_rect.x1 = this->draw_img_left.img_x;
        draw_rect.y1 = this->draw_img_left.img_y;
        draw_rect.x2 = draw_rect.x1 + this->draw_img_left.target_w - 1;
        draw_rect.y2 = draw_rect.y1 + this->draw_img_left.target_h - 1;
        CUBE_JUDEG_FULL_RANK(left)
        gui_acc_blit(left, dc, &draw_rect);
    }

    if (this->nz0154 > 0.0f)
    {
        draw_rect.x1 = this->draw_img_right.img_x;
        draw_rect.y1 = this->draw_img_right.img_y;
        draw_rect.x2 = draw_rect.x1 + this->draw_img_right.target_w - 1;
        draw_rect.y2 = draw_rect.y1 + this->draw_img_right.target_h - 1;
        CUBE_JUDEG_FULL_RANK(right)
        gui_acc_blit(right, dc, &draw_rect);
    }
}


static void cube_end(gui_obj_t *obj)
{

}
static void cube_destory(gui_obj_t *obj)
{
    // gui_log("%s \n", __FUNCTION__);
    gui_cube_t *this = (gui_cube_t *)obj;
    draw_img_t *cube_img[6];

    cube_img[0] = &this->draw_img_front;
    cube_img[1] = &this->draw_img_back;
    cube_img[2] = &this->draw_img_up;
    cube_img[3] = &this->draw_img_down;
    cube_img[4] = &this->draw_img_left;
    cube_img[5] = &this->draw_img_right;

    for (int i = 0; i < 6; i++)
    {
        gui_free(cube_img[i]->inverse);
        gui_free(cube_img[i]->matrix);
        if (cube_img[i]->src_mode)
        {
#ifdef _WIN32
            // free path transforming memory on win
            gui_free(cube_img[i]->data);
#endif
        }
    }
}

extern char *gui_img_filepath_transforming(void *addr);
static void gui_cube_ctor(gui_cube_t *this, gui_obj_t *parent, const char *name,
                          gui_cube_imgfile_t *img_file,
                          int16_t x,
                          int16_t y, int16_t w, int16_t h)
{

    //for base class
    gui_obj_t *base = (gui_obj_t *)this;
    gui_obj_ctor(base, parent, name, x, y, w, h);

    //for root class
    gui_obj_t *root = (gui_obj_t *)this;
    root->type = VG_LITE_CUBE;
    root->obj_prepare = cube_prepare;
    root->obj_draw = cube_draw_cb;
    root->obj_end = cube_end;
    root->obj_destory = cube_destory;
    //for self
    draw_img_t *cube_img[6];
    cube_img[0] = &this->draw_img_front;
    cube_img[1] = &this->draw_img_back;
    cube_img[2] = &this->draw_img_up;
    cube_img[3] = &this->draw_img_down;
    cube_img[4] = &this->draw_img_left;
    cube_img[5] = &this->draw_img_right;

    char *img_path[6];
    img_path[0] = img_file->img_path.img_path_front;
    img_path[1] = img_file->img_path.img_path_back;
    img_path[2] = img_file->img_path.img_path_up;
    img_path[3] = img_file->img_path.img_path_down;
    img_path[4] = img_file->img_path.img_path_left;
    img_path[5] = img_file->img_path.img_path_right;

    for (int i = 0; i < 6; i++)
    {
        if (img_file->src_mode[i] == IMG_SRC_FILESYS)
        {
            char *path = img_path[i];
#ifdef _WIN32
            path = gui_img_filepath_transforming(path);
#endif
            cube_img[i]->data = path;
        }
        else if (img_file->src_mode[i] == IMG_SRC_MEMADDR)
        {
            cube_img[i]->data = img_path[i];
        }

        cube_img[i]->src_mode = img_file->src_mode[i];
        cube_img[i]->opacity_value = UINT8_MAX;
        cube_img[i]->blend_mode = IMG_SRC_OVER_MODE;
        cube_img[i]->matrix = gui_malloc(sizeof(struct gui_matrix));
        cube_img[i]->inverse = gui_malloc(sizeof(struct gui_matrix));
    }

    gui_dispdev_t *dc = gui_get_dc();
    this->cbsize = dc->fb_height / 8.0;
    this->c_x = (dc->fb_width - this->cbsize) / 2.0f;
    this->c_y = (dc->fb_width - this->cbsize) / 2.0f;
    // Set the intial cube rotation degree and step.
    this->xrot = 20.0f;
    this->yrot = 0.0f;
    this->zrot = 20.0f;

}
/*============================================================================*
 *                           Public Functions
 *============================================================================*/

void gui_cube_auto_rotation_by_y(gui_cube_t *this, uint32_t internal_ms, float degree)
{
    this->auto_flag = true;
    this->interval_ms_y = internal_ms;
    this->step_degree_y = degree;
}
void gui_cube_set_center(gui_cube_t *this, float c_x, float c_y)
{
    this->c_x = c_x;
    this->c_y = c_y;
}
void gui_cube_set_size(gui_cube_t *this, float size)
{
    this->cbsize = size;
}

BLEND_MODE_TYPE gui_cube_get_mode(gui_cube_t *cube, CUBE_SIDE_TYPE side)
{
    GUI_ASSERT(cube != NULL);
    draw_img_t *cube_img[6];

    cube_img[0] = &cube->draw_img_front;
    cube_img[1] = &cube->draw_img_back;
    cube_img[2] = &cube->draw_img_up;
    cube_img[3] = &cube->draw_img_down;
    cube_img[4] = &cube->draw_img_left;
    cube_img[5] = &cube->draw_img_right;

    if (side == CUBE_SIDE_ALL)
    {
        // not supported
        return (BLEND_MODE_TYPE)0;
    }
    else
    {
        return (BLEND_MODE_TYPE)cube_img[side]->blend_mode;
    }
}

void gui_cube_set_mode(gui_cube_t *cube, CUBE_SIDE_TYPE side, BLEND_MODE_TYPE mode)
{
    GUI_ASSERT(cube != NULL);
    draw_img_t *cube_img[6];
    cube_img[0] = &cube->draw_img_front;
    cube_img[1] = &cube->draw_img_back;
    cube_img[2] = &cube->draw_img_up;
    cube_img[3] = &cube->draw_img_down;
    cube_img[4] = &cube->draw_img_left;
    cube_img[5] = &cube->draw_img_right;

    if (side == CUBE_SIDE_ALL)
    {
        for (int i = 0; i < 6; i++)
        {
            cube_img[i]->blend_mode = mode;
        }
    }
    else
    {
        cube_img[side]->blend_mode = mode;
    }
}

uint8_t gui_cube_get_opacity(gui_cube_t *cube, CUBE_SIDE_TYPE side)
{
    GUI_ASSERT(cube != NULL);
    draw_img_t *cube_img[6];

    cube_img[0] = &cube->draw_img_front;
    cube_img[1] = &cube->draw_img_back;
    cube_img[2] = &cube->draw_img_up;
    cube_img[3] = &cube->draw_img_down;
    cube_img[4] = &cube->draw_img_left;
    cube_img[5] = &cube->draw_img_right;

    if (side == CUBE_SIDE_ALL)
    {
        // not supported
        return 0;
    }
    else
    {
        return cube_img[side]->opacity_value;
    }
}

void gui_cube_set_opacity(gui_cube_t *cube, CUBE_SIDE_TYPE side, uint8_t opacity)
{
    GUI_ASSERT(cube != NULL);
    draw_img_t *cube_img[6];
    cube_img[0] = &cube->draw_img_front;
    cube_img[1] = &cube->draw_img_back;
    cube_img[2] = &cube->draw_img_up;
    cube_img[3] = &cube->draw_img_down;
    cube_img[4] = &cube->draw_img_left;
    cube_img[5] = &cube->draw_img_right;

    if (side == CUBE_SIDE_ALL)
    {
        for (int i = 0; i < 6; i++)
        {
            cube_img[i]->opacity_value = opacity;
        }
    }
    else
    {
        cube_img[side]->opacity_value = opacity;
    }
}

void gui_cube_set_img(gui_cube_t *cube, gui_cube_imgfile_t *img_file)
{
    gui_cube_t *this = cube;

    GUI_ASSERT(this != NULL);

    draw_img_t *cube_img[6];
    cube_img[0] = &this->draw_img_front;
    cube_img[1] = &this->draw_img_back;
    cube_img[2] = &this->draw_img_up;
    cube_img[3] = &this->draw_img_down;
    cube_img[4] = &this->draw_img_left;
    cube_img[5] = &this->draw_img_right;

    char *img_path[6];
    img_path[0] = img_file->img_path.img_path_front;
    img_path[1] = img_file->img_path.img_path_back;
    img_path[2] = img_file->img_path.img_path_up;
    img_path[3] = img_file->img_path.img_path_down;
    img_path[4] = img_file->img_path.img_path_left;
    img_path[5] = img_file->img_path.img_path_right;

    for (int i = 0; i < 6; i++)
    {
        // reset file data
        if (cube_img[i]->src_mode == IMG_SRC_FILESYS)
        {
#ifdef _WIN32
            gui_free(cube_img[i]->data);
#endif
            cube_img[i]->data = NULL;
        }

        // set new images
        cube_img[i]->src_mode = img_file->src_mode[i];
        if (img_file->src_mode[i] == IMG_SRC_FILESYS)
        {
            char *path = img_path[i];
#ifdef _WIN32
            path = gui_img_filepath_transforming(path);
#endif
            cube_img[i]->data = path;
        }
        else if (img_file->src_mode[i] == IMG_SRC_MEMADDR)
        {
            cube_img[i]->data = img_path[i];
        }
    }
}

gui_cube_t *gui_cube_create(void *parent,  const char *name, gui_cube_imgfile_t *img_file,
                            int16_t x,
                            int16_t y)
{
    GUI_ASSERT(parent != NULL);
    if (name == NULL)
    {
        name = "DEFAULT_CUBE";
    }
    gui_cube_t *cube = gui_malloc(sizeof(gui_cube_t));
    GUI_ASSERT(cube != NULL);
    memset(cube, 0x00, sizeof(gui_cube_t));

    gui_cube_ctor(cube, (gui_obj_t *)parent, name, img_file, x, y, 0, 0);
    gui_list_init(&(GET_BASE(cube)->child_list));
    if ((GET_BASE(cube)->parent) != NULL)
    {
        gui_list_insert_before(&((GET_BASE(cube)->parent)->child_list),
                               &(GET_BASE(cube)->brother_list));
    }
    GET_BASE(cube)->create_done = true;
    return cube;
}
/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */






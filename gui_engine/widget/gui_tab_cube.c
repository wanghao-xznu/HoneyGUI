/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file gui_tab_rotate.c
  * @brief tab widget
  * @details tab widget
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
#include <string.h>
#include <gui_server.h>
#include <gui_obj.h>
#include <tp_algo.h>
#include <kb_algo.h>
#include "gui_tab.h"
#include "gui_curtain.h"
#include "gui_img.h"
#include "gui_cube.h"



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

void gui_tab_cube(gui_obj_t *obj)
{
    gui_tab_t *this = (gui_tab_t *)obj;
    gui_dispdev_t *dc = gui_get_dc();
    gui_tabview_t *parent = (gui_tabview_t *)(obj->parent);
    float w = this->base.w;
    float h = this->base.h;
    float d = w;

    Vertex_t v0 = {-w, -h, d};
    Vertex_t v1 = {w,  -h, d};
    Vertex_t v2 = {w,  h,  d};
    Vertex_t v3 = {-w, h,  d};

    Vertex_t tv0, tv1, tv2, tv3;
    Vertex_t rv0, rv1, rv2, rv3;

    gui_matrix_t rotate_3D;
    float rotate_degree = 90 * parent->release_x / (this->base.w / 2);

    rotate_degree += 90.0 * (this->id.x - parent->cur_id.x);


    matrix_compute_rotate(0, rotate_degree, 0, &rotate_3D);

    matrix_multiply_normal(&rotate_3D, &this->normal);

    matrix_transfrom_rotate(&rotate_3D, &v0, &tv0, 0, 0, 0);
    matrix_transfrom_rotate(&rotate_3D, &v1, &tv1, 0, 0, 0);
    matrix_transfrom_rotate(&rotate_3D, &v2, &tv2, 0, 0, 0);
    matrix_transfrom_rotate(&rotate_3D, &v3, &tv3, 0, 0, 0);

    matrix_compute_rotate(0, 0, 0, &rotate_3D);
    float xoff = (float)dc->screen_width / 2;
    float yoff = (float)dc->screen_height / 2 ;
    float zoff = -(xoff + yoff) * 2;

    matrix_transfrom_rotate(&rotate_3D, &tv0, &rv0, xoff, yoff, zoff);
    matrix_transfrom_rotate(&rotate_3D, &tv1, &rv1, xoff, yoff, zoff);
    matrix_transfrom_rotate(&rotate_3D, &tv2, &rv2, xoff, yoff, zoff);
    matrix_transfrom_rotate(&rotate_3D, &tv3, &rv3, xoff, yoff, zoff);




    Vertex_t p = {(float)(dc->screen_width) / 2, (float)(dc->screen_height) / 2, xoff + yoff};
    gui_matrix_t temp;
    matrix_transfrom_blit(this->base.w, this->base.h, &p, &rv0, &rv1, &rv2, &rv3,
                          &temp);

#if 0
    float dot0 = this->normal.x * rv0.x + this->normal.y * rv0.y + this->normal.z * rv0.z;
    float dot1 = this->normal.x * rv1.x + this->normal.y * rv1.y + this->normal.z * rv1.z;
    float dot2 = this->normal.x * rv2.x + this->normal.y * rv2.y + this->normal.z * rv2.z;
    float dot3 = this->normal.x * rv3.x + this->normal.y * rv3.y + this->normal.z * rv3.z;

    if ((dot0 < 0) || (dot1 < 0) || (dot2 < 0) || (dot3 < 0))
    {
        obj->not_show = true;//todo
    }
#else

    if (rv0.x > rv1.x)
    {
        obj->not_show = true;
    }
#endif

    matrix_translate((this->id.x - parent->cur_id.x) * (int)this->base.w, \
                     (this->id.y - parent->cur_id.y) * (int)this->base.h, \
                     obj->matrix);
    if (abs(this->id.x - parent->cur_id.x) == 1)
    {
        matrix_translate(-(this->id.x - parent->cur_id.x) * (int)this->base.w, \
                         -(this->id.y - parent->cur_id.y) * (int)this->base.h, \
                         obj->matrix);
    }

    matrix_multiply(obj->matrix, &temp);

}

/** End of WIDGET_Exported_Functions
  * @}
  */

/** End of WIDGET
  * @}
  */

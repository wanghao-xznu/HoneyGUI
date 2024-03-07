#include "guidef.h"
#include "gui_img.h"
#include "gui_tabview.h"
#include "gui_tab.h"
#include "gui_img.h"
#include "gui_obj.h"
#include "gui_canvas.h"

#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include <gui_app.h>
#include "tiger_blue.txt"
#include "tiger_grey.txt"
#include "tiger_laven.txt"
#include "tiger_lime.txt"
#include "tiger_turk.txt"
#include "tiger_white.txt"
#include "tiger_yellow.txt"
#include "gui_server.h"
#include "gui_components_init.h"

#if 0
#include "gui_px.h"

static px_void PX_Object_3DModel_PixelShader(px_surface *psurface, px_int x, px_int y,
                                             px_point position, px_float u, px_float v, px_point4D normal, px_texture *pTexture, px_color color)
{
    px_float alpha;
    px_float cosv = PX_Point4DDot(PX_Point4DUnit(normal), PX_POINT4D(0, 0, 1));

    cosv = -cosv;
    if (cosv > 0)
    {
        alpha = (1 - cosv) * 128;
        //PX_SurfaceDrawPixel(psurface, x, y, PX_COLOR(255, (px_uchar)(color._argb.r + alpha), (px_uchar)(color._argb.g + alpha), (px_uchar)(color._argb.b + alpha)));
        PX_SurfaceDrawPixel(psurface, x, y, PX_COLOR(255, (px_uchar)(128 + alpha), (px_uchar)(0 + alpha),
                                                     (px_uchar)(128 + alpha)));
    }
}

static px_float rotY = 0;
static PX_OBJECT_RENDER_FUNCTION(PX_Object_3DModelRender)
{
    PX_Object_3DModel *pDesc = PX_ObjectGetDesc(PX_Object_3DModel, pObject);
    px_float objx, objy, objWidth, objHeight;
    px_float inheritX, inheritY;
    PX_ObjectGetInheritXY(pObject, &inheritX, &inheritY);
    objx = (pObject->x + inheritX);
    objy = (pObject->y + inheritY);
    objWidth = pObject->Width;
    objHeight = pObject->Height;

    PX_SurfaceClearAll(& pDesc->renderSurface, PX_COLOR(255, 0, 0, 0));
    PX_Object_3DModelSetWorld(pObject, 2, 2, 5.0f, 0, 90, 0, 1);
    PX_3D_Scene(&pDesc->renderlist, &pDesc->world, &pDesc->camera);
    PX_3D_Present(&pDesc->renderSurface, &pDesc->renderlist, &pDesc->camera);
    PX_SurfaceRender(psurface, &pDesc->renderSurface, (px_int)objx, (px_int)objy, PX_ALIGN_CENTER,
                     PX_NULL);
}

static void px_main(gui_px_t *this)
{
    gui_log("line = %d, func = %s \n", __LINE__, __FUNCTION__);
//    PX_Object_Firework01Create(this->mp_dynamic, this->px_root, 100, 300);
//    PX_Object_Firework01Create(this->mp_dynamic, this->px_root, 200, 300);
//    PX_Object_Firework01Create(this->mp_dynamic, this->px_root, 300, 300);

    PX_3D_ObjectData data;
    PX_IO_Data io;
    PX_Object *pObject;

#if defined _WIN32
    io = PX_LoadFileToIOData("win32_sim/assets/20255_Triangular_Prism_V1.obj");
#else
    io = PX_LoadFileToIOData("/win32_sim/assets/bunny.obj");
#endif
    if (io.size == 0)
    {
        gui_log("line = %d, func = %s \n", __LINE__, __FUNCTION__);
        return;
    }
    PX_3D_ObjectDataInitialize(this->mp_static, &data);
    if (!PX_3D_ObjectDataLoad(&data, io.buffer, io.size))
    {
        gui_log("line = %d, func = %s \n", __LINE__, __FUNCTION__);
        return;
    }

    pObject = PX_Object_3DModelCreate(this->mp_dynamic, this->px_root, 454 / 2, 454 / 2, 454, 454,
                                      &data);

    pObject->Func_ObjectRender = PX_Object_3DModelRender;

    PX_Object_3DModelSetWorld(pObject, 0, 0, 5.0f, 0, 0, 0, 1);
    PX_Object_3DModel *pdesc = PX_ObjectGetDesc(PX_Object_3DModel, pObject);
    PX_3D_RenderListSetPixelShader(&pdesc->renderlist, PX_Object_3DModel_PixelShader);
}


#endif

static void canvas_cb_black(gui_canvas_t *canvas)
{
    nvgRect(canvas->vg, 20, 256, 200, 180);
    nvgFillColor(canvas->vg, nvgRGBA(0, 0, 128, 200));
    nvgFill(canvas->vg);
}

#include "gui_cube.h"

static void app_dialing_ui_design(gui_app_t *app)
{
    gui_tabview_t *tv = gui_tabview_create(&(app->screen), "tabview", 0, 0, 0, 0);
    gui_tabview_set_style(tv, REDUCTION);
    gui_tab_t *tab_1 = gui_tab_create(tv, "tb_1",    0, 0, 0, 0, 0, 0);
    gui_tab_t *tab_2 = gui_tab_create(tv, "tb_2",    0, 0, 0, 0, 1, 0);

    gui_img_t *img_1 = gui_img_create_from_mem(tab_1,  "img_1_test", (void *)_actiger_blue, 0, 0, 0, 0);
    gui_img_create_from_mem(tab_1,  "img_1", (void *)_actiger_yellow, 250, 250, 0, 0);
    gui_img_t *img_2 = gui_img_create_from_mem(tab_2,  "img_2", (void *)_actiger_turk, 100, 100, 0, 0);

    gui_canvas_t *canvas = gui_canvas_create(tab_1, "canvas", 0, 0, 0, 454, 454);
    gui_canvas_set_canvas_cb(canvas, canvas_cb_black);

    // gui_px_t *px = gui_px_create(&app->screen, "px-test", px_main, 0, 0, 454, 454);

    gui_cube_imgfile_t image_file;
    memset(&image_file, 0, sizeof(gui_cube_imgfile_t));
    for (int i = 0; i < 6; i++)
    {
        image_file.src_mode[i] = IMG_SRC_MEMADDR;
    }
    image_file.data_addr.data_addr_up = (void *)_actiger_blue;
    image_file.data_addr.data_addr_down = (void *)_actiger_turk;
    image_file.data_addr.data_addr_front = (void *)_actiger_laven;
    image_file.data_addr.data_addr_left = (void *)_actiger_lime;
    image_file.data_addr.data_addr_right = (void *)_actiger_yellow;
    image_file.data_addr.data_addr_back = (void *)_actiger_white;
    gui_cube_t *ccc = gui_cube_create(tab_1, "ccc", &image_file, 0, 0);
    gui_cube_set_center(ccc, 227, 227);
    gui_cube_set_size(ccc, 75);
    gui_cube_auto_rotation_by_y(ccc, 100, 5.5f);

}


static gui_app_t rtk_gui_demo =
{
    .screen =
    {
        .name = "rtk_gui_demo",
        .x    = 0,
        .y    = 0,
    },
    .ui_design = app_dialing_ui_design,
    .active_ms = 1000 * 60 * 60,
};


static int app_init(void)
{
    gui_server_init();
    gui_app_startup(&rtk_gui_demo);
    return 0;
}



#ifdef __RTTHREAD__

#include "rtthread.h"

MSH_CMD_EXPORT(app_init, audio audio test);

#else
GUI_INIT_APP_EXPORT(app_init);
#endif







/**
*****************************************************************************************
*     Copyright(c) 2017, Realtek Semiconductor Corporation. All rights reserved.
*****************************************************************************************
  * @file guidef.h
  * @brief base define for bee gui
  * @details base define for bee gui
  * @author howie_wang@realsil.com.cn
  * @date 2023/10/19
  * @version 1.0
  ***************************************************************************************
    * @attention
  * <h2><center>&copy; COPYRIGHT 2017 Realtek Semiconductor Corporation</center></h2>
  ***************************************************************************************
  */

/*============================================================================*
 *               Define to prevent recursive inclusion
 *============================================================================*/
#ifndef __GUIDEF_H__
#define __GUIDEF_H__
#ifdef __cplusplus
extern "C" {
#endif

/*============================================================================*
 *                        Header Files
 *============================================================================*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "gui_list.h"
#include "gui_event.h"

#if defined(__CC_ARM) || defined(__CLANG_ARM)           /* ARM Compiler */
#include <stdarg.h>
#define gui_inline                   static __inline
#elif defined (__IAR_SYSTEMS_ICC__)     /* for IAR Compiler */
#include <stdarg.h>
#define gui_inline                   static inline
#elif defined (__GNUC__)                /* GNU GCC Compiler */
#define gui_inline                   static __inline
#elif defined _WIN32
#define gui_inline                   __inline
#else
#error not supported tool chain
#endif

/** @defgroup SUBMOUDLE SUBMOUDLE
  * @brief
  * @{
  */

/*============================================================================*
 *                         Types
 *============================================================================*/
/** @defgroup SUBMOUDLE_Exported_Types SUBMOUDLE Exported Types
  * @brief
  * @{
  */



typedef struct gui_touch_port_data
{
    uint8_t          event;                 /* The touch event of the data */
    uint8_t          track_id;              /* Track id of point */
    uint16_t         width;                 /* Point of width */
    uint16_t         x_coordinate;          /* Point of x coordinate */
    uint16_t         y_coordinate;          /* Point of y coordinate */
    uint32_t         timestamp_ms;             /* The timestamp when the data was received */
    void            *data;
} gui_touch_port_data_t;

typedef struct touch_info
{
    int16_t deltaX;
    int16_t deltaY;
    int16_t x;
    int16_t y;
    uint32_t type;
    uint8_t pressed : 1;
    uint8_t released : 1;
    uint8_t pressing : 1;
} touch_info_t;

typedef struct gui_kb_port_data
{
    uint8_t       event;
    uint8_t       name[10];
    uint32_t      timestamp_ms;             /* The timestamp when the data was received */
} gui_kb_port_data_t;

typedef struct kb_info
{
    uint32_t type;
    char *string[20];
    uint8_t pressed : 1;
    uint8_t released : 1;
    uint8_t pressing : 1;
} kb_info_t;

typedef struct gui_wheel_port_data
{
    uint8_t       event;
    int16_t       delta;
    uint32_t      timestamp_ms;             /* The timestamp when the data was received */
} gui_wheel_port_data_t;

typedef struct wheel_info
{
    uint32_t type;
    int16_t delta;
} wheel_info_t;


typedef struct gui_rect
{
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
    int16_t xboundleft;
    int16_t xboundright;
    int16_t yboundtop;
    int16_t yboundbottom;
} gui_rect_t;

typedef enum dc_type
{
    DC_RAMLESS,
    DC_SINGLE,
    DC_DOUBLE,
} gui_dc_type_t;

typedef struct gui_dispdev
{
    /* width and height */
    uint16_t screen_width;
    uint16_t screen_height;
    uint16_t fb_width;
    uint16_t fb_height;
    uint16_t driver_ic_fps;
    uint16_t driver_ic_hfp;
    uint16_t driver_ic_hbp;
    uint16_t driver_ic_active_width;
    gui_dc_type_t type;
    char gpu_type;
    /* pixel data */
    uint8_t *frame_buf;
    uint8_t *disp_buf_1;
    uint8_t *disp_buf_2;
    float scale_x;
    float scale_y;
    gui_rect_t section;
    //uint16_t section_height;
    uint32_t section_count;
    uint8_t bit_depth;
    void (*lcd_update)(struct gui_dispdev *dc);
    int (*flash_seq_trans_enable)(void);
    int (*flash_seq_trans_disable)(void);
    uint32_t (*get_lcd_us)(void);
    void (*reset_lcd_timer)(void);
    void (*lcd_te_wait)(void);
    void (*lcd_power_on)(void);
    void (*lcd_power_off)(void);
} gui_dispdev_t;
typedef struct
{
    int fd;     /* directory file */
    char buf[512];
    int num;
    int cur;
} gui_fs_DIR;
struct gui_fs_dirent
{
    uint8_t d_type;           /* The type of the file */
    uint8_t d_namlen;         /* The length of the not including the terminating null file name */
    uint16_t d_reclen;        /* length of this record */
    char d_name[256];   /* The null-terminated file name */
};
struct gui_fs
{
    int (*open)(const char *file, int flags, ...);
    int (*close)(int d);
    int (*read)(int fd, void *buf, size_t len);
    int (*write)(int fd, const void *buf, size_t len);
    int (*lseek)(int fd, int offset, int whence);
    /* directory api*/
    gui_fs_DIR *(*opendir)(const char *name);
    struct gui_fs_dirent *(*readdir)(gui_fs_DIR *d);
    int (*closedir)(gui_fs_DIR *d);
    int (*ioctl)(int fildes, int cmd, ...);
};

struct gui_indev
{
    uint16_t tp_witdh;
    uint16_t tp_height;
    uint32_t touch_timeout_ms;
    uint16_t long_button_time_ms;
    uint16_t short_button_time_ms;
    uint16_t kb_long_button_time_ms;
    uint16_t kb_short_button_time_ms;
    uint16_t quick_slide_time_ms;

    void (*ext_button_indicate)(void (*callback)(void));

    gui_touch_port_data_t *(*tp_get_data)(void);

    gui_kb_port_data_t *(*kb_get_port_data)(void);

    gui_wheel_port_data_t *(*wheel_get_port_data)(void);
};

typedef void (* log_func_t)(const char *fmt, ...);

struct gui_os_api
{
    char *name;
    void *(*thread_create)(const char *name, void (*entry)(void *param), void *param,
                           uint32_t stack_size, uint8_t priority);
    bool (*thread_delete)(void *handle);
    bool (*thread_suspend)(void *handle);
    bool (*thread_resume)(void *handle);
    bool (*thread_mdelay)(uint32_t ms);
    uint32_t (*thread_ms_get)(void);
    bool (*mq_create)(void *handle, const char *name, uint32_t msg_size, uint32_t max_msgs);
    bool (*mq_send)(void *handle, void *buffer, uint32_t size, uint32_t timeout);
    bool (*mq_send_urgent)(void *handle, void *buffer, uint32_t size, uint32_t timeout);
    bool (*mq_recv)(void *handle, void *buffer, uint32_t size, uint32_t timeout);


    void *(*f_malloc)(uint32_t);
    void *(*f_realloc)(void *ptr, uint32_t);
    void (*f_free)(void *rmem);
    void *mem_addr;
    uint32_t mem_size;
    log_func_t log;
    void (*gui_tick_hook)(void);
};

typedef struct gui_matrix
{
    float m[3][3];
} gui_matrix_t;

typedef struct gui_point
{
    float p[3];
} gui_point_t;

typedef enum
{
    TOUCH_INIT      = 0x100,
    TOUCH_HOLD_X,
    TOUCH_HOLD_Y,
    TOUCH_SHORT,
    TOUCH_LONG,
    TOUCH_ORIGIN_FROM_X,
    TOUCH_ORIGIN_FROM_Y,
    TOUCH_LEFT_SLIDE,
    TOUCH_RIGHT_SLIDE,
    TOUCH_UP_SLIDE,
    TOUCH_DOWN_SLIDE,
    TOUCH_SHORT_BUTTON,
    TOUCH_LONG_BUTTON,
    TOUCH_UP_SLIDE_TWO_PAGE,
    TOUCH_DOWN_SLIDE_TWO_PAGE,
    TOUCH_INVALIDE,

    KB_INIT      = 0x200,
    KB_SHORT,
    KB_LONG,
    KB_INVALIDE,

    WHEEL_INIT   = 0x300,
    WHEEL_ING,
    WHEEL_FINISHED,
    WHEEL_INVALIDE,


} GUI_InputType;

typedef enum gui_tab_style
{
    CLASSIC,
    REDUCTION,
    FADE,
    REDUCTION_FADE,
    STACKING,

    TAB_ROTATE,
    TAB_CUBE,
    TAB_PAGE,
} SLIDE_STYLE;



typedef enum obj_type
{
    SCREEN,
    WINDOW,
    TABVIEW,
    TAB,
    CURTAINVIEW,
    CURTAIN,
    IMAGE,
    BUTTON,
    ICONLIST,
    ICON,
    IMAGE_FROM_MEM,
    TEXTBOX,
    SCROLLTEXTBOX,
    SEEKBAR,
    PROGRESSBAR,
    CLICKSWITCH,
    PAGE,
    SCROLL_WHEEL,
    PAGEBAR,
    RETURNWIDGET,
    RECTANGLE,
    WINDOWWITHBORDER,
    CANVAS,
    VG_LITE_CLOCK,
    VG_LITE_CUBE,
    GRID,
    RADIO,
    RADIOSWITCH,
    ARC,
    JAVASCRIPT,
    MOVIE,
    IMAGE_SCOPE,
    CARDVIEW,
    CARD,
    PAGELIST,
    PAGELISTVIEW,
    MACRO_ANIMATETRANSFORM,
} obj_type_t;
typedef struct gui_msg
{
    /* the event type */
    uint8_t type;
    /* user field of event */
    union
    {
        uint32_t  param;
        void      *payload;
    } u;
    void *app;
    void *cb;

} gui_msg_t;

typedef struct gui_index
{
    int8_t x;
    int8_t y;
} gui_index_t;
typedef struct gui_jump
{
    bool jump_flag;
    gui_index_t jump_id;
} gui_jump_t;

typedef struct gui_animate
{
    uint32_t dur;
    int repeatCount;
    uint32_t current_repeat_count;
    uint32_t current_frame;
    float progress_percent;
    void (* callback)(void *p);
    void *p;
    bool animate;
} gui_animate_t;


typedef struct _gui_img_file_head
{
    unsigned char scan : 1;
    unsigned char align : 1;
    unsigned char rsvd : 3;
    unsigned char compress : 3;
    char type;

    union
    {
        int vector_size;
        struct
        {
            short img_w;
            short img_h;
        } img_size;
    } size;
    char version;
    char rsvd2;
} gui_img_file_head_t;

typedef struct _gui_obj_t
{
    const char *name;
    struct _gui_obj_t *parent;//point to father obj

    int16_t x;
    int16_t y;
    int16_t w;
    int16_t h;
    gui_list_t  child_list;
    gui_list_t  brother_list;

    //run time
    void (* obj_input_prepare)(struct _gui_obj_t *obj);
    void (* obj_prepare)(struct _gui_obj_t *obj);
    void (* obj_draw)(struct _gui_obj_t *obj);
    void (* obj_end)(struct _gui_obj_t *obj);
    void (* obj_destory)(struct _gui_obj_t *obj);

    //void (* obj_cb)(struct _gui_obj_t *obj); todo
    //run time
    obj_type_t type; //no need this , only use name
    uint32_t active         : 1;    // this flag means obj location in screen
    uint32_t not_show       : 1;
    uint32_t cover          : 1;
    //uint32_t focused        : 1;
    uint32_t skip_tp_left_hold    : 1;
    uint32_t skip_tp_right_hold   : 1;
    uint32_t skip_tp_up_hold      : 1;
    uint32_t skip_tp_down_hold    : 1;
    //uint32_t skip_tp_left_quick   : 1;


    uint32_t create_done    : 1;
    uint32_t flag_3d        : 1;
    uint32_t event_dsc_cnt  : 5;

    gui_event_dsc_t *event_dsc;


    // int16_t dx;//for touch
    // int16_t dy;//for touch
    // int16_t ax;//absolute value
    // int16_t ay;//absolute value
    // float sx;
    // float sy;
    // int16_t tx;//for tab, diffent view use this value
    // int16_t ty;//for tab, diffent view use this value
    unsigned char opacity_value;
    gui_matrix_t *matrix;

} gui_obj_t;

struct _gui_color
{
    union
    {
        unsigned char array[4];
        struct
        {
            unsigned char r, g, b, a;
        } rgba;
        uint32_t rgba_full;
    } color;
};
typedef struct _gui_color gui_color_t;
struct gui_rgb_data_head
{
    unsigned char scan : 1;
    unsigned char rsvd : 4;
    unsigned char compress : 3;
    char type;
    short w;
    short h;
    char version;
    char rsvd2;
};
typedef struct _gui_rect_file_head
{
    struct gui_rgb_data_head head;
    gui_color_t color;
} gui_rect_file_head_t;
/** End of SUBMOUDLE_Exported_Types
  * @}
  */

/*============================================================================*
 *                         Constants
 *============================================================================*/
/** @defgroup SUBMOUDLE_Exported_Constants SUBMOUDLE Exported Constants
  * @brief
  * @{
  */


/** End of SUBMOUDLE_Exported_Constants
  * @}
  */

/*============================================================================*
 *                         Macros
 *============================================================================*/
/** @defgroup SUBMOUDLE_Exported_Macros SUBMOUDLE Exported Macros
  * @brief
  * @{
  * The color used in the GUI:
  *
  *         bit        bit
  * RGB565  15 R,G,B   0
  * BGR565  15 B,G,R   0
  * RGB888  23 R,G,B   0
  * ARGB888 31 A,R,G,B 0
  * RGBA888 31 R,G,B,A 0
  * ABGR888 31 A,B,G,R 0
  *
  * The gui_color is defined as ARGB888.
  *        bit31 A,R,G,B bit0
  */


#define GUI_SWAP16(x)          ((uint16_t)(                         \
                                                                    (((uint16_t)(x) & (uint16_t)0x00ff) <<  8) |            \
                                                                    (((uint16_t)(x) & (uint16_t)0xff00) >>  8)))

#define GUI_SWAP32(x)          ((uint32_t)(                         \
                                                                    (((uint32_t)(x) & (uint32_t)0x000000ff) << 24) |            \
                                                                    (((uint32_t)(x) & (uint32_t)0x0000ff00) <<  8) |            \
                                                                    (((uint32_t)(x) & (uint32_t)0x00ff0000) >>  8) |            \
                                                                    (((uint32_t)(x) & (uint32_t)0xff000000) >> 24)))

#ifndef M_PI
#define M_PI    ((float)3.14159265358979323846)
#endif

#define _UI_MIN(x, y)           (((x)<(y))?(x):(y))
#define _UI_MAX(x, y)           (((x)>(y))?(x):(y))
#define _UI_BITBYTES(bits)      ((bits + 7)/8)
#define _UI_ABS(x)              ((x)>=0? (x):-(x))

#define GUI_UNUSED(x) (void)(x)             /* macro to get rid of 'unused parameter' warning */
#define GUI_TYPE(type, obj) ((type *)obj)

#if defined __WIN32


//#define GUI_ROOT_FOLDER "gui_engine\\example\\screen_480_480\\root\\"
extern char *defaultPath;
#define GUI_ROOT_FOLDER defaultPath
#else
#define GUI_ROOT_FOLDER "/"
#endif
#define GUI_NEW(type, constructor_cb, param) type *this = gui_malloc(sizeof(type));\
    memset(this, 0, sizeof(type));\
    constructor_cb(param);\
    /*todo enter critical section*/\
    gui_list_init(&(GET_BASE(this)->child_list));\
    if ((GET_BASE(this)->parent) != NULL)\
    {\
        gui_list_insert_before(&((GET_BASE(this)->parent)->child_list), &(GET_BASE(this)->brother_list));\
    }\
    GET_BASE(this)->create_done = true;\
    /*todo exit critical section*/\
    return this;

#define GET_BASE(_p) ((gui_obj_t *)_p)
#if defined _WIN32
#define GUI_FRAME_STEP 1
#else
#define GUI_FRAME_STEP 50
#endif


/** End of SUBMOUDLE_Exported_Macros
  * @}
  */

/*============================================================================*
 *                         Variables
 *============================================================================*/
/** @defgroup SUBMOUDLE_Exported_Variables SUBMOUDLE Exported Variables
  * @brief
  * @{
  */


/** End of SUBMOUDLE_Exported_Variables
  * @}
  */

/*============================================================================*
 *                         Functions
 *============================================================================*/
/** @defgroup SUBMOUDLE_Exported_Functions SUBMOUDLE Exported Functions
  * @brief
  * @{
  */


/**
 * @brief
 *
 * @param parent
 * @param name
 * @param addr
 * @param x
 * @param y
 * @param w
 * @param h
 * @return gui_widget_template_t*
 */


/** End of SUBMOUDLE_Exported_Functions
  * @}
  */

/** End of SUBMOUDLE
  * @}
  */


#ifdef __cplusplus
}
#endif

#endif


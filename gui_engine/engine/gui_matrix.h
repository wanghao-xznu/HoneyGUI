/*
 * File      : gui_win.h
 */
#ifndef __GUI_MATRIX_H__
#define __GUI_MATRIX_H__

#include <guidef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct VertexRec
{
    float x;                    //!< x axis
    float y;                    //!< y axis
    float z;                    //!< z axis
} Vertex_t;

typedef struct NormalRec
{
    float x;                    //!< x axis
    float y;                    //!< y axis
    float z;                    //!< z axis
} Normal_t;

typedef struct Point2f
{
    float x;
    float y;
} Point2f_t;


void matrix_transfrom_blit(float w, float h, Vertex_t *p, Vertex_t *v0, Vertex_t *v1, Vertex_t *v2,
                           Vertex_t *v3, struct gui_matrix *matrix);
void matrix_compute_rotate(float rx, float ry, float rz, struct gui_matrix *rotate);
void matrix_transfrom_rotate(struct gui_matrix *rotate, Vertex_t *vertex, Vertex_t *rc, float tx,
                             float ty, float tz);

void matrix_multiply(struct gui_matrix *matrix, struct gui_matrix *mult);
void matrix_identity(struct gui_matrix *matrix);
bool matrix_is_identity(struct gui_matrix *matrix);
void matrix_translate(float x, float y, struct gui_matrix *matrix);
void matrix_rotate(float degrees, struct gui_matrix *matrix);
void matrix_inverse(struct gui_matrix *matrix);
void matrix_scale(float scale_x, float scale_y, struct gui_matrix *matrix);
void matrix_perspective(float px, float py, struct gui_matrix *matrix);
void matrix_multiply_point(struct gui_matrix *matrix, struct gui_point *pox);
void matrix_multiply_normal(struct gui_matrix *matrix, Normal_t *normal);

float fix_cos(int angle);
float fix_sin(int angle);


#ifdef __cplusplus
}
#endif

#endif


/*
 * File      : gui_img.c
 * This file is part of GUI Engine
 */

#include <guidef.h>
#include <string.h>
#include <gui_matrix.h>
#include <math.h>

#define USE_FIX_SIN 1

#if USE_FIX_SIN
// Set the intial cube rotation degree and step.
static const int16_t sin_table[] =
{
    0,     572,   1144,  1715,  2286,  2856,  3425,  3993,  4560,  5126,  5690,  6252,  6813,  7371,  7927,  8481,
    9032,  9580,  10126, 10668, 11207, 11743, 12275, 12803, 13328, 13848, 14364, 14876, 15383, 15886, 16383, 16876,
    17364, 17846, 18323, 18794, 19260, 19720, 20173, 20621, 21062, 21497, 21925, 22347, 22762, 23170, 23571, 23964,
    24351, 24730, 25101, 25465, 25821, 26169, 26509, 26841, 27165, 27481, 27788, 28087, 28377, 28659, 28932, 29196,
    29451, 29697, 29934, 30162, 30381, 30591, 30791, 30982, 31163, 31335, 31498, 31650, 31794, 31927, 32051, 32165,
    32269, 32364, 32448, 32523, 32587, 32642, 32687, 32722, 32747, 32762, 32767
};

float fix_sin(int angle)
{
    int16_t ret = 0;
    angle       = angle % 360;

    if (angle < 0) { angle = 360 + angle; }

    if (angle < 90)
    {
        ret = sin_table[angle];
    }
    else if (angle >= 90 && angle < 180)
    {
        angle = 180 - angle;
        ret   = sin_table[angle];
    }
    else if (angle >= 180 && angle < 270)
    {
        angle = angle - 180;
        ret   = -sin_table[angle];
    }
    else     /*angle >=270*/
    {
        angle = 360 - angle;
        ret   = -sin_table[angle];
    }

    return ret / 32767.0f;
}

float fix_cos(int angle)
{
    return fix_sin(angle + 90);
}
#endif

/* Calculates coefficients of perspective transformation
* which maps (xi,yi) to (ui,vi), (i=1,2,3,4):
*
*      c00*xi + c01*yi + c02
* ui = ---------------------
*      c20*xi + c21*yi + c22
*
*      c10*xi + c11*yi + c12
* vi = ---------------------
*      c20*xi + c21*yi + c22
*
* Coefficients are calculated by solving linear system:
* / x0 y0  1  0  0  0 -x0*u0 -y0*u0 \ /c00\ /u0\
* | x1 y1  1  0  0  0 -x1*u1 -y1*u1 | |c01| |u1|
* | x2 y2  1  0  0  0 -x2*u2 -y2*u2 | |c02| |u2|
* | x3 y3  1  0  0  0 -x3*u3 -y3*u3 |.|c10|=|u3|,
* |  0  0  0 x0 y0  1 -x0*v0 -y0*v0 | |c11| |v0|
* |  0  0  0 x1 y1  1 -x1*v1 -y1*v1 | |c12| |v1|
* |  0  0  0 x2 y2  1 -x2*v2 -y2*v2 | |c20| |v2|
* \  0  0  0 x3 y3  1 -x3*v3 -y3*v3 / \c21/ \v3/
*
* where:
*   cij - matrix coefficients, c22 = 1
*/
static void swap(float *a, float *b)
{
    float c = *a;
    *a = *b;
    *b = c;
}
static bool Gauss(float A[][9], int equ, int var, float *answer)   //epu:A's row  var:A's col-1
{
    int row, col;
    for (row = 0, col = 0; col < var && row < equ; col++, row++)
    {
        int max_r = row;
        for (int i = row + 1; i < equ; i++)
        {
            if (fabs(A[i][col]) > fabs(A[max_r][col]))
            {
                max_r = i;
            }
        }
        if (max_r != row)
        {
            for (int j = 0; j < var + 1; j++)
            {
                swap(&A[row][j], &A[max_r][j]);
            }
        }

        if (fabs(A[row][col]) < (1e-6))
        {
            //row--;
            //continue;
            return false;
        }

        for (int i = row + 1; i < equ; i++)
        {

            if (fabs(A[i][col]) < (1e-6))
            {
                continue;
            }
            float tmp = -A[i][col] / A[row][col];
            for (int j = col; j < var + 1; j++)
            {
                A[i][j] += tmp * A[row][j];
            }
        }
    }


    for (int i = var - 1; i >= 0; i--)
    {
        float tmp = 0;
        for (int j = i + 1; j < var; j++)
        {
            tmp += A[i][j] * (*(answer + j));
        }
        answer[i] = (A[i][var] - tmp) / A[i][i];
    }
    return true;
}

static bool getPerspectiveTransform(Point2f_t *src, Point2f_t *dst, float *ret)
{
    float x0 = src[0].x, x1 = src[1].x, x2 = src[3].x, x3 = src[2].x;
    float y0 = src[0].y, y1 = src[1].y, y2 = src[3].y, y3 = src[2].y;
    float u0 = dst[0].x, u1 = dst[1].x, u2 = dst[3].x, u3 = dst[2].x;
    float v0 = dst[0].y, v1 = dst[1].y, v2 = dst[3].y, v3 = dst[2].y;
    float A[8][9] =
    {
        { x0, y0, 1, 0, 0, 0, -x0 * u0, -y0 * u0, u0 },
        { x1, y1, 1, 0, 0, 0, -x1 * u1, -y1 * u1, u1 },
        { x2, y2, 1, 0, 0, 0, -x2 * u2, -y2 * u2, u2 },
        { x3, y3, 1, 0, 0, 0, -x3 * u3, -y3 * u3, u3 },
        { 0, 0, 0, x0, y0, 1, -x0 * v0, -y0 * v0, v0 },
        { 0, 0, 0, x1, y1, 1, -x1 * v1, -y1 * v1, v1 },
        { 0, 0, 0, x2, y2, 1, -x2 * v2, -y2 * v2, v2 },
        { 0, 0, 0, x3, y3, 1, -x3 * v3, -y3 * v3, v3 },
    };
    if (Gauss(A, 8, 8, ret) == true)
    {
        *(ret + 8) = 1;
        return true;
    }
    return false;

}

// w means image width
// h means image height, use w and h can get four point in XY plane

// p means the perspective point,  use this parameter can get four point in XY plane
void matrix_transfrom_blit(float w, float h, Vertex_t *p, Vertex_t *v0, Vertex_t *v1, Vertex_t *v2,
                           Vertex_t *v3, struct gui_matrix *matrix)
{
    // Compute 3x3 image transform matrix to map a rectangle image (w,h) to
    // a parallelogram (x0,y0), (x1,y1), (x2,y2), (x3,y3) counterclock wise.

    float x0 = p->x - (p->x - v0->x) * p->z / (p->z - v0->z);
    float y0 = p->y - (p->y - v0->y) * p->z / (p->z - v0->z);
    float x1 = p->x - (p->x - v1->x) * p->z / (p->z - v1->z);
    float y1 = p->y - (p->y - v1->y) * p->z / (p->z - v1->z);
    float x2 = p->x - (p->x - v2->x) * p->z / (p->z - v2->z);
    float y2 = p->y - (p->y - v2->y) * p->z / (p->z - v2->z);
    float x3 = p->x - (p->x - v3->x) * p->z / (p->z - v3->z);
    float y3 = p->y - (p->y - v3->y) * p->z / (p->z - v3->z);


    Point2f_t src[4] = {{0, 0}, {w, 0}, {w, h}, {0, h}};
    Point2f_t dst[4] = {{x0, y0}, {x1, y1}, {x2, y2}, {x3, y3}};
    v0->x = x0;
    v1->x = x1;
    v2->x = x2;
    v3->x = x3;
    v0->y = y0;
    v1->y = y1;
    v2->y = y2;
    v3->y = y3;
    float *answer = (float *)matrix->m;
    getPerspectiveTransform(src, dst, answer);
}

bool matrix_is_identity(struct gui_matrix *matrix)
{
    if (matrix == NULL)
    {
        return false;
    }

    if (
        matrix->m[0][0] == 1.0f && \
        matrix->m[0][1] == 0.0f && \
        matrix->m[0][2] == 0.0f && \
        matrix->m[1][0] == 0.0f && \
        matrix->m[1][1] == 1.0f && \
        matrix->m[1][2] == 0.0f && \
        matrix->m[2][0] == 0.0f && \
        matrix->m[2][1] == 0.0f && \
        matrix->m[2][2] == 1.0f
    )
    {
        return true;
    }
    else
    {
        return false;
    }
}
void matrix_identity(struct gui_matrix *matrix)
{
    if (matrix == NULL)
    {
        return;
    }
    /* Set identify matrix. */
    matrix->m[0][0] = 1.0f;
    matrix->m[0][1] = 0.0f;
    matrix->m[0][2] = 0.0f;
    matrix->m[1][0] = 0.0f;
    matrix->m[1][1] = 1.0f;
    matrix->m[1][2] = 0.0f;
    matrix->m[2][0] = 0.0f;
    matrix->m[2][1] = 0.0f;
    matrix->m[2][2] = 1.0f;
}

void matrix_multiply_normal(struct gui_matrix *matrix, Normal_t *normal)
{
    Normal_t temp;

    /* Process all rows. */

    temp.x = (matrix->m[0][0] * normal->x) + (matrix->m[0][1] * normal->y) +
             (matrix->m[0][2] * normal->z);
    temp.y = (matrix->m[1][0] * normal->x) + (matrix->m[1][1] * normal->y) +
             (matrix->m[1][2] * normal->z);
    temp.z = (matrix->m[2][0] * normal->x) + (matrix->m[2][1] * normal->y) +
             (matrix->m[2][2] * normal->z);

    memcpy(normal, &temp, sizeof(temp));
}

void matrix_multiply(struct gui_matrix *matrix, struct gui_matrix *mult)
{
    struct gui_matrix temp;
    int row, column;

    /* Process all rows. */
    for (row = 0; row < 3; row++)
    {
        /* Process all columns. */
        for (column = 0; column < 3; column++)
        {
            /* Compute matrix entry. */
            temp.m[row][column] = (matrix->m[row][0] * mult->m[0][column])
                                  + (matrix->m[row][1] * mult->m[1][column])
                                  + (matrix->m[row][2] * mult->m[2][column]);
        }
    }

    /* Copy temporary matrix into result. */
    memcpy(matrix, &temp, sizeof(temp));
}


void matrix_multiply_point(struct gui_matrix *matrix, struct gui_point *pox)
{
    struct gui_point temp;
    int row;

    /* Process all rows. */
    for (row = 0; row < 3; row++)
    {
        temp.p[row] = (matrix->m[row][0] * pox->p[0])
                      + (matrix->m[row][1] * pox->p[1])
                      + (matrix->m[row][2] * pox->p[2]);
    }
    temp.p[0] = temp.p[0] / temp.p[2];
    temp.p[1] = temp.p[1] / temp.p[2];
    temp.p[2] = 1;

    memcpy(pox, &temp, sizeof(temp));
}

void matrix_translate(float x, float y, struct gui_matrix *matrix)
{
    /* Set translation matrix. */
    struct gui_matrix t = { { {1.0f, 0.0f, x},
            {0.0f, 1.0f, y},
            {0.0f, 0.0f, 1.0f}
        }
    };

    /* Multiply with current matrix. */
    matrix_multiply(matrix, &t);
}

void matrix_rotate(float degrees, struct gui_matrix *matrix)
{
    /* Convert degrees into radians. */
    float angle = degrees / 180.0f * 3.1415926f;

    /* Compuet cosine and sine values. */
    float cos_angle = cosf(angle);
    float sin_angle = sinf(angle);

    /* Set rotation matrix. */
    struct gui_matrix r = { { {cos_angle, -sin_angle, 0.0f},
            {sin_angle, cos_angle, 0.0f},
            {0.0f, 0.0f, 1.0f}
        }
    };

    /* Multiply with current matrix. */
    matrix_multiply(matrix, &r);
}

void matrix_inverse(struct gui_matrix *matrix)
{
    struct gui_matrix temp;

    float detal = matrix->m[0][0] * matrix->m[1][1] * matrix->m[2][2] + \
                  matrix->m[0][1] * matrix->m[1][2] * matrix->m[2][0] + \
                  matrix->m[0][2] * matrix->m[1][0] * matrix->m[2][1] - \
                  matrix->m[0][0] * matrix->m[1][2] * matrix->m[2][1] - \
                  matrix->m[0][1] * matrix->m[1][0] * matrix->m[2][2] - \
                  matrix->m[0][2] * matrix->m[1][1] * matrix->m[2][0];

    temp.m[0][0] = (matrix->m[1][1] * matrix->m[2][2] - matrix->m[1][2] * matrix->m[2][1]) / detal;
    temp.m[1][0] = -(matrix->m[2][2] * matrix->m[1][0] - matrix->m[1][2] * matrix->m[2][0]) / detal;
    temp.m[2][0] = (matrix->m[1][0] * matrix->m[2][1] - matrix->m[2][0] * matrix->m[1][1]) / detal;

    temp.m[0][1] = -(matrix->m[0][1] * matrix->m[2][2] - matrix->m[0][2] * matrix->m[2][1]) / detal;
    temp.m[1][1] = (matrix->m[2][2] * matrix->m[0][0] - matrix->m[2][0] * matrix->m[0][2]) / detal;
    temp.m[2][1] = -(matrix->m[0][0] * matrix->m[2][1] - matrix->m[2][0] * matrix->m[0][1]) / detal;

    temp.m[0][2] = (matrix->m[0][1] * matrix->m[1][2] - matrix->m[1][1] * matrix->m[0][2]) / detal;
    temp.m[1][2] = -(matrix->m[0][0] * matrix->m[1][2] - matrix->m[0][2] * matrix->m[1][0]) / detal;
    temp.m[2][2] = (matrix->m[0][0] * matrix->m[1][1] - matrix->m[0][1] * matrix->m[1][0]) / detal;


    /* Copy temporary matrix into result. */
    memcpy(matrix, &temp, sizeof(temp));
}

void matrix_scale(float scale_x, float scale_y, struct gui_matrix *matrix)
{
    /* Set scale matrix. */
    struct gui_matrix s = { { {scale_x, 0.0f, 0.0f},
            {0.0f, scale_y, 0.0f},
            {0.0f, 0.0f, 1.0f}
        }
    };

    /* Multiply with current matrix. */
    matrix_multiply(matrix, &s);
}

void matrix_perspective(float px, float py, struct gui_matrix *matrix)
{
    /* set prespective matrix */
    struct gui_matrix p = { { {1.0f, 0.0f, 0.0f},
            {0.0f, 1.0f, 0.0f},
            {px, py, 1.0f}
        }
    };
    /* Multiply with current matrix. */
    matrix_multiply(matrix, &p);
}

void matrix_compute_rotate(float rx, float ry, float rz, struct gui_matrix *rotate)
{
    // Rotation angles rx, ry, rz (degree) for axis X, Y, Z
    // Compute 3D rotation matrix base on rotation angle rx, ry, rz about axis X, Y, Z.
    //
#if USE_FIX_SIN
    int frz = round(rz), frx = round(rx), fry = round(ry);
    rotate->m[0][0] = fix_cos((int)(frz)) * fix_cos((int)(fry));
    rotate->m[0][1] = fix_cos((int)(frz)) * fix_sin((int)(fry)) * fix_sin((int)(frx)) - fix_sin((int)(
                          frz)) * fix_cos((int)(frx));
    rotate->m[0][2] = fix_cos((int)(frz)) * fix_sin((int)(fry)) * fix_cos((int)(frx)) + fix_sin((int)(
                          frz)) * fix_sin((int)(frx));
    rotate->m[1][0] = fix_sin((int)(frz)) * fix_cos((int)(fry));
    rotate->m[1][1] = fix_sin((int)(frz)) * fix_sin((int)(fry)) * fix_sin((int)(frx)) + fix_cos((int)(
                          frz)) * fix_cos((int)(frx));
    rotate->m[1][2] = fix_sin((int)(frz)) * fix_sin((int)(fry)) * fix_cos((int)(frx)) - fix_cos((int)(
                          frz)) * fix_sin((int)(frx));
    rotate->m[2][0] = -fix_sin((int)(fry));
    rotate->m[2][1] = fix_cos((int)(fry)) * fix_sin((int)(frx));
    rotate->m[2][2] = fix_cos((int)(fry)) * fix_cos((int)(frx));
#else
    rotate->m[0][0] = cos(RAD(rz)) * cos(RAD(ry));
    rotate->m[0][1] = cos(RAD(rz)) * sin(RAD(ry)) * sin(RAD(rx)) - sin(RAD(rz)) * cos(RAD(rx));
    rotate->m[0][2] = cos(RAD(rz)) * sin(RAD(ry)) * cos(RAD(rx)) + sin(RAD(rz)) * sin(RAD(rx));
    rotate->m[1][0] = sin(RAD(rz)) * cos(RAD(ry));
    rotate->m[1][1] = sin(RAD(rz)) * sin(RAD(ry)) * sin(RAD(rx)) + cos(RAD(rz)) * cos(RAD(rx));
    rotate->m[1][2] = sin(RAD(rz)) * sin(RAD(ry)) * cos(RAD(rx)) - cos(RAD(rz)) * sin(RAD(rx));
    rotate->m[2][0] = -sin(RAD(ry));
    rotate->m[2][1] = cos(RAD(ry)) * sin(RAD(rx));
    rotate->m[2][2] = cos(RAD(ry)) * cos(RAD(rx));
#endif
}

void matrix_transfrom_rotate(struct gui_matrix *rotate, Vertex_t *vertex, Vertex_t *rc, float tx,
                             float ty, float tz)
{
    // Compute the new cube vertex coordinates transformed by the rotation matrix.
    //
    rc->x = rotate->m[0][0] * vertex->x + rotate->m[0][1] * vertex->y + rotate->m[0][2] * vertex->z;
    rc->y = rotate->m[1][0] * vertex->x + rotate->m[1][1] * vertex->y + rotate->m[1][2] * vertex->z;
    rc->z = rotate->m[2][0] * vertex->x + rotate->m[2][1] * vertex->y + rotate->m[2][2] * vertex->z;

    // Translate the vertex in XYZ plane.
    //
    rc->x += tx;
    rc->y += ty;
    rc->z += tz;
}

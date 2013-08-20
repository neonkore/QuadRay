/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"
#include "rtgeom.h"
#include "system.h"

/******************************************************************************/
/*********************************   LOGGING   ********************************/
/******************************************************************************/

#define PRINT_STATE_BEG()                                                   \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("************** print state beg **************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

#define PRINT_TIME(time)                                                    \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("---------- update time -- %08u ----------", (rt_word)time);\
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

static
rt_void print_cam(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    RT_LOGI("%s", mgn);
    RT_LOGI("cam: %08X, ", (rt_word)obj);
    RT_LOGI("CAM: %08X, ", (rt_word)RT_NULL);
    RT_LOGI("elm: %08X, ", (rt_word)elm);
    if (obj != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("rot: {%f, %f, %f}",
            obj->trm->rot[RT_X], obj->trm->rot[RT_Y], obj->trm->rot[RT_Z]);
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z]);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("empty object");
    }
    RT_LOGI("\n");
}

#define PRINT_CAM(cam)                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("******************* CAMERA ******************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_cam("    ", RT_NULL, cam);                                    \
        RT_LOGI("\n")

static
rt_void print_lgt(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    rt_SIMD_LIGHT *s_lgt = elm != RT_NULL ? (rt_SIMD_LIGHT *)elm->simd :
                           obj != RT_NULL ?   ((rt_Light *)obj)->s_lgt :
                                                                RT_NULL;
    RT_LOGI("%s", mgn);
    RT_LOGI("lgt: %08X, ", (rt_word)obj);
    RT_LOGI("LGT: %08X, ", (rt_word)s_lgt);
    RT_LOGI("elm: %08X, ", (rt_word)elm);
    if (s_lgt != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("                                       ");
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            s_lgt->pos_x[0], s_lgt->pos_y[0], s_lgt->pos_z[0]);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("empty object");
    }
    RT_LOGI("\n");
}

#define PRINT_LGT(elm, lgt)                                                 \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define PRINT_LGT_INNER(elm, lgt)                                           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define PRINT_LGT_OUTER(elm, lgt)                                           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

static
rt_void print_srf(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    rt_SIMD_SURFACE *s_srf = elm != RT_NULL ? (rt_SIMD_SURFACE *)elm->simd :
                             obj != RT_NULL ?   ((rt_Surface *)obj)->s_srf :
                                                                    RT_NULL;
    RT_LOGI("%s", mgn);
    RT_LOGI("srf: %08X, ", (rt_word)obj);
    RT_LOGI("SRF: %08X, ", (rt_word)s_srf);
    RT_LOGI("elm: %08X, ", (rt_word)elm);
    if (s_srf != RT_NULL && obj != RT_NULL)
    {
        RT_LOGI("    ");
        RT_LOGI("tag: %d, trm: %d,                        ",
            obj->tag, s_srf->a_map[3]);
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z]);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("empty object, data = %d", elm != RT_NULL ? elm->data : 0);
    }
    RT_LOGI("\n");
}

#define PRINT_SRF(srf)                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("****************** SURFACE ******************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_srf("    ", RT_NULL, srf);                                    \
        RT_LOGI("\n")

static
rt_void print_lst(rt_pstr mgn, rt_ELEM *elm)
{
    for (; elm != RT_NULL; elm = elm->next)
    {
        rt_Object *obj = (rt_Object *)elm->temp;

        if (obj != RT_NULL && RT_IS_LIGHT(obj))
        {
            print_lgt(mgn, elm, obj);
        }
        else
        {
            print_srf(mgn, elm, obj);
        }
    }
}

#define PRINT_CLP(lst)                                                      \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- clp --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_LST(lst)                                                      \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_LST_INNER(lst)                                                \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_LST_OUTER(lst)                                                \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_SHW(lst)                                                      \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_SHW_INNER(lst)                                                \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_SHW_OUTER(lst)                                                \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_LGT_LIST(lst)                                                 \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_SRF_LIST(lst)                                                 \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- srf --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_TLS(lst, i, j)                                                \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("******** screen tiles[%02d][%02d] list: *********", i, j); \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define PRINT_STATE_END()                                                   \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("************** print state end **************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n")

/******************************************************************************/
/*********************************   THREAD   *********************************/
/******************************************************************************/

rt_SceneThread::rt_SceneThread(rt_Scene *scene, rt_cell index) :

    rt_Heap(scene->f_alloc, scene->f_free)
{
    this->scene = scene;
    this->index = index;

    /* allocate root SIMD structure */

    s_inf = (rt_SIMD_INFOX *)
            alloc(sizeof(rt_SIMD_INFOX),
                            RT_SIMD_ALIGN);

    memset(s_inf, 0, sizeof(rt_SIMD_INFOX));

    RT_SIMD_SET(s_inf->gpc01, +1.0);
    RT_SIMD_SET(s_inf->gpc02, -0.5);
    RT_SIMD_SET(s_inf->gpc03, +3.0);

    /* init framebuffer's dimensions and pointer */

    s_inf->frm_w   = scene->x_res;
    s_inf->frm_h   = scene->y_res;
    s_inf->frm_row = scene->x_row;
    s_inf->frame   = scene->frame;

    /* init tilebuffer's dimensions and pointer */

    s_inf->tile_w  = scene->tile_w;
    s_inf->tile_h  = scene->tile_h;
    s_inf->tls_row = scene->tiles_in_row;
    s_inf->tiles   = scene->tiles;

    /* allocate cam SIMD structure */

    s_cam = (rt_SIMD_CAMERA *)
            alloc(sizeof(rt_SIMD_CAMERA),
                            RT_SIMD_ALIGN);

    /* allocate ctx SIMD structure */

    s_ctx = (rt_SIMD_CONTEXT *)
            alloc(sizeof(rt_SIMD_CONTEXT) + /* +1 context step for shadows */
                            RT_STACK_STEP * (1 + scene->depth),
                            RT_SIMD_ALIGN);

    txmin = (rt_cell *)alloc(sizeof(rt_cell) * scene->tiles_in_col, RT_ALIGN);
    txmax = (rt_cell *)alloc(sizeof(rt_cell) * scene->tiles_in_col, RT_ALIGN);
    verts = (rt_VERT *)alloc(sizeof(rt_VERT) * RT_VERTS_LIMIT, RT_ALIGN);
}

#define RT_UPDATE_TILES_BOUNDS(cy, x1, x2)                              \
do                                                                      \
{                                                                       \
    if (x1 < x2)                                                        \
    {                                                                   \
        if (txmin[cy] > x1)                                             \
            txmin[cy] = RT_MAX(x1, xmin);                               \
                                                                        \
        if (txmax[cy] < x2)                                             \
            txmax[cy] = RT_MIN(x2, xmax);                               \
    }                                                                   \
    else                                                                \
    {                                                                   \
        if (txmin[cy] > x2)                                             \
            txmin[cy] = RT_MAX(x2, xmin);                               \
                                                                        \
        if (txmax[cy] < x1)                                             \
            txmax[cy] = RT_MIN(x1, xmax);                               \
    }                                                                   \
}                                                                       \
while (0)

/*
 * Update surface's projected bbox boundaries in tilebuffer
 * by processing one bbox edge at a time.
 * The tile buffer is reset for every surface from outside of this function.
 */
rt_void rt_SceneThread::tiling(rt_vec4 p1, rt_vec4 p2)
{
    rt_real *pt, n1[3][2], n2[3][2];
    rt_real dx, dy, xx, yy, rt, px;
    rt_cell x1, y1, x2, y2, i, n, t;

    /* swap points vertically if needed */

    if (p1[RT_Y] > p2[RT_Y])
    {
        pt = p1;
        p1 = p2;
        p2 = pt;
    }

    /* initialize delta variables */

    dx = p2[RT_X] - p1[RT_X];
    dy = p2[RT_Y] - p1[RT_Y];

    /* prepare new lines with margins */

    if ((RT_FABS(dx) <= RT_LINE_THRESHOLD)
    &&  (RT_FABS(dy) <= RT_LINE_THRESHOLD))
    {
        rt = 0.0f;
        xx = dx < 0.0f ? -1.0f : 1.0f;
        yy = 1.0f;
    }
    else
    if ((RT_FABS(dx) <= RT_LINE_THRESHOLD)
    ||  (RT_FABS(dy) <= RT_LINE_THRESHOLD))
    {
        rt = 0.0f;
        xx = dx;
        yy = dy;
    }
    else
    {
        rt = dx / dy;
        xx = dx;
        yy = dy;
    }

#if RT_TILING_EXT

    px  = RT_TILE_THRESHOLD / RT_SQRT(xx * xx + yy * yy);
    xx *= px;
    yy *= px;

    n1[0][RT_X] = p1[RT_X] - xx;
    n1[0][RT_Y] = p1[RT_Y] - yy;
    n2[0][RT_X] = p2[RT_X] + xx;
    n2[0][RT_Y] = p2[RT_Y] + yy;

    n1[1][RT_X] = n1[0][RT_X] - yy;
    n1[1][RT_Y] = n1[0][RT_Y] + xx;
    n2[1][RT_X] = n2[0][RT_X] - yy;
    n2[1][RT_Y] = n2[0][RT_Y] + xx;

    n1[2][RT_X] = n1[0][RT_X] + yy;
    n1[2][RT_Y] = n1[0][RT_Y] - xx;
    n2[2][RT_X] = n2[0][RT_X] + yy;
    n2[2][RT_Y] = n2[0][RT_Y] - xx;

    n = 3;

#else /* RT_TILING_EXT */

    n1[0][RT_X] = p1[RT_X];
    n1[0][RT_Y] = p1[RT_Y];
    n2[0][RT_X] = p2[RT_X];
    n2[0][RT_Y] = p2[RT_Y];

    n = 1;

#endif /* RT_TILING_EXT */

    /* set inclusive bounds */

    rt_cell xmin = 0;
    rt_cell ymin = 0;
    rt_cell xmax = scene->tiles_in_row - 1;
    rt_cell ymax = scene->tiles_in_col - 1;

    for (i = 0; i < n; i++)
    {
        /* calculate points floor */

        x1 = (rt_cell)RT_FLOOR(n1[i][RT_X]);
        y1 = (rt_cell)RT_FLOOR(n1[i][RT_Y]);
        x2 = (rt_cell)RT_FLOOR(n2[i][RT_X]);
        y2 = (rt_cell)RT_FLOOR(n2[i][RT_Y]);

        /* reject y-outer lines */

        if (y1 > ymax || y2 < ymin)
        {
            continue;
        }

        /* process nearly vertical, nearly horizontal or x-outer line */

        if ((x1 == x2  || y1 == y2 || rt == 0.0f)
        ||  (x1 < xmin && x2 < xmin)
        ||  (x1 > xmax && x2 > xmax))
        {
            if (y1 < ymin)
                y1 = ymin;
            if (y2 > ymax)
                y2 = ymax;

            for (t = y1; t <= y2; t++)
            {
                RT_UPDATE_TILES_BOUNDS(t, x1, x2);
            }

            continue;
        }

        /* process regular line */

        y1 = y1 < ymin ? ymin : y1 + 1;
        y2 = y2 > ymax ? ymax : y2 - 1;

        px = n1[i][RT_X] + (y1 - n1[i][RT_Y]) * rt;
        x2 = (rt_cell)RT_FLOOR(px);

        if (y1 > ymin)
        {
            RT_UPDATE_TILES_BOUNDS(y1 - 1, x1, x2);
        }

        x1 = x2;

        for (t = y1; t <= y2; t++)
        {
            px = px + rt;
            x2 = (rt_cell)RT_FLOOR(px);
            RT_UPDATE_TILES_BOUNDS(t, x1, x2);
            x1 = x2;
        }

        if (y2 < ymax)
        {
            x2 = (rt_cell)RT_FLOOR(n2[i][RT_X]);
            RT_UPDATE_TILES_BOUNDS(y2 + 1, x1, x2);
        }
    }
}

/*
 * Insert new element derived from "srf" to a list "ptr"
 * for a given object "obj". If "srf" is NULL and "obj" is LIGHT,
 * insert new element derived from "obj" to a list "ptr".
 */
rt_void rt_SceneThread::insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf)
{
    rt_ELEM *elm = RT_NULL;

    if (srf != RT_NULL)
    {
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        elm->data = 0;
        elm->simd = srf->s_srf;
        elm->temp = srf;
        elm->next = *ptr;
       *ptr = elm;
    }
    else
    if (RT_IS_LIGHT(obj))
    {
        rt_Light *lgt = (rt_Light *)obj;

        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        elm->data = (rt_cell)scene->slist; /* all srf are potential shadows */
        elm->simd = lgt->s_lgt;
        elm->temp = lgt;
        elm->next = *ptr;
       *ptr = elm;
    }
}

/*
 * Build tile list for a given "srf" based
 * on the area its projected bbox occupies in the tilebuffer.
 */
rt_void rt_SceneThread::stile(rt_Surface *srf)
{
    rt_ELEM *lst = RT_NULL;
    rt_ELEM *elm = RT_NULL;
    rt_cell i, j;
#if RT_TILING_OPT
    rt_cell verts_num;
    rt_cell k;

    rt_VERT *svr;
    rt_vec4 vec;
    rt_real dot;
    rt_cell ndx[2];
    rt_real tag[2], zed[2];

    verts_num = srf->verts_num;
    svr = srf->verts;

    /* project bbox onto the tilebuffer */

    if (verts_num != 0)
    {
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = scene->tiles_in_row;
            txmax[i] = -1;
        }

        /* process bbox vertices */

        memset(verts, 0, sizeof(rt_VERT) * verts_num);

        for (k = 0; k < srf->verts_num; k++)
        {
            vec[RT_X] = svr[k].pos[RT_X] - scene->org[RT_X];
            vec[RT_Y] = svr[k].pos[RT_Y] - scene->org[RT_Y];
            vec[RT_Z] = svr[k].pos[RT_Z] - scene->org[RT_Z];

            dot = RT_VECTOR_DOT(vec, scene->nrm);

            verts[k].pos[RT_Z] = dot;
            verts[k].pos[3] = -1.0f;

            if (dot >= 0.0f || RT_FABS(dot) <= RT_CLIP_THRESHOLD)
            {
                vec[RT_X] = svr[k].pos[RT_X] - scene->pos[RT_X];
                vec[RT_Y] = svr[k].pos[RT_Y] - scene->pos[RT_Y];
                vec[RT_Z] = svr[k].pos[RT_Z] - scene->pos[RT_Z];

                dot = RT_VECTOR_DOT(vec, scene->nrm) / scene->cam->pov;

                vec[RT_X] /= dot;
                vec[RT_Y] /= dot;
                vec[RT_Z] /= dot;

                vec[RT_X] -= scene->dir[RT_X];
                vec[RT_Y] -= scene->dir[RT_Y];
                vec[RT_Z] -= scene->dir[RT_Z];

                verts[k].pos[RT_X] = RT_VECTOR_DOT(vec, scene->htl);
                verts[k].pos[RT_Y] = RT_VECTOR_DOT(vec, scene->vtl);

                verts[k].pos[3] = +1.0f;

                if (verts[k].pos[RT_Z] < 0.0f)
                {
                    verts[verts_num].pos[RT_X] = verts[k].pos[RT_X];
                    verts[verts_num].pos[RT_Y] = verts[k].pos[RT_Y];
                    verts_num++;

                    verts[k].pos[3] = 0.0f;
                }
            }
        }

        /* process bbox edges */

        for (k = 0; k < srf->edges_num; k++)
        {
            for (i = 0; i < 2; i++)
            {
                ndx[i] = srf->edges[k].index[i];
                zed[i] = verts[ndx[i]].pos[RT_Z];
                tag[i] = verts[ndx[i]].pos[3];
            }

            if (tag[0] <= 0.0f && tag[1] <= 0.0f)
            {
                continue;
            }

            for (i = 0; i < 2; i++)
            {
                if (tag[i] >= 0.0f)
                {
                    continue;
                }

                j = 1 - i;

                vec[RT_X] = svr[ndx[i]].pos[RT_X] - svr[ndx[j]].pos[RT_X];
                vec[RT_Y] = svr[ndx[i]].pos[RT_Y] - svr[ndx[j]].pos[RT_Y];
                vec[RT_Z] = svr[ndx[i]].pos[RT_Z] - svr[ndx[j]].pos[RT_Z];

                dot = zed[j] / (zed[j] - zed[i]);

                vec[RT_X] *= dot;
                vec[RT_Y] *= dot;
                vec[RT_Z] *= dot;

                vec[RT_X] += svr[ndx[j]].pos[RT_X] - scene->org[RT_X];
                vec[RT_Y] += svr[ndx[j]].pos[RT_Y] - scene->org[RT_Y];
                vec[RT_Z] += svr[ndx[j]].pos[RT_Z] - scene->org[RT_Z];

                verts[verts_num].pos[RT_X] = RT_VECTOR_DOT(vec, scene->htl);
                verts[verts_num].pos[RT_Y] = RT_VECTOR_DOT(vec, scene->vtl);

                ndx[i] = verts_num;
                verts_num++;
            }

            tiling(verts[ndx[0]].pos, verts[ndx[1]].pos); 
        }

        for (i = srf->verts_num; i < verts_num - 1; i++)
        {
            for (j = i + 1; j < verts_num; j++)
            {
                tiling(verts[i].pos, verts[j].pos); 
            }
        }
    }
    else
#endif /* RT_TILING_OPT */
    {
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = 0;
            txmax[i] = scene->tiles_in_row - 1;
        }
    }

    /* fill marked tiles with surface data */

    rt_ELEM **ptr = &lst;

    for (i = 0; i < scene->tiles_in_col; i++)
    {
        for (j = txmin[i]; j <= txmax[i]; j++)
        {
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = (i << 16 | j);
            elm->simd = srf->s_srf;
            elm->temp = srf;
           *ptr = elm;
            ptr = (rt_ELEM **)&(elm->next);
        }
    }

   *ptr = RT_NULL;

    srf->s_srf->msc_p[0] = lst;
}

/*
 * Build surface lists for a given "obj".
 * Surfaces have separate surface lists for each side.
 */
rt_ELEM* rt_SceneThread::ssort(rt_Object *obj)
{
    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (RT_IS_SURFACE(obj))
    {
        srf = (rt_Surface *)obj;

        if (g_print && srf->s_srf->msc_p[2] != RT_NULL)
        {
            PRINT_CLP((rt_ELEM *)srf->s_srf->msc_p[2]);
        }

        pto = (rt_ELEM**)&srf->s_srf->lst_p[1];
        pti = (rt_ELEM**)&srf->s_srf->lst_p[3];

       *pto = scene->slist; /* all srf are potential rfl/rfr */
       *pti = scene->slist; /* all srf are potential rfl/rfr */

        return RT_NULL;
    }

    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (srf = scene->srf_head; srf != RT_NULL; srf = srf->next)
    {
        insert(obj, ptr, srf);
    }

    return lst;
}

/*
 * Build light/shadow lists for a given "obj".
 * Surfaces have separate light/shadow lists for each side.
 */
rt_ELEM* rt_SceneThread::lsort(rt_Object *obj)
{
    rt_Surface *srf = RT_NULL;
    rt_ELEM **pto = RT_NULL;
    rt_ELEM **pti = RT_NULL;

    if (RT_IS_SURFACE(obj))
    {
        srf = (rt_Surface *)obj;

        pto = (rt_ELEM **)&srf->s_srf->lst_p[0];
        pti = (rt_ELEM **)&srf->s_srf->lst_p[2];

       *pto = scene->llist; /* all lgt are potential sources */
       *pti = scene->llist; /* all lgt are potential sources */

        return RT_NULL;
    }

    rt_Light *lgt = RT_NULL;
    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (lgt = scene->lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        insert(lgt, ptr, RT_NULL);
    }

    return lst;
}

static
rt_void* init_threads(rt_cell thnum, rt_Scene *scn)
{
    return scn;
}

static
rt_void term_threads(rt_void *tdata, rt_cell thnum)
{

}

static
rt_void update_scene(rt_void *tdata, rt_cell thnum)
{
    rt_Scene *scn = (rt_Scene *)tdata;

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        scn->update_slice(i);
    }
}

static
rt_void render_scene(rt_void *tdata, rt_cell thnum)
{
    rt_Scene *scn = (rt_Scene *)tdata;

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        scn->render_slice(i);
    }
}

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

rt_Scene::rt_Scene(rt_SCENE *scn, /* frame must be SIMD-aligned */
                   rt_word x_res, rt_word y_res, rt_cell x_row, rt_word *frame,
                   rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free,
                   rt_FUNC_INIT f_init, rt_FUNC_TERM f_term,
                   rt_FUNC_UPDATE f_update,
                   rt_FUNC_RENDER f_render,
                   rt_FUNC_PRINT_LOG f_print_log,
                   rt_FUNC_PRINT_ERR f_print_err) : 

    rt_LogRedirect(f_print_log, f_print_err), /* must be first in scene init */
    rt_Registry(f_alloc, f_free)
{
    this->scn   = scn;

    /* init framebuffer's dimensions and pointer */

    this->x_res = x_res;
    this->y_res = y_res;
    this->x_row = x_row;

    if (frame == RT_NULL)
    {
        frame = (rt_word *)
                alloc(x_res * y_res * sizeof(rt_word),
                                RT_SIMD_ALIGN);
    }
    else
    if ((rt_word)frame & (RT_SIMD_ALIGN - 1) != 0)
    {
        throw rt_Exception("frame pointer is not SIMD-aligned in Scene");
    }

    this->frame = frame;

    /* init tilebuffer's dimensions and pointer */

    tile_w = RT_TILE_W;
    tile_h = RT_TILE_H;

    tiles_in_row = (x_res + tile_w - 1) / tile_w;
    tiles_in_col = (y_res + tile_h - 1) / tile_h;

    tiles = (rt_ELEM **)alloc(sizeof(rt_ELEM *) *
                                (tiles_in_row * tiles_in_col), RT_ALIGN);

    /* init aspect ratio, rays depth and fsaa */

    factor = 1.0f / (rt_real)x_res;
    aspect = (rt_real)y_res * factor;

    depth = RT_STACK_DEPTH;
    fsaa  = RT_FSAA_NO;

    /* init memory pool in the heap for temporary per-frame allocs */

    mpool = RT_NULL;
    msize = 0; /* estimate per-frame allocs here to reduce chunk allocs */

    /* init threads management functions */

    if (f_init != RT_NULL && f_term != RT_NULL
    &&  f_update != RT_NULL && f_render != RT_NULL)
    {
        this->f_init = f_init;
        this->f_term = f_term;
        this->f_update = f_update;
        this->f_render = f_render;
    }
    else
    {
        this->f_init = init_threads;
        this->f_term = term_threads;
        this->f_update = update_scene;
        this->f_render = render_scene;
    }

    /* create scene threads array */

    thnum = RT_THREADS_NUM;

    tharr = (rt_SceneThread **)
            alloc(sizeof(rt_SceneThread *) * thnum,
                            RT_ALIGN);
    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        tharr[i] = new rt_SceneThread(this, i);
    }

    /* create platform-specific worker threads */

    tdata = this->f_init(thnum, this);

    /* initialize rendering backend */

    render0(tharr[0]->s_inf);

    /* instantiate objects hierarchy */

    memset(&rootobj, 0, sizeof(rt_OBJECT));

    rootobj.trm.scl[RT_I] = 1.0f;
    rootobj.trm.scl[RT_J] = 1.0f;
    rootobj.trm.scl[RT_K] = 1.0f;
    rootobj.obj = scn->root;

    root = new rt_Array(this, RT_NULL, &rootobj);
    cam  = cam_head;

    /* setup surface list */

    slist = tharr[0]->ssort(cam);

    /* setup light/shadow list,
     * slist is needed inside */

    llist = tharr[0]->lsort(cam);
}

/*
 * Update current camera with given "action" for a given "time".
 */
rt_void rt_Scene::update(rt_long time, rt_cell action)
{
    cam->update(time, action);
}

/*
 * Update backend data structures and render the frame for a given "time".
 */
rt_void rt_Scene::render(rt_long time)
{
    rt_cell i;

    /* reserve memory for temporary per-frame allocs */

    mpool = reserve(msize, RT_ALIGN);

    for (i = 0; i < thnum; i++)
    {
        tharr[i]->mpool = tharr[i]->reserve(msize, RT_ALIGN);
    }

    /* print state beg */

    if (g_print)
    {
        PRINT_STATE_BEG();

        PRINT_TIME(time);
    }

    /* update the whole objects hierarchy */

    root->update(time, iden4, 0);

    /* update rays positioning and steppers */

    rt_real h, v;

    pos[RT_X] = cam->pos[RT_X];
    pos[RT_Y] = cam->pos[RT_Y];
    pos[RT_Z] = cam->pos[RT_Z];

    hor[RT_X] = cam->hor[RT_X];
    hor[RT_Y] = cam->hor[RT_Y];
    hor[RT_Z] = cam->hor[RT_Z];

    ver[RT_X] = cam->ver[RT_X];
    ver[RT_Y] = cam->ver[RT_Y];
    ver[RT_Z] = cam->ver[RT_Z];

    nrm[RT_X] = cam->nrm[RT_X];
    nrm[RT_Y] = cam->nrm[RT_Y];
    nrm[RT_Z] = cam->nrm[RT_Z];

            h = (1.0f);
            v = (aspect);

    /* aim rays at camera's top-left corner */

    dir[RT_X] = (nrm[RT_X] * cam->pov - (hor[RT_X] * h + ver[RT_X] * v) * 0.5f);
    dir[RT_Y] = (nrm[RT_Y] * cam->pov - (hor[RT_Y] * h + ver[RT_Y] * v) * 0.5f);
    dir[RT_Z] = (nrm[RT_Z] * cam->pov - (hor[RT_Z] * h + ver[RT_Z] * v) * 0.5f);

    /* update tiles positioning and steppers */

    org[RT_X] = (pos[RT_X] + dir[RT_X]);
    org[RT_Y] = (pos[RT_Y] + dir[RT_Y]);
    org[RT_Z] = (pos[RT_Z] + dir[RT_Z]);

            h = (1.0f / (factor * tile_w)); /* x_res / tile_w */
            v = (1.0f / (factor * tile_h)); /* x_res / tile_h */

    htl[RT_X] = (hor[RT_X] * h);
    htl[RT_Y] = (hor[RT_Y] * h);
    htl[RT_Z] = (hor[RT_Z] * h);

    vtl[RT_X] = (ver[RT_X] * v);
    vtl[RT_Y] = (ver[RT_Y] * v);
    vtl[RT_Z] = (ver[RT_Z] * v);

    /* multi-threaded update */

    if (g_print)
    {
        update_scene(this, thnum);

        PRINT_CAM(cam);

        PRINT_LGT_LIST(llist);

        PRINT_SRF_LIST(slist);
    }
    else
    {
        this->f_update(tdata, thnum);
    }

    rt_cell tline;
    rt_cell j;

#if RT_TILING_OPT

    memset(tiles, 0, sizeof(rt_ELEM *) * tiles_in_row * tiles_in_col);

    rt_ELEM *elm, *nxt;

    for (elm = slist; elm != RT_NULL; elm = elm->next)
    {
        rt_ELEM *tls = (rt_ELEM *)((rt_SIMD_SURFACE *)elm->simd)->msc_p[0];

        for (; tls != RT_NULL; tls = nxt)
        {
            i = (rt_word)tls->data >> 16;
            j = (rt_word)tls->data & 0xFFFF;

            nxt = (rt_ELEM *)tls->next;

            tls->data = 0;

            tline = i * tiles_in_row;

            tls->next = tiles[tline + j];
            tiles[tline + j] = tls;
        }
    }

    if (g_print)
    {
        rt_cell i = 0, j = 0;

        PRINT_TLS(tiles[i * tiles_in_row + j], i, j);
    }

#else /* RT_TILING_OPT */

    for (i = 0; i < tiles_in_col; i++)
    {
        tline = i * tiles_in_row;

        for (j = 0; j < tiles_in_row; j++)
        {
            tiles[tline + j] = slist;
        }
    }

#endif /* RT_TILING_OPT */

    /* aim rays at pixel centers */

    hor[RT_X] *= factor;
    hor[RT_Y] *= factor;
    hor[RT_Z] *= factor;

    ver[RT_X] *= factor;
    ver[RT_Y] *= factor;
    ver[RT_Z] *= factor;

    dir[RT_X] += (hor[RT_X] + ver[RT_X]) * 0.5f;
    dir[RT_Y] += (hor[RT_Y] + ver[RT_Y]) * 0.5f;
    dir[RT_Z] += (hor[RT_Z] + ver[RT_Z]) * 0.5f;

    /* accumulate ambient from camera and all light sources */

    amb[RT_R] = cam->cam->col.hdr[RT_R] * cam->cam->lum[0];
    amb[RT_G] = cam->cam->col.hdr[RT_G] * cam->cam->lum[0];
    amb[RT_B] = cam->cam->col.hdr[RT_B] * cam->cam->lum[0];

    rt_Light *lgt = RT_NULL;

    for (lgt = lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        amb[RT_R] += lgt->lgt->col.hdr[RT_R] * lgt->lgt->lum[0];
        amb[RT_G] += lgt->lgt->col.hdr[RT_G] * lgt->lgt->lum[0];
        amb[RT_B] += lgt->lgt->col.hdr[RT_B] * lgt->lgt->lum[0];
    }

    /* multi-threaded render */

    this->f_render(tdata, thnum);

    /* print state end */

    if (g_print)
    {
        PRINT_STATE_END();

        g_print = RT_FALSE;
    }

    /* release memory from temporary per-frame allocs */

    for (i = 0; i < thnum; i++)
    {
        tharr[i]->release(tharr[i]->mpool);
    }

    release(mpool);
}

rt_void rt_Scene::update_slice(rt_cell index)
{
    rt_cell i;

    rt_Surface *srf = RT_NULL;

    for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
    {
        if ((i % thnum) != index)
        {
            continue;
        }

        if (g_print)
        {
            PRINT_SRF(srf);
        }

        /* setup tile list */
        tharr[index]->stile(srf);

        /* setup surface lists */
        tharr[index]->ssort(srf);

        /* setup light/shadow lists */
        tharr[index]->lsort(srf);

        /* update backend-related parts */
        update0(srf->s_srf);
    }
}

rt_void rt_Scene::render_slice(rt_cell index)
{
    /* adjust ray steppers according to anti-aliasing mode */

    rt_real fdh[4], fdv[4];
    rt_real fhr, fvr;

    if (fsaa == RT_FSAA_4X)
    {
        rt_real as = 0.25f;
        rt_real ar = 0.08f;

        fdh[0] = (-ar-as);
        fdh[1] = (-ar+as);
        fdh[2] = (+ar-as);
        fdh[3] = (+ar+as);

        fdv[0] = (+ar-as) + index;
        fdv[1] = (-ar-as) + index;
        fdv[2] = (+ar+as) + index;
        fdv[3] = (-ar+as) + index;

        fhr = 1.0f;
        fvr = (rt_real)thnum;
    }
    else
    {
        fdh[0] = 0.0f;
        fdh[1] = 1.0f;
        fdh[2] = 2.0f;
        fdh[3] = 3.0f;

        fdv[0] = (rt_real)index;
        fdv[1] = (rt_real)index;
        fdv[2] = (rt_real)index;
        fdv[3] = (rt_real)index;

        fhr = 4.0f;
        fvr = (rt_real)thnum;
    }

/*  rt_SIMD_CAMERA */

    rt_SIMD_CAMERA *s_cam = tharr[index]->s_cam;

    RT_SIMD_SET(s_cam->t_max, RT_INF);

    s_cam->dir_x[0] = dir[RT_X] + fdh[0] * hor[RT_X] + fdv[0] * ver[RT_X];
    s_cam->dir_x[1] = dir[RT_X] + fdh[1] * hor[RT_X] + fdv[1] * ver[RT_X];
    s_cam->dir_x[2] = dir[RT_X] + fdh[2] * hor[RT_X] + fdv[2] * ver[RT_X];
    s_cam->dir_x[3] = dir[RT_X] + fdh[3] * hor[RT_X] + fdv[3] * ver[RT_X];

    s_cam->dir_y[0] = dir[RT_Y] + fdh[0] * hor[RT_Y] + fdv[0] * ver[RT_Y];
    s_cam->dir_y[1] = dir[RT_Y] + fdh[1] * hor[RT_Y] + fdv[1] * ver[RT_Y];
    s_cam->dir_y[2] = dir[RT_Y] + fdh[2] * hor[RT_Y] + fdv[2] * ver[RT_Y];
    s_cam->dir_y[3] = dir[RT_Y] + fdh[3] * hor[RT_Y] + fdv[3] * ver[RT_Y];

    s_cam->dir_z[0] = dir[RT_Z] + fdh[0] * hor[RT_Z] + fdv[0] * ver[RT_Z];
    s_cam->dir_z[1] = dir[RT_Z] + fdh[1] * hor[RT_Z] + fdv[1] * ver[RT_Z];
    s_cam->dir_z[2] = dir[RT_Z] + fdh[2] * hor[RT_Z] + fdv[2] * ver[RT_Z];
    s_cam->dir_z[3] = dir[RT_Z] + fdh[3] * hor[RT_Z] + fdv[3] * ver[RT_Z];

    RT_SIMD_SET(s_cam->hor_x, hor[RT_X] * fhr);
    RT_SIMD_SET(s_cam->hor_y, hor[RT_Y] * fhr);
    RT_SIMD_SET(s_cam->hor_z, hor[RT_Z] * fhr);

    RT_SIMD_SET(s_cam->ver_x, ver[RT_X] * fvr);
    RT_SIMD_SET(s_cam->ver_y, ver[RT_Y] * fvr);
    RT_SIMD_SET(s_cam->ver_z, ver[RT_Z] * fvr);

    RT_SIMD_SET(s_cam->clamp, 255.0f);
    RT_SIMD_SET(s_cam->cmask, 0xFF);

    RT_SIMD_SET(s_cam->col_r, amb[RT_R]);
    RT_SIMD_SET(s_cam->col_g, amb[RT_G]);
    RT_SIMD_SET(s_cam->col_b, amb[RT_B]);

/*  rt_SIMD_CONTEXT */

    rt_SIMD_CONTEXT *s_ctx = tharr[index]->s_ctx;

    RT_SIMD_SET(s_ctx->t_min, cam->pov);

    RT_SIMD_SET(s_ctx->org_x, pos[RT_X]);
    RT_SIMD_SET(s_ctx->org_y, pos[RT_Y]);
    RT_SIMD_SET(s_ctx->org_z, pos[RT_Z]);

/*  rt_SIMD_INFOX */

    rt_SIMD_INFOX *s_inf = tharr[index]->s_inf;

    s_inf->ctx = s_ctx;
    s_inf->cam = s_cam;
    s_inf->lst = slist;

    s_inf->index = index;
    s_inf->thnum = thnum;
    s_inf->depth = depth;
    s_inf->fsaa  = fsaa;

   /* render frame based on tilebuffer */

    render0(s_inf);
}

/*
 * Return pointer to the framebuffer.
 */
rt_word* rt_Scene::get_frame()
{
    return frame;
}

/*
 * Set full screen anti-aliasing mode.
 */
rt_void rt_Scene::set_fsaa(rt_cell fsaa)
{
    this->fsaa = fsaa;
}

/*
 * Print current state.
 */
rt_void rt_Scene::print_state()
{
    g_print = RT_TRUE;
}

rt_Scene::~rt_Scene()
{
    /* destroy worker threads */

    this->f_term(tdata, thnum);

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        delete tharr[i];
    }

    /* destroy objects hierarchy */

    delete root;

    /* destroy textures */

    while (tex_head)
    {
        rt_Texture *tex = tex_head->next;
        delete tex_head;
        tex_head = tex;
    }
}

/******************************************************************************/
/******************************   FPS RENDERING   *****************************/
/******************************************************************************/

#define II  0xFF000000
#define OO  0xFFFFFFFF

#define dW  5
#define dH  7

rt_word digits[10][dH][dW] = 
{
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, OO, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, OO, II, OO, OO,
        OO, II, II, OO, OO,
        OO, OO, II, OO, OO,
        OO, OO, II, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, II, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, OO, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
    {
        OO, OO, OO, OO, OO,
        OO, II, II, II, OO,
        OO, II, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, II, OO,
        OO, II, II, II, OO,
        OO, OO, OO, OO, OO,
    },
};

/*
 * Render given number "num" on the screen at given coords "x" and "y".
 * Parameters "d" and "z" specify direction and zoom respectively.
 */
rt_void rt_Scene::render_fps(rt_word x, rt_word y,
                             rt_cell d, rt_word z, rt_word num)
{
    rt_word arr[16], i, c, k;

    for (i = 0, c = 0; i < 16; i++)
    {
        k = num % 10;
        num /= 10;
        arr[i] = k;

        if (k != 0)
        {
            c = i;
        }
    }    

    c++;
    d = (d + 1) / 2;

    rt_word xd, yd, xz, yz;
    rt_word *src = NULL, *dst = NULL;

    for (i = 0; i < c; i++)
    {
        k = arr[i];
        src = &digits[k][0][0];
        dst = frame + y * x_res + x + (c * d - 1 - i) * dW * z;

        for (yd = 0; yd < dH; yd++)
        {
            for (yz = 0; yz < z; yz++)
            {
                for (xd = 0; xd < dW; xd++)
                {
                    for (xz = 0; xz < z; xz++)
                    {
                        *dst++ = *src;
                    }
                    src++;
                }
                dst += x_res - dW * z;
                src -= dW;
            }
            src += dW;
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

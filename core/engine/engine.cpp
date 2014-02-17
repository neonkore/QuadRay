/******************************************************************************/
/* Copyright (c) 2013-2014 VectorChief (at github, bitbucket, sourceforge)    */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#include <string.h>

#include "engine.h"
#include "rtgeom.h"

/******************************************************************************/
/*********************************   LEGEND   *********************************/
/******************************************************************************/

/*
 * engine.cpp: Implementation of the scene manager.
 *
 * Main file of the engine responsible for instantiating and managing the scene.
 * It contains the definition of SceneThread and Scene classes along with
 * the set of algorithms needed to process objects in the scene in order
 * to prepare data structures used by the rendering backend (tracer.cpp).
 *
 * Processing of objects consists of two major parts: update and render,
 * of which only update is handled by the engine, while render is delegated
 * to the rendering backend once all data structures have been prepared.
 *
 * Update in turn consists of three phases:
 * 0th phase (sequential) - hierarchical traversal of the objects tree
 * 1st phase (multi-threaded) - update auxiliary per-object data fields
 * 2nd phase (multi-threaded) - build cross-object lists based on relations
 *
 * First two update phases are handled by the object hierarchy (object.cpp),
 * while engine performs building of surface's tile lists, custom per-side
 * lights and shadows lists, reflection/refraction surface lists.
 *
 * Both update and render support multi-threading and use array of SceneThread
 * objects to separate working data sets and therefore avoid thread locking.
 */

/******************************************************************************/
/******************************   STATE-LOGGING   *****************************/
/******************************************************************************/

#define RT_PRINT_STATE_BEG()                                                \
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

#define RT_PRINT_TIME(time)                                                 \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("---------- update time -- %08u ----------", time);         \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n")

/*
 * Print camera properties.
 */
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

#define RT_PRINT_CAM(cam)                                                   \
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

/*
 * Print light properties.
 */
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
        RT_LOGI("                                    ");
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

#define RT_PRINT_LGT(elm, lgt)                                              \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define RT_PRINT_LGT_INNER(elm, lgt)                                        \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

#define RT_PRINT_LGT_OUTER(elm, lgt)                                        \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lgt("    ", elm, lgt);                                        \
        RT_LOGI("\n")

static
rt_pstr tags[RT_TAG_SURFACE_MAX] =
{
    "PL", "CL", "SP", "CN", "PB", "HB"
};

static
rt_pstr nodes[] =
{
    "tr",
    "bv",
    "xx",
    "xx",
};

static
rt_pstr sides[] =
{
    "out of range",
    "data = inner",
    "data = 0    ",
    "data = outer",
    "out of range",
};

static
rt_pstr markers[] =
{
    "out of range",
    "accum marker: enter",
    "empty object",
    "accum marker: leave",
    "out of range",
};

/*
 * Print surface/array properties.
 */
static
rt_void print_srf(rt_pstr mgn, rt_ELEM *elm, rt_Object *obj)
{
    rt_SIMD_SURFACE *s_srf = elm != RT_NULL ? (rt_SIMD_SURFACE *)elm->simd :
                             obj != RT_NULL ?      ((rt_Node *)obj)->s_srf :
                                                                    RT_NULL;
    RT_LOGI("%s", mgn);
    RT_LOGI("srf: %08X, ", (rt_word)obj);
    RT_LOGI("SRF: %08X, ", (rt_word)s_srf);
    RT_LOGI("elm: %08X, ", (rt_word)elm);

    rt_cell d = elm != RT_NULL ? elm->data : 0;
    rt_cell i = RT_MAX(0, d + 2), n = d & 0x3;

    if (s_srf != RT_NULL && obj != RT_NULL)
    {
        if (RT_IS_ARRAY(obj))
        {
            RT_LOGI("    ");
            RT_LOGI("tag: AR, trm: %d, data = %08X %s ",
                s_srf->a_map[3], d & 0xFFFFFFFC, nodes[n]);
        }
        else
        {
            RT_LOGI("    ");
            RT_LOGI("tag: %s, trm: %d, %s       ",
                tags[obj->tag], s_srf->a_map[3],
                sides[RT_MIN(i, RT_ARR_SIZE(sides) - 1)]);
        }
        RT_LOGI("    ");
        RT_LOGI("pos: {%f, %f, %f}",
            obj->pos[RT_X], obj->pos[RT_Y], obj->pos[RT_Z]);
    }
    else
    {
        RT_LOGI("    ");
        RT_LOGI("%s", markers[RT_MIN(i, RT_ARR_SIZE(markers) - 1)]);
    }
    RT_LOGI("\n");
}

#define RT_PRINT_SRF(srf)                                                   \
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

/*
 * Print list of objects.
 */
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

#define RT_PRINT_CLP(lst)                                                   \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- clp --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LST(lst)                                                   \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LST_INNER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LST_OUTER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lst - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SHW(lst)                                                   \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SHW_INNER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw - inner ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SHW_OUTER(lst)                                             \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- shw - outer ------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_LGT_LST(lst)                                               \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- lgt --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_SRF_LST(lst)                                               \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("-------------------- srf --------------------");           \
        RT_LOGI("---------------------------------------------");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_TLS_LST(lst, i, j)                                         \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("********* screen tiles[%2d][%2d] list: ********", i, j);   \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("*********************************************");           \
        RT_LOGI("\n");                                                      \
        RT_LOGI("\n");                                                      \
        print_lst("    ", lst);                                             \
        RT_LOGI("\n")

#define RT_PRINT_STATE_END()                                                \
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

/*
 * Instantiate scene thread.
 */
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
    RT_SIMD_SET(s_inf->gpc04, 0x7FFFFFFF);
    RT_SIMD_SET(s_inf->gpc05, 0x3F800000);

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

    memset(s_cam, 0, sizeof(rt_SIMD_CAMERA));

    /* allocate ctx SIMD structure */
    s_ctx = (rt_SIMD_CONTEXT *)
            alloc(sizeof(rt_SIMD_CONTEXT) + /* +1 context step for shadows */
                            RT_STACK_STEP * (1 + scene->depth),
                            RT_SIMD_ALIGN);

    memset(s_ctx, 0, sizeof(rt_SIMD_CONTEXT));

    /* init memory pool in the heap for temporary per-frame allocs */
    mpool = RT_NULL;
    msize = 0;

    /* allocate misc arrays for tiling */
    txmin = (rt_cell *)alloc(sizeof(rt_cell) * scene->tiles_in_col, RT_ALIGN);
    txmax = (rt_cell *)alloc(sizeof(rt_cell) * scene->tiles_in_col, RT_ALIGN);
    verts = (rt_VERT *)alloc(sizeof(rt_VERT) * 
                             (2 * RT_VERTS_LIMIT + RT_EDGES_LIMIT), RT_ALIGN);
}

#define RT_UPDATE_TILES_BOUNDS(cy, x1, x2)                                  \
do                                                                          \
{                                                                           \
    if (x1 < x2)                                                            \
    {                                                                       \
        if (txmin[cy] > x1)                                                 \
        {                                                                   \
            txmin[cy] = RT_MAX(x1, xmin);                                   \
        }                                                                   \
        if (txmax[cy] < x2)                                                 \
        {                                                                   \
            txmax[cy] = RT_MIN(x2, xmax);                                   \
        }                                                                   \
    }                                                                       \
    else                                                                    \
    {                                                                       \
        if (txmin[cy] > x2)                                                 \
        {                                                                   \
            txmin[cy] = RT_MAX(x2, xmin);                                   \
        }                                                                   \
        if (txmax[cy] < x1)                                                 \
        {                                                                   \
            txmax[cy] = RT_MIN(x1, xmax);                                   \
        }                                                                   \
    }                                                                       \
}                                                                           \
while (0) /* "do {...} while (0)" to enforce semicolon ";" at the end */

/*
 * Update surface's projected bbox boundaries in tilebuffer
 * by processing one bbox edge at a time.
 * The tilebuffer is reset for every surface from outside of this function.
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

#if RT_OPTS_TILING_EXT1 != 0
    if ((scene->opts & RT_OPTS_TILING_EXT1) != 0)
    {
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
    }
    else
#endif /* RT_OPTS_TILING_EXT1 */
    {
        n1[0][RT_X] = p1[RT_X];
        n1[0][RT_Y] = p1[RT_Y];
        n2[0][RT_X] = p2[RT_X];
        n2[0][RT_Y] = p2[RT_Y];

        n = 1;
    }

    /* set inclusive bounds */
    rt_cell xmin = 0;
    rt_cell ymin = 0;
    rt_cell xmax = scene->tiles_in_row - 1;
    rt_cell ymax = scene->tiles_in_col - 1;

    for (i = 0; i < n; i++)
    {
        /* calculate points floor */
        x1 = RT_FLOOR(n1[i][RT_X]);
        y1 = RT_FLOOR(n1[i][RT_Y]);
        x2 = RT_FLOOR(n2[i][RT_X]);
        y2 = RT_FLOOR(n2[i][RT_Y]);

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
            {
                y1 = ymin;
            }
            if (y2 > ymax)
            {
                y2 = ymax;
            }
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
        x2 = RT_FLOOR(px);

        if (y1 > ymin)
        {
            RT_UPDATE_TILES_BOUNDS(y1 - 1, x1, x2);
        }

        x1 = x2;

        for (t = y1; t <= y2; t++)
        {
            px = px + rt;
            x2 = RT_FLOOR(px);
            RT_UPDATE_TILES_BOUNDS(t, x1, x2);
            x1 = x2;
        }

        if (y2 < ymax)
        {
            x2 = RT_FLOOR(n2[i][RT_X]);
            RT_UPDATE_TILES_BOUNDS(y2 + 1, x1, x2);
        }
    }
}

/*
 * Insert new element derived from "srf" to a list "ptr"
 * for a given object "obj". If "srf" is NULL and "obj" is LIGHT,
 * insert new element derived from "obj" to a list "ptr".
 * Return outer-most new element (not always list's head).
 */
rt_ELEM* rt_SceneThread::insert(rt_Object *obj, rt_ELEM **ptr, rt_Surface *srf)
{
    rt_ELEM *elm = RT_NULL;

    if (srf == RT_NULL && RT_IS_LIGHT(obj))
    {
        rt_Light *lgt = (rt_Light *)obj;

        /* alloc new element for lgt */
        elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        elm->data = (rt_cell)scene->slist; /* all srf are potential shadows */
        elm->simd = lgt->s_lgt;
        elm->temp = lgt;
        /* insert element as list head */
        elm->next = *ptr;
       *ptr = elm;
    }

    if (srf == RT_NULL)
    {
        return elm;
    }

    /* alloc new element for srf */
    elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
    elm->data = 0;
    elm->simd = RT_NULL;
    elm->temp = srf;

    rt_ELEM *lst[2], *nxt;
    rt_Array *arr[2];

    /* determine if surface belongs to any trnode/bvnode objects,
     * index in the array below also serves as node's type in the list */
    lst[0] = RT_NULL;
    arr[0] = srf->trnode != RT_NULL && srf->trnode != srf ?
                    (rt_Array *)srf->trnode : RT_NULL;

    lst[1] = RT_NULL;
    arr[1] = srf->bvnode != RT_NULL && RT_IS_SURFACE(obj) ?
                    (rt_Array *)srf->bvnode : RT_NULL;

    rt_cell i, k = -1, n = (arr[0] != RT_NULL) + (arr[1] != RT_NULL);

    /* determine trnode/bvnode order on the branch,
     * "k" is set to the index of the outer-most node */
    if (n == 2)
    {
        /* if the same array serves as both trnode and bvnode,
         * select bvnode as outer-most */
        if (arr[0] == arr[1])
        {
            k = 1;
        }
        /* otherwise, determine the order by searching the nodes
         * among each other's parents */
        else
        {
            for (i = 0; i < 2 && k < 0; i++)
            {
                rt_Array *par = (rt_Array *)arr[1 - i]->parent;
                for (; par != RT_NULL; par = (rt_Array *)par->parent)
                {
                    if (par == arr[i])
                    {
                        k = i;
                        break;
                    }
                }
            }
            /* check if trnode/bvnode are not
             * among each other's parents */
            if (k == -1)
            {
                throw rt_Exception("trnode/bvnode are not on the same branch");
            }
        }
    }
    else
    if (arr[0] != RT_NULL)
    {
        k = 0;
    }
    else
    if (arr[1] != RT_NULL)
    {
        k = 1;
    }

    nxt = (rt_ELEM *)((rt_cell)*ptr & ~0x3);

    /* search matching existing trnode/bvnode for insertion,
     * run through the list hierachy to find the inner-most node,
     * node's "simd" field holds pointer to node's sublist
     * along with node's type in the lower 2 bits (trnode/bvnode) */
    for (i = 0; nxt != RT_NULL && i < n; nxt = nxt->next)
    {
        if (arr[k] == nxt->temp && ((rt_cell)nxt->simd & 0x3) == k)
        {
            lst[k] = nxt;
            /* set insertion point to existing node's sublist */
            ptr = (rt_ELEM **)&nxt->simd;
            /* search next node (inner-most) in existing node's sublist */
            nxt = (rt_ELEM *)((rt_cell)*ptr & ~0x3);
            k = 1 - k;
            i++;
        }
    }
    /* search above is desgined in such a way, that contents
     * of one array node can now be split across the boundary
     * of another array node by inserting two different node
     * elements of the same type belonging to the same array,
     * one into another array node's sublist and one outside,
     * this allows for greater flexibility in trnode/bvnode
     * relationship, something not allowed in previous versions */

    rt_ELEM **org = RT_NULL;
    /* allocate new node elements from outer-most to inner-most
     * if they are not already in the list */
    for (; i < n; i++, k = 1 - k)
    {
        /* alloc new trnode/bvnode element as none has been found */
        nxt = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
        nxt->data = 0;
        nxt->simd = (rt_pntr)k; /* node's type */
        nxt->temp = arr[k];
        /* insert element according to found position */
        nxt->next = (rt_ELEM *)((rt_cell)*ptr & ~0x3);
       *ptr = (rt_ELEM *)((rt_cell)nxt | (rt_cell)*ptr & 0x3);
        if (org == RT_NULL)
        {
            org = ptr;
        }
        /* set insertion point to new node's sublist */
        ptr = (rt_ELEM **)&nxt->simd;
    }

    /* insert element according to found position */
    elm->next = (rt_ELEM *)((rt_cell)*ptr & ~0x3);
   *ptr = (rt_ELEM *)((rt_cell)elm | (rt_cell)*ptr & 0x3);
    /* prepare outer-most new element for sorting
     * in order to figure out its optimal position in the list
     * and thus reduce potential overdraw in the rendering backend,
     * as array's bounding volume is final at this point
     * it is correct to pass it through the sorting routine below
     * before other elements are added into its node's sublist */
    if (org != RT_NULL)
    {
        ptr = org;
        elm = (rt_ELEM *)((rt_cell)*ptr & ~0x3);
    }

    /* sort nodes in the list "ptr" with the new element "elm"
     * based on the bounding volume order as seen from "obj",
     * sorting is always applied to a single flat list
     * whether it's a top-level list or array node's sublist
     * treating both surface and array nodes in that list
     * as single whole elements, thus sorting never violates
     * the boundaries of the array node sublists as they are
     * determined by the search/insert algorithm above */
#if RT_OPTS_INSERT == 1
    if ((scene->opts & RT_OPTS_INSERT) == 0)
#endif /* RT_OPTS_INSERT */
    {
        return elm;
    }

    /* "state" helps to avoid stored order value re-computation
     * when the whole sublist is being moved (one element at a time),
     * the term sublist used here and below refers to a continuous portion
     * of a single flat list as opposed to the same term used above
     * to separate different layers of the list hierarchy */
    rt_cell state = 0;
    rt_ELEM *prv = RT_NULL;

    /* phase 1, push "elm" through the list "ptr" for as long as possible,
     * order value re-computation is avoided via "state" variable */
    for (nxt = elm->next; nxt != RT_NULL; )
    {
        /* compute the order value between "elm" and "nxt" elements */
        rt_cell op = bbox_sort(obj, (rt_Node *)elm->temp,
                                    (rt_Node *)nxt->temp);
        switch (op)
        {
            /* move "elm" forward if the "op" is
             * either "do swap" or "neutral" */
            case 2:
            /* as the swap operation is performed below
             * the stored order value becomes "don't swap" */
            op = 1;
            case 3:
            elm->next = nxt->next;
            if (prv != RT_NULL)
            {
                if (state != 0)
                {
                    prv->data = state;
                }
                else
                {
                    prv->data = bbox_sort(obj, (rt_Node *)prv->temp,
                                               (rt_Node *)nxt->temp);
                }
                prv->next = nxt;
            }
            else
            {
               *ptr = (rt_ELEM *)((rt_cell)nxt | (rt_cell)*ptr & 0x3);
            }
            /* if current "elm" position is transitory, "state" keeps
             * previously computed order value between "prv" and "nxt",
             * thus the order value can be restored to "prv" data field
             * without re-computation as the "elm" advances further */
            state = nxt->data;
            nxt->data = op;
            nxt->next = elm;
            prv = nxt;
            nxt = elm->next;
            break;

            /* stop phase 1 if the "op" is
             * either "don't swap" or "unsortable" */
            default:
            elm->data = op;
            /* reset "state" as the "elm" has found its place */
            state = 0;
            nxt = RT_NULL;
            break;
        }
    }

    rt_ELEM *end, *tlp, *cur, *ipt, *jpt;

    /* phase 2, find the "end" of the strict-order-chain from "elm",
     * order values "don't swap" and "unsortable" are considered strict */
    for (end = elm; end->data == 1 || end->data == 4; end = end->next);

    /* phase 3, move the elements from behind "elm" strict-order-chain
     * right in front of the "elm" as computed order value dictates,
     * order value re-computation is avoided via "state" variables */
    for (tlp = end, nxt = end->next; nxt != RT_NULL; )
    {
        rt_bool gr = RT_FALSE;
        /* compute the order value between "elm" and "nxt" elements */
        rt_cell op = bbox_sort(obj, (rt_Node *)elm->temp,
                                    (rt_Node *)nxt->temp);
        switch (op)
        {
            /* move "nxt" in front of the "elm"
             * if the "op" is "do swap" */
            case 2:
            /* as the swap operation is performed below
             * the stored order value becomes "don't swap" */
            op = 1;
            /* check if there is a tail from "end->next"
             * up to "tlp" to comb out thoroughly before
             * moving "nxt" (along with its strict-order-chain
             * from "tlp->next") to the front of the "elm" */
            if (tlp != end)
            {
                /* local "state" helps to avoid stored order value
                 * re-computation for tail's elements joining the comb */
                rt_cell state = 0;
                cur = tlp;
                /* run through the tail area from "end->next"
                 * up to "tlp" backwards, while combing out
                 * elements to move along with "nxt" */
                while (cur != end)
                {
                    rt_bool mv = RT_FALSE;
                    /* search for "cur" previous element,
                     * can be optimized out for dual-linked list,
                     * though the overhead of managing dual-linked list
                     * can easily overweight the added benefit */
                    for (ipt = end; ipt->next != cur; ipt = ipt->next);
                    rt_ELEM *iel = ipt->next;
                    /* run through the strict-order-chain from "tlp->next"
                     * up to "nxt" (which serves as a comb for the tail)
                     * and compute new order values for each tail element */
                    for (jpt = tlp; jpt != nxt; jpt = jpt->next)
                    {
                        rt_cell op = 0;
                        rt_ELEM *jel = jpt->next;
                        /* if "tlp" stored order value to the first
                         * comb element is not reset, use it as "op",
                         * "cur" serves as "tlp" */
                        if (cur->next == jel && cur->data != 0)
                        {
                            op = cur->data;
                        }
                        else
                        /* if "state" is not reset, it stores the order value
                         * from "cur" to the first comb element (last moved),
                         * use it as "op" */
                        if (tlp->next == jel && state != 0)
                        {
                            op = state;
                        }
                        /* compute new order value */
                        else
                        {
                            op = bbox_sort(obj, (rt_Node *)cur->temp,
                                                (rt_Node *)jel->temp);
                        }
                        /* repair "tlp" stored order value to the first
                         * comb element, "cur" serves as "tlp" */
                        if (cur->next == jel)
                        {
                            cur->data = op;
                        }
                        else
                        /* remember "cur" computed order value to the first
                         * comb element in the "state", if "cur" != "tlp" */
                        if (tlp->next == jel)
                        {
                            state = op;
                        }
                        /* check if order is strict, then stop
                         * and mark "cur" as moving with "nxt",
                         * "cur" will then be added to the comb */
                        if (op == 1 || op == 4)
                        {
                            mv = RT_TRUE;
                            break;
                        }
                    }
                    /* check if "cur" needs to move (join the comb) */
                    if (mv == RT_TRUE)
                    {
                        gr = RT_TRUE;
                        /* check if "cur" was the last tail's element,
                         * then tail gets shorten by one element "cur",
                         * which at the same time joins the comb */
                        if (cur == tlp)
                        {
                            /* move "tlp" to its prev, its stored order value
                             * is always repaired in the combing stage above */
                            tlp = ipt;
                        }
                        /* move "cur" from the middle of the tail
                         * to the front of the comb, "iel" serves
                         * as "cur" and "ipt" serves as "cur" prev */
                        else
                        {
                            cur = tlp->next;
                            iel->data = state;
                            /* local "state" keeps previously computed order
                             * value between "cur" and the front of the comb,
                             * thus the order value can be restored to
                             * "cur" data field without re-computation */
                            state = ipt->data;
                            ipt->data = 0;
                            ipt->next = iel->next;
                            iel->next = cur;
                            tlp->data = 0;
                            tlp->next = iel;
                        }
                    }
                    /* "cur" doesn't move (stays in the tail) */
                    else
                    {
                        /* repair "cur" stored order value before it
                         * moves to its prev, "iel" serves as "cur" */
                        if (iel->data == 0)
                        {
                            cur = iel->next;
                            iel->data = 
                                 bbox_sort(obj, (rt_Node *)iel->temp,
                                                (rt_Node *)cur->temp);
                        }
                        /* reset local "state" as tail's sublist
                         * (joining the comb) is being broken */
                        state = 0;
                    }
                    /* move "cur" to its prev */
                    cur = ipt;
                }
                /* repair "end" stored order value (to the rest of the tail),
                 * "ipt" serves as "end" */
                if (ipt->data == 0)
                {
                    cur = ipt->next;
                    ipt->data = bbox_sort(obj, (rt_Node *)ipt->temp,
                                               (rt_Node *)cur->temp);
                }
            }
            /* reset "state" if the comb has grown with tail elements, thus
             * breaking the sublist moving to the front of the "elm" */
            if (gr == RT_TRUE)
            {
                state = 0;
            }
            /* move "nxt" along with its comb (if any)
             * from "tlp->next" to the front of the "elm" */
            cur = tlp->next;
            if (prv != RT_NULL)
            {
                if (state != 0)
                {
                    prv->data = state;
                }
                else
                {
                    prv->data = bbox_sort(obj, (rt_Node *)prv->temp,
                                               (rt_Node *)cur->temp);
                }
                prv->next = cur;
            }
            else
            {
               *ptr = (rt_ELEM *)((rt_cell)cur | (rt_cell)*ptr & 0x3);
            }
            cur = nxt->next;
            tlp->data = 0;
            tlp->next = cur;
            /* "state" keeps previously computed order value between "nxt"
             * and "nxt->next", thus the order value can be restored to
             * "prv" data field without re-computation if the whole
             * sublist is being moved from "nxt" to the front of the "elm" */
            state = nxt->data;
            nxt->data = op;
            nxt->next = elm;
            prv = nxt;
            nxt = cur;
            break;

            /* move "nxt" forward if the "op" is
             * "don't swap", "neutral" or "unsortable" */
            default:
            /* if "nxt" stored order value (to "nxt->next")
             * is "neutral", then strict-order-chain
             * from "tlp->next" up to "nxt" is being broken
             * as "nxt" moves, thus "tlp" catches up with "nxt" */
            if (nxt->data != 1 && nxt->data != 4)
            {
                /* repair "tlp" stored order value
                 * before it catches up with "nxt" */
                if (tlp->data == 0)
                {
                    cur = tlp->next;
                    tlp->data = bbox_sort(obj, (rt_Node *)tlp->temp,
                                               (rt_Node *)cur->temp);
                }
                /* reset "state" as "tlp" moves forward, thus
                 * breaking the sublist moving to the front of the "elm" */
                state = 0;
                /* move "tlp" to "nxt" before it advances */
                tlp = nxt;
            }
            /* when "nxt" runs away from "tlp" it grows a
             * strict-order-chain from "tlp->next" up to "nxt",
             * which then serves as a comb for the tail area
             * from "end->next" up to "tlp" */
            nxt = nxt->next;
            break;
        }
    }
    /* repair "tlp" stored order value
     * if there are elements left behind it */
    cur = tlp->next;
    if (tlp->data == 0 && cur != RT_NULL)
    {
        tlp->data = bbox_sort(obj, (rt_Node *)tlp->temp,
                                   (rt_Node *)cur->temp);
    }

    return elm;
}

/*
 * Filter the list "ptr" for a given object "obj" by
 * converting hierarchical sorted sublists back into
 * a single flat list suitable for rendering backend,
 * while cleaning "data" and "simd" fields at the same time.
 * Return last leaf element of the list hierarchy (recursive).
 */
rt_ELEM* rt_SceneThread::filter(rt_Object *obj, rt_ELEM **ptr)
{
    rt_ELEM *elm = RT_NULL, *nxt;

    if (ptr == RT_NULL)
    {
        return elm;
    }

    nxt = (rt_ELEM *)((rt_cell)*ptr & ~0x3);

    for (; nxt != RT_NULL; nxt = nxt->next)
    {
        /* only node elements are allowed in surface lists */
        rt_Node *nd = (rt_Node *)nxt->temp;

        /* if the list element is surface,
         * reset "data" field used as stored order value
         * in sorting to keep it clean for the backend */
        if (RT_IS_SURFACE(nd))
        {
            elm = nxt;
            nxt->data = 0;
            nxt->simd = nd->s_srf;
        }
        else
        /* if the list element is array,
         * find the last leaf element of its sublist hierarchy
         * and set it to the "data" field along with node's type,
         * previously kept in its "simd" field's lower 2 bits */
        if (RT_IS_ARRAY(nd))
        {
            ptr = (rt_ELEM **)&nxt->simd;
            elm = filter(obj, ptr);
            elm->next = nxt->next;
            nxt->data = (rt_cell)elm | (rt_cell)*ptr & 0x3;
            nxt->next = (rt_ELEM *)((rt_cell)*ptr & ~0x3);
            nxt->simd = nd->s_srf;
            nxt = elm;
        }
    }

    return elm;
}

/*
 * Build tile list for a given "srf" based
 * on the area its projected bbox occupies in the tilebuffer.
 */
rt_void rt_SceneThread::stile(rt_Surface *srf)
{
    srf->s_srf->msc_p[0] = RT_NULL;

#if RT_OPTS_TILING != 0
    if ((scene->opts & RT_OPTS_TILING) == 0)
#endif /* RT_OPTS_TILING */
    {
        return;
    }

    rt_ELEM *elm = RT_NULL;
    rt_cell i, j;
    rt_cell k;

    rt_vec4 vec;
    rt_real dot;
    rt_cell ndx[2];
    rt_real tag[2], zed[2];

    /* verts_num may grow, use srf->verts_num if original is needed */
    rt_cell verts_num = srf->verts_num;
    rt_VERT *vrt = srf->verts;

    /* project bbox onto the tilebuffer */
    if (verts_num != 0)
    {
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = scene->tiles_in_row;
            txmax[i] = -1;
        }

        /* process bbox vertices */
        memset(verts, 0, sizeof(rt_VERT) * (2 * verts_num + srf->edges_num));

        for (k = 0; k < srf->verts_num; k++)
        {
            vec[RT_X] = vrt[k].pos[RT_X] - scene->org[RT_X];
            vec[RT_Y] = vrt[k].pos[RT_Y] - scene->org[RT_Y];
            vec[RT_Z] = vrt[k].pos[RT_Z] - scene->org[RT_Z];

            dot = RT_VECTOR_DOT(vec, scene->nrm);

            verts[k].pos[RT_Z] = dot;
            verts[k].pos[RT_W] = -1.0f; /* tag: behind screen plane */

            /* process vertices in front of or near screen plane,
             * the rest are processed with edges */
            if (dot >= 0.0f || RT_FABS(dot) <= RT_CLIP_THRESHOLD)
            {
                vec[RT_X] = vrt[k].pos[RT_X] - scene->pos[RT_X];
                vec[RT_Y] = vrt[k].pos[RT_Y] - scene->pos[RT_Y];
                vec[RT_Z] = vrt[k].pos[RT_Z] - scene->pos[RT_Z];

                dot = RT_VECTOR_DOT(vec, scene->nrm) / scene->cam->pov;

                vec[RT_X] /= dot; /* dot >= (pov - RT_CLIP_THRESHOLD) */
                vec[RT_Y] /= dot; /* pov >= (2  *  RT_CLIP_THRESHOLD) */
                vec[RT_Z] /= dot; /* thus: (dot >= RT_CLIP_THRESHOLD) */

                vec[RT_X] -= scene->dir[RT_X];
                vec[RT_Y] -= scene->dir[RT_Y];
                vec[RT_Z] -= scene->dir[RT_Z];

                verts[k].pos[RT_X] = RT_VECTOR_DOT(vec, scene->htl);
                verts[k].pos[RT_Y] = RT_VECTOR_DOT(vec, scene->vtl);

                verts[k].pos[RT_W] = +1.0f; /* tag: in front of screen plane */

                /* slightly behind (near) screen plane,
                 * generate new vertex */
                if (verts[k].pos[RT_Z] < 0.0f)
                {
                    verts[verts_num].pos[RT_X] = verts[k].pos[RT_X];
                    verts[verts_num].pos[RT_Y] = verts[k].pos[RT_Y];
                    verts_num++;

                    verts[k].pos[RT_W] = 0.0f; /* tag: near screen plane */
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
                tag[i] = verts[ndx[i]].pos[RT_W];
            }

            /* skip edge if both vertices are eihter
             * behind or near screen plane */
            if (tag[0] <= 0.0f && tag[1] <= 0.0f)
            {
                continue;
            }

            for (i = 0; i < 2; i++)
            {
                /* skip vertex if in front of
                 * or near screen plane */
                if (tag[i] >= 0.0f)
                {
                    continue;
                }

                /* process edge with one "in front of"
                 * and one "behind" vertices */
                j = 1 - i;

                /* clip edge at screen plane crossing,
                 * generate new vertex */
                vec[RT_X] = vrt[ndx[i]].pos[RT_X] - vrt[ndx[j]].pos[RT_X];
                vec[RT_Y] = vrt[ndx[i]].pos[RT_Y] - vrt[ndx[j]].pos[RT_Y];
                vec[RT_Z] = vrt[ndx[i]].pos[RT_Z] - vrt[ndx[j]].pos[RT_Z];

                dot = zed[j] / (zed[j] - zed[i]); /* () >= RT_CLIP_THRESHOLD */

                vec[RT_X] *= dot;
                vec[RT_Y] *= dot;
                vec[RT_Z] *= dot;

                vec[RT_X] += vrt[ndx[j]].pos[RT_X] - scene->org[RT_X];
                vec[RT_Y] += vrt[ndx[j]].pos[RT_Y] - scene->org[RT_Y];
                vec[RT_Z] += vrt[ndx[j]].pos[RT_Z] - scene->org[RT_Z];

                verts[verts_num].pos[RT_X] = RT_VECTOR_DOT(vec, scene->htl);
                verts[verts_num].pos[RT_Y] = RT_VECTOR_DOT(vec, scene->vtl);

                ndx[i] = verts_num;
                verts_num++;
            }

            /* tile current edge */
            tiling(verts[ndx[0]].pos, verts[ndx[1]].pos); 
        }

        /* tile all newly generated vertex pairs */
        for (i = srf->verts_num; i < verts_num - 1; i++)
        {
            for (j = i + 1; j < verts_num; j++)
            {
                tiling(verts[i].pos, verts[j].pos); 
            }
        }
    }
    else
    {
        /* mark all tiles in the entire tilbuffer */
        for (i = 0; i < scene->tiles_in_col; i++)
        {
            txmin[i] = 0;
            txmax[i] = scene->tiles_in_row - 1;
        }
    }

    rt_ELEM **ptr = (rt_ELEM **)&srf->s_srf->msc_p[0];

    /* fill marked tiles with surface data */
    for (i = 0; i < scene->tiles_in_col; i++)
    {
        for (j = txmin[i]; j <= txmax[i]; j++)
        {
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = i << 16 | j;
            elm->simd = srf->s_srf;
            elm->temp = srf;
           *ptr = elm;
            ptr = &elm->next;
        }
    }

   *ptr = RT_NULL;
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
            RT_PRINT_CLP((rt_ELEM *)srf->s_srf->msc_p[2]);
        }

        pto = (rt_ELEM **)&srf->s_srf->lst_p[1];
        pti = (rt_ELEM **)&srf->s_srf->lst_p[3];

#if RT_OPTS_RENDER != 0
        if ((scene->opts & RT_OPTS_RENDER) != 0
        && (((rt_word)srf->s_srf->mat_p[1] & RT_PROP_REFLECT) != 0
        ||  ((rt_word)srf->s_srf->mat_p[3] & RT_PROP_REFLECT) != 0
        ||  ((rt_word)srf->s_srf->mat_p[1] & RT_PROP_OPAQUE) == 0
        ||  ((rt_word)srf->s_srf->mat_p[3] & RT_PROP_OPAQUE) == 0))
        {
           *pto = RT_NULL;
           *pti = RT_NULL;

            /* RT_LOGI("Building slist for surface\n"); */
        }
        else
#endif /* RT_OPTS_RENDER */
        {
           *pto = scene->slist; /* all srf are potential rfl/rfr */
           *pti = scene->slist; /* all srf are potential rfl/rfr */

            return RT_NULL;
        }
    }

    rt_Surface *ref = RT_NULL;
    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (ref = scene->srf_head; ref != RT_NULL; ref = ref->next)
    {
#if RT_OPTS_2SIDED != 0
        if ((scene->opts & RT_OPTS_2SIDED) != 0 && srf != RT_NULL)
        {
            rt_cell c = bbox_side(srf, ref);

            if (c & 2)
            {
                insert(obj, pto, ref);
            }
            if (c & 1)
            {
                insert(obj, pti, ref);
            }
        }
        else
#endif /* RT_OPTS_2SIDED */
        {
            insert(obj, ptr, ref);
        }
    }

#if RT_OPTS_INSERT != 0
    if ((scene->opts & RT_OPTS_INSERT) != 0)
    {
#if RT_OPTS_2SIDED != 0
        filter(obj, pto);
        filter(obj, pti);
#endif /* RT_OPTS_2SIDED */
        filter(obj, ptr);
    }
#endif /* RT_OPTS_INSERT */

    if (srf == RT_NULL)
    {
        return lst;
    }

    if (g_print)
    {
#if RT_OPTS_2SIDED != 0
        if (*pto != RT_NULL)
        {
            RT_PRINT_LST_OUTER(*pto);
        }
        if (*pti != RT_NULL)
        {
            RT_PRINT_LST_INNER(*pti);
        }
#endif /* RT_OPTS_2SIDED */
        if (*ptr != RT_NULL)
        {
            RT_PRINT_LST(*ptr);
        }
    }

#if RT_OPTS_2SIDED != 0
    if ((scene->opts & RT_OPTS_2SIDED) == 0)
#endif /* RT_OPTS_2SIDED */
    {
       *pto = lst;
       *pti = lst;

        return RT_NULL;
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

#if RT_OPTS_SHADOW != 0
        if ((scene->opts & RT_OPTS_SHADOW) != 0)
        {
           *pto = RT_NULL;
           *pti = RT_NULL;

            /* RT_LOGI("Building llist for surface\n"); */
        }
        else
#endif /* RT_OPTS_SHADOW */
        {
           *pto = scene->llist; /* all lgt are potential sources */
           *pti = scene->llist; /* all lgt are potential sources */

            return RT_NULL;
        }
    }

    rt_Light *lgt = RT_NULL;
    rt_ELEM *lst = RT_NULL;
    rt_ELEM **ptr = &lst;

    for (lgt = scene->lgt_head; lgt != RT_NULL; lgt = lgt->next)
    {
        rt_ELEM **psr = RT_NULL;
#if RT_OPTS_2SIDED != 0
        rt_ELEM **pso = RT_NULL;
        rt_ELEM **psi = RT_NULL;

        if ((scene->opts & RT_OPTS_2SIDED) != 0 && srf != RT_NULL)
        {
            rt_cell c = cbox_side(lgt->pos, srf);

            if (c & 2)
            {
                insert(lgt, pto, RT_NULL);

                pso = (rt_ELEM **)&(*pto)->data;
               *pso = RT_NULL;

                if (g_print)
                {
                    RT_PRINT_LGT_OUTER(*pto, lgt);
                }
            }
            if (c & 1)
            {
                insert(lgt, pti, RT_NULL);

                psi = (rt_ELEM **)&(*pti)->data;
               *psi = RT_NULL;

                if (g_print)
                {
                    RT_PRINT_LGT_INNER(*pto, lgt);
                }
            }
        }
        else
#endif /* RT_OPTS_2SIDED */
        {
            insert(lgt, ptr, RT_NULL);

            psr = (rt_ELEM **)&(*ptr)->data;

            if (g_print && srf != RT_NULL)
            {
                RT_PRINT_LGT(*ptr, lgt);
            }
        }

#if RT_OPTS_SHADOW != 0
        if (srf == RT_NULL)
        {
            continue;
        }

        if (psr != RT_NULL)
        {
           *psr = RT_NULL;
        }

        rt_Surface *shw = RT_NULL;

        for (shw = scene->srf_head; shw != RT_NULL; shw = shw->next)
        {
            if (bbox_shad(lgt, shw, srf) == 0)
            {
                continue;
            }

#if RT_OPTS_2SIDED != 0
            if ((scene->opts & RT_OPTS_2SIDED) != 0)
            {
                rt_cell c = bbox_side(srf, shw);

                if (c & 2 && pso != RT_NULL)
                {
                    insert(lgt, pso, shw);
                }
                if (c & 1 && psi != RT_NULL)
                {
                    insert(lgt, psi, shw);
                }
            }
            else
#endif /* RT_OPTS_2SIDED */
            {
                insert(lgt, psr, shw);
            }
        }

#if RT_OPTS_INSERT != 0
        if ((scene->opts & RT_OPTS_INSERT) != 0)
        {
#if RT_OPTS_2SIDED != 0
            filter(lgt, pso);
            filter(lgt, psi);
#endif /* RT_OPTS_2SIDED */
            filter(lgt, psr);
        }
#endif /* RT_OPTS_INSERT */

        if (g_print)
        {
#if RT_OPTS_2SIDED != 0
            if (pso != RT_NULL && *pso != RT_NULL)
            {
                RT_PRINT_SHW_OUTER(*pso);
            }
            if (psi != RT_NULL && *psi != RT_NULL)
            {
                RT_PRINT_SHW_INNER(*psi);
            }
#endif /* RT_OPTS_2SIDED */
            if (psr != RT_NULL && *psr != RT_NULL)
            {
                RT_PRINT_SHW(*psr);
            }
        }
#endif /* RT_OPTS_SHADOW */
    }

    if (srf == RT_NULL)
    {
        return lst;
    }

#if RT_OPTS_2SIDED != 0
    if ((scene->opts & RT_OPTS_2SIDED) == 0)
#endif /* RT_OPTS_2SIDED */
    {
       *pto = lst;
       *pti = lst;

        return RT_NULL;
    }

    return lst;
}

/*
 * Destroy scene thread.
 */
rt_SceneThread::~rt_SceneThread()
{

}

/******************************************************************************/
/*****************************   MULTI-THREADING   ****************************/
/******************************************************************************/

/*
 * Initialize platform-specific pool of "thnum" threads.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging.
 */
static
rt_void* init_threads(rt_cell thnum, rt_Scene *scn)
{
    return scn;
}

/*
 * Terminate platform-specific pool of "thnum" threads.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging.
 */
static
rt_void term_threads(rt_void *tdata, rt_cell thnum)
{

}

/*
 * Task platform-specific pool of "thnum" threads to update scene,
 * block until finished.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging. Simulate threading with sequential run.
 */
static
rt_void update_scene(rt_void *tdata, rt_cell thnum, rt_cell phase)
{
    rt_Scene *scn = (rt_Scene *)tdata;

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        scn->update_slice(i, phase);
    }
}

/*
 * Task platform-specific pool of "thnum" threads to render scene,
 * block until finished.
 * Local stub below is used when platform threading functions are not provided
 * or during state-logging. Simulate threading with sequential run.
 */
static
rt_void render_scene(rt_void *tdata, rt_cell thnum, rt_cell phase)
{
    rt_Scene *scn = (rt_Scene *)tdata;

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        scn->render_slice(i, phase);
    }
}

/******************************************************************************/
/**********************************   SCENE   *********************************/
/******************************************************************************/

/*
 * Instantiate scene.
 * Can only be called from single (main) thread.
 */
rt_Scene::rt_Scene(rt_SCENE *scn, /* frame ptr must be SIMD-aligned or NULL */
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
    this->scn = scn;

    /* check for locked scene data, not thread safe! */
    if (scn->lock != RT_NULL)
    {
        throw rt_Exception("scene data is locked by another instance");
    }

    /* x_row, frame's stride (in 32-bit pixels, not bytes!),
     * can be greater than x_res, in which case the frame
     * occupies only a portion (rectangle) of the framebuffer,
     * or negative, in which case frame starts at the last line
     * and consecutive lines are located backwards in memory,
     * x_row must contain the whole number of SIMD widths */
    if (x_res == 0 || RT_ABS(x_row) < x_res
    ||  y_res == 0 || RT_ABS(x_row) & (RT_SIMD_WIDTH - 1))
    {
        throw rt_Exception("frambuffer's dimensions are not valid");
    }

    /* init framebuffer's dimensions and pointer */
    this->x_res = x_res;
    this->y_res = y_res;
    this->x_row = x_row;

    if (frame == RT_NULL)
    {
        frame = (rt_word *)
                alloc(RT_ABS(x_row) * y_res * sizeof(rt_word), RT_SIMD_ALIGN);

        if (x_row < 0)
        {
            frame += RT_ABS(x_row) * (y_res - 1);
        }
    }
    else
    if ((rt_word)frame & (RT_SIMD_ALIGN - 1) != 0)
    {
        throw rt_Exception("frame pointer is not simd-aligned in scene");
    }

    this->frame = frame;

    /* init tilebuffer's dimensions and pointer */
    tile_w = RT_TILE_W;
    tile_h = RT_TILE_H;

    tiles_in_row = (x_res + tile_w - 1) / tile_w;
    tiles_in_col = (y_res + tile_h - 1) / tile_h;

    tiles = (rt_ELEM **)
            alloc(sizeof(rt_ELEM *) * (tiles_in_row * tiles_in_col), RT_ALIGN);

    /* init aspect ratio, rays depth and fsaa */
    factor = 1.0f / (rt_real)x_res;
    aspect = (rt_real)y_res * factor;

    depth = RT_STACK_DEPTH;
    fsaa  = RT_FSAA_NO;

    /* instantiate objects hierarchy */
    memset(&rootobj, 0, sizeof(rt_OBJECT));

    rootobj.trm.scl[RT_I] = 1.0f;
    rootobj.trm.scl[RT_J] = 1.0f;
    rootobj.trm.scl[RT_K] = 1.0f;
    rootobj.obj = scn->root;

    if (scn->root.tag != RT_TAG_ARRAY)
    {
        throw rt_Exception("scene's root is not an array");
    }

    root = new rt_Array(this, RT_NULL, &rootobj); /* also init *_num fields */

    if (cam_head == RT_NULL)
    {
        throw rt_Exception("scene doesn't contain camera");
    }

    cam = cam_head;

    /* lock scene data, when scene constructor can no longer fail */
    scn->lock = this;

    /* create scene threads array */
    thnum = RT_THREADS_NUM;

    tharr = (rt_SceneThread **)
            alloc(sizeof(rt_SceneThread *) * thnum, RT_ALIGN);

    rt_cell i;

    for (i = 0; i < thnum; i++)
    {
        tharr[i] = new rt_SceneThread(this, i);

        /* estimate per frame allocs to reduce system calls per thread */
        tharr[i]->msize =  /* upper bound per surface for tiling */
            (tiles_in_row * tiles_in_col + /* plus reflections/refractions */
            2 * (srf_num + arr_num * 2) + /* plus lights and shadows */
            2 * (lgt_num * (1 + srf_num + arr_num * 2))) * /* per side */
            sizeof(rt_ELEM) * (srf_num + thnum - 1) / thnum; /* per thread */
    }

    /* init memory pool in the heap for temporary per-frame allocs */
    mpool = RT_NULL; /* rough estimate for surface relations/templates */
    msize = ((srf_num + 1) * (srf_num + 1) * 2 + /* plus two surface lists */
            2 * (srf_num + arr_num) + /* plus one lights and shadows list */
            lgt_num * (1 + srf_num + arr_num * 2) + /* plus array nodes */
            tiles_in_row * tiles_in_col * arr_num) *  /* for tiling */
            sizeof(rt_ELEM);                        /* for main thread */

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

    /* create platform-specific worker threads */
    tdata = this->f_init(thnum, this);

    /* init rendering backend */
    render0(tharr[0]->s_inf);
}

/*
 * Update current camera with given "action" for a given "time".
 */
rt_void rt_Scene::update(rt_long time, rt_cell action)
{
    cam->update(time, action);
}

/*
 * Update backend data structures and render frame for a given "time".
 */
rt_void rt_Scene::render(rt_long time)
{
    rt_cell i;

    /* reserve memory for temporary per-frame allocs */
    mpool = reserve(msize, RT_ALIGN);

    for (i = 0; i < thnum; i++)
    {
        tharr[i]->mpool = tharr[i]->reserve(tharr[i]->msize, RT_ALIGN);
    }

    /* print state beg */
    if (g_print)
    {
        RT_PRINT_STATE_BEG();

        RT_PRINT_TIME(time);
    }

    /* update the whole objects hierarchy */
    root->update(time, iden4, RT_UPDATE_FLAG_OBJ);

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
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && g_print == RT_FALSE)
    {
        this->f_update(tdata, thnum, 1);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        if (g_print)
        {
            RT_PRINT_CAM(cam);
        }

        update_scene(this, thnum, 1);
    }

    root->update_bounds();

    /* rebuild global surface list */
    slist = tharr[0]->ssort(cam);

    /* rebuild global light/shadow list,
     * slist is needed inside */
    llist = tharr[0]->lsort(cam);

#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0 && g_print == RT_FALSE)
    {
        this->f_update(tdata, thnum, 2);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        if (g_print)
        {
            RT_PRINT_LGT_LST(llist);

            RT_PRINT_SRF_LST(slist);
        }

        update_scene(this, thnum, 2);
    }

    /* screen tiling */
    rt_cell tline;
    rt_cell j;

#if RT_OPTS_TILING != 0
    if ((opts & RT_OPTS_TILING) != 0)
    {
        memset(tiles, 0, sizeof(rt_ELEM *) * tiles_in_row * tiles_in_col);

        rt_ELEM *elm, *nxt, *stail = RT_NULL, **ptr = &stail;

        /* build exact copy of reversed slist (should be cheap),
         * trnode elements become tailing rather than heading,
         * elements grouping for cached transform is retained */
        for (nxt = slist; nxt != RT_NULL; nxt = nxt->next)
        {
            /* alloc new element as nxt copy */
            elm = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
            elm->data = nxt->data;
            elm->simd = nxt->simd;
            elm->temp = nxt->temp;
            /* insert element as list head */
            elm->next = *ptr;
           *ptr = elm;
        }

        /* traverse reversed slist to keep original slist order
         * and optimize trnode handling for each tile */
        for (elm = stail; elm != RT_NULL; elm = elm->next)
        {
            rt_Object *obj = (rt_Object *)elm->temp;

            /* skip trnode elements from reversed slist
             * as they are handled separately for each tile */
            if (RT_IS_ARRAY(obj))
            {
                continue;
            }

            rt_Surface *srf = (rt_Surface *)elm->temp;

            rt_ELEM *tls = (rt_ELEM *)srf->s_srf->msc_p[0], *trn;

            if (srf->trnode != RT_NULL && srf->trnode != srf)
            {
                for (; tls != RT_NULL; tls = nxt)
                {
                    i = (rt_word)tls->data >> 16;
                    j = (rt_word)tls->data & 0xFFFF;

                    nxt = tls->next;

                    tls->data = 0;

                    tline = i * tiles_in_row;

                    /* check matching existing trnode for insertion,
                     * only tile list head needs to be checked as elements
                     * grouping for cached transform is retained from slist */
                    trn = tiles[tline + j];

                    if (trn != RT_NULL && trn->temp == srf->trnode)
                    {
                        /* insert element under existing trnode */
                        tls->next = trn->next;
                        trn->next = tls;
                    }
                    else
                    {
                        /* insert element as list head */
                        tls->next = tiles[tline + j];
                        tiles[tline + j] = tls;

                        rt_Array *arr = (rt_Array *)srf->trnode;

                        /* alloc new trnode element as none has been found */
                        trn = (rt_ELEM *)alloc(sizeof(rt_ELEM), RT_ALIGN);
                        trn->data = (rt_cell)tls; /* trnode's last elem */
                        trn->simd = arr->s_srf;
                        trn->temp = arr;
                        /* insert element as list head */
                        trn->next = tiles[tline + j];
                        tiles[tline + j] = trn;
                    }
                }
            }
            else
            {
                for (; tls != RT_NULL; tls = nxt)
                {
                    i = (rt_word)tls->data >> 16;
                    j = (rt_word)tls->data & 0xFFFF;

                    nxt = tls->next;

                    tls->data = 0;

                    tline = i * tiles_in_row;

                    /* insert element as list head */
                    tls->next = tiles[tline + j];
                    tiles[tline + j] = tls;
                }
            }
        }

        if (g_print)
        {
            rt_cell i = 0, j = 0;

            tline = i * tiles_in_row;

            RT_PRINT_TLS_LST(tiles[tline + j], i, j);
        }
    }
    else
#endif /* RT_OPTS_TILING */
    {
        for (i = 0; i < tiles_in_col; i++)
        {
            tline = i * tiles_in_row;

            for (j = 0; j < tiles_in_row; j++)
            {
                tiles[tline + j] = slist;
            }
        }
    }

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
#if RT_OPTS_THREAD != 0
    if ((opts & RT_OPTS_THREAD) != 0)
    {
        this->f_render(tdata, thnum, 0);
    }
    else
#endif /* RT_OPTS_THREAD */
    {
        render_scene(this, thnum, 0);
    }

    /* print state end */
    if (g_print)
    {
        RT_PRINT_STATE_END();

        g_print = RT_FALSE;
    }

    /* release memory from temporary per-frame allocs */
    for (i = 0; i < thnum; i++)
    {
        tharr[i]->release(tharr[i]->mpool);
    }

    release(mpool);
}

/*
 * Update portion of the scene with given "index"
 * as part of the multi-threaded update.
 */
rt_void rt_Scene::update_slice(rt_cell index, rt_cell phase)
{
    rt_cell i;

    rt_Surface *srf = RT_NULL;

    if (phase == 1)
    {
        for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            srf->update(0, RT_NULL, RT_UPDATE_FLAG_SRF);

            /* rebuild per-surface tile list */
            tharr[index]->stile(srf);
        }
    }
    else
    if (phase == 2)
    {
        for (srf = srf_head, i = 0; srf != RT_NULL; srf = srf->next, i++)
        {
            if ((i % thnum) != index)
            {
                continue;
            }

            if (g_print)
            {
                RT_PRINT_SRF(srf);
            }

            /* rebuild per-surface surface lists */
            tharr[index]->ssort(srf);

            /* rebuild per-surface light/shadow lists */
            tharr[index]->lsort(srf);

            /* update per-surface backend-related parts */
            update0(srf->s_srf);
        }
    }
}

/*
 * Render portion of the frame with given "index"
 * as part of the multi-threaded render.
 */
rt_void rt_Scene::render_slice(rt_cell index, rt_cell phase)
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
 * Set fullscreen anti-aliasing mode.
 */
rt_void rt_Scene::set_fsaa(rt_cell fsaa)
{
    this->fsaa = fsaa;
}

/*
 * Set runtime optimization flags.
 */
rt_void rt_Scene::set_opts(rt_cell opts)
{
    this->opts = opts;

    /* trigger full hierarchy update,
     * safe to reset time as rootobj never has animator,
     * rootobj time is restored within the update */
    rootobj.time = -1;
}

/*
 * Print current state.
 */
rt_void rt_Scene::print_state()
{
    g_print = RT_TRUE;
}

/*
 * Destroy scene.
 */
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

    /* unlock scene data */
    scn->lock = RT_NULL;
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
        dst = frame + y * x_row + x + (c * d - 1 - i) * dW * z;

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

                dst += x_row - dW * z;
                src -= dW;
            }

            src += dW;
        }
    }
}

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

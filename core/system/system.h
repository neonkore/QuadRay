/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_SYSTEM_H
#define RT_SYSTEM_H

#include <stdio.h>
#include <stdarg.h>

#include "rtarch.h"
#include "rtbase.h"
#include "rtconf.h"

/* Classes */

class rt_File;
class rt_Heap;
class rt_Exception;

template <class rt_Class>
class rt_List;

/******************************************************************************/
/**********************************   FILE   **********************************/
/******************************************************************************/

class rt_File
{
/*  fields */

    FILE               *file;

/*  methods */

    public:

    rt_File(rt_pstr name, rt_pstr mode);

    virtual
   ~rt_File();

    rt_cell seek(rt_cell offset, rt_cell origin);
    rt_word read(rt_pntr data, rt_word size, rt_word num);
    rt_word write(rt_pntr data, rt_word size, rt_word num);
    rt_cell print(rt_pstr format, ...);
    rt_cell vprint(rt_pstr format, va_list args);
    rt_cell error(); /* 0 - no error */
};

/******************************************************************************/
/**********************************   HEAP   **********************************/
/******************************************************************************/

struct rt_CHUNK
{
    rt_byte            *ptr;
    rt_byte            *end;
    rt_word             size;
    rt_CHUNK           *next;
};

class rt_Heap
{
/*  fields */

    rt_CHUNK           *head;

    rt_void chunk_alloc(rt_word size, rt_word align);

    protected:

    rt_FUNC_ALLOC      f_alloc;
    rt_FUNC_FREE       f_free;

/*  methods */

    public:

    rt_Heap(rt_FUNC_ALLOC f_alloc, rt_FUNC_FREE f_free);

    virtual
   ~rt_Heap();

    rt_pntr alloc(rt_word size, rt_word align);
    rt_pntr reserve(rt_word size, rt_word align);
    rt_pntr release(rt_pntr ptr);
};

/******************************************************************************/
/*******************************   EXCEPTION   ********************************/
/******************************************************************************/

class rt_Exception
{
    public:

/*  fields */

    rt_pstr err;

/*  methods */

    rt_Exception(rt_pstr err) { this->err = err; }

    virtual
   ~rt_Exception() { }
};

/******************************************************************************/
/**********************************   LIST   **********************************/
/******************************************************************************/

template <class rt_Class>
class rt_List
{
    public:

/*  fields */

    rt_Class           *next;

/*  methods */

    rt_List(rt_Class *next) { this->next = next; }
};

/******************************************************************************/
/*********************************   LOGGING   ********************************/
/******************************************************************************/

extern rt_bool              g_print;

extern rt_File              g_log_file;
extern rt_File              g_err_file;

typedef rt_void (*rt_FUNC_PRINT_LOG)(rt_pstr format, ...);
typedef rt_void (*rt_FUNC_PRINT_ERR)(rt_pstr format, ...);

extern rt_FUNC_PRINT_LOG    f_print_log;
extern rt_FUNC_PRINT_ERR    f_print_err;

#define RT_LOGI             f_print_log
#define RT_LOGE             f_print_err

class rt_LogRedirect /* must be first in scene init */
{
    public:

/*  fields */

/*  methods */

    rt_LogRedirect(rt_FUNC_PRINT_LOG f_print_log,
                   rt_FUNC_PRINT_ERR f_print_err)
    { 
        if (f_print_log != RT_NULL)
          ::f_print_log = f_print_log;

        if (f_print_err != RT_NULL)
          ::f_print_err = f_print_err;
    }
};

#endif /* RT_SYSTEM_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

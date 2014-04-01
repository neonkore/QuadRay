/******************************************************************************/
/* Copyright (c) 2013 VectorChief (at github, bitbucket, sourceforge)         */
/* Distributed under the MIT software license, see the accompanying           */
/* file COPYING or http://www.opensource.org/licenses/mit-license.php         */
/******************************************************************************/

#ifndef RT_RTCONF_H
#define RT_RTCONF_H

#define RT_STACK_DEPTH      10
#define RT_THREADS_NUM      16

#define RT_CHUNK_SIZE       4096

#define RT_PATH_TEXTURES    "data/textures/"
#define RT_PATH_DUMP_LOG    "dump/log.txt"
#define RT_PATH_DUMP_ERR    "dump/err.txt"

#if RT_EMBED_FILEIO == 1
#define RT_EMBED_TEX        1
#endif /* RT_EMBED_FILEIO */

/* Optimization flags,
 * define particular flag as 0
 * to turn respective optimization off
 * at compile time */

#define RT_OPTS_THREAD      (1 << 0)

#endif /* RT_RTCONF_H */

/******************************************************************************/
/******************************************************************************/
/******************************************************************************/

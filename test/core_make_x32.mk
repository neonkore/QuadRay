
INC_PATH =                              \
        -I../core/config/               \
        -I../core/engine/               \
        -I../core/system/               \
        -I../core/tracer/               \
        -I../data/materials/            \
        -I../data/objects/              \
        -I../data/textures/             \
        -Iscenes/

SRC_LIST =                              \
        ../core/engine/engine.cpp       \
        ../core/engine/object.cpp       \
        ../core/engine/rtgeom.cpp       \
        ../core/engine/rtimag.cpp       \
        ../core/system/system.cpp       \
        ../core/tracer/tracer.cpp       \
        ../core/tracer/tracer_128v1.cpp \
        ../core/tracer/tracer_128v2.cpp \
        ../core/tracer/tracer_256v1.cpp \
        ../core/tracer/tracer_256v2.cpp \
        core_test.cpp

core_test:
	g++ -O3 -g -mx32 \
        -DRT_LINUX -DRT_X32 -DRT_128=1+2 -DRT_256=1+2 \
        -DRT_DEBUG=1 -DRT_PATH="../" \
        -DRT_EMBED_STDOUT=0 -DRT_EMBED_FILEIO=0 -DRT_EMBED_TEX=1 \
        ${INC_PATH} ${SRC_LIST} -o core_test.x32

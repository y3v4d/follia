#ifndef __3DMATH_H__
#define __3DMATH_H__

#define PI 3.14159

typedef struct {
    float x;
    float y;
} vec2f;

typedef struct {
    int x;
    int y;
} vec2i;

typedef struct {
    float x;
    float y;
    float z;
} vec3f;

typedef struct {
    int x;
    int y;
    int z;
} vec3i;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4f;

typedef struct {
    int x;
    int y;
    int z;
    int w;
} vec4i;

typedef struct {
    float m[9];
} mat3;

typedef struct {
    float m[16];
} mat4;

#endif

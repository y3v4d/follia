#include "follia.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#define PI 3.14159

typedef struct {
    float x;
    float y;
} vec2f;

typedef struct {
    float x;
    float y;
    float z;
} vec3f;

typedef struct {
    float x;
    float y;
    float z;
    float w;
} vec4f;

typedef struct {
    float m[9];
} mat3;

typedef struct {
    float m[16];
} mat4;

mat4 get_perspective_projection(float fov) {
    mat4 temp = {
        { 
            1.f / tan((fov / 2.f) * (PI / 180.f)), 0,                                     0, 0,
            0,                                     1.f / tan((fov / 2.f) * (PI / 180.f)), 0, 0,
            0,                                     0,                                     1, 0,
            0,                                     0,                                     1, 0
        }
    };

    return temp;
}

vec4f mat4_mul_vec4f(const mat4 *m, const vec3f *p, float w) {
    vec4f temp = {
        m->m[0]  * p->x + m->m[1]  * p->y + m->m[2]  * p->z + m->m[3]  * w,
        m->m[4]  * p->x + m->m[5]  * p->y + m->m[6]  * p->z + m->m[7]  * w,
        m->m[8]  * p->x + m->m[9]  * p->y + m->m[10] * p->z + m->m[11] * w,
        m->m[12] * p->x + m->m[13] * p->y + m->m[14] * p->z + m->m[15] * w
    };

    return temp;
}

vec3f homogoneus_to_cartesian(const vec4f *p) {
    vec3f temp = {
        p->x / p->w,
        p->y / p->w,
        p->z / p->w
    };

    return temp;
}

vec2f camera_to_screen(const vec3f *p) {
    int W2 = FL_GetWindowWidth() >> 1;
    int H2 = FL_GetWindowHeight() >> 1;

    vec2f temp = {
        W2 + W2 * p->x,
        H2 - H2 * p->y
    };

    return temp;
}

int main() {
    if(!FL_Initialize(640, 480))
        return -1;

    FL_SetTitle("Follia - 3D Example");

    FL_FontBDF *knxt = FL_LoadFontBDF("data/fonts/knxt.bdf");
    FL_FontBDF *bitocra_39 = FL_LoadFontBDF("data/fonts/bitocra-39.bdf");

    if(!knxt || !bitocra_39) {
        if(!knxt) FL_FreeFontBDF(knxt);
        if(!bitocra_39) FL_FreeFontBDF(bitocra_39);
        
        FL_Close();
        return -1;
    }

    vec3f points[3] = {
        { -0.5f,  0.5f, 1.0f },
        {  0.5f,  0.5f, 1.0f },
        {  0.5f, -0.5f, 1.0f }
    };

    vec3f camera_position = { 0.f, 0.f, 0.f };
    vec3f camera_velocity = { 0.f, 0.f, 0.f };
    mat4 perspective_projection = get_perspective_projection(90.f);

    const int fps_text_s = 64;
    char fps_text[fps_text_s];

    FL_Event event;
    while(!FL_WindowShouldClose()) {
        while(FL_GetEvent(&event)) {
            if(event.type == FL_EVENT_KEY_PRESSED) {
                const float CAM_SPEED = 0.001f;
                switch(event.key.code) {
                    case 'w': camera_velocity.z = CAM_SPEED; break;
                    case 's': camera_velocity.z = -CAM_SPEED; break;
                    case 'a': camera_velocity.x = -CAM_SPEED; break;
                    case 'd': camera_velocity.x = CAM_SPEED; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case 'w': case 's': camera_velocity.z = 0.f; break;
                    case 'a': case 'd': camera_velocity.x = 0.f; break;
                    default: break;
                }
            } else if(event.type == FL_EVENT_MOUSE_PRESSED) {
            }
        }

        camera_position.x += camera_velocity.x * FL_GetDeltaTime();
        camera_position.y += camera_velocity.y * FL_GetDeltaTime();
        camera_position.z += camera_velocity.z * FL_GetDeltaTime();

        snprintf(fps_text, fps_text_s, "MS: %.2f\nFPS: %.2f", FL_GetDeltaTime(), 1000.0 / FL_GetDeltaTime());

        FL_ClearScreen();
        { // render triangles
            for(int p = 0; p < 1; ++p) {
                vec3f translated[3] = {
                    {
                        points[p * 3].x - camera_position.x,
                        points[p * 3].y - camera_position.y,
                        points[p * 3].z - camera_position.z
                    },
                    {
                        points[p * 3 + 1].x - camera_position.x,
                        points[p * 3 + 1].y - camera_position.y,
                        points[p * 3 + 1].z - camera_position.z
                    },
                    {
                        points[p * 3 + 2].x - camera_position.x,
                        points[p * 3 + 2].y - camera_position.y,
                        points[p * 3 + 2].z - camera_position.z
                    }
                };

                vec4f homo[3] = {
                    mat4_mul_vec4f(&perspective_projection, &translated[0], 1.0f),
                    mat4_mul_vec4f(&perspective_projection, &translated[1], 1.0f),
                    mat4_mul_vec4f(&perspective_projection, &translated[2], 1.0f)
                };

                vec3f cartesian[3] = {
                    homogoneus_to_cartesian(&homo[0]),
                    homogoneus_to_cartesian(&homo[1]),
                    homogoneus_to_cartesian(&homo[2])
                };

                vec2f screen[3] = {
                    camera_to_screen(&cartesian[0]),
                    camera_to_screen(&cartesian[1]),
                    camera_to_screen(&cartesian[2])
                };

                for(int i = 0; i < 3; ++i) {
                    vec2f *p0 = &screen[i];
                    vec2f *p1 = &screen[(i < 2 ? i + 1 : 0)]; 

                    FL_DrawLine(p0->x, p0->y, p1->x, p1->y, 0xff0000);
                }
            }
        }

        FL_DrawTextBDF(10, 10, fps_text, fps_text_s, FL_GetWindowWidth(), knxt);
        FL_Render();
    }

    FL_FreeFontBDF(bitocra_39);
    FL_FreeFontBDF(knxt);

    FL_Close();
    return 0;
}

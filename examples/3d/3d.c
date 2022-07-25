#include "x11framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "3dmath.h"

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
    int W2 = XF_GetWindowWidth() >> 1;
    int H2 = XF_GetWindowHeight() >> 1;

    vec2f temp = {
        W2 + W2 * p->x,
        H2 - H2 * p->y
    };

    return temp;
}

int main() {
    if(!XF_Initialize(640, 480))
        return -1;

    XF_SetTitle("Follia - 3D Example");

    XF_FontBDF *knxt = XF_LoadFontBDF("data/fonts/knxt.bdf");
    XF_FontBDF *bitocra_39 = XF_LoadFontBDF("data/fonts/bitocra-39.bdf");

    if(!knxt || !bitocra_39) {
        if(!knxt) XF_FreeFontBDF(knxt);
        if(!bitocra_39) XF_FreeFontBDF(bitocra_39);
        
        XF_Close();
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

    XF_Event event;
    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                const float CAM_SPEED = 0.001f;
                switch(event.key.code) {
                    case 'w': camera_velocity.z = CAM_SPEED; break;
                    case 's': camera_velocity.z = -CAM_SPEED; break;
                    case 'a': camera_velocity.x = -CAM_SPEED; break;
                    case 'd': camera_velocity.x = CAM_SPEED; break;
                    default: break;
                }
            } else if(event.type == XF_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case 'w': case 's': camera_velocity.z = 0.f; break;
                    case 'a': case 'd': camera_velocity.x = 0.f; break;
                    default: break;
                }
            } else if(event.type == XF_EVENT_MOUSE_PRESSED) {
            }
        }

        camera_position.x += camera_velocity.x * XF_GetDeltaTime();
        camera_position.y += camera_velocity.y * XF_GetDeltaTime();
        camera_position.z += camera_velocity.z * XF_GetDeltaTime();

        snprintf(fps_text, fps_text_s, "MS: %.2f\nFPS: %.2f", XF_GetDeltaTime(), 1000.0 / XF_GetDeltaTime());

        XF_ClearScreen();
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

                    XF_DrawLine(p0->x, p0->y, p1->x, p1->y, 0xff0000);
                }
            }
        }

        XF_DrawText(10, 10, fps_text, fps_text_s, XF_GetWindowWidth(), knxt);
        XF_Render();
    }

    XF_FreeFontBDF(bitocra_39);
    XF_FreeFontBDF(knxt);

    XF_Close();
    return 0;
}

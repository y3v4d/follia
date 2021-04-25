#include "x11framework.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#define ALIENS_HOR 8 
#define ALIENS_VER 4
#define ALIENS_TOTAL ALIENS_HOR * ALIENS_VER
#define PLAYER_SPEED 0.5
#define BULLET_SPEED 0.7

struct Object {
    float x, y;
    float w, h;
    float v;

    int lives;
    boolean is_bottom;
};

struct Object* create_bullet(float x, float y) {
    struct Object* temp = (struct Object*)malloc(sizeof(struct Object));

    temp->w = 4; 
    temp->h = 16;

    temp->x = x - temp->w / 2;
    temp->y = y - temp->h / 2;

    return temp;
}

int main() {
    if(!XF_Initialize(640, 480))
        exit(-1);

    srand(time(NULL));

    XF_FontBDF* knxt_font = XF_LoadFontBDF("data/fonts/knxt.bdf");
    if(!knxt_font) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load knxt font!\n");
        
        XF_Close();
        return -1;
    }

    XF_Texture* ship_texture = XF_LoadBMP("data/space-invaders/ship.bmp");
    XF_Texture* alien0_texture = XF_LoadBMP("data/space-invaders/alien0.bmp");
    XF_Texture* alien1_texture = XF_LoadBMP("data/space-invaders/alien1.bmp");
    if(!ship_texture || !alien0_texture || !alien1_texture) {
        XF_WriteLog(XF_LOG_ERROR, "Couldn't load images!\n");

        if(ship_texture) XF_FreeTexture(ship_texture);
        if(alien0_texture) XF_FreeTexture(alien0_texture);
        if(alien1_texture) XF_FreeTexture(alien1_texture);

        XF_FreeFontBDF(knxt_font);

        XF_Close();
        return -1;
    }

    struct Object player = { 0.f, 0.f, (float)ship_texture->width, (float)ship_texture->height, 0.f, 3 };
    player.x = ((float)XF_GetWindowWidth() - player.w) / 2.f;
    player.y = (float)XF_GetWindowHeight() - 2.f * player.h;

    char lives_text[64];
    snprintf(lives_text, 64, "Lives: %d", player.lives);

    struct Object* bullets[2] = { NULL, NULL };

    const int TOTAL_ALIENS = ALIENS_HOR * ALIENS_VER;
    struct Object* aliens[TOTAL_ALIENS];

    const float ALIENS_GAP = 16;
    const int ALIENS_LAST_LAYER_START = TOTAL_ALIENS - ALIENS_HOR;
    for(int i = 0; i < TOTAL_ALIENS; ++i) {
        aliens[i] = (struct Object*)malloc(sizeof(struct Object));

        aliens[i]->w = (float)alien0_texture->width;
        aliens[i]->h = (float)alien0_texture->height;

        aliens[i]->x = (i % ALIENS_HOR) * (aliens[i]->w + ALIENS_GAP); 
        aliens[i]->y = (i / ALIENS_HOR) * (aliens[i]->h * 1.5f);

        aliens[i]->v = 0.05f;

        if(i >= ALIENS_LAST_LAYER_START) aliens[i]->is_bottom = true;
        else aliens[i]->is_bottom = false;
    }

    int aliens_on_bottom = ALIENS_HOR;
    boolean alien_walk_animation = false;

    XF_Timer alien_timer;
    XF_StartTimer(&alien_timer);

    XF_Timer alien_walk_timer;
    XF_StartTimer(&alien_walk_timer);

    XF_SetClearColor(0);
    XF_SetTextColor(0xffffff);

    XF_Event event;
    while(!XF_WindowShouldClose()) {
        while(XF_GetEvent(&event)) {
            if(event.type == XF_EVENT_KEY_PRESSED) {
                switch(event.key.code) {
                    case 'a': player.v = -PLAYER_SPEED * XF_GetDeltaTime(); break;
                    case 'd': player.v = PLAYER_SPEED * XF_GetDeltaTime(); break;
                    case ' ':
                        if(bullets[0] == NULL) {
                            bullets[0] = create_bullet(player.x + player.w / 2, player.y + player.h / 2);
                            bullets[0]->v = -BULLET_SPEED;
                        }

                        break;
                    default: break;
                }
            } else if(event.type == XF_EVENT_KEY_RELEASED) {
                switch(event.key.code) {
                    case 'a': case 'd': player.v = 0.f; break;
                    default: break;
                }
            }
        }

        player.x += player.v;
        if(bullets[0] != NULL) {
            bullets[0]->y += bullets[0]->v;

            if(bullets[0]->y + bullets[0]->h < 0) {
                free(bullets[0]);
                bullets[0] = NULL;
            }
        }

        for(int i = 0; i < TOTAL_ALIENS; ++i) {
            if(aliens[i] != NULL) {
                aliens[i]->x += aliens[i]->v;
                if(bullets[0] == NULL) continue;

                // check if bullet hit the alien
                if(bullets[0]->x + bullets[0]->w >= aliens[i]->x && bullets[0]->x <= aliens[i]->x + aliens[i]->w &&
                   bullets[0]->y + bullets[0]->h >= aliens[i]->y && bullets[0]->y <= aliens[i]->y + aliens[i]->h) {
                    free(bullets[0]); bullets[0] = NULL;

                    // assign bottom to another alien, or substract aliens_on_bottom when there are no more aliens behind
                    if(aliens[i]->is_bottom) {
                        int new_bottom = i - ALIENS_HOR; 
                        boolean success = false;
                        
                        while(new_bottom >= 0) {
                            if(aliens[new_bottom] != NULL) {
                                aliens[new_bottom]->is_bottom = true;
                                success = true;

                                break;
                            }
                            
                            new_bottom -= ALIENS_HOR;
                        }

                        if(!success) aliens_on_bottom--;
                    }
                    free(aliens[i]); aliens[i] = NULL;
                }
            }
        }

        // allow aliens to shoot bullets
        XF_StopTimer(&alien_timer);
        if(alien_timer.delta >= 1500 && bullets[1] == NULL && aliens_on_bottom > 0) {
            int r = rand() % aliens_on_bottom;

            int omitted = 0;
            for(int i = 0; i < TOTAL_ALIENS; ++i) {
                if(aliens[i] != NULL && aliens[i]->is_bottom) {
                    if(omitted == r) {
                        bullets[1] = create_bullet(aliens[i]->x + aliens[i]->w / 2, aliens[i]->y + aliens[i]->h / 2);
                        bullets[1]->v = BULLET_SPEED;

                        XF_StartTimer(&alien_timer);

                        break;
                    } else ++omitted;
                }
            }
        }

        if(player.x > XF_GetWindowWidth()) {
            player.x = -player.w;
        }

        if(bullets[1] != NULL) {
            bullets[1]->y += bullets[1]->v;

            if(bullets[1]->y > XF_GetWindowHeight()) {
                free(bullets[1]);
                bullets[1] = NULL;
            } else if(bullets[1]->x + bullets[1]->w >= player.x && bullets[1]->x <= player.x + player.w &&
               bullets[1]->y + bullets[1]->h >= player.y && bullets[1]->y <= player.y + player.h) {
                free(bullets[1]);
                bullets[1] = NULL;

                if(player.lives > 0) {
                    snprintf(lives_text, 64, "Lives: %d", --player.lives);
                }
            }
        }

        // 'bounce' aliens from screen edges
        for(int i = 0; i < TOTAL_ALIENS; ++i) {
            if(aliens[i] != NULL) {
                float offset = 0;

                if(aliens[i]->x + aliens[i]->w > XF_GetWindowWidth()) {
                    offset = aliens[i]->x + aliens[i]->w - (float)XF_GetWindowWidth();
                } else if(aliens[i]->x < 0) {
                    offset = aliens[i]->x;
                }

                if(offset != 0) {
                    for(int j = 0; j < TOTAL_ALIENS; ++j) {
                        if(aliens[j] != NULL) {
                            aliens[j]->v *= -1.f;
                            aliens[j]->x -= offset;
                            aliens[j]->y += aliens[j]->h / 2.f;
                        }
                    }

                    break;
                }
            }
        }

        XF_StopTimer(&alien_walk_timer);
        if(alien_walk_timer.delta >= 500) {
            alien_walk_animation = !alien_walk_animation;
            XF_StartTimer(&alien_walk_timer);
        }

        XF_ClearScreen();
        for(int i = 0; i < 2; ++i)
            if(bullets[i] != NULL) XF_DrawRect(bullets[i]->x, bullets[i]->y, bullets[i]->w, bullets[i]->h, 0xffffff, false);

        for(int i = 0; i < TOTAL_ALIENS; ++i) {
            if(aliens[i] != NULL) {
                XF_DrawTexture((!alien_walk_animation ? alien0_texture : alien1_texture), aliens[i]->x, aliens[i]->y);
            }
        }

        XF_DrawTexture(ship_texture, player.x, player.y);

        XF_DrawText(10, 10, lives_text, 16, 200, knxt_font);
        XF_Render();
    }

    for(int i = 0; i < 2; ++i) if(bullets[i] != NULL) free(bullets[i]);
    for(int i = 0; i < TOTAL_ALIENS; ++i) if(aliens[i] != NULL) free(aliens[i]);

    XF_FreeFontBDF(knxt_font);
    XF_FreeTexture(ship_texture);
    XF_FreeTexture(alien0_texture);
    XF_FreeTexture(alien1_texture);
    
    XF_Close();
    return 0;
}

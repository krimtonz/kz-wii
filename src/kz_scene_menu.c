#include <stdlib.h>
#include <libundermine.h>
#include "kz.h"
#include "kzresource.h"

static int room_idx;

static int collision_gen_event(event_handler_t *handler, menu_event_t event, void **event_data){
    if(event == MENU_EVENT_ACTIVATE){
        if(kz.collision_view_status == COL_VIEW_NONE){
            kz.collision_view_status = COL_VIEW_GENERATE;
        }else{
            kz.collision_view_status = COL_VIEW_KILL;
        }
    }else if (event == MENU_EVENT_UPDATE){
        menu_checkbox_set(handler->subscriber, kz.collision_view_status == COL_VIEW_SHOW);
    }
    return 1;
}

static int collision_switch_event(event_handler_t *handler, menu_event_t event, void **event_data){
    if(event == MENU_EVENT_ACTIVATE){
        int d = (int)handler->callback_data;
        if(d == 0){
            settings->col_view_red = !settings->col_view_red;
            if(kz.collision_view_status == COL_VIEW_SHOW){
                kz.collision_view_status = COL_VIEW_REGENERATE;
            }
        }else if(d == 1){
            settings->col_view_opq = !settings->col_view_opq;
        }else{
            settings->col_view_upd = !settings->col_view_upd;
        }
    }else if(event == MENU_EVENT_UPDATE){
        int d = (int)handler->callback_data;
        int set = 0;
        if(d == 0){
            set = settings->col_view_red;
        }else if(d == 1){
            set = settings->col_view_opq;
        }else{
            set = settings->col_view_upd;
        }
        menu_checkbox_set(handler->subscriber, set);
    }
    return 1;
}

static int hitbox_gen_event(event_handler_t *handler, menu_event_t event, void **event_data){
    if(event == MENU_EVENT_ACTIVATE){
        if(kz.hitbox_view_status == COL_VIEW_NONE){
            kz.hitbox_view_status = COL_VIEW_GENERATE;
        }else{
            kz.hitbox_view_status = COL_VIEW_KILL;
        }
    }else if(event == MENU_EVENT_UPDATE){
        menu_checkbox_set(handler->subscriber,kz.hitbox_view_status == COL_VIEW_SHOW);
    }
    return 1;
}

static int hitbox_opaque_event(event_handler_t *handler, menu_event_t event, void **event_data){
    if(event == MENU_EVENT_ACTIVATE){
        settings->hit_view_opq = !settings->hit_view_opq;
    }else if(event == MENU_EVENT_UPDATE){
        menu_checkbox_set(handler->subscriber,settings->hit_view_opq);
    }
    return 1;
}

static int unload_room_onactivate(event_handler_t *handler, menu_event_t event, void **event_data){
    z2_game.room_ctx.rooms[0].idx = -1;
    z2_game.room_ctx.rooms[0].file = NULL;
    z2_UnloadRoom(&z2_game, &z2_game.room_ctx);
    z2_game.room_ctx.rooms[0].idx = -1;
    z2_game.room_ctx.rooms[0].file = NULL;
    return 1;
}

static int load_room_onactivate(event_handler_t *handler, menu_event_t event, void **event_data){
    if(room_idx == z2_game.room_ctx.rooms[0].idx){
        return unload_room_onactivate(handler, event, event_data);
    }
    if(room_idx >= z2_game.room_ctx.transition_cnt){
        return 1;
    }
    z2_game.room_ctx.rooms[0].idx = room_idx;
    z2_game.room_ctx.rooms[0].file = NULL;
    z2_UnloadRoom(&z2_game, &z2_game.room_ctx);
    z2_game.room_ctx.rooms[0].idx = room_idx;
    z2_game.room_ctx.rooms[0].file = NULL;
    z2_LoadRoom(&z2_game, &z2_game.room_ctx, room_idx);
    return 1;
}

static int hide_actors_onactivate(event_handler_t *handler, menu_event_t event, void **event_data){
    kz.hide_actors = !kz.hide_actors;
    return 1;
}

static int hide_room_onactivate(event_handler_t *handler, menu_event_t event, void **event_data){
    kz.hide_room = !kz.hide_room;
    return 1;
}

menu_t *create_scene_menu(void){
    static menu_t scene;
    static menu_t collision;
    menu_init(&scene, 0, 0);
    menu_padding_set(&scene,0,2);
    scene.selected_item = menu_button_add(&scene, 0, 0, "return", menu_return, NULL);

    menu_submenu_add(&scene, 0, 1, "collision", &collision);

    menu_item_t *item;
    // Generate collision menu
    {
        menu_init(&collision, 0, 0);
        menu_padding_set(&collision, 0, 2);
        collision.selected_item = menu_button_add(&collision, 0, 0, "return", menu_return, NULL);
        // Collision Viewer
        {
            item = menu_checkbox_add(&collision, 0, 1);
            menu_item_register_event(item, MENU_EVENT_ACTIVATE | MENU_EVENT_UPDATE, collision_gen_event, NULL);
            menu_label_add(&collision, 2, 1, "enable collision viewer");
            item = menu_checkbox_add(&collision, 2, 2);
            menu_item_register_event(item, MENU_EVENT_ACTIVATE | MENU_EVENT_UPDATE, collision_switch_event, (void*)0);
            menu_label_add(&collision, 4, 2, "reduce");
            item = menu_checkbox_add(&collision, 2, 3);
            menu_item_register_event(item, MENU_EVENT_ACTIVATE | MENU_EVENT_UPDATE, collision_switch_event, (void*)1);
            menu_label_add(&collision, 4, 3, "opaque");
            item = menu_checkbox_add(&collision, 2, 4);
            menu_item_register_event(item, MENU_EVENT_ACTIVATE | MENU_EVENT_UPDATE, collision_switch_event, (void*)2);
            menu_label_add(&collision, 4, 4, "update on scene change");
        }
        item = menu_checkbox_add(&collision, 0, 5);
        menu_item_register_event(item, MENU_EVENT_ACTIVATE, hide_room_onactivate, NULL);
        menu_label_add(&collision, 2, 5, "hide room");

        // hitbox viewer
        {
            item = menu_checkbox_add(&collision, 0, 6);
            menu_item_register_event(item, MENU_EVENT_ACTIVATE | MENU_EVENT_UPDATE, hitbox_gen_event, NULL);
            menu_label_add(&collision, 2, 6, "enable hitbox viewer");
            item = menu_checkbox_add(&collision, 2, 7);
            menu_item_register_event(item, MENU_EVENT_ACTIVATE | MENU_EVENT_UPDATE, hitbox_opaque_event, NULL);
            menu_label_add(&collision, 4, 7, "opaque");
        }
        item = menu_checkbox_add(&collision, 0, 8);
        menu_item_register_event(item, MENU_EVENT_ACTIVATE, hide_actors_onactivate, NULL);
        menu_label_add(&collision, 2, 8, "hide actors");
    }

    item = menu_label_add(&scene, 0, 2, "current room");
    item->color = COLOR_FADED;
    static watch_t cur_room;
    cur_room.address = &z2_game.room_ctx.rooms[0].idx;
    cur_room.type = WATCH_TYPE_U8;
    cur_room.floating = 0;
    item = menu_watch_add(&scene, 13, 2, &cur_room, 1);
    item->color = COLOR_FADED;
    item = menu_label_add(&scene, 0, 3, "num rooms");
    item->color = COLOR_FADED;
    static watch_t num_rooms;
    num_rooms.address = &z2_game.room_ctx.transition_cnt;
    num_rooms.type = WATCH_TYPE_U8;
    num_rooms.floating = 0;
    item = menu_watch_add(&scene, 13, 3, &num_rooms, 1);
    item->color = COLOR_FADED;

    menu_button_add(&scene, 0, 4, "unload room", unload_room_onactivate, NULL);
    menu_button_add(&scene, 0, 5, "load room", load_room_onactivate, NULL);
    item = menu_number_input_add(&scene, 13, 5, 10, 2);
    menu_item_register_event(item, MENU_EVENT_NUMBER, menu_number_word_event, &room_idx);

    return &scene;
}
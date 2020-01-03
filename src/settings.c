#include <stdlib.h>
#include "settings.h"
#include "kz.h"
#include "input.h"

static _Alignas(128) struct settings settings_info;
struct settings_data *settings = &settings_info.data;

void load_default_settings(){
    settings_info.header.magic[0] = 'k';
    settings_info.header.magic[1] = 'z';
    settings_info.header.magic[2] = 'k';
    settings_info.header.magic[3] = 'z';
    settings_info.header.version = FULL_SETTINGS;
    list_destroy(&kz.watches);
    list_init(&kz.watches,sizeof(watch_t));
    settings->watch_cnt = 0;
    settings->binds[KZ_CMD_TOGGLE_MENU] = make_bind(2, BUTTON_R, BUTTON_L);
    settings->binds[KZ_CMD_RETURN] = make_bind(2, BUTTON_R, BUTTON_D_LEFT);
    settings->binds[KZ_CMD_LEVITATE] = make_bind(1, BUTTON_L);
    settings->binds[KZ_CMD_TURBO] = make_bind(1, BUTTON_D_LEFT);
    settings->binds[KZ_CMD_FALL] = BIND_END;
    settings->binds[KZ_CMD_RELOAD] = BIND_END;
    settings->binds[KZ_CMD_VOID] = make_bind(2, BUTTON_D_LEFT, BUTTON_A);
    settings->binds[KZ_CMD_BREAK] = make_bind(2, BUTTON_D_RIGHT, BUTTON_L);
    settings->binds[KZ_CMD_PAUSE] = make_bind(1, BUTTON_D_UP);
    settings->binds[KZ_CMD_ADVANCE] = make_bind(1, BUTTON_D_DOWN);
    settings->binds[KZ_CMD_TOGGLE_INPUT] = BIND_END;
    settings->binds[KZ_CMD_RESET_LAG] = BIND_END;
    settings->binds[KZ_CMD_TIMER] = BIND_END;
    settings->binds[KZ_CMD_TIMER_RESET] = BIND_END;
    settings->binds[KZ_CMD_SAVE_MEMFILE] = make_bind(2, BUTTON_C_RIGHT, BUTTON_D_LEFT);
    settings->binds[KZ_CMD_LOAD_MEMFILE] = make_bind(2, BUTTON_C_RIGHT, BUTTON_D_RIGHT);
    settings->binds[KZ_CMD_NEXT_MEMFILE] = BIND_END;
    settings->binds[KZ_CMD_PREV_MEMFILE] = BIND_END;
    settings->binds[KZ_CMD_SAVE_POSITION] = make_bind(2, BUTTON_C_LEFT, BUTTON_D_LEFT);
    settings->binds[KZ_CMD_LOAD_POSITION] = make_bind(2, BUTTON_C_LEFT, BUTTON_D_RIGHT);
    settings->binds[KZ_CMD_NEXT_POSITION] = BIND_END;
    settings->binds[KZ_CMD_PREV_POSITION] = BIND_END;
    settings->binds[KZ_CMD_TITLE_SCREEN] = BIND_END;
    settings->input_display = 1;
    settings->id_x = 16;
    settings->id_y = Z2_SCREEN_HEIGHT - 20;
    settings->lag_counter = 0;
    settings->lag_x = Z2_SCREEN_WIDTH - 38;
    settings->lag_y = 20;
    settings->timer = 0;
    settings->timer_x = Z2_SCREEN_WIDTH - 100;
    settings->timer_y = 30;
    settings->menu_x = 20;
    settings->menu_y = 30;
    settings->cheats = 0;
    settings->memfile_action = 0;
    settings->turbo_type = 0;
    kz_log("default settings loaded");
}

void save_settings_to_flashram(int profile){
    settings->watch_cnt = kz.watches.size;
    if(kz.watches.first){
        int i=0;
        for(watch_t *watch = kz.watches.first;watch;watch = list_next(watch)){
            settings->watch_addresses[i] = (uint32_t)watch->address;
            settings->watch_x[i] = watch->x;
            settings->watch_y[i] = watch->y;
            settings->watch_info[i].floating = watch->floating;
            settings->watch_info[i].type = watch->type;
            if(watch->label){
                memcpy(settings->watch_labels[i],watch->label,20);
            }
            i++;
        }
    }
    struct settings *settings_temp = malloc(sizeof(*settings_temp) * SETTINGS_MAX);
    z2_dmaflashtoram(settings_temp,SIZE_TO_BLOCK(SETTINGS_ADDR),SIZE_TO_BLOCK(sizeof(*settings_temp) * SETTINGS_MAX));
    memcpy(settings_temp + profile,&settings_info,sizeof(settings_info));
    z2_dmaramtoflash(settings_temp,SIZE_TO_BLOCK(SETTINGS_ADDR),SIZE_TO_BLOCK(sizeof(*settings_temp) * SETTINGS_MAX));
    kz_log("saved settings profile %d",profile);
    free(settings_temp);
}

void load_settings_from_flashram(int profile){
    struct settings *settings_temp = malloc(sizeof(*settings_temp) * SETTINGS_MAX);
    z2_dmaflashtoram(settings_temp,SIZE_TO_BLOCK(SETTINGS_ADDR),SIZE_TO_BLOCK(sizeof(*settings_temp) * SETTINGS_MAX));
    struct settings *settings_profile = &settings_temp[profile];
    if(settings_profile->header.version>0 && settings_profile->header.version == FULL_SETTINGS){
        if(settings_profile->header.magic[0] == 'k' && settings_profile->header.magic[1]=='z' && settings_profile->header.magic[2] == 'k' && settings_profile->header.magic[3] == 'z'){
            memcpy((void*)&settings_info,(void*)settings_profile,sizeof(*settings_profile));
            kz_log("loaded settings profile %d",profile);
        }else{
            load_default_settings();
            kz_log("invalid magic");
        }
    }else{
        // if settings version is not the same as the current version, delete the profile from the save file, and load default settings
        // in the future might provide an update path? 
        load_default_settings();
        save_settings_to_flashram(profile);
        kz_log("invalid settings version");
    }
    free(settings_temp);
}

void kz_apply_settings(){
    if(kz.ready){
        clear_watches();
    }
    for(int i=0;i<settings->watch_cnt;i++){
        watch_t *watch = list_push_back(&kz.watches,NULL);
        watch->address = (void*)settings->watch_addresses[i];
        watch->x = settings->watch_x[i];
        watch->y = settings->watch_y[i];
        watch->floating = settings->watch_info[i].floating;
        watch->type = settings->watch_info[i].type;
        watch->label = malloc(21);
        memcpy(watch->label,settings->watch_labels[i],20);
        watch->label[20] = 0;
    }
    if(kz.ready){
        init_watch_rows();
    }
}

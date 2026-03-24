/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 *
 * Copyright (C) 2014 Jonathan Gordon
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY OF ANY
 * KIND, either express or implied.
 *
 ****************************************************************************/

/* mandatory include for all plugins */
#include "plugin.h"
#include <string.h>

static int menu_item_count;
#define MAX_ITEM_NAME 256
#define MAX_ITEMS 1000

struct items
{
    char *name;
    char string[MAX_ITEM_NAME];
};

static struct items menu_items[MAX_ITEMS];
static char** current_episodes = NULL;
static int current_podcast_index = -1;
static bool showing_episodes = false;

static char** get_episodes_list(int podcast_num)
{    
    char** episodes = rb->android_podcast_get_episode_list(podcast_num);
    if (episodes == NULL) {
        return NULL;
    }   
    return episodes;
}

static char** get_podcasts_list(void)
{
    char** podcasts = rb->android_podcast_get_podcast_names();
    if (podcasts == NULL) {
        return NULL;
    }
    
    return podcasts;
}

static const char * menu_get_name(int selected_item, void * data,
                                   char * buffer, size_t buffer_len)
{
    (void)data;

    rb->snprintf(buffer, buffer_len, "%s", menu_items[selected_item].string);
    
    return buffer;
}

static int menu_speak_item(int selected_item, void *data)
{
    (void) data;
    rb->talk_number(selected_item + 1, false);
    if (showing_episodes) {
        rb->talk_id(LANG_MAIN_MENU, true);
    } else {
        rb->talk_id(LANG_MAIN_MENU, true);
    }
    
    return 0;
}

static struct gui_synclist reload_podcast_list(struct gui_synclist list)
{
    char** podcasts = get_podcasts_list();
    if (podcasts == NULL) {
        rb->splash(HZ, "No podcasts found");
        return list;
    }
    
    int count = rb->android_podcast_get_list_count(podcasts);
    menu_item_count = count;
    
    if (menu_item_count > MAX_ITEMS) {
        menu_item_count = MAX_ITEMS;
    }

    menu_items[0].name = "Podcasts";
    rb->strcpy(menu_items[0].string, "Podcasts");
    menu_items[1].name = "===";
    rb->strcpy(menu_items[1].string, "===");

    for (int i = 0; i < menu_item_count; i++) {
        menu_items[i+2].name = podcasts[i];
        rb->strcpy(menu_items[i+2].string, podcasts[i]);
    }

    rb->gui_synclist_set_nb_items(&list, menu_item_count +2);
    rb->gui_synclist_draw(&list);

    return list;
}

static struct gui_synclist reload_episodes_list(struct gui_synclist list, int podcast_num, char* podcast_name)
{
    // Clear cached episodes to force reload
    if (current_episodes != NULL) {
        rb->free_array(current_episodes);
        current_episodes = NULL;
    }
    
    char** episodes = get_episodes_list(podcast_num);
    if (episodes == NULL) {
        rb->splash(HZ, "No episodes found");
        return list;
    }
    current_episodes = episodes;
    
    int count = rb->android_podcast_get_list_count(current_episodes);
    menu_item_count = count;
    
    if (menu_item_count > MAX_ITEMS) {
        menu_item_count = MAX_ITEMS;
    }

    menu_items[0].name = podcast_name;
    rb->strcpy(menu_items[0].string, podcast_name);
    menu_items[1].name = "===";
    rb->strcpy(menu_items[1].string, "===");
    menu_items[2].name = "> Download latest 5";
    rb->strcpy(menu_items[2].string, "> Download latest 5");
    menu_items[3].name = "> Download latest 10";
    rb->strcpy(menu_items[3].string, "> Download latest 10");
    menu_items[4].name = "> Download latest 15";
    rb->strcpy(menu_items[4].string, "> Download latest 15");
    menu_items[5].name = "---";
    rb->strcpy(menu_items[5].string, "---");
    menu_items[6].name = "Episodes:";
    rb->strcpy(menu_items[6].string, "Episodes:");

    for (int i = 0; i < menu_item_count; i++) {
        menu_items[i+7].name = current_episodes[i];
        rb->strcpy(menu_items[i+7].string, current_episodes[i]);
    }
    
    rb->gui_synclist_set_nb_items(&list, menu_item_count + 7);
    rb->gui_synclist_draw(&list);

    return list;
}

/* this is the plugin entry point */
enum plugin_status plugin_start(const void* parameter)
{
    (void)parameter;
    rb->global_settings->show_icons = false;
    struct gui_synclist list;
    bool done = false;
    int action, cur_sel;
    int ret = PLUGIN_OK;

    rb->splash(HZ, "Connecting to WiFi...");

    const char* wifi_ret;
    wifi_ret = rb->android_podcast_connect_wifi();
    if (strcmp(wifi_ret, "Success") != 0){
        rb->splash(HZ, "Failed connecting to WiFi, please check .rockbox/wifi.cfg and make sure the network is available.");
        return PLUGIN_ERROR;
    }

    // Get the podcast list
    char** podcasts = get_podcasts_list();
    if (podcasts == NULL) {
        rb->splash(HZ, "No podcasts found");
        return PLUGIN_ERROR;
    }

    // Populate menu items with podcasts
    int podcast_count = rb->android_podcast_get_list_count(podcasts);
    menu_item_count = podcast_count;
    
    if (menu_item_count > MAX_ITEMS) {
        menu_item_count = MAX_ITEMS;
    }
    
    menu_items[0].name = "Podcasts";
    rb->strcpy(menu_items[0].string, "Podcasts");
    menu_items[1].name = "===";
    rb->strcpy(menu_items[1].string, "===");

    for (int i = 0; i < menu_item_count; i++) {
        menu_items[i+2].name = podcasts[i];
        rb->strcpy(menu_items[i+2].string, podcasts[i]);
    }

    showing_episodes = false;
    rb->gui_synclist_init(&list, menu_get_name, NULL, false, 1, NULL);
    if (rb->global_settings->talk_menu)
        rb->gui_synclist_set_voice_callback(&list, menu_speak_item);
    rb->gui_synclist_set_nb_items(&list, menu_item_count +2);
    rb->gui_synclist_set_title(&list, rb->str(LANG_MAIN_MENU), Icon_Rockbox);
    rb->gui_synclist_draw(&list);
    rb->gui_synclist_speak_item(&list);

    while (!done)
    {
        cur_sel = rb->gui_synclist_get_sel_pos(&list);
        action = rb->get_action(CONTEXT_LIST, HZ/10);
        if (rb->gui_synclist_do_button(&list, &action))
            continue;

        switch (action)
        {
            case ACTION_STD_OK:
                if (showing_episodes) {
                    // handle episode actions
                    if (cur_sel < 7){
                        if (cur_sel < 5){
                            // handle download latest 5, 10, 15 episodes
                            switch (cur_sel)
                            {
                                case 2:
                                    rb->splash_progress(0, 5, "Downloading latest episodes...");
                                    for (int i = 0; i < 5; i++) {
                                        rb->android_podcast_download_episode(current_podcast_index, i);
                                        rb->splash_progress(i+1, 5, "Downloading latest episodes...");
                                    }
                                    // Reload episodes list to show updated status
                                    list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                                    break;
                                case 3:
                                    rb->splash_progress(0, 10, "Downloading latest episodes...");
                                    for (int i = 0; i < 10; i++) {
                                        rb->android_podcast_download_episode(current_podcast_index, i);
                                        rb->splash_progress(i+1, 10, "Downloading latest episodes...");
                                    }
                                    // Reload episodes list to show updated status
                                    list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                                    break;
                                case 4:
                                    rb->splash_progress(0, 15, "Downloading latest episodes...");
                                    for (int i = 0; i < 15; i++) {
                                        rb->android_podcast_download_episode(current_podcast_index, i);
                                        rb->splash_progress(i+1, 15, "Downloading latest episodes...");
                                    }
                                    // Reload episodes list to show updated status
                                    list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                                    break;
                                default:
                                    continue;
                            }
                            rb->gui_synclist_draw(&list);
                        } else {
                            break;
                        }
                    }
                    else {
                        rb->gui_synclist_speak_item(&list);
                        rb->splash(HZ, "Downloading Episode...");
                        rb->android_podcast_download_episode(current_podcast_index, cur_sel-7);
                        rb->splash(HZ, "Done.");
                        // Reload episodes list to show updated status
                        list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                        rb->gui_synclist_draw(&list);
                        break;
                    }
                } else {
                    if (cur_sel > 1){
                        current_podcast_index = cur_sel-2;
                        rb->splash(HZ, "Loading Episodes...");
                        showing_episodes = true;
                        list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                        rb->gui_synclist_set_title(&list, menu_items[cur_sel-2].string, Icon_Rockbox);
                        cur_sel = 6;
                    }
                }
                break;
            case ACTION_STD_CONTEXT:
                if (showing_episodes) {
                    // Context menu for episodes
                    if (cur_sel < 7){
                        break;
                    }
                    MENUITEM_STRINGLIST(menu, ID2P(LANG_MAIN_MENU), NULL,
                                        "Play",
                                        "Force-Download",
                                        "Delete");
                    switch (rb->do_menu(&menu, NULL, NULL, false))
                    {
                        case 0:
                            rb->gui_synclist_select_item(&list, cur_sel - 1); /* speaks */
                            rb->splash(HZ, "Downloading Episode... (if necessary)");
                            rb->android_podcast_download_episode(current_podcast_index, cur_sel-7);
                            rb->playlist_create(NULL, NULL);
                            const char* podcast_path = rb->android_podcast_get_episode_path(current_podcast_index, cur_sel-7);
                            rb->playlist_insert_track(NULL, podcast_path, 0, false, true);
                            rb->playlist_start(0,0,0);
                            // Reload episodes list to show updated status
                            list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                            break;
                        case 1:
                            if (cur_sel + 1 == menu_item_count)
                            {
                                rb->splash(HZ, ID2P(LANG_FAILED));
                                break;
                            }
                            rb->gui_synclist_select_item(&list, cur_sel + 1); /* speaks */
                            rb->splash(HZ, "Deleting Episode...");
                            rb->android_podcast_delete_episode(current_podcast_index, cur_sel-7);
                            rb->splash(HZ, "Downloading Episode...");
                            rb->android_podcast_download_episode(current_podcast_index, cur_sel-7);
                            rb->splash(HZ, "Downloaded.");
                            // Reload episodes list to show updated status
                            list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                            break;
                        case 2:
                            rb->gui_synclist_speak_item(&list);
                            rb->splash(HZ, "Deleting Episode...");
                            rb->android_podcast_delete_episode(current_podcast_index, cur_sel-7 );
                            rb->splash(HZ, "Deleted.");
                            // Reload episodes list to show updated status
                            list = reload_episodes_list(list, current_podcast_index, podcasts[current_podcast_index]);
                            break;
                        default:
                            rb->gui_synclist_speak_item(&list);
                    }
                    rb->gui_synclist_draw(&list);
                } else {
                    if (cur_sel > 1){
                        MENUITEM_STRINGLIST(menu, ID2P(LANG_MAIN_MENU), NULL,
                                            "Download ALL episodes. This will take a long time and doesn't show a progress bar.");
                        switch (rb->do_menu(&menu, NULL, NULL, false))
                        {
                            case 0:
                                rb->gui_synclist_select_item(&list, cur_sel - 1); /* speaks */
                                rb->splash(HZ, "Downloading all episodes...");
                                rb->android_podcast_download_episode(current_podcast_index, -1);
                                break;
                            default:
                                rb->gui_synclist_speak_item(&list);
                        }
                        rb->gui_synclist_draw(&list);
                    }
                }
                break;
            case ACTION_STD_CANCEL:
            case ACTION_STD_MENU:
                cur_sel = 0;
                if (showing_episodes) {
                    // Go back to podcasts list
                    showing_episodes = false;
                    // Clear the cached episodes list
                    if (current_episodes != NULL) {
                        rb->free_array(current_episodes);
                        current_episodes = NULL;
                    }
                    list = reload_podcast_list(list);
                    rb->gui_synclist_set_title(&list, rb->str(LANG_MAIN_MENU), Icon_Rockbox);
                } else {
                    // exit
                    list = reload_podcast_list(list);
                    done = true;
                }
                break;
            default:
                if (rb->default_event_handler(action) == SYS_USB_CONNECTED)
                {
                    ret = PLUGIN_USB_CONNECTED;
                    done = true;
                }
                continue;
        }
    }

    // clean up
    if (current_episodes != NULL) {
        rb->free_array(current_episodes);
        current_episodes = NULL;
    }
    rb->android_podcast_disconnect_wifi();

    return ret;
}

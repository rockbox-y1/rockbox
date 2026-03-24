/***************************************************************************
 *             __________               __   ___.
 *   Open      \______   \ ____   ____ |  | _\_ |__   _______  ___
 *   Source     |       _//  _ \_/ ___\|  |/ /| __ \ /  _ \  \/  /
 *   Jukebox    |    |   (  <_> )  \___|    < | \_\ (  <_> > <  <
 *   Firmware   |____|_  /\____/ \___  >__|_ \|___  /\____/__/\_ \
 *                     \/            \/     \/    \/            \/
 * $Id$
 *
 * Copyright (C) 2010 Maurus Cuelenaere
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

/* Button Code Definitions for Innioasis Y1 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "config.h"
#include "action.h"
#include "button.h"
#include "settings.h"

/*
 * The format of the list is as follows
 * { Action Code,   Button code,    Prereq button code }
 * if there's no need to check the previous button's value, use BUTTON_NONE
 * Insert LAST_ITEM_IN_LIST at the end of each mapping
 */

static const struct button_mapping button_context_standard[]  = {
    { ACTION_STD_PREV,          BUTTON_SCROLL_BACK,                 BUTTON_NONE },
    { ACTION_STD_PREVREPEAT,    BUTTON_SCROLL_BACK|BUTTON_REPEAT,   BUTTON_NONE },
    { ACTION_STD_NEXT,          BUTTON_SCROLL_FWD,                  BUTTON_NONE },
    { ACTION_STD_NEXTREPEAT,    BUTTON_SCROLL_FWD|BUTTON_REPEAT,    BUTTON_NONE },
    { ACTION_STD_CANCEL,        BUTTON_LEFT,                        BUTTON_NONE },
    { ACTION_STD_OK,            BUTTON_RIGHT,                       BUTTON_NONE },
    { ACTION_STD_OK,            BUTTON_SELECT|BUTTON_REL,           BUTTON_SELECT },
    { ACTION_STD_MENU,          BUTTON_MENU|BUTTON_REL,             BUTTON_MENU },
    { ACTION_STD_QUICKSCREEN,   BUTTON_MENU|BUTTON_REPEAT,          BUTTON_MENU },
    { ACTION_STD_CONTEXT,       BUTTON_SELECT|BUTTON_REL,           BUTTON_SELECT|BUTTON_REPEAT },
    { ACTION_STD_CANCEL,        BUTTON_PLAY|BUTTON_REPEAT,          BUTTON_NONE },

    LAST_ITEM_IN_LIST
}; /* button_context_standard */

static const struct button_mapping button_context_tree[]  = {
    { ACTION_TREE_WPS,          BUTTON_PLAY|BUTTON_REL,      BUTTON_PLAY },
    { ACTION_TREE_STOP,         BUTTON_PLAY|BUTTON_REPEAT,   BUTTON_PLAY },
    { ACTION_TREE_HOTKEY,       BUTTON_SELECT|BUTTON_PLAY,   BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_tree */

static const struct button_mapping button_context_tree_scroll_lr[]  = {
    { ACTION_NONE,              BUTTON_LEFT,                BUTTON_NONE },
    { ACTION_STD_CANCEL,        BUTTON_LEFT|BUTTON_REL,     BUTTON_LEFT },
    { ACTION_TREE_ROOT_INIT,    BUTTON_LEFT|BUTTON_REPEAT,  BUTTON_LEFT },
    { ACTION_TREE_PGLEFT,       BUTTON_LEFT|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_NONE,              BUTTON_RIGHT,               BUTTON_NONE },
    { ACTION_STD_OK,            BUTTON_RIGHT|BUTTON_REL,    BUTTON_RIGHT },
    { ACTION_TREE_PGRIGHT,      BUTTON_RIGHT|BUTTON_REPEAT, BUTTON_NONE },
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_CUSTOM|CONTEXT_TREE),
}; /* button_context_tree_scroll_lr */

static const struct button_mapping button_context_wps[]  = {
    { ACTION_WPS_PLAY,      BUTTON_PLAY|BUTTON_REL,         BUTTON_PLAY },
    { ACTION_WPS_STOP,      BUTTON_PLAY|BUTTON_REPEAT,      BUTTON_PLAY },
    { ACTION_WPS_SKIPPREV,  BUTTON_LEFT|BUTTON_REL,         BUTTON_LEFT },
    { ACTION_WPS_SEEKBACK,  BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_WPS_STOPSEEK,  BUTTON_LEFT|BUTTON_REL,         BUTTON_LEFT|BUTTON_REPEAT },
    { ACTION_WPS_SKIPNEXT,  BUTTON_RIGHT|BUTTON_REL,        BUTTON_RIGHT },
    { ACTION_WPS_SEEKFWD,   BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },
    { ACTION_WPS_STOPSEEK,  BUTTON_RIGHT|BUTTON_REL,        BUTTON_RIGHT|BUTTON_REPEAT },
    { ACTION_WPS_VOLDOWN,   BUTTON_SCROLL_BACK,                 BUTTON_NONE },
    { ACTION_WPS_VOLDOWN,   BUTTON_SCROLL_BACK|BUTTON_REPEAT,   BUTTON_NONE },
    { ACTION_WPS_VOLUP,     BUTTON_SCROLL_FWD,                  BUTTON_NONE },
    { ACTION_WPS_VOLUP,     BUTTON_SCROLL_FWD|BUTTON_REPEAT,    BUTTON_NONE },
    { ACTION_WPS_BROWSE,    BUTTON_SELECT|BUTTON_REL,           BUTTON_SELECT }, 
    { ACTION_WPS_CONTEXT,   BUTTON_SELECT|BUTTON_REL,           BUTTON_SELECT|BUTTON_REPEAT },
    { ACTION_WPS_HOTKEY,        BUTTON_SELECT|BUTTON_PLAY,      BUTTON_NONE },
    { ACTION_WPS_MENU,          BUTTON_MENU|BUTTON_REL,         BUTTON_MENU },
    { ACTION_WPS_QUICKSCREEN,   BUTTON_MENU|BUTTON_REPEAT,      BUTTON_MENU },

    LAST_ITEM_IN_LIST,
}; /* button_context_wps */

static const struct button_mapping button_context_settings[]  = {
    { ACTION_SETTINGS_INC,          BUTTON_SCROLL_FWD,         BUTTON_NONE },
    { ACTION_SETTINGS_INCREPEAT,    BUTTON_SCROLL_FWD|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_SETTINGS_DEC,          BUTTON_SCROLL_BACK,          BUTTON_NONE },
    { ACTION_SETTINGS_DECREPEAT,    BUTTON_SCROLL_BACK|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_STD_PREV,              BUTTON_LEFT,                  BUTTON_NONE },
    { ACTION_STD_PREVREPEAT,        BUTTON_LEFT|BUTTON_REPEAT,    BUTTON_NONE },
    { ACTION_STD_NEXT,              BUTTON_RIGHT,                BUTTON_NONE },
    { ACTION_STD_NEXTREPEAT,        BUTTON_RIGHT|BUTTON_REPEAT,  BUTTON_NONE },
    { ACTION_STD_OK,                BUTTON_SELECT|BUTTON_REL,    BUTTON_NONE },
    { ACTION_STD_CANCEL,            BUTTON_MENU|BUTTON_REL,      BUTTON_MENU },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_settings */

static const struct button_mapping button_context_bmark[]  = {
    { ACTION_BMS_DELETE,          BUTTON_MENU|BUTTON_REPEAT,       BUTTON_MENU },
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_LIST),
}; /* button_context_bmark */

static const struct button_mapping button_context_quickscreen[]  = {
    { ACTION_QS_TOP,        BUTTON_MENU,                    BUTTON_NONE },
    { ACTION_QS_TOP,        BUTTON_MENU|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_QS_DOWN,       BUTTON_PLAY,                    BUTTON_NONE },
    { ACTION_QS_DOWN,       BUTTON_PLAY|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_QS_LEFT,       BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_QS_LEFT,       BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_QS_RIGHT,      BUTTON_RIGHT,                   BUTTON_NONE },
    { ACTION_QS_RIGHT,      BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },
    { ACTION_STD_CANCEL,    BUTTON_SELECT|BUTTON_REL,       BUTTON_SELECT },
    { ACTION_QS_VOLDOWN,   BUTTON_SCROLL_BACK,                 BUTTON_NONE },
    { ACTION_QS_VOLDOWN,   BUTTON_SCROLL_BACK|BUTTON_REPEAT,   BUTTON_NONE },
    { ACTION_QS_VOLUP,     BUTTON_SCROLL_FWD,                  BUTTON_NONE },
    { ACTION_QS_VOLUP,     BUTTON_SCROLL_FWD|BUTTON_REPEAT,    BUTTON_NONE },

    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_quickscreen */

static const struct button_mapping button_context_pitchscreen[]  = {
    { ACTION_PS_INC_BIG,        BUTTON_RIGHT|BUTTON_REPEAT,     BUTTON_NONE },
    { ACTION_PS_DEC_BIG,        BUTTON_LEFT|BUTTON_REPEAT,      BUTTON_NONE },
    { ACTION_PS_INC_SMALL,      BUTTON_RIGHT,                   BUTTON_NONE },
    { ACTION_PS_DEC_SMALL,      BUTTON_LEFT,                    BUTTON_NONE },
    { ACTION_PS_EXIT,           BUTTON_MENU,                    BUTTON_NONE },
    { ACTION_PS_RESET,          BUTTON_SELECT,                  BUTTON_NONE },
    { ACTION_PS_TOGGLE_MODE,    BUTTON_PLAY,                    BUTTON_NONE },
    { ACTION_PS_SLOWER,         BUTTON_SCROLL_BACK,             BUTTON_NONE },
    { ACTION_PS_FASTER,         BUTTON_SCROLL_FWD,              BUTTON_NONE },
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_pitchscreen */


static const struct button_mapping button_context_list[]  = {
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_STD)
}; /* button_context_list */

static const struct button_mapping button_context_listtree_scroll_with_combo[]  = {
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_CUSTOM|CONTEXT_TREE),
};

static const struct button_mapping button_context_listtree_scroll_without_combo[]  = {
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_CUSTOM|CONTEXT_TREE),
};

static const struct button_mapping button_context_radio[]  = {
    LAST_ITEM_IN_LIST__NEXTLIST(CONTEXT_SETTINGS)
}; /* button_context_radio */

const struct button_mapping* get_context_mapping(int context)
{
    switch (context & ~CONTEXT_LOCKED)
    {
        case CONTEXT_STD:
            return button_context_standard;
        case CONTEXT_WPS:
            return button_context_wps;

        case CONTEXT_LIST:
            return button_context_list;
        case CONTEXT_MAINMENU:
        case CONTEXT_TREE:
            if (global_settings.hold_lr_for_scroll_in_list)
                return button_context_listtree_scroll_without_combo;
            else
                return button_context_listtree_scroll_with_combo;
        case CONTEXT_CUSTOM|CONTEXT_TREE:
            return button_context_tree;

        case CONTEXT_SETTINGS:
            return button_context_settings;
        case CONTEXT_CUSTOM|CONTEXT_SETTINGS:
        /*
        case CONTEXT_SETTINGS_RECTRIGGER:
            return button_context_settings_right_is_inc;
        */
        case CONTEXT_SETTINGS_COLOURCHOOSER:
            return button_context_settings;
        /*
        case CONTEXT_SETTINGS_EQ:
            return button_context_eq;
*/
        case CONTEXT_SETTINGS_TIME:
            return button_context_settings;

        case CONTEXT_FM:
            return button_context_radio;
        case CONTEXT_BOOKMARKSCREEN:
            return button_context_bmark;
        case CONTEXT_QUICKSCREEN:
            return button_context_quickscreen;
        case CONTEXT_PITCHSCREEN:
            return button_context_pitchscreen;
    }
    return button_context_standard;
}

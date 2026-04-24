#include "config.h"
#include "system.h"
#include "lang.h"
#include "menu.h"
#include "list.h"
#include "action.h"
#include "screens.h"
#include "yesno.h"
#include "settings.h"
#include "splash.h"

#ifdef INNIOASIS_Y1
/* wrapper for the system fm radio */
static int fm_radio_app_func(void)
{
    system("am start -n com.mediatek.FMRadio/.FMRadioActivity");
    
    return 0;
}

/* FM Radio app menu item */
MENUITEM_FUNCTION(fm_radio_app_item, 0, ID2P(LANG_FM_RADIO),
                  fm_radio_app_func, NULL, Icon_Radio_screen);
#endif 
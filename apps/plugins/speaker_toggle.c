#include "plugin.h"
#include "speaker_toggle_conf.h"
#include <stdlib.h>

#define CONF_PATH "/system/etc/audio_policy.conf"
#define CONF_SPEAKER_ON PLUGIN_APPS_DATA_DIR "/audio_policy_speaker.conf"
#define CONF_SPEAKER_OFF PLUGIN_APPS_DATA_DIR "/audio_policy_nospeaker.conf"
#define SPEAKER_TOKEN "AUDIO_DEVICE_OUT_SPEAKER"

static bool file_exists(const char *path)
{
    int fd = rb->open(path, O_RDONLY, 0);
    if (fd < 0)
        return false;
    rb->close(fd);
    return true;
}

static bool write_config(const char *path, const char *data)
{
    int fd = rb->open(path, O_WRONLY | O_CREAT | O_TRUNC, 0666);
    if (fd < 0)
        return false;
    size_t len = rb->strlen(data);
    ssize_t n = rb->write(fd, data, len);
    rb->close(fd);
    return n == (ssize_t)len;
}

static bool ensure_config_files(void)
{
    if (!file_exists(CONF_SPEAKER_ON)
        && !write_config(CONF_SPEAKER_ON, speaker_on_conf))
        return false;

    if (!file_exists(CONF_SPEAKER_OFF)
        && !write_config(CONF_SPEAKER_OFF, speaker_off_conf))
        return false;

    return true;
}

static int get_speaker_state(void)
{
    int fd = rb->open(CONF_PATH, O_RDONLY, 0);
    if (fd < 0) return -1;
    char buf[4096];
    ssize_t n = rb->read(fd, buf, sizeof(buf) - 1);
    rb->close(fd);
    if (n <= 0) return -1;
    buf[n] = '\0';
    char *p = buf;
    while (*p) {
        if (rb->strncmp(p, SPEAKER_TOKEN, sizeof(SPEAKER_TOKEN) - 1) == 0)
            return 1;
        p++;
    }
    return 0;
}

static void apply_state(bool enable)
{
    const char *src = enable ? CONF_SPEAKER_ON : CONF_SPEAKER_OFF;
    char cmd[512];
    rb->snprintf(cmd, sizeof(cmd),
        "su -c 'mount -o rw,remount /system"
        " && cp \"%s\" " CONF_PATH
        " && chmod 644 " CONF_PATH
        " && mount -o ro,remount /system'", src);
    system(cmd);
}

static void do_reboot(void)
{
    /* rb->sys_reboot() only exits Rockbox on Android, not the device */
    system("su -c reboot");
}

static bool is_dismiss_action(int action)
{
    /* Y1 maps Android BACK to BUTTON_MENU, not ACTION_STD_CANCEL */
    return action == ACTION_STD_CANCEL || action == ACTION_STD_MENU;
}

enum plugin_status plugin_start(const void *parameter)
{
    (void)parameter;
    if (!ensure_config_files()) {
        rb->splash(HZ * 2, "Error creating configs.");
        return PLUGIN_OK;
    }
    int state = get_speaker_state();
    if (state < 0) { rb->splash(HZ * 2, "Error reading config."); return PLUGIN_OK; }
    bool speaker_on = (state == 1);
    char msg[64];
    rb->snprintf(msg, sizeof(msg), "Speaker: %s\nOK=toggle BACK=cancel", speaker_on ? "ON" : "OFF");
    rb->splash(0, msg);
    int btn;
    while (true) {
        btn = rb->get_action(CONTEXT_STD, HZ * 15);
        if (btn == ACTION_STD_OK) break;
        if (is_dismiss_action(btn)) {
            rb->splash(HZ, "Cancelled."); return PLUGIN_OK;
        }
    }
    rb->splash(0, speaker_on ? "Disabling..." : "Enabling...");
    apply_state(!speaker_on);
    rb->sleep(HZ);
    rb->splash(0, speaker_on ? "Disabled.\nOK=reboot BACK=later" : "Enabled.\nOK=reboot BACK=later");
    while (true) {
        btn = rb->get_action(CONTEXT_STD, HZ * 15);
        if (btn == ACTION_STD_OK) { do_reboot(); break; }
        if (is_dismiss_action(btn)) break;
    }
    return PLUGIN_OK;
}

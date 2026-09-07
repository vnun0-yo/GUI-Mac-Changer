/*
============================================================
YenGuiMacChanger - Professional MAC Changer Tool
Version: 4.0
Language: C (with GTK+3)
============================================================
*/

#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_arp.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>

#define VERSION "4.0"
#define APP_NAME "YenGuiMacChanger"
#define MAX_MAC_LEN 18
#define LOG_FILE "/var/log/yen_mac_changer.log"
#define CONFIG_FILE "/etc/yen_mac_changer.conf"

// ========== GLOBAL VARIABLES ==========
GtkWidget *window;
GtkWidget *main_box;
GtkWidget *header_bar;
GtkWidget *content_box;
GtkWidget *combo;
GtkWidget *entry_mac;
GtkWidget *label_current;
GtkWidget *label_status;
GtkWidget *button_change;
GtkWidget *button_random;
GtkWidget *button_refresh;
GtkWidget *button_restore;
GtkWidget *button_fullscreen;
GtkWidget *button_close;
GtkWidget *button_theme;
GtkWidget *textview_log;
GtkTextBuffer *log_buffer;
GtkWidget *vpaned;
GtkWidget *top_paned;
GtkWidget *left_box;
GtkWidget *right_box;
GtkWidget *bottom_box;
GtkWidget *progress_bar;
GtkWidget *spinner;
int isFullscreen = 0;
int isDarkTheme = 1;
int changeCount = 0;
time_t startTime;
pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

// ========== CSS STYLES ==========
const char *dark_css = 
    "* { font-family: 'DejaVu Sans', 'Arial', sans-serif; font-size: 11px; }"
    "window { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
    " stop:0 #2d1b4e, stop:0.4 #4a2c6a, stop:0.7 #3a1f5c, stop:1 #1f0f3a); }"
    "frame { background: rgba(255,255,255,0.03); border: 1px solid rgba(200,150,255,0.10); border-radius: 8px; }"
    "frame label { color: #c8a8ff; font-size: 10px; }"
    "label { color: #d4c8e8; }"
    "entry { background: rgba(0,0,0,0.20); color: #d4c8e8; border: 1px solid rgba(180,140,255,0.15); border-radius: 6px; padding: 6px 10px; }"
    "entry:focus { border: 1px solid #b088ff; box-shadow: 0 0 15px rgba(160,120,255,0.08); }"
    "button { background: rgba(255,255,255,0.04); color: #d4c8e8; border: 1px solid rgba(255,255,255,0.04); border-radius: 6px; padding: 6px 12px; }"
    "button:hover { background: rgba(180,140,255,0.08); border-color: rgba(180,140,255,0.15); }"
    ".btn-change { background: #7c4dff; color: #ffffff; border: none; font-weight: bold; }"
    ".btn-change:hover { background: #9168ff; box-shadow: 0 0 20px rgba(124,77,255,0.15); }"
    ".btn-random { background: rgba(255,100,200,0.08); border: 1px solid #ff66cc; color: #ff66cc; }"
    ".btn-random:hover { background: rgba(255,100,200,0.15); }"
    ".btn-refresh { background: rgba(100,200,255,0.06); border: 1px solid #66ccff; color: #66ccff; }"
    ".btn-refresh:hover { background: rgba(100,200,255,0.12); }"
    ".btn-restore { background: rgba(255,200,80,0.06); border: 1px solid #ffcc44; color: #ffcc44; }"
    ".btn-restore:hover { background: rgba(255,200,80,0.12); }"
    ".btn-danger { background: rgba(255,60,60,0.08); border: 1px solid #ff4444; color: #ff4444; }"
    ".btn-danger:hover { background: rgba(255,60,60,0.15); }"
    ".btn-glass { background: rgba(255,255,255,0.02); border: 1px solid rgba(255,255,255,0.04); }"
    ".btn-glass:hover { background: rgba(255,255,255,0.05); }"
    "scrolledwindow { background: rgba(0,0,0,0.10); border-radius: 6px; border: 1px solid rgba(255,255,255,0.02); }"
    "textview { background: rgba(0,0,0,0.12); color: #b8e8d0; }"
    "textview text { background: rgba(0,0,0,0.12); color: #b8e8d0; }"
    "combo { background: rgba(0,0,0,0.15); color: #d4c8e8; border: 1px solid rgba(180,140,255,0.10); border-radius: 6px; padding: 4px 8px; }"
    "combo:hover { border-color: #b088ff; }"
    ".status-success { color: #88ffbb; font-weight: bold; }"
    ".status-error { color: #ff6666; font-weight: bold; }"
    ".status-info { color: #66ccff; font-weight: bold; }"
    ".status-warning { color: #ffcc44; font-weight: bold; }"
    ".mac-current { color: #b088ff; font-weight: bold; }"
    "progressbar progress { background: #7c4dff; border-radius: 4px; }"
    "progressbar trough { background: rgba(255,255,255,0.03); border-radius: 4px; }"
    "headerbar { background: rgba(0,0,0,0.20); border-bottom: 1px solid rgba(255,255,255,0.02); }";

const char *light_css = 
    "* { font-family: 'DejaVu Sans', 'Arial', sans-serif; font-size: 11px; }"
    "window { background: qlineargradient(x1:0,y1:0,x2:1,y2:1,"
    " stop:0 #e8d8f0, stop:0.4 #d4c0e8, stop:0.7 #c8b0d8, stop:1 #b8a0c8); }"
    "frame { background: rgba(255,255,255,0.30); border: 1px solid rgba(100,80,120,0.10); border-radius: 8px; }"
    "frame label { color: #5a4a6a; font-size: 10px; }"
    "label { color: #2a1a3a; }"
    "entry { background: rgba(255,255,255,0.50); color: #2a1a3a; border: 1px solid rgba(100,80,120,0.15); border-radius: 6px; padding: 6px 10px; }"
    "entry:focus { border: 1px solid #7c4dff; box-shadow: 0 0 15px rgba(124,77,255,0.10); }"
    "button { background: rgba(255,255,255,0.30); color: #2a1a3a; border: 1px solid rgba(100,80,120,0.08); border-radius: 6px; padding: 6px 12px; }"
    "button:hover { background: rgba(124,77,255,0.08); border-color: rgba(124,77,255,0.20); }"
    ".btn-change { background: #7c4dff; color: #ffffff; border: none; font-weight: bold; }"
    ".btn-change:hover { background: #9168ff; box-shadow: 0 0 20px rgba(124,77,255,0.15); }"
    ".btn-random { background: rgba(255,100,200,0.10); border: 1px solid #cc66aa; color: #8a4a6a; }"
    ".btn-random:hover { background: rgba(255,100,200,0.20); }"
    ".btn-refresh { background: rgba(100,200,255,0.08); border: 1px solid #66aacc; color: #3a6a8a; }"
    ".btn-refresh:hover { background: rgba(100,200,255,0.15); }"
    ".btn-restore { background: rgba(255,200,80,0.08); border: 1px solid #ccaa44; color: #8a7a3a; }"
    ".btn-restore:hover { background: rgba(255,200,80,0.15); }"
    ".btn-danger { background: rgba(255,60,60,0.08); border: 1px solid #cc4444; color: #8a3a3a; }"
    ".btn-danger:hover { background: rgba(255,60,60,0.15); }"
    ".btn-glass { background: rgba(255,255,255,0.20); border: 1px solid rgba(100,80,120,0.06); }"
    ".btn-glass:hover { background: rgba(255,255,255,0.30); }"
    "scrolledwindow { background: rgba(255,255,255,0.15); border-radius: 6px; border: 1px solid rgba(100,80,120,0.04); }"
    "textview { background: rgba(255,255,255,0.20); color: #2a1a3a; }"
    "textview text { background: rgba(255,255,255,0.20); color: #2a1a3a; }"
    "combo { background: rgba(255,255,255,0.30); color: #2a1a3a; border: 1px solid rgba(100,80,120,0.10); border-radius: 6px; padding: 4px 8px; }"
    "combo:hover { border-color: #7c4dff; }"
    ".status-success { color: #2a8a5a; font-weight: bold; }"
    ".status-error { color: #cc4444; font-weight: bold; }"
    ".status-info { color: #3a7aaa; font-weight: bold; }"
    ".status-warning { color: #aa8a2a; font-weight: bold; }"
    ".mac-current { color: #7c4dff; font-weight: bold; }"
    "progressbar progress { background: #7c4dff; border-radius: 4px; }"
    "progressbar trough { background: rgba(100,80,120,0.08); border-radius: 4px; }"
    "headerbar { background: rgba(255,255,255,0.20); border-bottom: 1px solid rgba(100,80,120,0.04); }";

// ========== FUNCTION PROTOTYPES ==========
int getCurrentMac(const char *iface, char *mac);
int validateMac(const char *mac);
int setMac(const char *iface, const char *mac);
int isRoot();
char** getInterfaces(int *count);
char* getInterfaceInfo(const char *iface);
char* getInterfaceStatus(const char *iface);
char* getInterfaceSpeed(const char *iface);
char* getInterfaceMTU(const char *iface);
char* getInterfaceAddress(const char *iface);
void generateRandomMac(char *mac);
void logAction(const char *action);
void saveConfig(const char *iface, const char *mac);
void loadConfig(char *iface, char *mac);
void apply_css(GtkWidget *widget, const char *css);
void toggleFullscreen(GtkWidget *widget, gpointer data);
void toggleTheme(GtkWidget *widget, gpointer data);
void closeTool(GtkWidget *widget, gpointer data);
void updateInterfaceInfo(GtkComboBox *widget, gpointer data);
void refreshInterfaces(GtkWidget *widget, gpointer data);
void changeMac(GtkWidget *widget, gpointer data);
void randomMac(GtkWidget *widget, gpointer data);
void restoreMac(GtkWidget *widget, gpointer data);
void clearLog(GtkWidget *widget, gpointer data);
void onWindowDestroy(GtkWidget *widget, gpointer data);
void showNotification(const char *message, const char *type);
gboolean idle_change_mac(gpointer data);
void *thread_change_mac(void *data);
void createUI();

// ========== IMPLEMENTATION ==========

int getCurrentMac(const char *iface, char *mac) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return 0;
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
        close(sock);
        return 0;
    }
    snprintf(mac, MAX_MAC_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
             (unsigned char)ifr.ifr_hwaddr.sa_data[0],
             (unsigned char)ifr.ifr_hwaddr.sa_data[1],
             (unsigned char)ifr.ifr_hwaddr.sa_data[2],
             (unsigned char)ifr.ifr_hwaddr.sa_data[3],
             (unsigned char)ifr.ifr_hwaddr.sa_data[4],
             (unsigned char)ifr.ifr_hwaddr.sa_data[5]);
    close(sock);
    return 1;
}

int validateMac(const char *mac) {
    int len = strlen(mac);
    if (len != 17) return 0;
    for (int i = 0; i < len; i++) {
        if (i % 3 == 2) {
            if (mac[i] != ':') return 0;
        } else {
            if (!isxdigit(mac[i])) return 0;
        }
    }
    return 1;
}

int setMac(const char *iface, const char *mac) {
    struct ifreq ifr;
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return 0;
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ip link set %s down", iface);
    system(cmd);
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
    unsigned char mac_bytes[6];
    sscanf(mac, "%02hhx:%02hhx:%02hhx:%02hhx:%02hhx:%02hhx",
           &mac_bytes[0], &mac_bytes[1], &mac_bytes[2],
           &mac_bytes[3], &mac_bytes[4], &mac_bytes[5]);
    memcpy(ifr.ifr_hwaddr.sa_data, mac_bytes, 6);
    if (ioctl(sock, SIOCSIFHWADDR, &ifr) < 0) {
        close(sock);
        return 0;
    }
    close(sock);
    snprintf(cmd, sizeof(cmd), "ip link set %s up", iface);
    system(cmd);
    return 1;
}

int isRoot() {
    return geteuid() == 0;
}

char** getInterfaces(int *count) {
    char **interfaces = NULL;
    *count = 0;
    FILE *file = fopen("/proc/net/dev", "r");
    if (!file) return NULL;
    char line[512];
    fgets(line, sizeof(line), file);
    fgets(line, sizeof(line), file);
    while (fgets(line, sizeof(line), file)) {
        char *pos = strchr(line, ':');
        if (pos) {
            char iface[64];
            int len = pos - line;
            strncpy(iface, line, len);
            iface[len] = '\0';
            char *start = iface;
            while (*start == ' ' || *start == '\t') start++;
            if (strcmp(start, "lo") != 0 && strcmp(start, "docker0") != 0 && strcmp(start, "virbr0") != 0) {
                interfaces = realloc(interfaces, (*count + 1) * sizeof(char*));
                interfaces[*count] = strdup(start);
                (*count)++;
            }
        }
    }
    fclose(file);
    return interfaces;
}

char* getInterfaceInfo(const char *iface) {
    static char mac[MAX_MAC_LEN];
    if (getCurrentMac(iface, mac)) {
        return mac;
    }
    return "Unknown";
}

char* getInterfaceStatus(const char *iface) {
    static char status[64];
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", iface);
    FILE *file = fopen(path, "r");
    if (file) {
        fgets(status, sizeof(status), file);
        fclose(file);
        char *nl = strchr(status, '\n');
        if (nl) *nl = '\0';
        return status;
    }
    return "unknown";
}

char* getInterfaceSpeed(const char *iface) {
    static char speed[64];
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s/speed", iface);
    FILE *file = fopen(path, "r");
    if (file) {
        fgets(speed, sizeof(speed), file);
        fclose(file);
        char *nl = strchr(speed, '\n');
        if (nl) *nl = '\0';
        strcat(speed, " Mbps");
        return speed;
    }
    return "N/A";
}

char* getInterfaceMTU(const char *iface) {
    static char mtu[64];
    char path[256];
    snprintf(path, sizeof(path), "/sys/class/net/%s/mtu", iface);
    FILE *file = fopen(path, "r");
    if (file) {
        fgets(mtu, sizeof(mtu), file);
        fclose(file);
        char *nl = strchr(mtu, '\n');
        if (nl) *nl = '\0';
        return mtu;
    }
    return "N/A";
}

char* getInterfaceAddress(const char *iface) {
    static char ip[64];
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ip -4 addr show %s 2>/dev/null | grep -oP '(?<=inet\\s)\\d+(\\.\\d+){3}' | head -1", iface);
    FILE *pipe = popen(cmd, "r");
    if (!pipe) return "N/A";
    if (fgets(ip, sizeof(ip), pipe)) {
        char *nl = strchr(ip, '\n');
        if (nl) *nl = '\0';
        pclose(pipe);
        return ip;
    }
    pclose(pipe);
    return "N/A";
}

void generateRandomMac(char *mac) {
    unsigned char bytes[6];
    srand(time(NULL) ^ (getpid() << 16));
    bytes[0] = 0x02;
    for (int i = 1; i < 6; i++) {
        bytes[i] = rand() % 256;
    }
    snprintf(mac, MAX_MAC_LEN, "%02x:%02x:%02x:%02x:%02x:%02x",
             bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
}

void logAction(const char *action) {
    pthread_mutex_lock(&logMutex);
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    char timestamp[64];
    strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", tm_info);
    FILE *file = fopen(LOG_FILE, "a");
    if (file) {
        fprintf(file, "[%s] %s\n", timestamp, action);
        fclose(file);
    }
    pthread_mutex_unlock(&logMutex);
}

void saveConfig(const char *iface, const char *mac) {
    FILE *file = fopen(CONFIG_FILE, "w");
    if (file) {
        fprintf(file, "interface=%s\n", iface);
        fprintf(file, "mac=%s\n", mac);
        fprintf(file, "fullscreen=%s\n", isFullscreen ? "true" : "false");
        fprintf(file, "theme=%s\n", isDarkTheme ? "dark" : "light");
        fclose(file);
    }
}

void loadConfig(char *iface, char *mac) {
    FILE *file = fopen(CONFIG_FILE, "r");
    if (file) {
        char line[256];
        while (fgets(line, sizeof(line), file)) {
            char *pos = strchr(line, '=');
            if (pos) {
                *pos = '\0';
                char *key = line;
                char *value = pos + 1;
                char *nl = strchr(value, '\n');
                if (nl) *nl = '\0';
                if (strcmp(key, "interface") == 0) strcpy(iface, value);
                if (strcmp(key, "mac") == 0) strcpy(mac, value);
                if (strcmp(key, "fullscreen") == 0) isFullscreen = (strcmp(value, "true") == 0);
                if (strcmp(key, "theme") == 0) isDarkTheme = (strcmp(value, "dark") == 0);
            }
        }
        fclose(file);
    }
}

void apply_css(GtkWidget *widget, const char *css) {
    GtkStyleContext *context = gtk_widget_get_style_context(widget);
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    g_object_unref(provider);
}

void showNotification(const char *message, const char *type) {
    GtkWidget *popup = gtk_window_new(GTK_WINDOW_POPUP);
    gtk_window_set_decorated(GTK_WINDOW(popup), FALSE);
    gtk_window_set_keep_above(GTK_WINDOW(popup), TRUE);
    gtk_window_set_default_size(GTK_WINDOW(popup), 350, 50);
    gtk_window_set_position(GTK_WINDOW(popup), GTK_WIN_POS_CENTER);
    
    const char *bg_color = isDarkTheme ? "rgba(20,10,40,0.92)" : "rgba(240,230,248,0.92)";
    char css_buf[256];
    snprintf(css_buf, sizeof(css_buf), 
        "window { background: %s; border-radius: 8px; border: 1px solid rgba(180,140,255,0.10); }", bg_color);
    apply_css(popup, css_buf);
    
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 10);
    gtk_container_add(GTK_CONTAINER(popup), box);
    
    const char *color = isDarkTheme ? "#c8a8ff" : "#5a4a6a";
    if (strcmp(type, "success") == 0) color = isDarkTheme ? "#88ffbb" : "#2a8a5a";
    else if (strcmp(type, "error") == 0) color = isDarkTheme ? "#ff6666" : "#cc4444";
    else if (strcmp(type, "warning") == 0) color = isDarkTheme ? "#ffcc44" : "#aa8a2a";
    
    char label_text[256];
    snprintf(label_text, sizeof(label_text), "<span font='12' color='%s'>%s</span>", color, message);
    GtkWidget *label = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), label_text);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    
    gtk_widget_show_all(popup);
    g_timeout_add(2500, (GSourceFunc)gtk_widget_destroy, popup);
}

void toggleTheme(GtkWidget *widget, gpointer data) {
    isDarkTheme = !isDarkTheme;
    apply_css(window, isDarkTheme ? dark_css : light_css);
    gtk_button_set_label(GTK_BUTTON(button_theme), isDarkTheme ? "Light" : "Dark");
    
    // Update colors in UI dynamically
    updateInterfaceInfo(NULL, NULL);
    saveConfig("", "");
}

void toggleFullscreen(GtkWidget *widget, gpointer data) {
    isFullscreen = !isFullscreen;
    if (isFullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(window));
        gtk_button_set_label(GTK_BUTTON(button_fullscreen), "Window");
    } else {
        gtk_window_unfullscreen(GTK_WINDOW(window));
        gtk_button_set_label(GTK_BUTTON(button_fullscreen), "Fullscreen");
    }
    saveConfig("", "");
}

void closeTool(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void updateInterfaceInfo(GtkComboBox *widget, gpointer data) {
    gchar *iface = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    if (!iface) return;
    char mac[MAX_MAC_LEN];
    if (getCurrentMac(iface, mac)) {
        char label_text[256];
        snprintf(label_text, sizeof(label_text), "<span class='mac-current'>Current MAC: %s</span>", mac);
        gtk_label_set_markup(GTK_LABEL(label_current), label_text);
    }
    char *info = getInterfaceInfo(iface);
    char *status = getInterfaceStatus(iface);
    char *speed = getInterfaceSpeed(iface);
    char *mtu = getInterfaceMTU(iface);
    char *ip = getInterfaceAddress(iface);
    
    const char *status_color = isDarkTheme ? "#ff6666" : "#cc4444";
    if (strcmp(status, "up") == 0) status_color = isDarkTheme ? "#88ffbb" : "#2a8a5a";
    else if (strcmp(status, "down") == 0) status_color = isDarkTheme ? "#ff6666" : "#cc4444";
    else status_color = isDarkTheme ? "#ffcc44" : "#aa8a2a";
    
    const char *title_color = isDarkTheme ? "#c8a8ff" : "#5a4a6a";
    const char *label_color = isDarkTheme ? "#8877aa" : "#6a5a7a";
    const char *value_color = isDarkTheme ? "#b088ff" : "#7c4dff";
    const char *ip_color = isDarkTheme ? "#ffcc44" : "#aa8a2a";
    const char *info_color = isDarkTheme ? "#8877aa" : "#7a6a8a";
    
    char status_text[1024];
    snprintf(status_text, sizeof(status_text),
        "<span font='13' font-weight='bold' color='%s'>%s</span>\n\n"
        "<span color='%s'>MAC Address</span>\n"
        "<span color='%s' font-weight='bold'>%s</span>\n\n"
        "<span color='%s'>Status</span>\n"
        "<span color='%s' font-weight='bold'>%s</span>\n\n"
        "<span color='%s'>IP Address</span>\n"
        "<span color='%s'>%s</span>\n\n"
        "<span color='%s'>Speed / MTU</span>\n"
        "<span color='%s'>%s / %s</span>",
        title_color, iface,
        label_color, value_color, info,
        label_color, status_color, status,
        label_color, ip_color, ip,
        label_color, info_color, speed, mtu);
    gtk_label_set_markup(GTK_LABEL(label_status), status_text);
    g_free(iface);
}

void refreshInterfaces(GtkWidget *widget, gpointer data) {
    gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(combo));
    int count;
    char **ifaces = getInterfaces(&count);
    for (int i = 0; i < count; i++) {
        gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), ifaces[i]);
        free(ifaces[i]);
    }
    free(ifaces);
    if (count > 0) {
        gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
        updateInterfaceInfo(NULL, NULL);
    }
}

typedef struct {
    char iface[64];
    char mac[18];
    int success;
} ChangeMacData;

void *thread_change_mac(void *data) {
    ChangeMacData *d = (ChangeMacData*)data;
    d->success = setMac(d->iface, d->mac);
    g_idle_add(idle_change_mac, d);
    return NULL;
}

gboolean idle_change_mac(gpointer data) {
    ChangeMacData *d = (ChangeMacData*)data;
    if (d->success) {
        changeCount++;
        char msg[256];
        snprintf(msg, sizeof(msg), "<span class='status-success'>MAC changed to %s</span>", d->mac);
        gtk_label_set_markup(GTK_LABEL(label_status), msg);
        showNotification("MAC changed successfully", "success");
        
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(log_buffer, &iter);
        char log_msg[512];
        snprintf(log_msg, sizeof(log_msg), "[%s] MAC changed on %s to %s\n",
                 g_date_time_format(g_date_time_new_now_local(), "%Y-%m-%d %H:%M:%S"), d->iface, d->mac);
        gtk_text_buffer_insert(log_buffer, &iter, log_msg, -1);
        
        char action[512];
        snprintf(action, sizeof(action), "MAC changed on %s to %s", d->iface, d->mac);
        logAction(action);
        updateInterfaceInfo(NULL, NULL);
        saveConfig(d->iface, d->mac);
    } else {
        showNotification("Failed to change MAC", "error");
        gtk_label_set_markup(GTK_LABEL(label_status), "<span class='status-error'>Failed to change MAC</span>");
        char action[512];
        snprintf(action, sizeof(action), "Failed to change MAC on %s", d->iface);
        logAction(action);
    }
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0);
    gtk_widget_hide(progress_bar);
    gtk_spinner_stop(GTK_SPINNER(spinner));
    gtk_widget_hide(spinner);
    gtk_widget_set_sensitive(button_change, TRUE);
    free(d);
    return FALSE;
}

void changeMac(GtkWidget *widget, gpointer data) {
    gchar *iface = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    const gchar *mac = gtk_entry_get_text(GTK_ENTRY(entry_mac));
    
    if (!iface || !mac || strlen(mac) == 0) {
        showNotification("Select interface and enter MAC", "error");
        gtk_label_set_markup(GTK_LABEL(label_status), "<span class='status-error'>Select interface and enter MAC</span>");
        return;
    }
    if (!validateMac(mac)) {
        showNotification("Invalid MAC format", "error");
        gtk_label_set_markup(GTK_LABEL(label_status), "<span class='status-error'>Invalid MAC format</span>");
        return;
    }
    if (!isRoot()) {
        showNotification("Run as root (sudo)", "error");
        gtk_label_set_markup(GTK_LABEL(label_status), "<span class='status-error'>Run as root (sudo)</span>");
        return;
    }
    
    gtk_progress_bar_pulse(GTK_PROGRESS_BAR(progress_bar));
    gtk_widget_show(progress_bar);
    gtk_spinner_start(GTK_SPINNER(spinner));
    gtk_widget_show(spinner);
    gtk_widget_set_sensitive(button_change, FALSE);
    
    ChangeMacData *d = malloc(sizeof(ChangeMacData));
    strncpy(d->iface, iface, sizeof(d->iface) - 1);
    strncpy(d->mac, mac, sizeof(d->mac) - 1);
    g_free(iface);
    
    pthread_t thread;
    pthread_create(&thread, NULL, thread_change_mac, d);
    pthread_detach(thread);
}

void randomMac(GtkWidget *widget, gpointer data) {
    char mac[MAX_MAC_LEN];
    generateRandomMac(mac);
    gtk_entry_set_text(GTK_ENTRY(entry_mac), mac);
    showNotification("Random MAC generated", "info");
    gtk_label_set_markup(GTK_LABEL(label_status), "<span class='status-info'>Random MAC generated</span>");
}

void restoreMac(GtkWidget *widget, gpointer data) {
    gchar *iface = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    if (!iface) return;
    char current[MAX_MAC_LEN];
    if (getCurrentMac(iface, current)) {
        gtk_entry_set_text(GTK_ENTRY(entry_mac), current);
        showNotification("Original MAC restored", "warning");
        gtk_label_set_markup(GTK_LABEL(label_status), "<span class='status-warning'>Restored original MAC</span>");
    }
    g_free(iface);
}

void clearLog(GtkWidget *widget, gpointer data) {
    gtk_text_buffer_set_text(log_buffer, "", -1);
    showNotification("Log cleared", "info");
}

void onWindowDestroy(GtkWidget *widget, gpointer data) {
    gtk_main_quit();
}

void createUI() {
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), APP_NAME " v" VERSION);
    gtk_window_set_default_size(GTK_WINDOW(window), 1100, 750);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(onWindowDestroy), NULL);

    if (isFullscreen) {
        gtk_window_fullscreen(GTK_WINDOW(window));
    }

    apply_css(window, isDarkTheme ? dark_css : light_css);

    main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_container_add(GTK_CONTAINER(window), main_box);

    // ===== HEADER BAR =====
    header_bar = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(header_bar), 8);
    apply_css(header_bar, "headerbar { }");
    gtk_box_pack_start(GTK_BOX(main_box), header_bar, FALSE, FALSE, 0);

    GtkWidget *title_label = gtk_label_new(NULL);
    const char *title_color = isDarkTheme ? "#b088ff" : "#5a4a6a";
    char title_text[128];
    snprintf(title_text, sizeof(title_text), "<span font='18' color='%s' font-weight='bold'>YenGuiMacChanger</span>", title_color);
    gtk_label_set_markup(GTK_LABEL(title_label), title_text);
    gtk_box_pack_start(GTK_BOX(header_bar), title_label, FALSE, FALSE, 0);

    GtkWidget *version_label = gtk_label_new(NULL);
    const char *ver_color = isDarkTheme ? "#776688" : "#8a7a9a";
    char ver_text[64];
    snprintf(ver_text, sizeof(ver_text), "<span color='%s'>v%s</span>", ver_color, VERSION);
    gtk_label_set_markup(GTK_LABEL(version_label), ver_text);
    gtk_box_pack_start(GTK_BOX(header_bar), version_label, FALSE, FALSE, 0);

    GtkWidget *header_spacer = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
    gtk_box_pack_start(GTK_BOX(header_bar), header_spacer, TRUE, TRUE, 0);

    // Theme toggle button
    button_theme = gtk_button_new_with_label(isDarkTheme ? "Light" : "Dark");
    apply_css(button_theme, ".btn-glass { }");
    g_signal_connect(button_theme, "clicked", G_CALLBACK(toggleTheme), NULL);
    gtk_box_pack_start(GTK_BOX(header_bar), button_theme, FALSE, FALSE, 0);

    // Fullscreen button
    button_fullscreen = gtk_button_new_with_label(isFullscreen ? "Window" : "Fullscreen");
    apply_css(button_fullscreen, ".btn-glass { }");
    g_signal_connect(button_fullscreen, "clicked", G_CALLBACK(toggleFullscreen), NULL);
    gtk_box_pack_start(GTK_BOX(header_bar), button_fullscreen, FALSE, FALSE, 0);

    // Close button
    button_close = gtk_button_new_with_label("Close");
    apply_css(button_close, ".btn-danger { }");
    g_signal_connect(button_close, "clicked", G_CALLBACK(closeTool), NULL);
    gtk_box_pack_start(GTK_BOX(header_bar), button_close, FALSE, FALSE, 0);

    // ===== CONTENT =====
    content_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    gtk_box_pack_start(GTK_BOX(main_box), content_box, TRUE, TRUE, 0);

    vpaned = gtk_paned_new(GTK_ORIENTATION_VERTICAL);
    gtk_box_pack_start(GTK_BOX(content_box), vpaned, TRUE, TRUE, 0);

    top_paned = gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(vpaned), top_paned, TRUE, TRUE);

    // ===== LEFT PANEL =====
    left_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
    gtk_container_set_border_width(GTK_CONTAINER(left_box), 20);
    gtk_paned_pack1(GTK_PANED(top_paned), left_box, TRUE, TRUE);

    GtkWidget *config_header = gtk_label_new(NULL);
    const char *header_color = isDarkTheme ? "#c8a8ff" : "#5a4a6a";
    char header_text[128];
    snprintf(header_text, sizeof(header_text), "<span font='14' color='%s' font-weight='bold'>CONFIGURATION</span>", header_color);
    gtk_label_set_markup(GTK_LABEL(config_header), header_text);
    gtk_box_pack_start(GTK_BOX(left_box), config_header, FALSE, FALSE, 0);

    // Interface selection
    GtkWidget *iface_frame = gtk_frame_new(" Interface ");
    gtk_box_pack_start(GTK_BOX(left_box), iface_frame, FALSE, FALSE, 0);
    GtkWidget *iface_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(iface_box), 10);
    gtk_container_add(GTK_CONTAINER(iface_frame), iface_box);

    GtkWidget *iface_label = gtk_label_new(NULL);
    const char *label_color = isDarkTheme ? "#8877aa" : "#6a5a7a";
    char label_text[128];
    snprintf(label_text, sizeof(label_text), "<span color='%s'>Interface</span>", label_color);
    gtk_label_set_markup(GTK_LABEL(iface_label), label_text);
    gtk_box_pack_start(GTK_BOX(iface_box), iface_label, FALSE, FALSE, 0);
    combo = gtk_combo_box_text_new();
    gtk_box_pack_start(GTK_BOX(iface_box), combo, TRUE, TRUE, 0);
    button_refresh = gtk_button_new_with_label("Refresh");
    apply_css(button_refresh, ".btn-refresh");
    g_signal_connect(button_refresh, "clicked", G_CALLBACK(refreshInterfaces), NULL);
    gtk_box_pack_start(GTK_BOX(iface_box), button_refresh, FALSE, FALSE, 0);

    // MAC input
    GtkWidget *mac_frame = gtk_frame_new(" MAC Address ");
    gtk_box_pack_start(GTK_BOX(left_box), mac_frame, FALSE, FALSE, 0);
    GtkWidget *mac_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(mac_box), 10);
    gtk_container_add(GTK_CONTAINER(mac_frame), mac_box);

    GtkWidget *mac_label = gtk_label_new(NULL);
    snprintf(label_text, sizeof(label_text), "<span color='%s'>New MAC</span>", label_color);
    gtk_label_set_markup(GTK_LABEL(mac_label), label_text);
    gtk_box_pack_start(GTK_BOX(mac_box), mac_label, FALSE, FALSE, 0);
    entry_mac = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(entry_mac), "00:11:22:33:44:55");
    gtk_box_pack_start(GTK_BOX(mac_box), entry_mac, TRUE, TRUE, 0);
    button_random = gtk_button_new_with_label("Random");
    apply_css(button_random, ".btn-random");
    g_signal_connect(button_random, "clicked", G_CALLBACK(randomMac), NULL);
    gtk_box_pack_start(GTK_BOX(mac_box), button_random, FALSE, FALSE, 0);

    // Actions
    GtkWidget *action_frame = gtk_frame_new(" Actions ");
    gtk_box_pack_start(GTK_BOX(left_box), action_frame, FALSE, FALSE, 0);
    GtkWidget *action_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(action_box), 10);
    gtk_container_add(GTK_CONTAINER(action_frame), action_box);

    button_change = gtk_button_new_with_label("Change MAC");
    apply_css(button_change, ".btn-change");
    g_signal_connect(button_change, "clicked", G_CALLBACK(changeMac), NULL);
    gtk_box_pack_start(GTK_BOX(action_box), button_change, TRUE, TRUE, 0);

    button_restore = gtk_button_new_with_label("Restore");
    apply_css(button_restore, ".btn-restore");
    g_signal_connect(button_restore, "clicked", G_CALLBACK(restoreMac), NULL);
    gtk_box_pack_start(GTK_BOX(action_box), button_restore, FALSE, FALSE, 0);

    // Current MAC
    label_current = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_current), "<span class='mac-current'>Current MAC: N/A</span>");
    gtk_box_pack_start(GTK_BOX(left_box), label_current, FALSE, FALSE, 5);

    // Progress
    GtkWidget *progress_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(left_box), progress_box, FALSE, FALSE, 0);
    progress_bar = gtk_progress_bar_new();
    gtk_progress_bar_set_pulse_step(GTK_PROGRESS_BAR(progress_bar), 0.05);
    gtk_widget_set_size_request(progress_bar, 200, 20);
    gtk_box_pack_start(GTK_BOX(progress_box), progress_bar, TRUE, TRUE, 0);
    spinner = gtk_spinner_new();
    gtk_widget_hide(spinner);
    gtk_box_pack_start(GTK_BOX(progress_box), spinner, FALSE, FALSE, 0);

    // ===== RIGHT PANEL =====
    right_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_container_set_border_width(GTK_CONTAINER(right_box), 20);
    gtk_paned_pack2(GTK_PANED(top_paned), right_box, TRUE, TRUE);

    GtkWidget *info_header = gtk_label_new(NULL);
    snprintf(header_text, sizeof(header_text), "<span font='14' color='%s' font-weight='bold'>INTERFACE INFO</span>", header_color);
    gtk_label_set_markup(GTK_LABEL(info_header), header_text);
    gtk_box_pack_start(GTK_BOX(right_box), info_header, FALSE, FALSE, 0);

    GtkWidget *info_frame = gtk_frame_new(" Details ");
    gtk_box_pack_start(GTK_BOX(right_box), info_frame, TRUE, TRUE, 0);
    label_status = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_status), "<span color='#8877aa'>Select an interface</span>");
    gtk_label_set_justify(GTK_LABEL(label_status), GTK_JUSTIFY_LEFT);
    gtk_container_add(GTK_CONTAINER(info_frame), label_status);

    // ===== BOTTOM - LOG =====
    bottom_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(bottom_box), 10);
    gtk_paned_pack2(GTK_PANED(vpaned), bottom_box, TRUE, TRUE);

    GtkWidget *log_header_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    gtk_box_pack_start(GTK_BOX(bottom_box), log_header_box, FALSE, FALSE, 0);
    GtkWidget *log_header = gtk_label_new(NULL);
    snprintf(header_text, sizeof(header_text), "<span font='13' color='%s' font-weight='bold'>ACTIVITY LOG</span>", header_color);
    gtk_label_set_markup(GTK_LABEL(log_header), header_text);
    gtk_box_pack_start(GTK_BOX(log_header_box), log_header, TRUE, TRUE, 0);
    GtkWidget *clear_log_btn = gtk_button_new_with_label("Clear");
    apply_css(clear_log_btn, ".btn-glass");
    g_signal_connect(clear_log_btn, "clicked", G_CALLBACK(clearLog), NULL);
    gtk_box_pack_start(GTK_BOX(log_header_box), clear_log_btn, FALSE, FALSE, 0);

    GtkWidget *log_frame = gtk_frame_new(NULL);
    gtk_box_pack_start(GTK_BOX(bottom_box), log_frame, TRUE, TRUE, 0);

    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled), GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(log_frame), scrolled);

    textview_log = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview_log), FALSE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview_log), GTK_WRAP_WORD);
    log_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview_log));
    gtk_container_add(GTK_CONTAINER(scrolled), textview_log);

    // ===== INIT =====
    refreshInterfaces(NULL, NULL);
    
    char saved_iface[64] = "";
    char saved_mac[18] = "";
    loadConfig(saved_iface, saved_mac);
    if (strlen(saved_mac) > 0) {
        gtk_entry_set_text(GTK_ENTRY(entry_mac), saved_mac);
    }

    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(log_buffer, &iter);
    char init_log[256];
    snprintf(init_log, sizeof(init_log), "[%s] " APP_NAME " v" VERSION " started\n",
             g_date_time_format(g_date_time_new_now_local(), "%Y-%m-%d %H:%M:%S"));
    gtk_text_buffer_insert(log_buffer, &iter, init_log, -1);

    gtk_widget_show_all(window);
}

int main(int argc, char *argv[]) {
    startTime = time(NULL);
    gtk_init(&argc, &argv);
    createUI();
    gtk_main();
    return 0;
}
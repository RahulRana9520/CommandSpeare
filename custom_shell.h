#ifndef CUSTOM_SHELL_H
#define CUSTOM_SHELL_H
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <glib.h>
#include <dirent.h>

#define MAX_HISTORY 50
#define MAX_COMMAND_LENGTH 1000
#define MAX_SUGGESTIONS 10
#define MAX_BUILTIN_COMMANDS 7

typedef struct {
    GtkWidget *window;
    GtkWidget *entry;
    GtkTextView *textview;
    GtkTextBuffer *buffer;
    GtkWidget *run_button;
    GtkWidget *clear_button;
    GtkWidget *time_button;
    GtkWidget *theme_button;
    GtkWidget *history_button;
    GtkWidget *voice_button;
    int theme_index; // 0: light, 1: dark, 2: hacker, 3: solarized
    char *command_history[MAX_HISTORY];
    int history_count;
    int history_index;
    GtkCssProvider *css_provider;
    GtkWidget *suggestion_popup;
    GtkWidget *suggestion_listbox;
    char *suggestions[MAX_SUGGESTIONS];
    int suggestion_count;
    int selected_suggestion;
    gboolean is_recording;
} AppData;

// Function declarations
void execute_command(const char *command, GtkTextBuffer *buffer, GtkTextView *textview);
void apply_css(AppData *app, const char *css);
void cycle_theme(AppData *app);
void add_to_history(AppData *app, const char *command);
void show_history(AppData *app);
void on_run_clicked(GtkButton *button, gpointer user_data);
void on_clear_clicked(GtkButton *button, gpointer user_data);
void on_time_clicked(GtkMenuItem *menuitem, gpointer user_data);
void on_theme_clicked(GtkButton *button, gpointer user_data);
void on_history_clicked(GtkButton *button, gpointer user_data);
void on_voice_clicked(GtkButton *button, gpointer user_data);
gboolean on_entry_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data);
void on_entry_changed(GtkEditable *editable, gpointer user_data);
void on_window_destroy(GtkWidget *widget, gpointer user_data);
void activate(GtkApplication *app, gpointer user_data);
void destroy_app_data(AppData *app_data);
double calculate_expression(const char *expr); // Calculator function
int precedence(char op);
double apply_operator(double a, double b, char op);

void generate_suggestions(AppData *app, const char *input);
void show_suggestions(AppData *app);
void hide_suggestions(AppData *app);
void apply_suggestion(AppData *app, int index);
void clear_suggestions(AppData *app);

void start_voice_recognition(AppData *app);
void stop_voice_recognition(AppData *app);
char* recognize_speech_from_mic(void);

// Kernel-level function declarations
typedef struct {
    int pid;
    char name[256];
    char state;
    int ppid;
    float cpu_percent;
    long memory_kb;
    char user[64];
} ProcessInfo;

// System Information Functions
void display_system_info(void);
void display_memory_info(void);
void display_cpu_info(void);
void display_network_info(void);
void analyze_filesystem(const char *path);
void display_loaded_modules(void);
void display_disk_io_stats(void);
void display_temperature_info(void);

// Process Management Functions
void list_processes_detailed(void);
int kill_process_by_pid(int pid);
int get_process_list(ProcessInfo **processes);
void monitor_syscalls(int pid, int duration_seconds);

// Real-time Statistics
void get_realtime_stats(double *cpu_usage, double *memory_usage);

// GTK Integration Functions
GtkWidget* create_system_info_dialog(GtkWindow *parent);
GtkWidget* create_memory_info_dialog(GtkWindow *parent);
GtkWidget* create_filesystem_dialog(GtkWindow *parent);
GtkWidget* create_process_manager_dialog(GtkWindow *parent);
void show_system_monitor(AppData *app);

// Menu callback for System Monitor
void on_system_monitor_clicked(GtkMenuItem *menuitem, gpointer user_data);
void on_memory_monitor_clicked(GtkMenuItem *menuitem, gpointer user_data);
void on_filesystem_monitor_clicked(GtkMenuItem *menuitem, gpointer user_data);
void on_process_manager_clicked(GtkMenuItem *menuitem, gpointer user_data);
void on_network_info_clicked(GtkMenuItem *menuitem, gpointer user_data);
void on_disk_usage_clicked(GtkMenuItem *menuitem, gpointer user_data);

#endif // CUSTOM_SHELL_H

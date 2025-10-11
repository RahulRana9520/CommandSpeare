// Kernel-level features for Command Sphere
// Advanced system monitoring and process management

#include "custom_shell.h"
#include <sys/sysinfo.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>

// ProcessInfo structure is now defined in custom_shell.h

// 1. System Information Functions
void display_system_info() {
    struct sysinfo info;
    sysinfo(&info);
    
    printf("=== SYSTEM INFORMATION ===\n");
    printf("Uptime: %ld seconds\n", info.uptime);
    printf("Total RAM: %lu MB\n", info.totalram / 1024 / 1024);
    printf("Free RAM: %lu MB\n", info.freeram / 1024 / 1024);
    printf("Process count: %d\n", info.procs);
    
    // Add to GUI with GTK labels
}

// 2. Process Management
void list_processes_detailed() {
    DIR *proc_dir;
    struct dirent *entry;
    FILE *status_file;
    char path[256], line[256], name[256];
    int pid;
    
    proc_dir = opendir("/proc");
    while ((entry = readdir(proc_dir)) != NULL) {
        pid = atoi(entry->d_name);
        if (pid > 0) {
            snprintf(path, sizeof(path), "/proc/%d/status", pid);
            status_file = fopen(path, "r");
            if (status_file) {
                fgets(line, sizeof(line), status_file);
                sscanf(line, "Name:\t%s", name);
                printf("PID: %d, Name: %s\n", pid, name);
                fclose(status_file);
            }
        }
    }
    closedir(proc_dir);
}

// 3. Memory Management
void display_memory_info() {
    FILE *meminfo = fopen("/proc/meminfo", "r");
    char line[256];
    
    printf("=== MEMORY INFORMATION ===\n");
    while (fgets(line, sizeof(line), meminfo)) {
        if (strncmp(line, "MemTotal:", 9) == 0 ||
            strncmp(line, "MemFree:", 8) == 0 ||
            strncmp(line, "MemAvailable:", 13) == 0) {
            printf("%s", line);
        }
    }
    fclose(meminfo);
}

// 4. File System Analysis
void analyze_filesystem(const char *path) {
    struct statvfs stats;
    
    if (statvfs(path, &stats) == 0) {
        unsigned long total = stats.f_blocks * stats.f_frsize;
        unsigned long free = stats.f_bavail * stats.f_frsize;
        unsigned long used = total - free;
        
        printf("=== FILESYSTEM: %s ===\n", path);
        printf("Total: %lu MB\n", total / 1024 / 1024);
        printf("Used: %lu MB\n", used / 1024 / 1024);
        printf("Free: %lu MB\n", free / 1024 / 1024);
        printf("Usage: %.1f%%\n", (double)used / total * 100);
    }
}

// 5. Network Information
void display_network_info() {
    FILE *net_dev = fopen("/proc/net/dev", "r");
    char line[256];
    
    printf("=== NETWORK INTERFACES ===\n");
    // Skip header lines
    fgets(line, sizeof(line), net_dev);
    fgets(line, sizeof(line), net_dev);
    
    while (fgets(line, sizeof(line), net_dev)) {
        char interface[16];
        unsigned long rx_bytes, tx_bytes;
        
        sscanf(line, "%[^:]:%lu %*u %*u %*u %*u %*u %*u %*u %lu", 
               interface, &rx_bytes, &tx_bytes);
        
        printf("Interface: %s\n", interface);
        printf("  RX: %lu KB\n", rx_bytes / 1024);
        printf("  TX: %lu KB\n", tx_bytes / 1024);
    }
    fclose(net_dev);
}

// 6. CPU Information
void display_cpu_info() {
    FILE *cpuinfo = fopen("/proc/cpuinfo", "r");
    char line[256];
    
    printf("=== CPU INFORMATION ===\n");
    while (fgets(line, sizeof(line), cpuinfo)) {
        if (strncmp(line, "model name", 10) == 0 ||
            strncmp(line, "cpu cores", 9) == 0 ||
            strncmp(line, "cpu MHz", 7) == 0) {
            printf("%s", line);
        }
    }
    fclose(cpuinfo);
}

// 7. Advanced Process Management with Kill Function
int kill_process_by_pid(int pid) {
    if (pid <= 0) {
        printf("Invalid PID: %d\n", pid);
        return -1;
    }
    
    // Check if process exists
    if (kill(pid, 0) == -1) {
        printf("Process %d does not exist or no permission\n", pid);
        return -1;
    }
    
    // Send SIGTERM first (graceful shutdown)
    if (kill(pid, SIGTERM) == 0) {
        printf("Sent SIGTERM to process %d\n", pid);
        sleep(2); // Wait 2 seconds
        
        // Check if process still exists
        if (kill(pid, 0) == 0) {
            // Process still running, force kill
            if (kill(pid, SIGKILL) == 0) {
                printf("Force killed process %d\n", pid);
                return 0;
            }
        } else {
            printf("Process %d terminated gracefully\n", pid);
            return 0;
        }
    }
    
    printf("Failed to kill process %d\n", pid);
    return -1;
}

// 8. Real-time System Statistics
void get_realtime_stats(double *cpu_usage, double *memory_usage) {
    static unsigned long long prev_idle = 0, prev_total = 0;
    FILE *stat_file;
    char line[256];
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;
    unsigned long long total, total_idle, totald, idled;
    
    // CPU Usage calculation
    stat_file = fopen("/proc/stat", "r");
    if (stat_file) {
        fgets(line, sizeof(line), stat_file);
        sscanf(line, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
        fclose(stat_file);
        
        total = user + nice + system + idle + iowait + irq + softirq + steal;
        total_idle = idle + iowait;
        
        totald = total - prev_total;
        idled = total_idle - prev_idle;
        
        if (totald > 0) {
            *cpu_usage = (double)(totald - idled) / totald * 100.0;
        } else {
            *cpu_usage = 0.0;
        }
        
        prev_total = total;
        prev_idle = total_idle;
    }
    
    // Memory Usage calculation
    struct sysinfo info;
    if (sysinfo(&info) == 0) {
        *memory_usage = (double)(info.totalram - info.freeram) / info.totalram * 100.0;
    }
}

// 9. Process List with Details
int get_process_list(ProcessInfo **processes) {
    DIR *proc_dir;
    struct dirent *entry;
    FILE *stat_file, *status_file;
    char path[512], line[512];
    ProcessInfo *proc_list = NULL;
    int count = 0, capacity = 100;
    
    proc_list = malloc(capacity * sizeof(ProcessInfo));
    if (!proc_list) return 0;
    
    proc_dir = opendir("/proc");
    if (!proc_dir) {
        free(proc_list);
        return 0;
    }
    
    while ((entry = readdir(proc_dir)) != NULL) {
        int pid = atoi(entry->d_name);
        if (pid <= 0) continue;
        
        if (count >= capacity) {
            capacity *= 2;
            proc_list = realloc(proc_list, capacity * sizeof(ProcessInfo));
            if (!proc_list) break;
        }
        
        ProcessInfo *proc = &proc_list[count];
        proc->pid = pid;
        
        // Get process name and state from /proc/[pid]/stat
        snprintf(path, sizeof(path), "/proc/%d/stat", pid);
        stat_file = fopen(path, "r");
        if (stat_file) {
            fscanf(stat_file, "%*d (%255[^)]) %c %d", proc->name, &proc->state, &proc->ppid);
            fclose(stat_file);
        } else {
            continue;
        }
        
        // Get memory information from /proc/[pid]/status
        snprintf(path, sizeof(path), "/proc/%d/status", pid);
        status_file = fopen(path, "r");
        if (status_file) {
            while (fgets(line, sizeof(line), status_file)) {
                if (strncmp(line, "VmRSS:", 6) == 0) {
                    sscanf(line, "VmRSS: %ld kB", &proc->memory_kb);
                    break;
                }
            }
            fclose(status_file);
        }
        
        // Get process owner
        struct stat st;
        snprintf(path, sizeof(path), "/proc/%d", pid);
        if (stat(path, &st) == 0) {
            struct passwd *pw = getpwuid(st.st_uid);
            if (pw) {
                strncpy(proc->user, pw->pw_name, sizeof(proc->user) - 1);
                proc->user[sizeof(proc->user) - 1] = '\0';
            } else {
                snprintf(proc->user, sizeof(proc->user), "%d", st.st_uid);
            }
        }
        
        proc->cpu_percent = 0.0; // Would need more complex calculation for real CPU%
        count++;
    }
    
    closedir(proc_dir);
    *processes = proc_list;
    return count;
}

// 10. System Call Monitoring (simplified)
void monitor_syscalls(int pid, int duration_seconds) {
    char command[256];
    printf("=== MONITORING SYSCALLS FOR PID %d ===\n", pid);
    printf("Duration: %d seconds\n", duration_seconds);
    
    // Use strace to monitor system calls
    snprintf(command, sizeof(command), 
             "timeout %d strace -p %d -c 2>/dev/null || echo 'Process monitoring completed'", 
             duration_seconds, pid);
    
    system(command);
}

// 11. Kernel Module Information
void display_loaded_modules() {
    FILE *modules_file = fopen("/proc/modules", "r");
    char line[512], name[64];
    int size, instances;
    
    if (!modules_file) {
        printf("Cannot access kernel modules information\n");
        return;
    }
    
    printf("=== LOADED KERNEL MODULES ===\n");
    printf("%-20s %10s %5s\n", "Module", "Size", "Used");
    printf("----------------------------------------\n");
    
    while (fgets(line, sizeof(line), modules_file)) {
        sscanf(line, "%s %d %d", name, &size, &instances);
        printf("%-20s %10d %5d\n", name, size, instances);
    }
    
    fclose(modules_file);
}

// 12. Disk I/O Statistics
void display_disk_io_stats() {
    FILE *diskstats = fopen("/proc/diskstats", "r");
    char line[256], device[32];
    unsigned long reads, writes;
    
    if (!diskstats) {
        printf("Cannot access disk statistics\n");
        return;
    }
    
    printf("=== DISK I/O STATISTICS ===\n");
    printf("%-10s %10s %10s\n", "Device", "Reads", "Writes");
    printf("--------------------------------\n");
    
    while (fgets(line, sizeof(line), diskstats)) {
        if (sscanf(line, "%*d %*d %s %lu %*u %*u %*u %lu", device, &reads, &writes) >= 3) {
            // Only show actual disks (sda, nvme, etc.)
            if (strstr(device, "sd") || strstr(device, "nvme") || strstr(device, "hd")) {
                printf("%-10s %10lu %10lu\n", device, reads, writes);
            }
        }
    }
    
    fclose(diskstats);
}

// 13. Temperature Monitoring (if available)
void display_temperature_info() {
    FILE *temp_file;
    char path[256], line[128];
    int temp;
    
    printf("=== TEMPERATURE INFORMATION ===\n");
    
    // Try common temperature paths
    const char* temp_paths[] = {
        "/sys/class/thermal/thermal_zone0/temp",
        "/sys/class/thermal/thermal_zone1/temp",
        NULL
    };
    
    for (int i = 0; temp_paths[i]; i++) {
        temp_file = fopen(temp_paths[i], "r");
        if (temp_file) {
            if (fgets(line, sizeof(line), temp_file)) {
                temp = atoi(line) / 1000; // Convert from millidegrees
                printf("Zone %d: %d°C\n", i, temp);
            }
            fclose(temp_file);
        }
    }
}

// 14. GTK Integration Functions for GUI Display
GtkWidget* create_system_info_dialog(GtkWindow *parent) {
    GtkWidget *dialog, *content_area, *grid, *label;
    struct sysinfo info;
    char text[256];
    double cpu_usage, memory_usage;
    
    dialog = gtk_dialog_new_with_buttons("System Information",
                                         parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Close", GTK_RESPONSE_CLOSE,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 500, 400);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);
    gtk_container_add(GTK_CONTAINER(content_area), grid);
    
    // System information
    sysinfo(&info);
    get_realtime_stats(&cpu_usage, &memory_usage);
    
    // Uptime
    snprintf(text, sizeof(text), "Uptime: %ld seconds (%.1f hours)", 
             info.uptime, (double)info.uptime / 3600);
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 2, 1);
    
    // CPU Usage
    snprintf(text, sizeof(text), "CPU Usage: %.1f%%", cpu_usage);
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 2, 1);
    
    // Memory Information
    snprintf(text, sizeof(text), "Total RAM: %lu MB", info.totalram / 1024 / 1024);
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 2, 2, 1);
    
    snprintf(text, sizeof(text), "Free RAM: %lu MB", info.freeram / 1024 / 1024);
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 3, 2, 1);
    
    snprintf(text, sizeof(text), "Memory Usage: %.1f%%", memory_usage);
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 4, 2, 1);
    
    // Process count
    snprintf(text, sizeof(text), "Running Processes: %d", info.procs);
    label = gtk_label_new(text);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 2, 1);
    
    gtk_widget_show_all(dialog);
    return dialog;
}

// 15. Process Manager Dialog with Kill Functionality
static void on_kill_process_clicked(GtkButton *button, gpointer user_data) {
    int pid = GPOINTER_TO_INT(user_data);
    
    GtkWidget *confirm_dialog = gtk_message_dialog_new(NULL,
                                                       GTK_DIALOG_MODAL,
                                                       GTK_MESSAGE_QUESTION,
                                                       GTK_BUTTONS_YES_NO,
                                                       "Are you sure you want to kill process %d?", pid);
    
    int response = gtk_dialog_run(GTK_DIALOG(confirm_dialog));
    gtk_widget_destroy(confirm_dialog);
    
    if (response == GTK_RESPONSE_YES) {
        if (kill_process_by_pid(pid) == 0) {
            GtkWidget *success_dialog = gtk_message_dialog_new(NULL,
                                                              GTK_DIALOG_MODAL,
                                                              GTK_MESSAGE_INFO,
                                                              GTK_BUTTONS_OK,
                                                              "Process %d killed successfully", pid);
            gtk_dialog_run(GTK_DIALOG(success_dialog));
            gtk_widget_destroy(success_dialog);
        } else {
            GtkWidget *error_dialog = gtk_message_dialog_new(NULL,
                                                            GTK_DIALOG_MODAL,
                                                            GTK_MESSAGE_ERROR,
                                                            GTK_BUTTONS_OK,
                                                            "Failed to kill process %d", pid);
            gtk_dialog_run(GTK_DIALOG(error_dialog));
            gtk_widget_destroy(error_dialog);
        }
    }
}

GtkWidget* create_process_manager_dialog(GtkWindow *parent) {
    GtkWidget *dialog, *content_area, *scrolled, *listbox, *hbox, *label, *button;
    ProcessInfo *processes;
    int process_count;
    char text[512];
    
    dialog = gtk_dialog_new_with_buttons("Process Manager",
                                         parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Refresh", 1,
                                         "_Close", GTK_RESPONSE_CLOSE,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 900, 700);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Create scrolled window
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled);
    
    // Create list box
    listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(scrolled), listbox);
    
    // Get process list
    process_count = get_process_list(&processes);
    
    // Add header
    hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
    label = gtk_label_new("PID");
    gtk_widget_set_size_request(label, 60, -1);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    label = gtk_label_new("Name");
    gtk_widget_set_size_request(label, 200, -1);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    label = gtk_label_new("User");
    gtk_widget_set_size_request(label, 100, -1);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    label = gtk_label_new("Memory (KB)");
    gtk_widget_set_size_request(label, 100, -1);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    label = gtk_label_new("State");
    gtk_widget_set_size_request(label, 60, -1);
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    label = gtk_label_new("Action");
    gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
    
    gtk_list_box_insert(GTK_LIST_BOX(listbox), hbox, 0);
    
    // Add processes (show more processes)
    for (int i = 0; i < process_count && i < 200; i++) { // Show up to 200 processes
        ProcessInfo *proc = &processes[i];
        
        hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);
        
        // PID
        snprintf(text, sizeof(text), "%d", proc->pid);
        label = gtk_label_new(text);
        gtk_widget_set_size_request(label, 60, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        
        // Name (truncated)
        if (strlen(proc->name) > 25) {
            snprintf(text, sizeof(text), "%.22s...", proc->name);
        } else {
            strcpy(text, proc->name);
        }
        label = gtk_label_new(text);
        gtk_widget_set_size_request(label, 200, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        
        // User
        label = gtk_label_new(proc->user);
        gtk_widget_set_size_request(label, 100, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        
        // Memory
        snprintf(text, sizeof(text), "%ld", proc->memory_kb);
        label = gtk_label_new(text);
        gtk_widget_set_size_request(label, 100, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        
        // State
        snprintf(text, sizeof(text), "%c", proc->state);
        label = gtk_label_new(text);
        gtk_widget_set_size_request(label, 60, -1);
        gtk_box_pack_start(GTK_BOX(hbox), label, FALSE, FALSE, 0);
        
        // Kill button (only for non-system processes)
        if (proc->pid > 1) {
            button = gtk_button_new_with_label("Kill");
            gtk_widget_set_size_request(button, 60, -1);
            g_signal_connect(button, "clicked", G_CALLBACK(on_kill_process_clicked), 
                           GINT_TO_POINTER(proc->pid));
            gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 0);
        }
        
        gtk_list_box_insert(GTK_LIST_BOX(listbox), hbox, i + 1);
    }
    
    free(processes);
    gtk_widget_show_all(dialog);
    return dialog;
}

// 16. System Monitor Integration Function
void show_system_monitor(AppData *app) {
    GtkWidget *dialog = create_system_info_dialog(GTK_WINDOW(app->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// 17. Memory Information Dialog
GtkWidget* create_memory_info_dialog(GtkWindow *parent) {
    GtkWidget *dialog, *content_area, *scrolled, *textview;
    GtkTextBuffer *text_buffer;
    FILE *meminfo;
    char line[256], full_text[4096] = "";
    
    dialog = gtk_dialog_new_with_buttons("Memory Information",
                                         parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Close", GTK_RESPONSE_CLOSE,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 500);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Create scrolled window with text view
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled);
    
    textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textview), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), textview);
    
    // Read memory information
    meminfo = fopen("/proc/meminfo", "r");
    if (meminfo) {
        strcat(full_text, "=== MEMORY INFORMATION ===\n\n");
        while (fgets(line, sizeof(line), meminfo)) {
            strcat(full_text, line);
        }
        fclose(meminfo);
    } else {
        strcat(full_text, "Error: Cannot read memory information\n");
    }
    
    // Set text in buffer
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(text_buffer, full_text, -1);
    
    gtk_widget_show_all(dialog);
    return dialog;
}

// 18. Filesystem Information Dialog  
GtkWidget* create_filesystem_dialog(GtkWindow *parent) {
    GtkWidget *dialog, *content_area, *scrolled, *textview;
    GtkTextBuffer *text_buffer;
    char full_text[4096] = "";
    char temp[256];
    struct statvfs stats;
    
    dialog = gtk_dialog_new_with_buttons("Filesystem Information",
                                         parent,
                                         GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                         "_Close", GTK_RESPONSE_CLOSE,
                                         NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 600, 500);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Create scrolled window with text view
    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_container_add(GTK_CONTAINER(content_area), scrolled);
    
    textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textview), TRUE);
    gtk_container_add(GTK_CONTAINER(scrolled), textview);
    
    // Get filesystem information for common mount points
    strcat(full_text, "=== FILESYSTEM INFORMATION ===\n\n");
    
    const char* paths[] = {"/", "/home", "/tmp", "/var", "/usr", NULL};
    
    for (int i = 0; paths[i]; i++) {
        if (statvfs(paths[i], &stats) == 0) {
            unsigned long total = (stats.f_blocks * stats.f_frsize) / (1024 * 1024);
            unsigned long free = (stats.f_bavail * stats.f_frsize) / (1024 * 1024);
            unsigned long used = total - free;
            double usage_percent = total > 0 ? (double)used / total * 100 : 0;
            
            snprintf(temp, sizeof(temp), "Mount Point: %s\n", paths[i]);
            strcat(full_text, temp);
            snprintf(temp, sizeof(temp), "  Total Space: %lu MB\n", total);
            strcat(full_text, temp);
            snprintf(temp, sizeof(temp), "  Used Space:  %lu MB\n", used);
            strcat(full_text, temp);
            snprintf(temp, sizeof(temp), "  Free Space:  %lu MB\n", free);
            strcat(full_text, temp);
            snprintf(temp, sizeof(temp), "  Usage:       %.1f%%\n\n", usage_percent);
            strcat(full_text, temp);
        }
    }
    
    // Add disk I/O statistics
    strcat(full_text, "=== DISK I/O STATISTICS ===\n\n");
    FILE *diskstats = fopen("/proc/diskstats", "r");
    if (diskstats) {
        char line[256], device[32];
        unsigned long reads, writes;
        
        while (fgets(line, sizeof(line), diskstats)) {
            if (sscanf(line, "%*d %*d %s %lu %*u %*u %*u %lu", device, &reads, &writes) >= 3) {
                if (strstr(device, "sd") || strstr(device, "nvme") || strstr(device, "hd")) {
                    snprintf(temp, sizeof(temp), "Device: %s - Reads: %lu, Writes: %lu\n", 
                            device, reads, writes);
                    strcat(full_text, temp);
                }
            }
        }
        fclose(diskstats);
    }
    
    // Set text in buffer
    text_buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(text_buffer, full_text, -1);
    
    gtk_widget_show_all(dialog);
    return dialog;
}

// 19. System Monitor Menu Callback
void on_system_monitor_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    show_system_monitor(app);
}

// 20. Memory Monitor Callback
void on_memory_monitor_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkWidget *dialog = create_memory_info_dialog(GTK_WINDOW(app->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// 21. Filesystem Monitor Callback
void on_filesystem_monitor_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkWidget *dialog = create_filesystem_dialog(GTK_WINDOW(app->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// 22. Process Manager Callback
void on_process_manager_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    GtkWidget *dialog = create_process_manager_dialog(GTK_WINDOW(app->window));
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}
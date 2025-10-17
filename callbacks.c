#include "custom_shell.h"
#include <sys/statvfs.h>

void on_run_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    const char *command = gtk_entry_get_text(GTK_ENTRY(app->entry));
    
    if (strlen(command) == 0) return;
    
    hide_suggestions(app);
    
    add_to_history(app, command);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    
    gtk_text_buffer_insert(app->buffer, &iter, "$ ", -1);
    
    char *cmd_copy = g_strdup(command);
    char *cmd_name = strtok(cmd_copy, " \t");
    const char *tag_name = "default";
    if (cmd_name) {
        if (strcmp(cmd_name, "cd") == 0) {
            tag_name = "cd";
        } else if (strcmp(cmd_name, "ls") == 0 || strcmp(cmd_name, "dir") == 0) {
            tag_name = "ls";
        } else if (strcmp(cmd_name, "pwd") == 0) {
            tag_name = "pwd";
        } else if (strcmp(cmd_name, "echo") == 0) {
            tag_name = "echo";
        } else if (strcmp(cmd_name, "cat") == 0) {
            tag_name = "cat";
        } else if (strcmp(cmd_name, "calc") == 0) {
            tag_name = "calc";
        } else if (strcmp(cmd_name, "help") == 0) {
            tag_name = "help";
        }
    }
    
    gtk_text_buffer_insert_with_tags_by_name(app->buffer, &iter, command, -1, tag_name, NULL);
    gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);
    
    g_free(cmd_copy);
    
    execute_command(command, app->buffer, app->textview);
    
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    GtkTextMark *mark = gtk_text_buffer_create_mark(app->buffer, "end", &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(app->textview), mark, 0.0, FALSE, 0.0, 0.0);
    gtk_text_buffer_delete_mark(app->buffer, mark);
    
    gtk_entry_set_text(GTK_ENTRY(app->entry), "");
}

void on_clear_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    gtk_text_buffer_set_text(app->buffer, "", -1);
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, "$ ", -1);
}

void on_time_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = user_data;
    time_t now = time(NULL);
    char time_buffer[64];
    struct tm *time_info = localtime(&now);
    strftime(time_buffer, sizeof(time_buffer), "%a %b %d %H:%M:%S %Y\n", time_info);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, "$ ", -1);
    gtk_text_buffer_insert(app->buffer, &iter, "Current time: ", -1);
    gtk_text_buffer_insert(app->buffer, &iter, time_buffer, -1);
}

void on_theme_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    cycle_theme(app);
}

void on_history_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    show_history(app);
}

// Thread function for voice recognition
gpointer voice_recognition_thread(gpointer user_data) {
    AppData *app = user_data;
    
    // Call the voice recognition function (runs in separate thread)
    char* recognized_command = recognize_speech_from_mic();
    
    // Schedule UI update on the main thread using g_idle_add
    VoiceResult *result = g_malloc(sizeof(VoiceResult));
    result->app = app;
    result->recognized_text = recognized_command; // Transfer ownership
    
    g_idle_add((GSourceFunc)voice_recognition_complete, result);
    
    return NULL;
}

// UI update callback (runs on main GTK thread)
gboolean voice_recognition_complete(gpointer user_data) {
    VoiceResult *result = (VoiceResult *)user_data;
    AppData *app = result->app;
    char *recognized_command = result->recognized_text;
    
    // Reset button appearance
    app->is_recording = FALSE;
    GtkWidget *mic_image = gtk_image_new_from_file("mic.svg");
    gtk_button_set_image(GTK_BUTTON(app->voice_button), mic_image);
    
    GtkTextIter iter;
    
    if (recognized_command) {
        // Show recognized command with visual confirmation
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        gtk_text_buffer_insert(app->buffer, &iter, "🎯 Voice Recognized: ", -1);
        gtk_text_buffer_insert_with_tags_by_name(app->buffer, &iter, recognized_command, -1, "ls", NULL);
        gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);
        
        // Try to suggest corrections if it looks wrong
        char* typo_fix = suggest_typo_fix(recognized_command);
        if (typo_fix) {
            gtk_text_buffer_insert(app->buffer, &iter, "💡 Did you mean: ", -1);
            gtk_text_buffer_insert_with_tags_by_name(app->buffer, &iter, typo_fix, -1, "cd", NULL);
            gtk_text_buffer_insert(app->buffer, &iter, " ? (Edit above if needed)\n", -1);
            
            // Put the corrected version in entry
            gtk_entry_set_text(GTK_ENTRY(app->entry), typo_fix);
            g_free(typo_fix);
        } else {
            // Put the recognized command into the entry for user confirmation
            gtk_entry_set_text(GTK_ENTRY(app->entry), recognized_command);
        }
        
        gtk_editable_set_position(GTK_EDITABLE(app->entry), -1);

        // Show hint to user to run it
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        gtk_text_buffer_insert(app->buffer, &iter, "▶️  Review command above, then press Enter to execute.\n", -1);

        g_free(recognized_command);
    } else {
        // Voice recognition failed
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        gtk_text_buffer_insert(app->buffer, &iter, "❌ Voice recognition failed or cancelled\n", -1);
        gtk_text_buffer_insert(app->buffer, &iter, "💡 Speak clearly and loudly, ensure good microphone input.\n", -1);
    }
    
    // Scroll to bottom to show results
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app->textview), &iter, 0.0, FALSE, 0.0, 0.0);
    
    // Clean up
    g_free(result);
    
    return FALSE; // Remove this idle callback after execution
}

void on_voice_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    
    // Prevent multiple simultaneous voice recognition attempts
    if (app->is_recording) {
        return;
    }
    
    app->is_recording = TRUE;
    
    // Update button appearance
    GtkWidget *mic_rec_image = gtk_image_new_from_file("mic_rec.svg");
    gtk_button_set_image(GTK_BUTTON(app->voice_button), mic_rec_image);
    
    // Show status in terminal
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, "🎤 Voice Recognition Starting...\n", -1);
    
    // Scroll to bottom
    gtk_text_view_scroll_to_iter(GTK_TEXT_VIEW(app->textview), &iter, 0.0, FALSE, 0.0, 0.0);
    
    // Run voice recognition in a separate thread to keep UI responsive
    g_thread_new("voice_recognition", voice_recognition_thread, app);
}

gboolean on_entry_key_press(GtkWidget *widget, GdkEventKey *event, gpointer user_data) {
    AppData *app = user_data;
    
    if (event->keyval == GDK_KEY_Tab) {
        if (app->suggestion_count > 0) {
            apply_suggestion(app, app->selected_suggestion >= 0 ? app->selected_suggestion : 0);
            return TRUE;
        }
    }
    
    if (event->keyval == GDK_KEY_Down && app->suggestion_count > 0) {
        app->selected_suggestion = (app->selected_suggestion + 1) % app->suggestion_count;
        GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(app->suggestion_listbox), 
                                                           app->selected_suggestion);
        if (row) {
            gtk_list_box_select_row(GTK_LIST_BOX(app->suggestion_listbox), row);
        }
        return TRUE;
    }
    
    if (event->keyval == GDK_KEY_Up) {
        if (app->suggestion_count > 0) {
            app->selected_suggestion = (app->selected_suggestion - 1 + app->suggestion_count) % app->suggestion_count;
            GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(app->suggestion_listbox), 
                                                               app->selected_suggestion);
            if (row) {
                gtk_list_box_select_row(GTK_LIST_BOX(app->suggestion_listbox), row);
            }
            return TRUE;
        }
        // Otherwise, navigate command history
        if (app->history_count == 0) return TRUE;
        
        if (app->history_index < 0) {
            app->history_index = app->history_count - 1;
        } else if (app->history_index > 0) {
            app->history_index--;
        }
        
        if (app->history_index >= 0 && app->history_index < app->history_count) {
            gtk_entry_set_text(GTK_ENTRY(app->entry), app->command_history[app->history_index]);
            gtk_editable_set_position(GTK_EDITABLE(app->entry), -1);
        }
        return TRUE;
    }
    
    if (event->keyval == GDK_KEY_Escape) {
        hide_suggestions(app);
        return TRUE;
    }
    
    if (event->keyval == GDK_KEY_Return) {
        gtk_widget_activate(app->run_button);
        return TRUE;
    }
    
    return FALSE;
}

void on_entry_changed(GtkEditable *editable, gpointer user_data) {
    AppData *app = user_data;
    const char *text = gtk_entry_get_text(GTK_ENTRY(app->entry));
    
    if (strlen(text) > 0) {
        generate_suggestions(app, text);
        if (app->suggestion_count > 0) {
            show_suggestions(app);
        } else {
            hide_suggestions(app);
        }
    } else {
        hide_suggestions(app);
    }
}

void on_window_destroy(GtkWidget *widget, gpointer user_data) {
    AppData *app_data = user_data;
    destroy_app_data(app_data);
}

// Network Information Menu Callback  
void on_network_info_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Network Information",
                                                     GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     "_Close", GTK_RESPONSE_CLOSE,
                                                     NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 600);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Set margins for content area
    gtk_widget_set_margin_top(content_area, 10);
    gtk_widget_set_margin_bottom(content_area, 10);
    gtk_widget_set_margin_start(content_area, 10);
    gtk_widget_set_margin_end(content_area, 10);
    
    // Create scrolled window with proper expansion
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_size_request(scrolled, 680, 550);
    
    GtkWidget *textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textview), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    gtk_widget_set_hexpand(textview, TRUE);
    gtk_widget_set_vexpand(textview, TRUE);
    
    // Set margins for textview
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), 10);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(textview), 10);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(textview), 10);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    
    // Build network information directly
    char network_info[4096] = "";
    strcat(network_info, "=== NETWORK INTERFACES ===\n\n");
    
    FILE *net_dev = fopen("/proc/net/dev", "r");
    if (net_dev) {
        char line[256];
        // Skip header lines
        fgets(line, sizeof(line), net_dev);
        fgets(line, sizeof(line), net_dev);
        
        while (fgets(line, sizeof(line), net_dev)) {
            char interface[16];
            unsigned long rx_bytes, tx_bytes;
            
            if (sscanf(line, "%[^:]:%lu %*u %*u %*u %*u %*u %*u %*u %lu", 
                      interface, &rx_bytes, &tx_bytes) >= 3) {
                char temp[256];
                snprintf(temp, sizeof(temp), "Interface: %s\n", interface);
                strcat(network_info, temp);
                snprintf(temp, sizeof(temp), "  RX: %lu KB (%lu MB)\n", 
                        rx_bytes / 1024, rx_bytes / 1024 / 1024);
                strcat(network_info, temp);
                snprintf(temp, sizeof(temp), "  TX: %lu KB (%lu MB)\n\n", 
                        tx_bytes / 1024, tx_bytes / 1024 / 1024);
                strcat(network_info, temp);
            }
        }
        fclose(net_dev);
    }
    
    // Add IP information
    strcat(network_info, "=== IP INFORMATION ===\n\n");
    FILE *ip_info = popen("ip addr show 2>/dev/null | head -20", "r");
    if (ip_info) {
        char line[256];
        while (fgets(line, sizeof(line), ip_info)) {
            strcat(network_info, line);
        }
        pclose(ip_info);
    }
    
    gtk_text_buffer_set_text(buffer, network_info, -1);
    
    gtk_container_add(GTK_CONTAINER(scrolled), textview);
    gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Disk Usage Menu Callback
void on_disk_usage_clicked(GtkMenuItem *menuitem, gpointer user_data) {
    AppData *app = (AppData *)user_data;
    
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Disk Usage Information",
                                                     GTK_WINDOW(app->window),
                                                     GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                     "_Close", GTK_RESPONSE_CLOSE,
                                                     NULL);
    
    gtk_window_set_default_size(GTK_WINDOW(dialog), 700, 600);
    gtk_window_set_resizable(GTK_WINDOW(dialog), TRUE);
    
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    
    // Set margins for content area
    gtk_widget_set_margin_top(content_area, 10);
    gtk_widget_set_margin_bottom(content_area, 10);
    gtk_widget_set_margin_start(content_area, 10);
    gtk_widget_set_margin_end(content_area, 10);
    
    // Create scrolled window with proper expansion
    GtkWidget *scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_hexpand(scrolled, TRUE);
    gtk_widget_set_vexpand(scrolled, TRUE);
    gtk_widget_set_size_request(scrolled, 680, 550);
    
    GtkWidget *textview = gtk_text_view_new();
    gtk_text_view_set_editable(GTK_TEXT_VIEW(textview), FALSE);
    gtk_text_view_set_monospace(GTK_TEXT_VIEW(textview), TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    gtk_widget_set_hexpand(textview, TRUE);
    gtk_widget_set_vexpand(textview, TRUE);
    
    // Set margins for textview
    gtk_text_view_set_left_margin(GTK_TEXT_VIEW(textview), 10);
    gtk_text_view_set_right_margin(GTK_TEXT_VIEW(textview), 10);
    gtk_text_view_set_top_margin(GTK_TEXT_VIEW(textview), 10);
    gtk_text_view_set_bottom_margin(GTK_TEXT_VIEW(textview), 10);
    
    GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    
    // Build disk information directly
    char disk_info[4096] = "";
    strcat(disk_info, "=== FILESYSTEM USAGE ===\n\n");
    
    // Check common mount points
    const char* paths[] = {"/", "/home", "/tmp", "/var", "/usr", NULL};
    struct statvfs stats;
    
    for (int i = 0; paths[i]; i++) {
        if (statvfs(paths[i], &stats) == 0) {
            unsigned long total = (stats.f_blocks * stats.f_frsize) / (1024 * 1024);
            unsigned long free = (stats.f_bavail * stats.f_frsize) / (1024 * 1024);
            unsigned long used = total - free;
            double usage_percent = total > 0 ? (double)used / total * 100 : 0;
            
            char temp[256];
            snprintf(temp, sizeof(temp), "Mount Point: %s\n", paths[i]);
            strcat(disk_info, temp);
            snprintf(temp, sizeof(temp), "  Total Space: %lu MB (%.1f GB)\n", 
                    total, (double)total / 1024);
            strcat(disk_info, temp);
            snprintf(temp, sizeof(temp), "  Used Space:  %lu MB (%.1f GB)\n", 
                    used, (double)used / 1024);
            strcat(disk_info, temp);
            snprintf(temp, sizeof(temp), "  Free Space:  %lu MB (%.1f GB)\n", 
                    free, (double)free / 1024);
            strcat(disk_info, temp);
            snprintf(temp, sizeof(temp), "  Usage:       %.1f%%\n\n", usage_percent);
            strcat(disk_info, temp);
        }
    }
    
    // Add disk I/O statistics
    strcat(disk_info, "=== DISK I/O STATISTICS ===\n\n");
    FILE *diskstats = fopen("/proc/diskstats", "r");
    if (diskstats) {
        char line[256], device[32];
        unsigned long reads, writes;
        
        while (fgets(line, sizeof(line), diskstats)) {
            if (sscanf(line, "%*d %*d %s %lu %*u %*u %*u %lu", device, &reads, &writes) >= 3) {
                if (strstr(device, "sd") || strstr(device, "nvme") || strstr(device, "hd")) {
                    char temp[256];
                    snprintf(temp, sizeof(temp), "Device: %s\n", device);
                    strcat(disk_info, temp);
                    snprintf(temp, sizeof(temp), "  Read Operations:  %lu\n", reads);
                    strcat(disk_info, temp);
                    snprintf(temp, sizeof(temp), "  Write Operations: %lu\n\n", writes);
                    strcat(disk_info, temp);
                }
            }
        }
        fclose(diskstats);
    }
    
    // Add df command output for additional info
    strcat(disk_info, "=== DETAILED FILESYSTEM INFO ===\n\n");
    FILE *df_output = popen("df -h 2>/dev/null | head -10", "r");
    if (df_output) {
        char line[256];
        while (fgets(line, sizeof(line), df_output)) {
            strcat(disk_info, line);
        }
        pclose(df_output);
    }
    
    gtk_text_buffer_set_text(buffer, disk_info, -1);
    
    gtk_container_add(GTK_CONTAINER(scrolled), textview);
    gtk_box_pack_start(GTK_BOX(content_area), scrolled, TRUE, TRUE, 0);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

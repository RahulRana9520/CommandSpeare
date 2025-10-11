#include "custom_shell.h"

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

void on_voice_clicked(GtkButton *button, gpointer user_data) {
    AppData *app = user_data;
    
    if (!app->is_recording) {
        start_voice_recognition(app);
    } else {
        stop_voice_recognition(app);
    }
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

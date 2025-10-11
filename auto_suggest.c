#include "custom_shell.h"


// Built-in commands for suggestions
static const char *builtin_commands[] = {
    "cd", "ls", "pwd", "echo", "cat", "calc", "help"
};

void clear_suggestions(AppData *app) {
    for (int i = 0; i < app->suggestion_count; i++) {
        if (app->suggestions[i]) {
            g_free(app->suggestions[i]);
            app->suggestions[i] = NULL;
        }
    }
    app->suggestion_count = 0;
    app->selected_suggestion = -1;
    
    // Clear listbox
    GList *children = gtk_container_get_children(GTK_CONTAINER(app->suggestion_listbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
}

void generate_suggestions(AppData *app, const char *input) {
    clear_suggestions(app);
    
    if (strlen(input) == 0) return;
    
    // Get the first word (command)
    char *input_copy = g_strdup(input);
    char *first_word = strtok(input_copy, " \t");
    const char *rest = input + strlen(first_word);
    while (*rest && isspace(*rest)) rest++;
    
    // If we're still typing the command (no space yet)
    if (strlen(rest) == 0) {
        // Suggest matching built-in commands
        for (int i = 0; i < MAX_BUILTIN_COMMANDS && app->suggestion_count < MAX_SUGGESTIONS; i++) {
            if (strncmp(builtin_commands[i], first_word, strlen(first_word)) == 0) {
                app->suggestions[app->suggestion_count++] = g_strdup(builtin_commands[i]);
            }
        }
        
        // Suggest matching commands from history
        for (int i = app->history_count - 1; i >= 0 && app->suggestion_count < MAX_SUGGESTIONS; i--) {
            if (strncmp(app->command_history[i], input, strlen(input)) == 0) {
                // Check if not already in suggestions
                gboolean already_exists = FALSE;
                for (int j = 0; j < app->suggestion_count; j++) {
                    if (strcmp(app->suggestions[j], app->command_history[i]) == 0) {
                        already_exists = TRUE;
                        break;
                    }
                }
                if (!already_exists) {
                    app->suggestions[app->suggestion_count++] = g_strdup(app->command_history[i]);
                }
            }
        }
    } else {
        // If we're typing arguments, suggest files/directories
        if (strcmp(first_word, "cd") == 0 || strcmp(first_word, "cat") == 0 || 
            strcmp(first_word, "ls") == 0) {
            
            DIR *dir = opendir(".");
            if (dir) {
                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL && app->suggestion_count < MAX_SUGGESTIONS) {
                    if (entry->d_name[0] == '.') continue; // Skip hidden files
                    
                    if (strncmp(entry->d_name, rest, strlen(rest)) == 0) {
                        char suggestion[MAX_COMMAND_LENGTH];
                        snprintf(suggestion, sizeof(suggestion), "%s %s", first_word, entry->d_name);
                        app->suggestions[app->suggestion_count++] = g_strdup(suggestion);
                    }
                }
                closedir(dir);
            }
        }
    }
    
    g_free(input_copy);
}

void show_suggestions(AppData *app) {
    if (app->suggestion_count == 0) {
        hide_suggestions(app);
        return;
    }
    
    // Clear existing items
    GList *children = gtk_container_get_children(GTK_CONTAINER(app->suggestion_listbox));
    for (GList *iter = children; iter != NULL; iter = g_list_next(iter)) {
        gtk_widget_destroy(GTK_WIDGET(iter->data));
    }
    g_list_free(children);
    
    // Add suggestions to listbox
    for (int i = 0; i < app->suggestion_count; i++) {
        GtkWidget *label = gtk_label_new(app->suggestions[i]);
        gtk_label_set_xalign(GTK_LABEL(label), 0.0);
        gtk_widget_set_margin_start(label, 5);
        gtk_widget_set_margin_end(label, 5);
        gtk_widget_set_margin_top(label, 3);
        gtk_widget_set_margin_bottom(label, 3);
        gtk_list_box_insert(GTK_LIST_BOX(app->suggestion_listbox), label, -1);
    }
    
    gtk_widget_show_all(app->suggestion_popup);
    app->selected_suggestion = 0;
    
    // Select first item
    GtkListBoxRow *row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(app->suggestion_listbox), 0);
    if (row) {
        gtk_list_box_select_row(GTK_LIST_BOX(app->suggestion_listbox), row);
    }
}

void hide_suggestions(AppData *app) {
    gtk_widget_hide(app->suggestion_popup);
    clear_suggestions(app);
}

void apply_suggestion(AppData *app, int index) {
    if (index >= 0 && index < app->suggestion_count) {
        gtk_entry_set_text(GTK_ENTRY(app->entry), app->suggestions[index]);
        gtk_editable_set_position(GTK_EDITABLE(app->entry), -1);
        hide_suggestions(app);
    }
}

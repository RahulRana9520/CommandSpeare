// CustomCommand.c
// Page for custom kernel feature commands and hamburger menu UI

#include "custom_shell.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

// Forward declarations for kernel features
void display_system_info();
void list_processes_detailed();
void display_memory_info();

// Custom command handler
void handle_custom_command(const char* command) {
    if (strcmp(command, "system_info") == 0) {
        display_system_info();
    } else if (strcmp(command, "process_list") == 0) {
        list_processes_detailed();
    } else if (strcmp(command, "memory_info") == 0) {
        display_memory_info();
    } else {
        printf("Unknown custom command: %s\n", command);
    }
}

// Hamburger menu UI (simple terminal version)
// GTK-based hamburger menu dialog
void show_hamburger_menu_dialog(AppData *app) {
    GtkWidget *dialog = gtk_dialog_new_with_buttons("Custom Commands",
                                                    GTK_WINDOW(app->window),
                                                    GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                                                    "_Close", GTK_RESPONSE_CLOSE,
                                                    NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 200);

    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 8);
    gtk_container_set_border_width(GTK_CONTAINER(box), 12);

    GtkWidget *label = gtk_label_new("Select a custom command:");
    gtk_box_pack_start(GTK_BOX(box), label, FALSE, FALSE, 0);

    GtkWidget *btn1 = gtk_button_new_with_label("System Info");
    GtkWidget *btn2 = gtk_button_new_with_label("Process List");
    GtkWidget *btn3 = gtk_button_new_with_label("Memory Info");

    gtk_box_pack_start(GTK_BOX(box), btn1, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), btn2, TRUE, TRUE, 0);
    gtk_box_pack_start(GTK_BOX(box), btn3, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(content_area), box);

    g_signal_connect_swapped(btn1, "clicked", G_CALLBACK(create_system_info_dialog), GTK_WINDOW(app->window));
    g_signal_connect_swapped(btn2, "clicked", G_CALLBACK(create_process_manager_dialog), GTK_WINDOW(app->window));
    g_signal_connect_swapped(btn3, "clicked", G_CALLBACK(create_memory_info_dialog), GTK_WINDOW(app->window));

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
}

// Entry point for custom command page (GTK aware)
void open_custom_command_page(AppData *app) {
    show_hamburger_menu_dialog(app);
}

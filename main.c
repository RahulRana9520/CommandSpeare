#include "custom_shell.h"

void activate(GtkApplication* app, gpointer user_data) {
    AppData *app_data = g_new0(AppData, 1);
    
    // Main window
    app_data->window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(app_data->window), "Command Sphere");
    gtk_window_set_default_size(GTK_WINDOW(app_data->window), 800, 600);
    
    // Create header bar with title and hamburger menu
    GtkWidget *header_bar = gtk_header_bar_new();
    gtk_header_bar_set_show_close_button(GTK_HEADER_BAR(header_bar), TRUE);
    gtk_window_set_titlebar(GTK_WINDOW(app_data->window), header_bar);
    
    // Title label
    GtkWidget *title_label = gtk_label_new("Command Sphere");
    gtk_widget_set_name(title_label, "title-label");
    gtk_header_bar_pack_start(GTK_HEADER_BAR(header_bar), title_label);
    
    // Create popup menu
    GtkWidget *menu = gtk_menu_new();
    
    // Menu items
    GtkWidget *theme_item = gtk_menu_item_new_with_label("Switch Theme");
    g_signal_connect(theme_item, "activate", G_CALLBACK(on_theme_clicked), app_data);
    gtk_widget_show(theme_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), theme_item);
    
    GtkWidget *time_item = gtk_menu_item_new_with_label("Show Time");
    g_signal_connect(time_item, "activate", G_CALLBACK(on_time_clicked), app_data);
    gtk_widget_show(time_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), time_item);
    
    GtkWidget *history_item = gtk_menu_item_new_with_label("Show History");
    g_signal_connect(history_item, "activate", G_CALLBACK(on_history_clicked), app_data);
    gtk_widget_show(history_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), history_item);
    
    GtkWidget *clear_item = gtk_menu_item_new_with_label("Clear Output");
    g_signal_connect(clear_item, "activate", G_CALLBACK(on_clear_clicked), app_data);
    gtk_widget_show(clear_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), clear_item);
    
    // Separator
    GtkWidget *separator = gtk_separator_menu_item_new();
    gtk_widget_show(separator);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), separator);
    
    // System Monitor menu items
    GtkWidget *system_monitor_item = gtk_menu_item_new_with_label("System Monitor");
    g_signal_connect(system_monitor_item, "activate", G_CALLBACK(on_system_monitor_clicked), app_data);
    gtk_widget_show(system_monitor_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), system_monitor_item);
    
    GtkWidget *process_manager_item = gtk_menu_item_new_with_label("Process Manager");
    g_signal_connect(process_manager_item, "activate", G_CALLBACK(on_process_manager_clicked), app_data);
    gtk_widget_show(process_manager_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), process_manager_item);
    
    GtkWidget *memory_monitor_item = gtk_menu_item_new_with_label("Memory Monitor");
    g_signal_connect(memory_monitor_item, "activate", G_CALLBACK(on_memory_monitor_clicked), app_data);
    gtk_widget_show(memory_monitor_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), memory_monitor_item);
    
    GtkWidget *filesystem_monitor_item = gtk_menu_item_new_with_label("Filesystem Monitor");
    g_signal_connect(filesystem_monitor_item, "activate", G_CALLBACK(on_filesystem_monitor_clicked), app_data);
    gtk_widget_show(filesystem_monitor_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), filesystem_monitor_item);
    
    GtkWidget *network_info_item = gtk_menu_item_new_with_label("Network Info");
    g_signal_connect(network_info_item, "activate", G_CALLBACK(on_network_info_clicked), app_data);
    gtk_widget_show(network_info_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), network_info_item);
    
    GtkWidget *disk_usage_item = gtk_menu_item_new_with_label("Disk Usage");
    g_signal_connect(disk_usage_item, "activate", G_CALLBACK(on_disk_usage_clicked), app_data);
    gtk_widget_show(disk_usage_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), disk_usage_item);
    
    // Show the menu
    gtk_widget_show_all(menu);
    
    // Hamburger menu button (positioned on the right)
    GtkWidget *menu_button = gtk_menu_button_new();
    gtk_button_set_label(GTK_BUTTON(menu_button), "☰");
    gtk_menu_button_set_popup(GTK_MENU_BUTTON(menu_button), menu);
    gtk_widget_set_name(menu_button, "menu-button");
    gtk_widget_show(menu_button);
    gtk_header_bar_pack_end(GTK_HEADER_BAR(header_bar), menu_button);
    
    // Main vertical box
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_add(GTK_CONTAINER(app_data->window), vbox);
    
    // Text view with scrolled window
    GtkWidget *scrolled_window = gtk_scrolled_window_new(NULL, NULL);
    app_data->textview = GTK_TEXT_VIEW(gtk_text_view_new());
    gtk_text_view_set_editable(app_data->textview, FALSE);
    gtk_text_view_set_cursor_visible(app_data->textview, FALSE);
    gtk_container_add(GTK_CONTAINER(scrolled_window), GTK_WIDGET(app_data->textview));
    gtk_box_pack_start(GTK_BOX(vbox), scrolled_window, TRUE, TRUE, 0);
    
    // Buffer for text view
    app_data->buffer = gtk_text_view_get_buffer(app_data->textview);
    
    // Entry container for input and buttons
    GtkWidget *entry_container = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    // Horizontal box for entry and buttons
    GtkWidget *entry_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    // Command entry
    app_data->entry = gtk_entry_new();
    gtk_entry_set_placeholder_text(GTK_ENTRY(app_data->entry), "Enter command...");
    g_signal_connect(app_data->entry, "activate", G_CALLBACK(on_run_clicked), app_data);
    g_signal_connect(app_data->entry, "key-press-event", G_CALLBACK(on_entry_key_press), app_data);
    g_signal_connect(app_data->entry, "changed", G_CALLBACK(on_entry_changed), app_data);
    gtk_box_pack_start(GTK_BOX(entry_box), app_data->entry, TRUE, TRUE, 0);
    
    // Run button
    app_data->run_button = gtk_button_new_with_label("▶");
    gtk_widget_set_size_request(app_data->run_button, 50, 40);
    gtk_widget_set_name(app_data->run_button, "run-button");
    g_signal_connect(app_data->run_button, "clicked", G_CALLBACK(on_run_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(entry_box), app_data->run_button, FALSE, FALSE, 0);
    
    // Voice button with microphone image
    GtkWidget *mic_image = gtk_image_new_from_file("mic.svg");
    app_data->voice_button = gtk_button_new();
    gtk_button_set_image(GTK_BUTTON(app_data->voice_button), mic_image);
    gtk_widget_set_size_request(app_data->voice_button, 60, 40);
    gtk_widget_set_name(app_data->voice_button, "voice-button");
    g_signal_connect(app_data->voice_button, "clicked", G_CALLBACK(on_voice_clicked), app_data);
    gtk_box_pack_start(GTK_BOX(entry_box), app_data->voice_button, FALSE, FALSE, 0);
    
    // Pack entry box into container
    gtk_box_pack_start(GTK_BOX(entry_container), entry_box, FALSE, FALSE, 0);
    
    // Auto-suggestion popup
    app_data->suggestion_popup = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(app_data->suggestion_popup),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(app_data->suggestion_popup, -1, 150);
    
    app_data->suggestion_listbox = gtk_list_box_new();
    gtk_container_add(GTK_CONTAINER(app_data->suggestion_popup), app_data->suggestion_listbox);
    gtk_box_pack_start(GTK_BOX(entry_container), app_data->suggestion_popup, FALSE, FALSE, 0);
    gtk_widget_set_no_show_all(app_data->suggestion_popup, TRUE);
    
    // Add entry container to main box
    gtk_box_pack_start(GTK_BOX(vbox), entry_container, FALSE, FALSE, 0);
    
    // CSS styling for professional appearance
    GtkCssProvider *css_provider = gtk_css_provider_new();
    const char *css_data =
        "window { "
        "  background: #1a1a1a; "
        "} "
        "#run-button { "
        "  background: #333333; "
        "  color: white; "
        "  border: 1px solid #666666; "
        "  border-radius: 8px; "
        "  font-size: 20px; "
        "  font-weight: bold; "
        "} "
        "#run-button:hover { "
        "  background: #555555; "
        "  border: 1px solid #888888; "
        "} "
        "#voice-button { "
        "  background: #333333; "
        "  color: white; "
        "  border: 1px solid #666666; "
        "  border-radius: 8px; "
        "  font-size: 12px; "
        "  font-weight: bold; "
        "} "
        "#voice-button:hover { "
        "  background: #555555; "
        "  border: 1px solid #888888; "
        "} "
        "#menu-button { "
        "  background: #333333; "
        "  color: white; "
        "  border-radius: 8px; "
        "  border: 1px solid #666666; "
        "  padding: 8px 12px; "
        "  font-size: 16px; "
        "} "
        "#menu-button:hover { "
        "  background: #555555; "
        "  border: 1px solid #888888; "
        "} "
        "#title-label { "
        "  color: white; "
        "  font-size: 18px; "
        "  font-weight: bold; "
        "  border: 2px solid #666666; "
        "  border-radius: 12px; "
        "  padding: 12px 20px; "
        "  background: #333333; "
        "} "
        "entry { "
        "  background: #2a2a2a; "
        "  color: white; "
        "  border: 2px solid #666666; "
        "  border-radius: 8px; "
        "  padding: 10px 15px; "
        "  font-size: 14px; "
        "  min-height: 40px; "
        "} "
        "entry:focus { "
        "  border-color: #888888; "
        "  box-shadow: 0 0 0 3px rgba(255, 255, 255, 0.1); "
        "} "
        "textview { "
        "  background: #1a1a1a; "
        "  color: white; "
        "} "
        "menu { "
        "  background: #2a2a2a; "
        "  border: 1px solid #666666; "
        "} "
        "menuitem { "
        "  background: #2a2a2a; "
        "  color: white; "
        "  padding: 8px 12px; "
        "} "
        "menuitem:hover { "
        "  background: #444444; "
        "} ";
    
    gtk_css_provider_load_from_data(css_provider, css_data, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                              GTK_STYLE_PROVIDER(css_provider),
                                              GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    
    // Initialize app data
    app_data->theme_index = 0;
    app_data->css_provider = gtk_css_provider_new();
    app_data->history_count = 0;
    app_data->history_index = -1;
    app_data->is_recording = FALSE;
    
    for (int i = 0; i < MAX_HISTORY; i++) {
        app_data->command_history[i] = NULL;
    }
    
    // Create text buffer tags
    gtk_text_buffer_create_tag(app_data->buffer, "default", "foreground", "black", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "cd", "foreground", "blue", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "ls", "foreground", "green", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "pwd", "foreground", "purple", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "echo", "foreground", "orange", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "cat", "foreground", "red", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "calc", "foreground", "brown", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "help", "foreground", "gray", NULL);
    gtk_text_buffer_create_tag(app_data->buffer, "welcome", "font", "24", NULL);
    
    // Add welcome message
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app_data->buffer, &iter);
    gtk_text_buffer_insert(app_data->buffer, &iter, "\n=== Command Sphere ===\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "\nWelcome to Command Sphere!\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "\nUnique Features:\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Built-in Calculator (e.g., calc 2+3*5)\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Theme Switching (Light, Dark, Hacker, Solarized)\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Command History with Up/Down arrows\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Real-time Time Display\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Color-coded Command Output\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Voice Recognition (Click MIC button)\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "- Smart Auto-Suggestions (Start typing)\n", -1);
    gtk_text_buffer_insert(app_data->buffer, &iter, "\n$ ", -1);
    
    gtk_widget_show_all(app_data->window);
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("org.example.shell", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return status;
}

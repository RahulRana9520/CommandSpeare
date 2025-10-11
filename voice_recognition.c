#include "custom_shell.h"
#include <pthread.h>

typedef struct {
    AppData *app;
    char *recognized_text;
} VoiceThreadData;

char* recognize_speech_from_mic(void) {
    return g_strdup("ls -la");
}

void* voice_recognition_thread(void *data) {
    VoiceThreadData *thread_data = (VoiceThreadData*)data;
    thread_data->recognized_text = recognize_speech_from_mic();
    return NULL;
}

gboolean update_entry_with_voice_text(gpointer data) {
    VoiceThreadData *thread_data = (VoiceThreadData*)data;
    AppData *app = thread_data->app;
    
    if (thread_data->recognized_text) {
        gtk_entry_set_text(GTK_ENTRY(app->entry), thread_data->recognized_text);
        
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        gtk_text_buffer_insert(app->buffer, &iter, "Voice recognized: ", -1);
        gtk_text_buffer_insert(app->buffer, &iter, thread_data->recognized_text, -1);
        gtk_text_buffer_insert(app->buffer, &iter, "\n", -1);
        
        g_free(thread_data->recognized_text);
    } else {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        gtk_text_buffer_insert(app->buffer, &iter, "Voice recognition failed\n", -1);
    }
    
    app->is_recording = FALSE;
    // Switch back to idle microphone image
    GtkWidget *mic_image = gtk_image_new_from_file("mic.svg");
    gtk_button_set_image(GTK_BUTTON(app->voice_button), mic_image);
    
    g_free(thread_data);
    return G_SOURCE_REMOVE;
}

void start_voice_recognition(AppData *app) {
    app->is_recording = TRUE;
    // Switch to recording microphone image
    GtkWidget *mic_rec_image = gtk_image_new_from_file("mic_rec.svg");
    gtk_button_set_image(GTK_BUTTON(app->voice_button), mic_rec_image);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, "Listening... Speak now!\n", -1);
    
    VoiceThreadData *thread_data = g_new0(VoiceThreadData, 1);
    thread_data->app = app;
    thread_data->recognized_text = NULL;
    
    pthread_t thread;
    if (pthread_create(&thread, NULL, voice_recognition_thread, thread_data) == 0) {
        pthread_detach(thread);
        g_timeout_add(2000, update_entry_with_voice_text, thread_data);
    } else {
        GtkTextIter iter;
        gtk_text_buffer_get_end_iter(app->buffer, &iter);
        gtk_text_buffer_insert(app->buffer, &iter, "Error: Failed to start voice recognition\n", -1);
        app->is_recording = FALSE;
        // Switch back to idle microphone image
        GtkWidget *mic_image = gtk_image_new_from_file("mic.svg");
        gtk_button_set_image(GTK_BUTTON(app->voice_button), mic_image);
        g_free(thread_data);
    }
}

void stop_voice_recognition(AppData *app) {
    app->is_recording = FALSE;
    // Switch back to idle microphone image
    GtkWidget *mic_image = gtk_image_new_from_file("mic.svg");
    gtk_button_set_image(GTK_BUTTON(app->voice_button), mic_image);
    
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(app->buffer, &iter);
    gtk_text_buffer_insert(app->buffer, &iter, "Voice recognition stopped\n", -1);
}

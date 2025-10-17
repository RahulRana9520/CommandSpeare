/*
 * VOICE RECOGNITION - Pure Transcript Mode
 * Speak anything -> exact text in prompt -> press Enter to run
 */

#include "custom_shell.h"
#include <sys/wait.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>

#define VOICE_DEBUG 1
#define AUDIO_SAMPLE_RATE 16000
#define AUDIO_CHANNELS 1
#define RECORDING_DURATION 3

int record_audio_robust(const char* output_file, int duration_seconds) {
    char command[512];
    int result;
    
    if (VOICE_DEBUG) printf("\n🎤 Recording for %d seconds...\n", duration_seconds);
    
    // Try PulseAudio/PipeWire with explicit source (most reliable for modern systems)
    snprintf(command, sizeof(command), 
        "timeout --signal=SIGTERM %d parecord --format=s16le --rate=%d --channels=%d --file-format=wav --volume=65536 %s 2>/dev/null", 
        duration_seconds, AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, output_file);
    result = system(command);
    // timeout returns 124 when it kills the process, which is success for us
    if (result == 0 || result == 124 || WEXITSTATUS(result) == 124) {
        struct stat st;
        if (stat(output_file, &st) == 0 && st.st_size > 1024) {
            if (VOICE_DEBUG) printf("✅ Recorded: %ld bytes (PulseAudio/PipeWire)\n", st.st_size);
            return 0;
        }
    }
    
    // Try ALSA with default device
    snprintf(command, sizeof(command),
        "arecord -D default -f S16_LE -r %d -c %d -d %d %s 2>/dev/null",
        AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, duration_seconds, output_file);
    result = system(command);
    if (result == 0) {
        struct stat st;
        if (stat(output_file, &st) == 0 && st.st_size > 1024) {
            if (VOICE_DEBUG) printf("✅ Recorded: %ld bytes (ALSA)\n", st.st_size);
            return 0;
        }
    }
    
    // Try ALSA with pipewire device
    snprintf(command, sizeof(command),
        "arecord -D pipewire -f S16_LE -r %d -c %d -d %d %s 2>/dev/null",
        AUDIO_SAMPLE_RATE, AUDIO_CHANNELS, duration_seconds, output_file);
    result = system(command);
    if (result == 0) {
        struct stat st;
        if (stat(output_file, &st) == 0 && st.st_size > 1024) {
            if (VOICE_DEBUG) printf("✅ Recorded: %ld bytes (PipeWire)\n", st.st_size);
            return 0;
        }
    }
    
    printf("❌ Recording failed - microphone may not be working\n");
    printf("💡 This might be a virtual machine - try host system microphone settings\n");
    return -1;
}

int transcribe_with_python(const char* wav_file, char* out_text, size_t out_len) {
    if (!wav_file || !out_text || out_len == 0) return -1;
    out_text[0] = '\0';
    
    struct stat st;
    if (stat("./stt_helper.py", &st) != 0) {
        if (VOICE_DEBUG) printf("⚠️  stt_helper.py not found\n");
        return -1;
    }
    
    char command[1024];
    snprintf(command, sizeof(command), "python3 ./stt_helper.py '%s' 2>/dev/null", wav_file);
    
    if (VOICE_DEBUG) printf("🐍 Transcribing...\n");
    
    FILE* pipe = popen(command, "r");
    if (!pipe) return -1;
    
    if (fgets(out_text, out_len, pipe)) {
        size_t len = strlen(out_text);
        while (len > 0 && (out_text[len-1] == '\n' || out_text[len-1] == '\r' || out_text[len-1] == ' ')) {
            out_text[--len] = '\0';
        }
    }
    
    pclose(pipe);
    
    if (strlen(out_text) > 0) {
        if (VOICE_DEBUG) printf("✅ \"%s\"\n", out_text);
        return 0;
    }
    
    return -1;
}

char* recognize_speech_from_mic(void) {
    const char* temp_audio = "/tmp/voice_recording.wav";
    
    printf("\n━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n");
    printf("🎙️  SPEAK NOW\n");
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
    
    if (record_audio_robust(temp_audio, RECORDING_DURATION) != 0) {
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        return NULL;
    }
    
    char transcript[512] = {0};
    if (transcribe_with_python(temp_audio, transcript, sizeof(transcript)) != 0) {
        printf("❌ Transcription failed\n");
        printf("💡 Run: pip3 install SpeechRecognition\n");
        printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
        unlink(temp_audio);
        return NULL;
    }
    
    unlink(temp_audio);
    printf("━━━━━━━━━━━━━━━━━━━━━━━━━━━━\n\n");
    
    return g_strdup(transcript);
}

// Stub implementations for header compatibility
int check_audio_file_valid(const char* filename) { return 1; }
int analyze_audio_advanced(const char* audio_file, AudioAnalysis* analysis) { return 0; }
int detect_voice_segments(short* samples, size_t count) { return 0; }
double estimate_fundamental_frequency(short* samples, size_t count) { return 0; }
double calculate_voice_confidence(AudioAnalysis* analysis) { return 0; }
void initialize_default_patterns(void) {}
void add_voice_pattern(const char* command, double freq, double energy, double duration) {}
char* match_voice_pattern(AudioAnalysis* analysis) { return NULL; }
void update_voice_pattern(const char* command, AudioAnalysis* analysis) {}
void load_voice_profile(void) {}
void save_voice_profile(void) {}
char* interactive_command_learning(AudioAnalysis* analysis) { return NULL; }
void learn_new_pattern(const char* command, AudioAnalysis* analysis) {}
char* voice_training_session(void) { return NULL; }
char* voice_fallback_system(void) { return NULL; }
int try_external_stt(const char* wav_file, char* out_text, size_t out_len) { return -1; }

#include "custom_shell.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

// Calculate Levenshtein distance (edit distance) between two strings
static int levenshtein_distance(const char *s1, const char *s2) {
    int len1 = strlen(s1);
    int len2 = strlen(s2);
    
    if (len1 == 0) return len2;
    if (len2 == 0) return len1;
    
    // Create matrix
    int matrix[len1 + 1][len2 + 1];
    
    // Initialize first row and column
    for (int i = 0; i <= len1; i++) matrix[i][0] = i;
    for (int j = 0; j <= len2; j++) matrix[0][j] = j;
    
    // Fill matrix
    for (int i = 1; i <= len1; i++) {
        for (int j = 1; j <= len2; j++) {
            int cost = (tolower(s1[i-1]) == tolower(s2[j-1])) ? 0 : 1;
            
            int deletion = matrix[i-1][j] + 1;
            int insertion = matrix[i][j-1] + 1;
            int substitution = matrix[i-1][j-1] + cost;
            
            matrix[i][j] = deletion < insertion ? deletion : insertion;
            matrix[i][j] = matrix[i][j] < substitution ? matrix[i][j] : substitution;
        }
    }
    
    return matrix[len1][len2];
}

// Common shell commands to check against
static const char* common_commands[] = {
    "ls", "cd", "pwd", "cat", "echo", "mkdir", "rmdir", "rm", "cp", "mv",
    "touch", "chmod", "chown", "grep", "find", "ps", "kill", "top", "df",
    "du", "free", "clear", "exit", "man", "help", "history", "date", "cal",
    "wc", "sort", "uniq", "head", "tail", "less", "more", "vim", "nano",
    "git", "make", "gcc", "python", "node", "npm", "pip", "wget", "curl",
    "tar", "zip", "unzip", "ssh", "scp", "rsync", "htop", "awk", "sed",
    NULL
};

// Find the closest matching command
char* suggest_command(const char* wrong_command) {
    if (!wrong_command || strlen(wrong_command) == 0) {
        return NULL;
    }
    
    // Extract just the command name (before first space)
    char cmd_only[256];
    const char* space = strchr(wrong_command, ' ');
    if (space) {
        int len = space - wrong_command;
        if (len > 255) len = 255;
        strncpy(cmd_only, wrong_command, len);
        cmd_only[len] = '\0';
    } else {
        strncpy(cmd_only, wrong_command, 255);
        cmd_only[255] = '\0';
    }
    
    int min_distance = 999;
    const char* best_match = NULL;
    
    // Check against common commands
    for (int i = 0; common_commands[i] != NULL; i++) {
        int distance = levenshtein_distance(cmd_only, common_commands[i]);
        
        // Only suggest if distance is small (similar enough)
        if (distance < min_distance && distance <= 2) {
            min_distance = distance;
            best_match = common_commands[i];
        }
    }
    
    if (best_match) {
        return g_strdup(best_match);
    }
    
    return NULL;
}

// Check if a command likely exists in PATH
gboolean command_exists_in_path(const char* command) {
    char check_cmd[512];
    snprintf(check_cmd, sizeof(check_cmd), "command -v %s >/dev/null 2>&1", command);
    return (system(check_cmd) == 0);
}

// Suggest corrections for common typos
char* suggest_typo_fix(const char* text) {
    if (!text) return NULL;
    
    // Common typo patterns
    struct {
        const char* wrong;
        const char* correct;
    } typos[] = {
        {"sl", "ls"},
        {"dc", "cd"},
        {"pc", "cp"},
        {"vm", "mv"},
        {"celar", "clear"},
        {"claer", "clear"},
        {"exot", "exit"},
        {"grpe", "grep"},
        {"mkdri", "mkdir"},
        {"toch", "touch"},
        {"pytohn", "python"},
        {"pythno", "python"},
        {"gti", "git"},
        {"mroe", "more"},
        {"lses", "less"},
        {"ehco", "echo"},
        {NULL, NULL}
    };
    
    // Extract command
    char cmd_only[256];
    const char* space = strchr(text, ' ');
    if (space) {
        int len = space - text;
        if (len > 255) len = 255;
        strncpy(cmd_only, text, len);
        cmd_only[len] = '\0';
    } else {
        strncpy(cmd_only, text, 255);
        cmd_only[255] = '\0';
    }
    
    // Check for exact typo matches
    for (int i = 0; typos[i].wrong != NULL; i++) {
        if (strcasecmp(cmd_only, typos[i].wrong) == 0) {
            return g_strdup(typos[i].correct);
        }
    }
    
    return NULL;
}

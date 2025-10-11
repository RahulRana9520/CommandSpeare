#include "custom_shell.h"
#include <math.h>
// Function to compile and execute C code from a file (disabled)
gboolean compile_and_run_c_file(const char *filename, GtkTextBuffer *buffer) {
    GtkTextIter iter;
    gtk_text_buffer_get_end_iter(buffer, &iter);
    gtk_text_buffer_insert(buffer, &iter, "Error: C file compilation is currently disabled\n", -1);
    return FALSE;
}

// Updated precedence function
int precedence(char op) {
    if (op == '^') return 3;
    if (op == '*' || op == '/') return 2;
    if (op == '+' || op == '-') return 1;
    return 0;
}

// Updated apply_operator function
double apply_operator(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return (b != 0) ? a / b : 0; // Still returns 0 for division by zero
        case '^': return pow(a, b);
        default: return 0;
    }
}

// Helper function to check if character is part of a number
int is_number_char(char c) {
    return isdigit((unsigned char)c) || c == '.' || c == '-';
}

double calculate_expression(const char *expr) {
    double result = 0.0;
    char *copy = g_strdup(expr);
    char *pos = copy;
    double numbers[20];
    char operators[20];
    int num_count = 0, op_count = 0;
    int negative_number = 0;

    while (*pos) {
        while (*pos && isspace((unsigned char)*pos)) pos++;
        if (!*pos) break;

        // Handle negative numbers
        if (*pos == '-' && (pos == copy || strchr("+-*/^(", *(pos-1)))) {
            negative_number = 1;
            pos++;
            continue;
        }

        if (isdigit((unsigned char)*pos) || *pos == '.') {
            char *end;
            double num = strtod(pos, &end);
            if (negative_number) {
                num = -num;
                negative_number = 0;
            }
            if (end == pos) {
                g_free(copy);
                return 0.0;
            }
            if (num_count >= 20) {
                g_free(copy);
                return 0.0;
            }
            numbers[num_count++] = num;
            pos = end;
        } 
        else if (*pos == '(') {
            if (op_count >= 20) {
                g_free(copy);
                return 0.0;
            }
            operators[op_count++] = *pos++;
        } 
        else if (*pos == ')') {
            while (op_count > 0 && operators[op_count-1] != '(') {
                if (num_count < 2 || op_count < 1) {
                    g_free(copy);
                    return 0.0;
                }
                double b = numbers[--num_count];
                double a = numbers[--num_count];
                char op = operators[--op_count];
                numbers[num_count++] = apply_operator(a, b, op);
            }
            if (op_count == 0) {
                g_free(copy);
                return 0.0; // Mismatched parentheses
            }
            op_count--; // Remove the '('
            pos++;
        }
        else if (strchr("+-*/^", *pos)) {
            while (op_count > 0 && operators[op_count-1] != '(' && 
                   precedence(operators[op_count-1]) >= precedence(*pos)) {
                if (num_count < 2 || op_count < 1) {
                    g_free(copy);
                    return 0.0;
                }
                double b = numbers[--num_count];
                double a = numbers[--num_count];
                char op = operators[--op_count];
                numbers[num_count++] = apply_operator(a, b, op);
            }
            if (op_count >= 20) {
                g_free(copy);
                return 0.0;
            }
            operators[op_count++] = *pos++;
        }
        else {
            // Handle simple functions (like sqrt)
            if (strncmp(pos, "sqrt(", 5) == 0) {
                char *end = pos + 5;
                double arg = calculate_expression(end); // Recursively calculate argument
                if (num_count >= 20) {
                    g_free(copy);
                    return 0.0;
                }
                numbers[num_count++] = sqrt(arg);
                // Skip past the function and its argument
                while (*end && *end != ')') end++;
                if (*end == ')') end++;
                pos = end;
            }
            else if (strncmp(pos, "sin(", 4) == 0) {
                char *end = pos + 4;
                double arg = calculate_expression(end); // Recursively calculate argument
                if (num_count >= 20) {
                    g_free(copy);
                    return 0.0;
                }
                numbers[num_count++] = sin(arg);
                // Skip past the function and its argument
                while (*end && *end != ')') end++;
                if (*end == ')') end++;
                pos = end;
            }
            else if (strncmp(pos, "cos(", 4) == 0) {
                char *end = pos + 4;
                double arg = calculate_expression(end); // Recursively calculate argument
                if (num_count >= 20) {
                    g_free(copy);
                    return 0.0;
                }
                numbers[num_count++] = cos(arg);
                // Skip past the function and its argument
                while (*end && *end != ')') end++;
                if (*end == ')') end++;
                pos = end;
            }
            else if (strncmp(pos, "tan(", 4) == 0) {
                char *end = pos + 4;
                double arg = calculate_expression(end); // Recursively calculate argument
                if (num_count >= 20) {
                    g_free(copy);
                    return 0.0;
                }
                numbers[num_count++] = tan(arg);
                // Skip past the function and its argument
                while (*end && *end != ')') end++;
                if (*end == ')') end++;
                pos = end;
            }
            else {
                pos++; // Skip unknown characters
            }
        }
    }

    while (op_count > 0) {
        if (operators[op_count-1] == '(') {
            g_free(copy);
            return 0.0; // Mismatched parentheses
        }
        if (num_count < 2 || op_count < 1) {
            g_free(copy);
            return 0.0;
        }
        double b = numbers[--num_count];
        double a = numbers[--num_count];
        char op = operators[--op_count];
        numbers[num_count++] = apply_operator(a, b, op);
    }

    result = (num_count > 0) ? numbers[0] : 0.0;
    g_free(copy);
    return result;
}

void execute_command(const char *command, GtkTextBuffer *buffer, GtkTextView *textview) {
    GtkTextIter iter;
    if (strlen(command) > MAX_COMMAND_LENGTH) {
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, "Error: Command too long\n", -1);
        return;
    }

    if (!g_utf8_validate(command, -1, NULL)) {
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, "Error: Invalid command (contains non-UTF-8 characters)\n", -1);
        return;
    }

    char *sanitized_command = g_strdup(command);
    char *start = sanitized_command;
    while (*start && isspace((unsigned char)*start)) start++;
    char *end = start + strlen(start);
    while (end > start && isspace((unsigned char)*(end - 1))) end--;
    *end = '\0';

    // Auto-detect math expressions (if starts with digit, -, or ( and contains math operators)
    if (strlen(start) > 0) {
        gboolean looks_like_math = FALSE;
        if (isdigit((unsigned char)start[0]) || start[0] == '-' || start[0] == '(' || start[0] == '.') {
            // Check if it contains math operators and only valid math characters
            gboolean has_operator = FALSE;
            for (const char *p = start; *p; p++) {
                if (*p == '+' || *p == '*' || *p == '/' || *p == '^') {
                    has_operator = TRUE;
                }
                if (!isdigit((unsigned char)*p) && !isspace((unsigned char)*p) && 
                    *p != '+' && *p != '-' && *p != '*' && *p != '/' && *p != '^' && 
                    *p != '(' && *p != ')' && *p != '.') {
                    has_operator = FALSE; // Invalid character for math
                    break;
                }
            }
            looks_like_math = has_operator;
        }
        
        if (looks_like_math) {
            // Treat as calculator expression
            gtk_text_buffer_get_end_iter(buffer, &iter);
            double result = calculate_expression(start);
            char result_str[128];
            snprintf(result_str, sizeof(result_str), "🧮 Expression: %s\n   Result: %.6g\n", start, result);
            gtk_text_buffer_insert(buffer, &iter, result_str, -1);
            g_free(sanitized_command);
            return;
        }
    }

    if (strlen(start) > 2 && strcmp(start + strlen(start) - 2, ".c") == 0) {
        compile_and_run_c_file(start, buffer);
        g_free(sanitized_command);
        return;
    }

    if (strncmp(start, "cd ", 3) == 0 || strcmp(start, "cd") == 0) {
        const char *path = (strcmp(start, "cd") == 0) ? getenv("HOME") : start + 3;
        if (!path || !*path) path = getenv("HOME");
        if (!path) {
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, "cd: HOME not set\n", -1);
            g_free(sanitized_command);
            return;
        }

        char *expanded_path = g_strdup(path);
        if (path[0] == '~' && (path[1] == '/' || path[1] == '\0')) {
            const char *home = getenv("HOME");
            if (!home) {
                gtk_text_buffer_get_end_iter(buffer, &iter);
                gtk_text_buffer_insert(buffer, &iter, "cd: HOME not set\n", -1);
                g_free(sanitized_command);
                g_free(expanded_path);
                return;
            }
            if (path[1] == '\0') {
                g_free(expanded_path);
                expanded_path = g_strdup(home);
            } else {
                char *temp = g_strconcat(home, path + 1, NULL);
                g_free(expanded_path);
                expanded_path = temp;
            }
        }

        char *path_copy = g_strdup(expanded_path);
        char *path_start = path_copy;
        while (*path_start && isspace((unsigned char)*path_start)) path_start++;
        char *path_end = path_start + strlen(path_start);
        while (path_end > path_start && isspace((unsigned char)*(path_end - 1))) path_end--;
        *path_end = '\0';

        gtk_text_buffer_get_end_iter(buffer, &iter);
        if (chdir(path_start) != 0) {
            gtk_text_buffer_insert(buffer, &iter, "cd: ", -1);
            gtk_text_buffer_insert(buffer, &iter, strerror(errno), -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        } else {
            char cwd[1024];
            if (getcwd(cwd, sizeof(cwd))) {
                gtk_text_buffer_insert(buffer, &iter, "Changed to: ", -1);
                gtk_text_buffer_insert(buffer, &iter, cwd, -1);
                gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            }
        }
        g_free(path_copy);
        g_free(expanded_path);
    } else if (strncmp(start, "calc ", 5) == 0 || strcmp(start, "calc") == 0) {
        // Built-in calculator - handles arithmetic expressions
        gtk_text_buffer_get_end_iter(buffer, &iter);
        
        if (strcmp(start, "calc") == 0) {
            // No expression provided
            gtk_text_buffer_insert(buffer, &iter, "🧮 Calculator - Usage: calc <expression>\n", -1);
            gtk_text_buffer_insert(buffer, &iter, "\nExamples:\n", -1);
            gtk_text_buffer_insert(buffer, &iter, "  calc 2+3*5     → Result: 17\n", -1);
            gtk_text_buffer_insert(buffer, &iter, "  calc (2+3)*5   → Result: 25\n", -1);
            gtk_text_buffer_insert(buffer, &iter, "  calc 2^3       → Result: 8\n", -1);
            gtk_text_buffer_insert(buffer, &iter, "  calc -5+10     → Result: 5\n", -1);
            gtk_text_buffer_insert(buffer, &iter, "  calc 10/3      → Result: 3.33333\n", -1);
        } else {
            // Expression provided
            const char *expr = start + 5;
            
            // Skip leading whitespace in expression
            while (*expr && isspace((unsigned char)*expr)) expr++;
            
            if (strlen(expr) == 0) {
                gtk_text_buffer_insert(buffer, &iter, "Error: No expression provided. Usage: calc 2+3*5\n", -1);
            } else {
                double result = calculate_expression(expr);
                char result_str[128];
                snprintf(result_str, sizeof(result_str), "🧮 Expression: %s\n   Result: %.6g\n", expr, result);
                gtk_text_buffer_insert(buffer, &iter, result_str, -1);
            }
        }
        g_free(sanitized_command);
        return; // Important: return here to prevent falling through to popen
    } else if (strncmp(start, "help", 4) == 0 && (strlen(start) == 4 || isspace((unsigned char)start[4]))) {
        gtk_text_buffer_get_end_iter(buffer, &iter);
        gtk_text_buffer_insert(buffer, &iter, "\nBuilt-in Commands:\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  cd [path]    - Change directory (use 'cd' for home or 'cd ~' for home)\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  ls           - List directory contents\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  pwd          - Print working directory\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  echo [text]  - Display text or variables\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  cat [file]   - Display file contents\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  calc [expr]  - Evaluate arithmetic expression (e.g., calc 2+3*5)\n", -1);
        gtk_text_buffer_insert(buffer, &iter, "  help [cmd]   - Show this help or command-specific documentation\n", -1);
        if (strlen(start) > 5) {
            const char *cmd = start + 5;
            while (*cmd && isspace((unsigned char)*cmd)) cmd++;
            if (strcmp(cmd, "cd") == 0) {
                gtk_text_buffer_insert(buffer, &iter, "  cd: Changes the current directory. Use 'cd' for home or 'cd /path' to specify.\n", -1);
            } else if (strcmp(cmd, "ls") == 0) {
                gtk_text_buffer_insert(buffer, &iter, "  ls: Lists files and directories in the current directory.\n", -1);
            } else if (strcmp(cmd, "calc") == 0) {
                gtk_text_buffer_insert(buffer, &iter, "  calc: Evaluates simple arithmetic expressions (e.g., calc 2+3*5).\n", -1);
            } else if (strcmp(cmd, "sin") == 0) {
                gtk_text_buffer_insert(buffer, &iter, "  sin: Calculates the sine of an angle in radians.\n", -1);
            } else if (strcmp(cmd, "cos") == 0) {
                gtk_text_buffer_insert(buffer, &iter, "  cos: Calculates the cosine of an angle in radians.\n", -1);
            } else if (strcmp(cmd, "tan") == 0) {
                gtk_text_buffer_insert(buffer, &iter, "  tan: Calculates the tangent of an angle in radians.\n", -1);
            } else {
                gtk_text_buffer_insert(buffer, &iter, "  No specific help for this command.\n", -1);
            }
        }
    } else {
        char full_command[2048];
        snprintf(full_command, sizeof(full_command), "%s 2>&1", start);

        FILE *fp = popen(full_command, "r");
        if (!fp) {
            gtk_text_buffer_get_end_iter(buffer, &iter);
            gtk_text_buffer_insert(buffer, &iter, "Failed to execute command: ", -1);
            gtk_text_buffer_insert(buffer, &iter, strerror(errno), -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
            g_free(sanitized_command);
            return;
        }

        gtk_text_buffer_get_end_iter(buffer, &iter);
        char path[1024];
        while (fgets(path, sizeof(path), fp)) {
            if (!g_utf8_validate(path, -1, NULL)) {
                char *valid_utf8 = g_utf8_make_valid(path, -1);
                gtk_text_buffer_insert(buffer, &iter, valid_utf8, -1);
                g_free(valid_utf8);
            } else {
                gtk_text_buffer_insert(buffer, &iter, path, -1);
            }
        }

        int status = pclose(fp);
        if (status != 0) {
            char status_str[32];
            snprintf(status_str, sizeof(status_str), "%d", WEXITSTATUS(status));
            gtk_text_buffer_insert(buffer, &iter, "Command exited with non-zero status: ", -1);
            gtk_text_buffer_insert(buffer, &iter, status_str, -1);
            gtk_text_buffer_insert(buffer, &iter, "\n", -1);
        }
    }

    g_free(sanitized_command);

    gtk_text_buffer_get_end_iter(buffer, &iter);
    GtkTextMark *mark = gtk_text_buffer_create_mark(buffer, "end", &iter, FALSE);
    gtk_text_view_scroll_to_mark(GTK_TEXT_VIEW(textview), mark, 0.0, FALSE, 0.0, 0.0);
    gtk_text_buffer_delete_mark(buffer, mark);
}

void apply_css(AppData *app, const char *css) {
    if (app->css_provider) {
        gtk_style_context_remove_provider_for_screen(gdk_screen_get_default(),
                                                    GTK_STYLE_PROVIDER(app->css_provider));
        g_object_unref(app->css_provider);
    }

    app->css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(app->css_provider, css, -1, NULL);
    
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(),
                                             GTK_STYLE_PROVIDER(app->css_provider),
                                             GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
}

void cycle_theme(AppData *app) {
    app->theme_index = (app->theme_index + 1) % 4;

    const char *css;

    if (app->theme_index == 0) { // Light Theme
        css = "* { font-family: 'Lucida Console', 'Courier New', monospace; font-size: 15px; background-color: #f5f7fa; color: #2d3436; } "
              "GtkWindow { background-color: #ffffff; } "
              "GtkTextView, GtkEntry { background-color: #ffffff; border: 1px solid #ccc5a8; border-radius: 10px; padding: 12px; } "
              "GtkEntry:focus { border: 2px solid #3498db; background-color: #f0f4f8; } "
              "GtkButton { background-image: linear-gradient(to bottom, #f5eac5, #e0dbb8); color: #2d3436; border: 1px solid #c2ba94; border-radius: 12px; padding: 10px 16px; box-shadow: 0 2px 4px rgba(0,0,0,0.2); transition: all 0.2s ease-in-out; } "
              "GtkButton:hover { background-color: #e3ddc4; } "
              "GtkButton:active { background-color: #d8d1b8; } ";
        gtk_button_set_label(GTK_BUTTON(app->theme_button), "Light Theme");
    } else if (app->theme_index == 1) { // Dark Theme
        css = "* { font-family: 'Menlo', 'Fira Code', monospace; font-size: 15px; background-color: #1e1e1e; color: #ecf0f1; } "
              "GtkWindow { background-color: #1e1e1e; } "
              "GtkTextView, GtkEntry { background-color: #2c2c2e; border: 1px solid #ccc5a8; border-radius: 10px; padding: 12px; } "
              "GtkEntry:focus { border: 2px solid #0a84ff; background-color: #383838; } "
              "GtkButton { background-image: linear-gradient(to bottom, #3a3a3c, #2c2c2e); color: #ecf0f1; border: 1px solid #c2ba94; border-radius: 12px; padding: 10px 16px; box-shadow: 0 2px 4px rgba(0,0,0,0.4); transition: all 0.2s ease-in-out; } "
              "GtkButton:hover { background-color: #4d4d4f; } "
              "GtkButton:active { background-color: #3b3b3c; } ";
        gtk_button_set_label(GTK_BUTTON(app->theme_button), "Dark Theme");
    } else if (app->theme_index == 2) { // Hacker Theme
        css = "* { font-family: 'Hack', 'Courier New', monospace; font-size: 15px; background-color: #000000; color: #00ff00; } "
              "GtkWindow { background-color: #000000; } "
              "GtkTextView, GtkEntry { background-color: #0f0f0f; border: 1px solid #ccc5a8; border-radius: 10px; padding: 12px; } "
              "GtkEntry:focus { border: 2px solid #00ff00; background-color: #002b36; } "
              "GtkButton { background-color: #111; color: #00ff00; border: 1px solid #c2ba94; border-radius: 12px; padding: 10px 16px; box-shadow: 0 0 10px rgba(0,255,0,0.3); transition: all 0.3s ease-in-out; } "
              "GtkButton:hover { background-color: #003300; } "
              "GtkButton:active { background-color: #001a00; } ";
        gtk_button_set_label(GTK_BUTTON(app->theme_button), "Hacker Theme");
    } else { // Solarized Theme
        css = "* { font-family: 'Menlo', 'DejaVu Sans Mono', monospace; font-size: 15px; background-color: #fdf6e3; color: #657b83; } "
              "GtkWindow { background-color: #fdf6e3; } "
              "GtkTextView, GtkEntry { background-color: #eee8d5; border: 1px solid #ccc5a8; border-radius: 10px; padding: 12px; } "
              "GtkEntry:focus { border: 2px solid #b58900; background-color: #f5eac5; } "
              "GtkButton { background-image: linear-gradient(to bottom, #f5eac5, #e0dbb8); color: #586e75; border: 1px solid #c2ba94; border-radius: 12px; padding: 10px 16px; box-shadow: 0 2px 4px rgba(0,0,0,0.2); transition: all 0.2s ease-in-out; } "
              "GtkButton:hover { background-color: #e3ddc4; } "
              "GtkButton:active { background-color: #d8d1b8; } ";
        gtk_button_set_label(GTK_BUTTON(app->theme_button), "Solarized Theme");
    }

    apply_css(app, css);
}

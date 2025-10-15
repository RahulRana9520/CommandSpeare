# Custom Shell in C with Voice Recognition & Auto-Suggestions

A feature-rich custom shell built with GTK+3 in C, featuring voice recognition and intelligent auto-suggestions.

## Features

### Core Features
- **Command Execution**: Execute shell commands with real-time output
- **Built-in Calculator**: Evaluate arithmetic expressions (e.g., `calc 2+3*5`)
- **Theme Switching**: 4 beautiful themes (Light, Dark, Hacker, Solarized)
- **Command History**: Navigate with Up/Down arrows
- **Color-coded Output**: Different colors for different commands

### New Features
- **🎤 Voice Recognition**: Click the microphone button to speak commands
- **💡 Smart Auto-Suggestions**: Real-time command suggestions as you type
  - Suggests built-in commands
  - Suggests from command history
  - Suggests files/directories for relevant commands
  - Navigate suggestions with arrow keys
  - Apply suggestions with Tab key

## Installation

### Prerequisites
\`\`\`bash
sudo apt update
sudo apt install build-essential libgtk-3-dev libpthread-stubs0-dev
\`\`\`

### Compilation
\`\`\`bash
gcc main.c shell_functions.c callbacks.c utils.c auto_suggest.c voice_recognition.c -o main `pkg-config --cflags --libs gtk+-3.0` -lm -lpthread
\`\`\`

### Run
\`\`\`bash
./main
\`\`\`

## Usage

### Auto-Suggestions
1. Start typing a command
2. Suggestions appear automatically below the input field
3. Use **↓/↑** arrow keys to navigate suggestions
4. Press **Tab** to apply the selected suggestion
5. Press **Esc** to hide suggestions

### Voice Recognition
1. Click the **🎤 Voice** button
2. Speak your command clearly
3. The recognized text appears in the input field
4. Press Enter or click "Run Command" to execute

**Note**: The current implementation uses simulated voice recognition. For production use, integrate a real speech recognition library like PocketSphinx, Vosk, or Google Speech API.

## Implementing Real Voice Recognition

### Option 1: PocketSphinx (Offline)
\`\`\`bash
sudo apt install libpocketsphinx-dev pocketsphinx-en-us
\`\`\`
Update `voice_recognition.c` to use PocketSphinx API.

### Option 2: Vosk (Offline, Modern)
\`\`\`bash
# Download Vosk library and models from https://alphacephei.com/vosk/
\`\`\`
Integrate Vosk API in `voice_recognition.c`.

### Option 3: Google Speech API (Online)
Use libcurl to send audio to Google Speech API and parse JSON response.

## Built-in Commands
- `cd [path]` - Change directory
- `ls` - List directory contents
- `pwd` - Print working directory
- `echo [text]` - Display text
- `cat [file]` - Display file contents
- `calc [expression]` - Calculate arithmetic expressions
- `help [command]` - Show help information

## Keyboard Shortcuts
- **Enter** - Execute command
- **↑/↓** - Navigate command history (when no suggestions)
- **↑/↓** - Navigate suggestions (when suggestions visible)
- **Tab** - Apply selected suggestion
- **Esc** - Hide suggestions

## Authors
Made by Rahul, Priyanshu, and Sarthak

## License
Open source - feel free to modify and distribute!



//commands
1) for pID : echo "my process id is $$"
2) ls -la : for the directory files info 
3) cat README.md  ( to print all detail of that file ). 
4) 


//feaure : 
1) C compile 
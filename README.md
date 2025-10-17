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
  - Improved accuracy with noise reduction
  - Auto-correction for misheard commands
  - Visual confirmation of what was recognized
- **💡 Smart Auto-Suggestions**: Real-time command suggestions as you type
  - Suggests built-in commands
  - Suggests from command history
  - Suggests files/directories for relevant commands
  - Navigate suggestions with arrow keys
  - Apply suggestions with Tab key
- **🔧 Smart Error Correction**: When commands fail, get helpful suggestions
  - Typo detection (e.g., "sl" → suggests "ls")
  - Fuzzy matching with common commands
  - Levenshtein distance algorithm for similarity
  - Works for both typed and voice commands

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
2. Speak your command clearly for 3 seconds
3. The transcribed text appears with visual confirmation
4. **Auto-correction**: If the command looks wrong, you'll see a suggestion
5. Review/edit the text if needed
6. Press Enter or click "Run Command" to execute

**How it works**:
- Uses Google Web Speech API (free, accurate) via Python's SpeechRecognition library
- **Noise reduction** and ambient adjustment for better accuracy
- Fixes common transcription errors (e.g., "ls - la" → "ls -la")
- **Auto-converts to lowercase** (Google says "PWD" → becomes "pwd")
- **Smart suggestions**: If you say "celar", it suggests "clear"
- Transcribes ANY spoken text verbatim (commands, questions, nonsense, etc.)
- Valid commands execute normally; invalid text produces shell errors naturally
- UI remains responsive during recording (runs in separate thread)

**Requirements**:
\`\`\`bash
pip3 install SpeechRecognition --break-system-packages
\`\`\`

## Voice Recognition Architecture

The voice recognition system uses a hybrid Python/C approach:

**Workflow**:
1. C code (`voice_recognition.c`) records 3 seconds of audio using `parecord` or `arecord`
2. Python helper script (`stt_helper.py`) transcribes audio using Google Web Speech API
3. Transcribed text is returned to C code and displayed in prompt box
4. User presses Enter to execute the command

**Fallback Support**:
- Primary: Google Web Speech API (free, requires internet)
- Fallback: PocketSphinx (offline, install via: `pip3 install pocketsphinx`)

**Key Design Choice**:
- Pure transcription mode - NO pattern matching or command learning
- Speaks exactly what you say into the prompt box
- Shell naturally handles invalid commands with error messages

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
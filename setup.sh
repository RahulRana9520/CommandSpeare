#!/bin/bash
# Command Sphere Graphical Setup Script
# Requires: zenity, sudo privileges

APP_NAME="Command Sphere"
BIN_NAME="command-sphere"
ICON_NAME="command-sphere.png"
DESKTOP_FILE="$HOME/.local/share/applications/$BIN_NAME.desktop"
INSTALL_DIR="$HOME/.local/bin"

# Check for zenity
if ! command -v zenity &> /dev/null; then
    echo "Zenity not found. Installing..."
    sudo apt update && sudo apt install -y zenity
fi

zenity --info --title="$APP_NAME Setup" --text="Welcome to $APP_NAME setup!\n\nThis will install dependencies, build the app, and create a desktop shortcut."

# Install dependencies
(sleep 1; echo "20"; sleep 1; echo "40"; sleep 1; echo "60"; sleep 1; echo "80"; sleep 1; echo "100") |
zenity --progress --title="Installing dependencies" --text="Installing build tools, Python, GTK..." --percentage=0 --auto-close

sudo apt update
sudo apt install -y build-essential python3 python3-pip libgtk-3-dev zenity
pip3 install --user SpeechRecognition

# Build project
if make; then
    zenity --info --title="$APP_NAME" --text="Build successful!"
else
    zenity --error --title="$APP_NAME" --text="Build failed. Please check your environment."
    exit 1
fi

# Install binary
mkdir -p "$INSTALL_DIR"
cp main "$INSTALL_DIR/$BIN_NAME"


# Install icon to /usr/share/pixmaps for desktop environments
ICON_PATH="command-sphere"
if [ -f "$ICON_NAME" ]; then
    sudo cp "$ICON_NAME" /usr/share/pixmaps/command-sphere.png
fi

# Create .desktop file with icon name only (no extension)
cat > "$DESKTOP_FILE" <<EOF
[Desktop Entry]
Type=Application
Name=$APP_NAME
Exec=$INSTALL_DIR/$BIN_NAME
Icon=$ICON_PATH
Terminal=false
Categories=Utility;System;
EOF

zenity --info --title="$APP_NAME" --text="Installation complete!\nYou can now find '$APP_NAME' in your application menu."

exit 0

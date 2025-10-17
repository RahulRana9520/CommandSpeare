#!/usr/bin/env python3
"""
Simple Speech-to-Text helper for custom shell voice recognition.
Reads a WAV file and outputs the raw transcript to stdout.
Uses Google Web Speech API (free, no key needed) with pocketsphinx fallback if installed.
"""
import sys
import os

def transcribe_audio(wav_path):
    """Transcribe audio file and return raw text."""
    try:
        import speech_recognition as sr
    except ImportError:
        print("ERROR: speech_recognition not installed. Run: pip3 install SpeechRecognition", file=sys.stderr)
        return None

    recognizer = sr.Recognizer()
    
    # Adjust for ambient noise and energy threshold for better accuracy
    recognizer.energy_threshold = 300
    recognizer.dynamic_energy_threshold = True
    recognizer.pause_threshold = 0.5
    
    try:
        with sr.AudioFile(wav_path) as source:
            # Remove background noise
            recognizer.adjust_for_ambient_noise(source, duration=0.3)
            audio = recognizer.record(source)
        
        # Try Google Web Speech API first (free, no API key)
        # Use language hints for better command recognition
        try:
            text = recognizer.recognize_google(
                audio,
                language="en-US",
                show_all=False
            )
            # Clean up common transcription issues
            text = text.strip()
            # Fix common command mishearings
            text = text.replace(" - ", "-")  # "ls - la" → "ls -la"
            text = text.replace(" -- ", "--")  # "git -- help" → "git --help"
            # Convert to lowercase for Linux commands (they're case-sensitive!)
            text = text.lower()
            return text
        except sr.UnknownValueError:
            # Speech was unintelligible
            return None
        except sr.RequestError:
            # Google API unavailable, try pocketsphinx if available
            try:
                text = recognizer.recognize_sphinx(audio)
                return text.strip().lower() if text else None
            except (sr.UnknownValueError, sr.RequestError, AttributeError):
                return None
    except Exception as e:
        print(f"ERROR: {e}", file=sys.stderr)
        return None

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: stt_helper.py <audio.wav>", file=sys.stderr)
        sys.exit(1)
    
    wav_file = sys.argv[1]
    
    if not os.path.exists(wav_file):
        print(f"ERROR: File not found: {wav_file}", file=sys.stderr)
        sys.exit(1)
    
    transcript = transcribe_audio(wav_file)
    
    if transcript:
        # Output only the transcript (clean, no extra formatting)
        print(transcript.strip())
    else:
        # Empty output means no speech detected or API failed
        pass

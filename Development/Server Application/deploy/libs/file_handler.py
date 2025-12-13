# file_handler.py
import os
import sqlite3
import time

# Config
BASE_DIR = os.path.dirname(os.path.realpath(__file__))
LOGS_DIR = os.path.join(BASE_DIR, "../logs/logs")
DB_FILE = os.path.join(BASE_DIR, "../logs/can_logs.db")

def ensure_folder(path):
    if not os.path.exists(path):
        os.makedirs(path)

def save_file(client_name, filename, file_data):
    client_folder = os.path.join(LOGS_DIR, client_name)
    ensure_folder(client_folder)

    filepath = os.path.join(client_folder, filename)
    with open(filepath, 'wb') as f:
        f.write(file_data)
    return filepath

def insert_file_record(filename, filepath, client_name, event_name):
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()
    timestamp = int(time.time())

    cursor.execute("""
        INSERT INTO log_files (filename, filepath, client_name, event_name, timestamp)
        VALUES (?, ?, ?, ?, ?)
    """, (filename, filepath, client_name, event_name, timestamp))

    conn.commit()
    conn.close()

def process_received_file(client_name, event_name, filename, file_data):
    filepath = save_file(client_name, filename, file_data)
    insert_file_record(filename, filepath, client_name, event_name)
    print(f"[FileHandler] Saved and logged: {filename} from {client_name} ({event_name})")

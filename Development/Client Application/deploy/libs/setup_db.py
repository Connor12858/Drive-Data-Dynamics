import sqlite3
import os
import time

# Database file path
DB_FILE = os.path.join(os.path.dirname(os.path.realpath(__file__)), "../libs/can_logs.db")

def create_database():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    cursor.execute("""
    CREATE TABLE IF NOT EXISTS log_files (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        filename TEXT NOT NULL,
        client_name TEXT NOT NULL,
        event_name TEXT NOT NULL,
        timestamp INTEGER NOT NULL,
        filepath TEXT NOT NULL
    );
    """)

    conn.commit()
    conn.close()
    print(f"Database created at {DB_FILE}")

if __name__ == "__main__":
    create_database()

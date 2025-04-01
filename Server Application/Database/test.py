import sqlite3
import os
import time

# Database file path
DB_FILE = os.path.join(os.path.dirname(os.path.realpath(__file__)), "can_logs.db")

def insert_test_data():
    conn = sqlite3.connect(DB_FILE)
    cursor = conn.cursor()

    # Insert test data
    cursor.execute("""
    INSERT INTO log_files (filename, filepath, client_name, event_name, timestamp) 
    VALUES (?, ?, ?, ?, ?)
    """, ("test_log_1.log", "/logs/client_1/test_log_1.log", "Client_1", "TestEvent1", int(time.time())))

    cursor.execute("""
    INSERT INTO log_files (filename, filepath, client_name, event_name, timestamp) 
    VALUES (?, ?, ?, ?, ?)
    """, ("test_log_2.log", "/logs/client_2/test_log_2.log", "Client_2", "TestEvent2", int(time.time())))

    conn.commit()
    conn.close()
    print("Test data inserted into database.")

if __name__ == "__main__":
    insert_test_data()

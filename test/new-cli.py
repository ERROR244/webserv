import requests
import threading
import random
import time

# URL to target
URL = "http://localhost:7070"

# Number of threads (simulated clients)
CLIENT_COUNT = 50

# Duration to run in seconds
DURATION = 30

# Sample User-Agent strings
USER_AGENTS = [
    "Mozilla/5.0 (Windows NT 10.0; Win64; x64)",
    "Mozilla/5.0 (X11; Linux x86_64)",
    "Mozilla/5.0 (Macintosh; Intel Mac OS X 10_15_7)",
    "curl/7.79.1",
    "Python/3.11 Requests",
    "PostmanRuntime/7.32.2"
]

def send_requests():
    end_time = time.time() + DURATION
    while time.time() < end_time:
        headers = {
            "User-Agent": random.choice(USER_AGENTS)
        }
        try:
            response = requests.get(URL, headers=headers)
            print(f"[{threading.current_thread().name}] {response.status_code} - {response.elapsed.total_seconds():.3f}s")
        except Exception as e:
            print(f"[{threading.current_thread().name}] ERROR: {e}")
        time.sleep(random.uniform(0.1, 0.5))  # random delay to mimic real clients

# Start threads
threads = []
for i in range(CLIENT_COUNT):
    t = threading.Thread(target=send_requests, name=f"client-{i+1}")
    t.start()
    threads.append(t)

# Wait for all to finish
for t in threads:
    t.join()

print("🔁 Test finished.")

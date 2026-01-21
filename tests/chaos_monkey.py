# tests/chaos_monkey.py
import os
import time
import subprocess
import sys

def create_zombie():
    print(f"[TEST] Creating a Zombie process...")
    pid = os.fork()
    if pid == 0:
        # CHILD: Die immediately
        sys.exit(0)
    else:
        # PARENT: Sleep without calling wait(). 
        # The child becomes a ZOMBIE until this parent wakes up or dies.
        print(f"[TEST] Zombie created! PID: {pid}. Check your logs now.")
        time.sleep(10)
        print(f"[TEST] Parent waking up. Zombie {pid} should disappear now.")

def create_memory_hog():
    print(f"[TEST] Creating a Memory Hog...")
    # Allocate 150MB of RAM
    try:
        data = bytearray(150 * 1024 * 1024)
        print(f"[TEST] 150MB allocated. PID: {os.getpid()}")
        # Touch the memory to ensure RSS (Resident Set Size) actually increases
        for i in range(0, len(data), 4096):
            data[i] = 1
        print("[TEST] Memory touched. Holding for 10 seconds...")
        time.sleep(10)
    except MemoryError:
        print("[TEST] Failed to allocate memory.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 chaos_monkey.py [zombie|mem]")
        sys.exit(1)
    
    mode = sys.argv[1]
    if mode == "zombie":
        create_zombie()
    elif mode == "mem":
        create_memory_hog()
    else:
        print("Unknown mode")
# tests/chaos_monkey.py
import os
import time
import sys
import signal

def create_zombie():
    print(f"[TEST] Creating a Zombie process...")
    pid = os.fork()
    if pid == 0:
        # CHILD: Die immediately
        sys.exit(0)
    else:
        # PARENT: Sleep without calling wait(). 
        print(f"[TEST] Zombie created! PID: {pid}. Check your logs now.")
        time.sleep(10)
        print(f"[TEST] Parent waking up. Zombie {pid} should disappear now.")

def create_memory_hog():
    print(f"[TEST] Creating a Memory Hog...")
    # Allocate 600MB of RAM to exceed the 500MB dashboard threshold
    target_mb = 600
    try:
        data = bytearray(target_mb * 1024 * 1024)
        print(f"[TEST] {target_mb}MB allocated. PID: {os.getpid()}")
        
        # Touch the memory to ensure RSS (Resident Set Size) actually increases
        # Linux uses "lazy allocation", so we must write to the pages.
        for i in range(0, len(data), 4096):
            data[i] = 1
            
        print("[TEST] Memory touched. Holding for 10 seconds...")
        time.sleep(10)
    except MemoryError:
        print("[TEST] Failed to allocate memory. Your system might be running low.")

def create_fork_bomb():
    print("[TEST] Simulating Fork Bomb (Safe Mode)...")
    print("[TEST] Spawning 60 processes rapidly to trigger spike detection...")
    
    children = []
    
    for i in range(60):
        try:
            pid = os.fork()
            if pid == 0:
                # CHILD: Sleep for 5 seconds to keep process count high, then exit
                time.sleep(5)
                sys.exit(0)
            else:
                # PARENT: Keep track of children
                children.append(pid)
        except OSError as e:
            print(f"Fork failed: {e}")
            break

    print(f"[TEST] Spawned {len(children)} processes. Waiting for detector...")
    time.sleep(6) # Wait for children to finish sleeping
    
    # Cleanup (reaping children to prevent actual zombies)
    for child in children:
        try:
            os.waitpid(child, 0)
        except ChildProcessError:
            pass
    print("[TEST] Simulation complete. All children reaped.")

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python3 chaos_monkey.py [zombie|mem|fork]")
        sys.exit(1)
    
    mode = sys.argv[1]
    
    if mode == "zombie":
        create_zombie()
    elif mode == "mem":
        create_memory_hog()
    elif mode == "fork":
        create_fork_bomb()
    else:
        print("Unknown mode. Use: zombie, mem, or fork")
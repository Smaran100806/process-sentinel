# Process Sentinel

A Linux system daemon written in C that monitors processes in real-time by parsing the `/proc` filesystem directly.

## Features
- **Zombie Detection**: Identifies processes in 'Z' state.
- **Fork Bomb Detection**: Alerts on rapid process count spikes.
- **Memory Hog Monitoring**: Flags processes using excessive RSS memory.

## Build & Run
```bash
make
./sentinel

python3 tests/chaos_monkey.py zombie
python3 tests/chaos_monkey.py mem
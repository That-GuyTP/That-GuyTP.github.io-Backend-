# Coin Row Robot Visualizer

This desktop C++ app visualizes the robot coin-pickup dynamic-programming process.

## What it does

- Opens a standalone window (not connected to the webpage viewer).
- Provides a `Play` button in the window.
- Shows a populated coin maze.
- Animates DP "learning" cell-by-cell.
- Animates the robot traversal over the optimal path.
- Highlights the final path with total steps and coins collected.

## Build (Windows / MinGW g++)

```powershell
cd "C:\Users\hdriv\Documents\Coding\GitHub\That-GuyTP.github.io-Backend-\C++Applications\CoinRowRobot"
g++ -std=c++17 -Wall -Wextra -O2 .\coin_row_robot_gui.cpp -o .\coin_row_robot_gui.exe -lgdi32 -mwindows
```

## Build (CMake)

```powershell
cd "C:\Users\hdriv\Documents\Coding\GitHub\That-GuyTP.github.io-Backend-\C++Applications\CoinRowRobot"
cmake -S . -B build
cmake --build build --config Release
```

## Run

```powershell
.\coin_row_robot_gui.exe
```

If you used CMake, run `.\build\Release\coin_row_robot_gui.exe` (or `.\build\coin_row_robot_gui.exe` with single-config generators).

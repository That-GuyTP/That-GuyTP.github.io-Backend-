# C++ Applications Build/Test

Run all C++ app builds (and available smoke checks) with one command:

```powershell
cd "C:\Users\hdriv\Documents\Coding\GitHub\That-GuyTP.github.io-Backend-\C++Applications"
.\build_and_test_cpp.ps1 -Clean
```

Notes:

- The script refreshes PATH for the current shell so `g++` can be resolved.
- Current coverage:
  - `CoinRowRobot`: builds `coin_row_robot_gui.exe`
  - `EquationChecker`: builds `program1.exe` and runs a smoke test
  - `PrimeNumbers`: currently reported as skipped if no source files exist

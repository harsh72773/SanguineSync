# Sanguine Sync (C++)

C++ port of the Sanguine Sync Java application.

## Build

### Option 1: CMake (if installed)
```bash
mkdir build
cd build
cmake ..
cmake --build .
```
Run: `./sanguine_sync` (or `sanguine_sync.exe` on Windows).

### Option 2: g++ (MinGW / GCC)
From the project folder:
```bash
g++ -std=c++11 -o sanguine_sync.exe Index.cpp LogInPage.cpp RegisterUser.cpp RegisterHospital.cpp UserLogIn.cpp UserMenu.cpp HospLogIn.cpp HospMenu.cpp
```
Run: `sanguine_sync.exe`

### Option 3: Visual Studio
Create a new Console Application project, add all `.cpp` files and `common.h`, then build.

## Data files
- `users.txt` – user accounts (username,aadhar,password)
- `hospitals.txt` – hospital accounts (hospitalId,hospitalName,password)

Place these in the same directory as the executable, or they will be created there when you register.

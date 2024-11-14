@ECHO OFF
g++ Program.cpp -o "./bin/Game" -std=c++17 -luser32 -lgdi32 -lopengl32 -lgdiplus -lShlwapi -ldwmapi -lstdc++fs -O3
bin\Game.exe

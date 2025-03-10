#!/usr/bin/bash

libs="-luser32 -lopengl32 -lgdi32"
warnings="-Wno-writable-strings -Wno-deprecated-declarations -Wno-format-security"
includes="-Ithird_party -Ithird_party/Include"

clang $includes -g src/main.cpp -oPlatformerGame.exe $libs $warnings
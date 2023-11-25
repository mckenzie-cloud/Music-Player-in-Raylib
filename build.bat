@echo off
gcc -Wall -O0 -std=c11 main.c -o run -I include/ -L lib/ -lraylib -lopengl32 -lgdi32 -lwinmm
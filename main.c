
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#include "include/raylib.h"

#define SCREEN_WIDTH 512
#define SCREEN_HEIGHT 512

typedef struct
{
    /* data */
    float music_time_length;
    struct tm current_time;
    char time_buffer[10];
}Timer;

// Timers
Timer timer;

void setTimer(int time_played)
{
    timer.current_time.tm_sec = floorf(time_played % 60);
    timer.current_time.tm_min = floorf((time_played / 60) % 60);
    timer.current_time.tm_hour = floorf(time_played / 3600);
}

int main(void)
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth  = 512;
    const int screenHeight = 512;

    InitWindow(screenWidth, screenHeight, "Mckenzie - MUSIC MP3 PLAYER");
    InitAudioDevice();              // Initialize audio device

    //--------------------------------------------------------------------------------------

    // Font size
    int fontsize = 12;

    // Set Font
    Font font = LoadFontEx("resources/Fonts/CONSOLA.ttf", fontsize, NULL, 0);

    // Load directory files
    const char *dir_path = "C:/Users/Anony/Music";                        // Set the directory where music files are stored.
    FilePathList dir_file_path;

    if (DirectoryExists(dir_path))
    {
        dir_file_path = LoadDirectoryFilesEx(dir_path, ".mp3", 0);       // Get the mp3 files.
    } 
    else {
        printf("%s PATH DOES NOT EXIT!\n", dir_path);
        return EXIT_FAILURE;
    }

    int N_music_files = (int) dir_file_path.count;
    int n_rows = 10, counter = 0, max = floorf((float) N_music_files / n_rows), min = 0;

    Music music_stream;

    // Color state of the music track.
    Color default_color = GRAY, hover = BLACK, *color;

    int current_track = -1;
    int previous_track = -1;

    // Initialize timer value to zero.
    timer.current_time.tm_sec = 0; timer.current_time.tm_min = 0, timer.current_time.tm_hour = 0;

    //----------------------------------------------------------------------------------
    SetTargetFPS(60);               // Set to render at 60 frames-per-second

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        if (current_track >= 0)                 // If there's Music stream loaded,
        {
            UpdateMusicStream(music_stream);   // Update music buffer with new stream data
        }
        
        //----------------------------------------------------------------------------------
        // Handle Mouse input.
        if (IsMouseButtonPressed(0))
        {
            int Mposy = (int) GetMousePosition().y;
            for (int i=0; i < n_rows; i++)
            {
                int e = (counter * n_rows) + i;

                if (e < N_music_files)
                {
                    int filePosY1 = i*fontsize, filePosY2 = (i+1)*fontsize;
                    if (Mposy >= filePosY1 && Mposy < filePosY2)
                    {
                        if ((previous_track != current_track) || 
                        ((current_track >= 0 && previous_track >=0) && (current_track == previous_track)))       // Unload previously loaded music stream.
                        {
                            UnloadMusicStream(music_stream);                           // Unload music stream
                            previous_track = current_track;
                        }
                        music_stream = LoadMusicStream(dir_file_path.paths[e]);        // Load music stream
                        music_stream.looping = false;                                  // Don't loop.
                        current_track = e;
                        if (IsMusicReady(music_stream))                                // Checks if a music stream is ready
                        {
                            PlayMusicStream(music_stream);
                            timer.music_time_length = GetMusicTimeLength(music_stream);
                        }
                    }
                }
            }
        }
        //----------------------------------------------------------------------------------
        // Handle Keyboard Input.
        if (IsKeyPressed(KEY_DOWN))
        {
            if(counter < max) { counter++; }
        }

        if (IsKeyPressed(KEY_UP))
        {
            if (counter > min) { counter--; }
        }
        
        if (IsKeyPressed(KEY_RIGHT))
        {
            if (IsMusicStreamPlaying(music_stream))
            {
                float music_time_played = GetMusicTimePlayed(music_stream);
                float position = music_time_played + 30.0;                       // Skip forward for 30 seconds.
                if (position > timer.music_time_length)
                {
                    StopMusicStream(music_stream);
                    setTimer((int) timer.music_time_length);
                    strftime(timer.time_buffer, sizeof(timer.time_buffer), "%H:%M:%S", &timer.current_time);
                }
                else 
                {
                    SeekMusicStream(music_stream, position);
                }
            }
        }

        if (IsKeyPressed(KEY_LEFT))
        {
            if (IsMusicStreamPlaying(music_stream))
            {
                float music_time_played = GetMusicTimePlayed(music_stream);
                float position = music_time_played - 10.0;                      // Skip backward for 10 seconds.
                if (position < 0)
                {
                    StopMusicStream(music_stream);
                    PlayMusicStream(music_stream);
                }
                else 
                {
                    SeekMusicStream(music_stream, position);
                }
            }
        }
        
        // Update timer
        if (IsMusicStreamPlaying(music_stream))
        {
            int time_played = (int) GetMusicTimePlayed(music_stream);
            setTimer(time_played);
            strftime(timer.time_buffer, sizeof(timer.time_buffer), "%H:%M:%S", &timer.current_time);
        }

        //----------------------------------------------------------------------------------
        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();
            ClearBackground(RAYWHITE);
            DrawText("Click 'LEFT' | 'RIGHT' to skip the music & 'UP' | 'DOWN' to scroll the files.", 20, 256, 10, RED);

            const char *filename;
            Vector2 textpos = {5.0, 0.0};
            for (int i=0; i<n_rows; i++)
            {
                int e = (counter * n_rows) + i;

                if (e < N_music_files)
                {
                    int Mposy = (int) GetMousePosition().y;
                    int filePosY1 = i*fontsize, filePosY2 = (i+1)*fontsize;

                    if (Mposy >= filePosY1 && Mposy < filePosY2)
                    {
                        color = &hover;
                    } else {
                        color = &default_color;
                    }

                    textpos.y = i*font.baseSize;
                    filename = GetFileName(dir_file_path.paths[e]);
                    DrawTextEx(font, filename, textpos, font.baseSize, 1.0, *color);
                }
            }

            if (current_track >= 0)
            {
                filename = GetFileName(dir_file_path.paths[current_track]);
                float current_track_text_posX = MeasureTextEx(font, filename, fontsize, 1.0).x;
                DrawTextEx(font, filename, (Vector2) {(SCREEN_WIDTH - current_track_text_posX) / 2, SCREEN_HEIGHT-100}, font.baseSize, 1.0, MAROON);
            }

            DrawText(timer.time_buffer, 256, SCREEN_HEIGHT-50, fontsize, GRAY);
        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadDirectoryFiles(dir_file_path);  // Unload filepaths from memory
    //--------------------------------------------------------------------------------------
    if (current_track >= 0)
    {
        UnloadMusicStream(music_stream);   // Unload music stream
    }
    //--------------------------------------------------------------------------------------
    UnloadFont(font);

    CloseAudioDevice();         // Close audio device (music streaming is automatically stopped)

    CloseWindow();              // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return EXIT_SUCCESS;
}

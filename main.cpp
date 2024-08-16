#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <ncurses.h>
#include <iostream>
#include <string>
#include <thread>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <ctime>

void drawEqualizer(int height, int width, int pausecount) {
    int bars = 10;  //number of equalizer bars
    int barWidth = width / bars;

    std::vector<int> barHeights(bars);

    for (int i = 0; i < bars; ++i) {
        barHeights[i] = rand() % (height / 4); //randomize the bar heights each time
    }

    int yOffset = height / 2 + 2; //position equalizer below the progress bar

    if (pausecount % 2 == 0) {  //only display bars if pausecount is even -- simple method to handle pause
        for (int i = 0; i < bars; ++i) {
            for (int j = 0; j < barHeights[i]; ++j) {
                attron(COLOR_PAIR(1));
                mvprintw(yOffset - j, i * barWidth + (barWidth / 2), "|");
            }
        }
    }

    refresh();
}

void updateDisplay(const std::string &trackName, const std::string &appname, Mix_Music *music, int &pausecount) {
    initscr();
    noecho();
    curs_set(0);
    start_color();
    init_pair(1, COLOR_BLACK, COLOR_GREEN);

    int height, width;
    getmaxyx(stdscr, height, width);

    srand(static_cast<unsigned int>(time(0)));  //seed for random equalizer

    int elapsed_time = 0;

    while (Mix_PlayingMusic()) {
        clear();

        mvprintw(height / 5 - 2, (width - appname.size()) / 2, "listening on %s", appname.c_str());
        mvprintw(height / 4 - 2, (width - trackName.size()) / 2, "Playing: %s", trackName.c_str());

        //display equalizer below the progress bar, with dynamic movement
        drawEqualizer(height, width, pausecount);

        mvprintw(height - 2, (width - 20) / 2, "Press 'p' to pause, Press 'q' to quit");

        refresh();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); 
        elapsed_time++;

    //this is important if window has resized while program runs
        timeout(1);
       int ch = getch();

         if (ch == 'q') {
            Mix_HaltMusic();
           break;
        } 

        if (ch == 'p') {
            pausecount++;
            if (Mix_PausedMusic()) {
                Mix_ResumeMusic();
            } else {
                Mix_PauseMusic();
            }
        }
    }

    endwin();
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <path/to/music/file.mp3>" << std::endl;
        return -1;
    }

    std::string trackName = argv[1];
    std::string appname = "simple terminal music player (stmp)";

    //initialize SDL
    if (SDL_Init(SDL_INIT_AUDIO) < 0) {
        std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return -1;
    }

    //initialize SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
        std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: " << Mix_GetError() << std::endl;
        return -1;
    }

    //here we load the music file
    Mix_Music *music = Mix_LoadMUS(trackName.c_str());
    if (music == nullptr) {
        std::cerr << "Failed to load music! SDL_mixer Error: " << Mix_GetError() << std::endl;
        SDL_Quit();
        return -1;
    }

    //play the music
    Mix_PlayMusic(music, 1);

    int pausecount = 0;

    std::thread displayThread(updateDisplay, trackName, appname, music, std::ref(pausecount));

    //control the playback
    char command;
    while (Mix_PlayingMusic()) {
        command = std::cin.get();

        if (command == 'p') {
            pausecount++;
            if (Mix_PausedMusic()) {
                Mix_ResumeMusic();
            } else {
                Mix_PauseMusic();
            }
        }  else if (command == 'q') {
            Mix_HaltMusic();
            break;
        }
    }

    displayThread.join();

    //clean up all
    Mix_FreeMusic(music);
    Mix_CloseAudio();
    SDL_Quit();

    return 0;
}

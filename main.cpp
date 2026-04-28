#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <ncurses.h>
#include <string>
#include <thread>
#include <vector>

std::atomic<int> pausecount(0);
std::atomic<int> volume(MIX_MAX_VOLUME);
std::atomic<bool> running(true);

void updateDisplay(const std::string &trackName, const std::string &appname) {
  initscr();
  noecho();
  curs_set(0);
  start_color();
  // green foreground, black background for the equalizer
  init_pair(1, COLOR_GREEN, COLOR_BLACK);

  int height, width;
  getmaxyx(stdscr, height, width);
  srand(static_cast<unsigned int>(time(0)));

  int elapsed_seconds = 0;
  int tick = 0;

  while (running) {
    clear();

    // header section
    mvprintw(height / 5 - 2, (width - appname.size()) / 2, "%s",
             appname.c_str());
    mvprintw(height / 4 - 2, (width - (trackName.size() + 9)) / 2,
             "Playing: %s", trackName.c_str());

    // volume and time info
    mvprintw(height / 4, (width - 30) / 2, "Volume: %d%% | Time: %ds",
             (volume * 100 / MIX_MAX_VOLUME), elapsed_seconds);

    // draw the enhanced equalizer
    // drawEqualizer(height, width, tick);

    // footer controls
    mvprintw(
        height - 2, (width - 60) / 2,
        "Controls: [p] Pause/Resume | [+] Vol Up | [-] Vol Down | [q] Quit");

    refresh();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    tick++;
    if (tick % 10 == 0)
      elapsed_seconds++;

    // non-blocking input handling
    timeout(0);
    int ch = getch();
    if (ch == 'q') {
      running = false;
    }
    if (ch == 'p') {
      pausecount++;
      if (Mix_PausedMusic()) {
        Mix_ResumeMusic();
      } else {
        Mix_PauseMusic();
      }
    }
    if (ch == '+') {
      if (volume < MIX_MAX_VOLUME) {
        volume += 500;
        Mix_VolumeMusic(volume);
      }
    }
    if (ch == '-') {
      if (volume > 0) {
        volume -= 500;
        Mix_VolumeMusic(volume);
      }
    }
  }
  endwin();
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cerr << "Usage: " << argv[0] << " <path/to/music/file.mp3>"
              << std::endl;
    return -1;
  }

  std::string trackName = argv[1];
  std::string appname =
      "--- listening on simple terminal music player (stmp) ---";

  // initialize SDL
  if (SDL_Init(SDL_INIT_AUDIO) < 0) {
    std::cerr << "SDL could not initialize! SDL_Error: " << SDL_GetError()
              << std::endl;
    return -1;
  }

  // initialize SDL_mixer
  if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
    std::cerr << "SDL_mixer could not initialize! SDL_mixer Error: "
              << Mix_GetError() << std::endl;
    return -1;
  }

  // here we load the music file
  Mix_Music *music = Mix_LoadMUS(trackName.c_str());
  if (music == nullptr) {
    std::cerr << "Failed to load music! SDL_mixer Error: " << Mix_GetError()
              << std::endl;
    SDL_Quit();
    return -1;
  }

  // play the music
  Mix_PlayMusic(music, 1);

  // start the display thread
  std::thread displayThread(updateDisplay, trackName, appname);

  // the main thread now simply waits for the display thread to finish
  // (triggered by q)
  //  or for the music to end.
  while (running) {
    if (!Mix_PlayingMusic() && !Mix_PausedMusic()) {
      running = false;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
  }

  // signal music to stop and wait for thread to join
  Mix_HaltMusic();
  if (displayThread.joinable()) {
    displayThread.join();
  }

  // clean up all
  Mix_FreeMusic(music);
  Mix_CloseAudio();
  SDL_Quit();

  return 0;
}

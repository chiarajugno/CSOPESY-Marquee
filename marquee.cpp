#include <iostream>
#include <conio.h>
#include <windows.h>
#include <thread>
#include <cstdlib>
#include <ctime>
#include <atomic>
#include <mutex>

using namespace std;

const int SCREEN_WIDTH = 80;
const int SCREEN_HEIGHT = 25;
const string MESSAGE = "This is a marquee!";

atomic<bool> running(true);
mutex consoleMutex;

const int REFRESH_RATE = 100;  
const int POLL_RATE = 10;        

void gotoxy(int x, int y) {
    COORD coord = { static_cast<SHORT>(x), static_cast<SHORT>(y) };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void marquee() {
    // Display message at random position
    srand(time(0));
    int x = rand() % (SCREEN_WIDTH - MESSAGE.length());
    int y = rand() % (SCREEN_HEIGHT - 3); 

    // Movement direction
    int dx = (rand() % 2 == 0) ? 1 : -1;
    int dy = (rand() % 2 == 0) ? 1 : -1;

    while (running) {
        // Display message
        {
            lock_guard<mutex> lock(consoleMutex);
            gotoxy(x, y);
            std::cout << MESSAGE;
        }

        // Delay
        this_thread::sleep_for(chrono::milliseconds(REFRESH_RATE));

        // Clear message
        {
            lock_guard<mutex> lock(consoleMutex);
            gotoxy(x, y);
            std::cout << string(MESSAGE.length(), ' ');
        }

        // Increment position
        x += dx;
        y += dy;

        // Bounce at edges
        if (x <= 0 || x >= SCREEN_WIDTH - MESSAGE.length()) dx *= -1;
        if (y <= 0 || y >= SCREEN_HEIGHT - 3) dy *= -1;
    }
}

int main() {
    system("cls");

    thread marqueeThread(marquee);
    string command;
    string command_buffer;

    // Initial prompt display
    {
        lock_guard<mutex> lock(consoleMutex);
        gotoxy(0, SCREEN_HEIGHT - 2);
        std::cout << "Enter command (type 'exit' to quit): ";
        std::cout.flush();
    }

    while (running) {
        if (_kbhit()) {
            int c = _getch();

            {
                lock_guard<mutex> lock(consoleMutex);

                if (c == '\r') { // Enter key
                    if (command_buffer == "exit") {
                        running = false;
                        break;
                    }

                    // Display entered command
                    gotoxy(0, SCREEN_HEIGHT - 1);
                    std::cout << "You entered: " << command_buffer
                        << string(SCREEN_WIDTH - 12 - command_buffer.length(), ' ');

                    command_buffer.clear();

                    // Clear and reprint prompt
                    gotoxy(0, SCREEN_HEIGHT - 2);
                    std::cout << "Enter command (type 'exit' to quit): ";
                    std::cout.flush();
                } else if (c == 8 || c == 127) { // Backspace
                    if (!command_buffer.empty()) {
                        command_buffer.pop_back();
                    }
                } else if (c >= 32 && c <= 126) { // Printable characters
                    command_buffer += (char)c;
                }

                // Update input display after each key press
                gotoxy(0, SCREEN_HEIGHT - 2);
                std::cout << "Enter command (type 'exit' to quit): " << command_buffer
                    << string(SCREEN_WIDTH - 38 - command_buffer.length(), ' ');
                std::cout.flush();
            }
        }

        this_thread::sleep_for(chrono::milliseconds(POLL_RATE));
    }

    marqueeThread.join();

    gotoxy(0, SCREEN_HEIGHT);
    std::cout << "\nExiting program. Bye!" << std::endl;
    return 0;
}

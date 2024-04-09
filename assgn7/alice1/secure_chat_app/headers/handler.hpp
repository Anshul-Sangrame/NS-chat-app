#pragma once
#include "connection.hpp"

#include <ncurses.h>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>

using namespace std;

class Display {
    private:
        std::mutex screenMutex;
        std::mutex bufferMutex;

        std::vector<message> display_buffer; // buffer of shown messages (sent+received)
        std::vector<string> sender_list;

        int y_display=1, x_display;
        int y_input=1, x_input=1;

        int H, h1, h2;
        int W, w;
        int H_input = 3;

        WINDOW *win1;
        WINDOW *win2;
        WINDOW *win1box;
        WINDOW *win2box;

        int display_index = -1;

        void open_screen();
        void display_messages();
        void display_input();

    public:
        string input = "";
        Display();
        ~Display();

        void draw_screen();
        void addMessage(message msg, string sender);
        void displayInputChar(int c);
        void scrollMessages(int delta);
        int getInputChar();

};


class Handler {
    Display ui;
    std::thread inputThread; 
    bool terminated = false;
    connection* con;

    void inputHandler();
    void receiver();
    
    public:
        message to_message(string msg);
        Handler(connection* c);
};


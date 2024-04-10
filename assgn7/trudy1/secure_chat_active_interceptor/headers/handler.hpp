#pragma once
#include "connection.hpp"

#include <ncurses.h>
#include <thread>
#include <chrono>
#include <vector>
#include <mutex>

using namespace std;

enum Mode {
    HOST,   
    ACTIVE, 
    PASSIVE 
};

enum ChatColor {
    CHAT_WHITE = 1,
    CHAT_RED = 2,
    CHAT_GREEN = 3
};

class Display {
    private:
        std::mutex screenMutex;
        std::mutex bufferMutex;

        std::vector<message> display_buffer; // buffer of shown messages (sent+received)
        std::vector<string> sender_list;
        std::vector<ChatColor> color_list;

        int y_display=1, x_display;
        int y_input=1, x_input=1;

        int H, h1, h2;
        int W, w;
        int H_input = 3;

        WINDOW *win1;
        WINDOW *win2;
        WINDOW *win1box;
        WINDOW *win2box;

        Mode mode; // host, passive, active
        int display_index = -1;
        string input_prefix = "";

        void open_screen();
        void display_messages();
        void display_input();

    public:
        string input = "";
        Display();
        ~Display();

        void setMode(Mode m);
        void draw_screen();
        void addMessage(message msg, string sender, ChatColor col);
        void displayInputChar(int c);
        void scrollMessages(int delta);
        int getInputChar();
        int copyMessage(message &msg, string &sender);
        void setInputPrefix(string prefix);

};


class Handler {
    Display ui;
    std::thread inputThread; 
    bool terminated = false;
    connection* con;

    void inputHandler();
    void receiver();
    
    public:
        Handler(connection* c);
};

class PassiveHandler {
    Display ui;
    std::thread t1, t2; 
    bool terminated = false;

    void inputHandler();
    void receiver(connection* c1, connection* c2);
    
    public:
        PassiveHandler(connection* c1, connection* c2);
};


class ActiveHandler {
    Display ui;
    std::thread t1, t2; 
    bool terminated = false;

    void inputHandler(connection* c1, connection* c2);
    void receiver(connection* c1, connection* c2);
    bool drop_packets = false;
    
    public:
        ActiveHandler(connection* c1, connection* c2);
};
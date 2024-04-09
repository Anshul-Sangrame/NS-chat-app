// g++ gui.cpp -lpthread -lncurses

#include <ncurses.h>
#include <thread>
#include <chrono>
#include <vector>
#include <queue>
#include <mutex>

using namespace std;

class Message {
    public:
        string message;
        string sender;
};

class UI {
    private:
        std::thread inputThread; 
        bool terminated = false;

        std::mutex screenMutex;
        std::mutex bufferMutex;

        std::queue<std::string> input_buffer; // queue of messages to send
        std::vector<Message> display_buffer; // buffer of shown messages (sent+received)

        int display_y=1, display_x=1;
        int input_y=1, input_x=1;

        int H, W;
        WINDOW *win1;
        WINDOW *win2;

        void inputHandler() {
            string inp;
            wmove(win2, 1, 1);
            while (!terminated) {
                char c = getch();
                waddch(win2, c);
                if(c=='\n'){
                    bufferMutex.lock();
                    input_buffer.push(inp);
                    bufferMutex.unlock();
                    displayMessage(Message{inp, "user1"});
                    inp = "";

                    wclear(win2);
                    wmove(win2, 1, 1);
                    box(win2,0,0);
    
                }
                else {
                    inp += c;
                }
                wrefresh(win2);
                
            }
        }

    public:
        UI(){
            initscr();
            cbreak();
            noecho();
            keypad(stdscr, TRUE);

            getmaxyx(stdscr, H, W);
            win1 = newwin(H-3, W, 0, 0);
            win2 = newwin(3, W, H-3, 0);

            refresh();

            // scrollok(win1,TRUE);
            // scrollok(win2,TRUE);

            box(win1, 0, 0);
            box(win2, 0, 0);

            // DUMMY values
            for(int i=0; i<3; i++)
            display_buffer.push_back(Message{"Hello"+to_string(i), "user"});


            wmove(win1, display_y, display_x);
            for(int i=0; i <display_buffer.size() ; i++){
                string displayString = display_buffer[i].sender + ": " + display_buffer[i].message + "\n";
                wprintw(win1, displayString.c_str());
                getyx(win1, display_y, display_x);
                wmove(win1, display_y, ++display_x);
            }
            // wprintw(win2, "Window 2");

            // Refresh windows to display changes
            wrefresh(win1);
            wrefresh(win2);


            inputThread = std::thread(&UI::inputHandler, this);
        }


        void close(){
            terminated = true;
            inputThread.join();
            delwin(win1);
            delwin(win2);
            endwin();
        }


        void displayMessage(Message msg){
            bufferMutex.lock();
            display_buffer.push_back(msg);
            bufferMutex.unlock();
        
            screenMutex.lock();
            string displayString = msg.sender + ": " + msg.message + "\n";   
            wmove(win1, display_y, display_x);
            wprintw(win1, displayString.c_str());
            getyx(win1, display_y, display_x);
            wmove(win1, display_y, ++display_x);
            box(win1, 0, 0);

            refresh();
            wrefresh(win1);
            screenMutex.unlock();
        }

        std::string getMessage(){
            bufferMutex.lock();
            if(input_buffer.size() > 0){
                string message = input_buffer.front();
                input_buffer.pop();
                bufferMutex.unlock();
                return message;
            }
            bufferMutex.unlock();
            return "";
        }

};

int main() {
    UI ui;
    // ui.displayMessage("Hello");
    // getch();

    for(int i=4; i<8; i++) {
        ui.displayMessage(Message{"Hello"+to_string(i), "user"});
    std::this_thread::sleep_for(std::chrono::seconds(2));
    }

    ui.close();
    return 0;
}

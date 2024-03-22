// g++ handler.cpp -lpthread -lncurses

#include "handler.hpp"

using namespace std;

void Display::open_screen(){
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
}

void Display::display_messages(){
    y_display = h1;
    x_display = 0;
    char time_str[15];

    wmove(win1, y_display, x_display);
    for(int i=display_index; i >= 0 ; i--){
        string displayString = sender_list[i] + ": " + display_buffer[i].body + "\n";
        int msg_height = (displayString.size()-1) / w;
        if(y_display-msg_height-3<0) {return;}

        y_display -= 3;
        strftime(time_str, sizeof(time_str), "[%H:%M %b %d]", localtime(&display_buffer[i].hdr.time) );
        wmove(win1, y_display, w-sizeof(time_str));
        wprintw(win1, time_str);

        y_display -= 1+ msg_height;
        wmove(win1, y_display, x_display);
        wprintw(win1, displayString.c_str());
    }
    // getyx(win1, y_display, x_display);
}

void Display::display_input(){
    y_input = 0;
    x_input = 0; 
    wmove(win2, y_input, x_input);
    if(input==""){
        wprintw(win2, "Type here:");   
    }
    else {
        wprintw(win2, input.c_str());   
        getyx(win2, y_input, x_input);
        x_input--;  
    }
}


Display::Display(){
    open_screen();
    draw_screen();
    // DUMMY values
    // for(int i=0; i<3; i++)
    // display_buffer.push_back(Message{"Hello"+to_string(i), "user"});

}

Display::~Display(){
    delwin(win1);
    delwin(win2);
    endwin();
}

void Display::draw_screen(){
    // screenMutex.lock();
    getmaxyx(stdscr, H, W);
    H_input = input.size()/(W-2)+3;
    win1box = newwin(H-H_input, W, 0, 0);
    win2box = newwin(H_input, W, H-H_input, 0);
    h1 = H-H_input-2;
    h2 = H_input-2;
    w = W-4;
    win1 = newwin(h1, w, 1, 2);
    win2 = newwin(h2, w, H-H_input+1, 2);


    refresh();
    box(win2box, 0, 0);  
    box(win1box, 0, 0);
    display_messages();
    display_input();

    wrefresh(win1box);
    wrefresh(win2box);
    wrefresh(win1);
    wrefresh(win2);
    // screenMutex.unlock();
}

void Display::addMessage(message msg, string sender){
    // bufferMutex.lock();
    display_buffer.push_back(msg);
    sender_list.push_back(sender);
    // bufferMutex.unlock();
    scrollMessages(1);
    draw_screen();
}

void Display::displayInputChar(int c){
    // screenMutex.lock();

    if(c==KEY_BACKSPACE){
        if(input.size() > 0) {
            wmove(win2, y_input, x_input); 
            waddch(win2, ' ');
            wmove(win2, y_input, x_input--); 
            input.pop_back();

            if(input==""){
                wmove(win2, y_input, 0); 
                wprintw(win2, "Type here:"); 
                x_input--;
            }

        }
        wrefresh(win2);
        return;
    }

    if(input==""){
        wmove(win2, y_input, 0); 
        wprintw(win2, "            ");   
        wmove(win2, y_input, 0); 
        wrefresh(win2);
        x_input = -1; 
    }
    input += (char) c;
    if(input.size()>w){
        draw_screen();
        return;
    }

    wmove(win2, y_input, ++x_input); 
    waddch(win2, c);
    wrefresh(win2);  
    // screenMutex.unlock();
}

void Display::scrollMessages(int delta){
    // bufferMutex.lock();
    // display_index = display_buffer.size()-1;return;
    if(display_index+delta >= 0 && display_index+delta < display_buffer.size()){
        display_index += delta;
    }
    // bufferMutex.unlock();
}



void Handler::inputHandler() {
    while (!terminated) {
        int c = wgetch(stdscr);       

        if(c=='\n'){
            message msg = to_message(ui.input);
            ui.input = "";
            ui.addMessage(msg, string("you"));
            con->send_msg(msg);
        }   
        else if (c==KEY_END){
            terminated = true;
        }      
        else if (c==KEY_DOWN){
            ui.scrollMessages(1);
            ui.draw_screen(); 
        } 
        else if (c==KEY_UP){
            ui.scrollMessages(-1);
            ui.draw_screen(); 
        }             
        else if (c==KEY_RESIZE){
            ui.draw_screen();
        }
        else {
            ui.displayInputChar(c);
        }
    }
}

void Handler::receiver(){
    int i = 0;
    while(!terminated){
        message msg = con->read_msg();
        // message msg = to_message("test"+to_string(i++));
        // sleep(4);
        ui.addMessage(msg, string("them"));
    }
}

message Handler::to_message(string msg)
{
    message res;

    res.hdr.type = DATA;
    res.hdr.time = time(NULL);
    res.body = msg;

    return res;
}

Handler::Handler(connection* c){ 
    con = c;
    inputThread = std::thread(&Handler::inputHandler, this);
    receiver();
    inputThread.join();
}



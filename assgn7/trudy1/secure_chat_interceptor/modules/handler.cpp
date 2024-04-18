// g++ handler.cpp -lpthread -lncurses

#include "handler.hpp"

using namespace std;

message to_message(string msg)
{
    message res;

    res.hdr.type = DATA;
    res.hdr.time = time(NULL);
    res.body = msg;

    return res;
}

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
        wattron(win1, COLOR_PAIR(color_list[i]));
        string displayString = sender_list[i] + ": " + display_buffer[i].body + "\n";
        int msg_height = (displayString.size()-1) / w;
        if(y_display-msg_height-3<0) {break;}

        y_display -= 3;
        strftime(time_str, sizeof(time_str), "[%H:%M %b %d]", localtime(&display_buffer[i].hdr.time) );
        wmove(win1, y_display, w-sizeof(time_str));
        wprintw(win1, time_str);

        y_display -= 1+ msg_height;
        wmove(win1, y_display, x_display);
        wprintw(win1, displayString.c_str());
        wattroff(win1, COLOR_PAIR(color_list[i]));
    }
    wmove(win1, 0, 0);
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
        wprintw(win2, (input_prefix+input).c_str());   
        getyx(win2, y_input, x_input);
        x_input--;  
    }
}


Display::Display(){
    ;
}

Display::~Display(){
    delwin(win1);
    delwin(win2);
    endwin();
}

void Display::draw_screen(){
    screenMutex.lock();
    getmaxyx(stdscr, H, W);
    H_input = input.size()/(W-2)+3;
    win1box = newwin(H-H_input, W, 0, 0);
    win2box = newwin(H_input, W, H-H_input, 0);
    h1 = H-H_input-2;
    h2 = H_input-2;
    w = W-4;
    win1 = newwin(h1, w, 1, 2);
    win2 = newwin(h2, w, H-H_input+1, 2);

    start_color(); // check for color
    init_pair(1, COLOR_WHITE, COLOR_BLACK);
    init_pair(2, COLOR_RED, COLOR_BLACK);
    init_pair(3, COLOR_GREEN, COLOR_BLACK);

    refresh();
    box(win1box, 0, 0);                 // message box
    display_messages();
    wrefresh(win1box);
    wrefresh(win1);

    if(mode!=PASSIVE){                // input box 
        box(win2box, 0, 0);  
        display_input();
        wrefresh(win2box);
        wrefresh(win2);
    }
    screenMutex.unlock();
}

void Display::addMessage(message msg, string sender, ChatColor col){
    bufferMutex.lock();
    display_buffer.push_back(msg);
    sender_list.push_back(sender);
    color_list.push_back(col);
    bufferMutex.unlock();
    scrollMessages(1);
    draw_screen();
}

int Display::copyMessage(message &msg, string &sender){
    if(display_index < 0) return 1;
    msg = display_buffer[display_index];
    sender = sender_list[display_index];
    return 0;
}

void Display::displayInputChar(int c){
    screenMutex.lock();

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
        screenMutex.unlock();
        return;
    }

    if(input==""){
        wmove(win2, y_input, 0); 
        wprintw(win2, "            ");   
        wmove(win2, y_input, 0); 
        if(mode==ACTIVE)
            wprintw(win2, input_prefix.c_str());   
        wrefresh(win2);
        x_input = input_prefix.size() -1; 
    }
    input += (char) c;
    if(input.size()>w){
        draw_screen();
        screenMutex.unlock();
        return;
    }

    wmove(win2, y_input, ++x_input); 
    waddch(win2, c);
    wrefresh(win2);  
    screenMutex.unlock();
}

void Display::scrollMessages(int delta){
    // bufferMutex.lock();
    // display_index = display_buffer.size()-1;return;
    if(display_index+delta >= 0 && display_index+delta < display_buffer.size()){
        display_index += delta;
    }
    // bufferMutex.unlock();
}

void Display::setMode(Mode m){
    mode = m;
    open_screen();
    draw_screen();
    // DUMMY values
    // for(int i=0; i<3; i++)
    // display_buffer.push_back(Message{"Hello"+to_string(i), "user"});

}

void Display::setInputPrefix(string prefix){
    bufferMutex.lock();
    input_prefix = prefix;
    bufferMutex.unlock();
}







/* HOST HANDLER */

// void Handler::inputHandler() {
//     while (!terminated) {
//         int c = wgetch(stdscr);       

//         switch (c) {
//             case '\n': {
//                 message msg = to_message(ui.input);
//                 ui.input = "";
//                 ui.addMessage(msg, string("you"), CHAT_GREEN);
//                 con->send_msg(msg);
//                 break;
//             }
//             case KEY_END: {
//                 terminated = true;
//                 break;
//             }
//             case KEY_DOWN: {
//                 ui.scrollMessages(1);
//                 ui.draw_screen(); 
//                 break;
//             }
//             case KEY_UP: {
//                 ui.scrollMessages(-1);
//                 ui.draw_screen(); 
//                 break;
//             }
//             case KEY_RESIZE: {
//                 ui.draw_screen();
//                 break;
//             }
//             default: {
//                 ui.displayInputChar(c);
//                 break;
//             }
//         }

//     }
// }

// void Handler::receiver(){
//     int i = 0;
//     while(!terminated){
//         message msg = con->read_msg();
//         // message msg = to_message("test"+to_string(i++));
//         sleep(4);
//         ui.addMessage(msg, con->to_addr, CHAT_WHITE);     // EDIT
//     }
// }

// Handler::Handler(connection* c){ 
//     ui.setMode(HOST);
//     con = c;
//     inputThread = std::thread(&Handler::receiver, this);
//     inputHandler();
//     inputThread.join();
// }




/* PASSIVE HANDLER */

// void PassiveHandler::receiver(connection* c1, connection* c2){
//     int i = 0;
//     string sender_name = c1->to_name;  // "them"
//     while(!terminated){
//         // message msg = to_message("test"+to_string(i++));
//         // sleep(4);
//         message msg = c1->read_msg();
//         c2->send_msg(msg);
//         ui.addMessage(msg, string(sender_name), CHAT_WHITE);
//     }
// }

void PassiveHandler::receiver(connection* c1, connection* c2){
    int i = 0;
    string sender_name = c1->to_name;  // "them"
    while(!terminated){
        if (c1->poll_msg())
        {
            message msg = c1->read_msg();
            c2->send_msg(msg);
            if (msg.hdr.type == CONTROL &&  && msg.body == "CHAT_CLOSE_ACK")
            {
                terminated = true;
                break;
            }
            else if (msg.hdr.type != CONTROL) {
                ui.addMessage(msg, string(sender_name), CHAT_WHITE);
            }
        }
    }
}


void PassiveHandler::inputHandler() {
    while (!terminated) {
        int c = wgetch(stdscr);       

        switch (c) {
            // case KEY_END: {
            //     terminated = true;
            //     break;
            // }
            case KEY_DOWN: {
                ui.scrollMessages(1);
                ui.draw_screen(); 
                break;
            }
            case KEY_UP: {
                ui.scrollMessages(-1);
                ui.draw_screen(); 
                break;
            }
            case KEY_RESIZE: {
                ui.draw_screen();
                break;
            }
        }

    }
}

PassiveHandler::PassiveHandler(connection* c1, connection* c2){
    ui.setMode(PASSIVE);
    t1 = std::thread(&PassiveHandler::receiver, this, c1, c2);
    t2 = std::thread(&PassiveHandler::receiver, this, c2, c1);
    inputHandler();
    t1.join();
    t2.join();
}



/* ACTIVE HANDLER */


void ActiveHandler::receiver(connection* c1, connection* c2){
    int i = 0;
    string from_name = c1->to_name;
    while(!terminated){
        if (c1->poll_msg())
        {
            message msg = c1->read_msg();
            
            if (msg.hdr.type == CONTROL && msg.body == "CHAT_CLOSE_ACK") {
                c2->send_msg(msg);
                terminated = true;
                break;
            }

            else if (msg.hdr.type == CONTROL) {
                c2->send_msg(msg);
            }

            else if(drop_packets){
                ui.addMessage(msg, from_name, CHAT_RED);
            }
            else {
                c2->send_msg(msg);
                ui.addMessage(msg, from_name, CHAT_WHITE);
            }
        }
    }
}


void ActiveHandler::inputHandler(connection* c1, connection* c2) {
    message temp_msg;
    string fake_name = c1->to_name;   
    // string fake_name = c1 ==nullptr? "alice": "bob";   // TESTING init name
    ui.setInputPrefix("As "+fake_name+": ");

    while (!terminated) {
      int c = wgetch(stdscr);       
      switch (c) {
        case ERR :  {
            break;
        }
        case '\n': {
            message msg = to_message(ui.input);
            ui.input = "";
            ui.addMessage(msg, fake_name, CHAT_GREEN);  // fake messages
            // logic to decide which connection to use (not equals fake_name)
            if(c1->to_name == fake_name)
                c2->send_msg(msg);
            else if(c2->to_name == fake_name)
                c1->send_msg(msg);
            break;
        }
        // case KEY_END: {
        //     terminated = true;
        //     break;
        // }
        case KEY_DOWN: {
            ui.scrollMessages(1);
            ui.draw_screen(); 
            break;
        }
        case KEY_UP: {
            ui.scrollMessages(-1);
            ui.draw_screen(); 
            break;
        }
        case KEY_RESIZE: {
            ui.draw_screen();
            break;
        }
        case KEY_RIGHT: {   // copy bottom message to input box
            if(ui.copyMessage(temp_msg, fake_name) == 0){
                ui.input = temp_msg.body;
                ui.setInputPrefix("As "+fake_name+": ");
                ui.draw_screen();
            }
            break;
        }
        case KEY_LEFT: {     // toggle sender 
            fake_name = (fake_name == c1->to_name) ? c2->to_name : c1->to_name ;
            ui.setInputPrefix("As "+fake_name+": ");
            ui.draw_screen();
            break;
        }
        case KEY_NPAGE: {    // pg up = packets dropped
            drop_packets = true;
            break;
        }
        case KEY_PPAGE: {   // pg down = packets repeated
            drop_packets = false; 
            break;
        }
        default: {
            ui.displayInputChar(c);
            break;
        }
      }

    }
}

ActiveHandler::ActiveHandler(connection* c1, connection* c2){
    ui.setMode(ACTIVE);
    t1 = std::thread(&ActiveHandler::receiver, this, c1, c2);
    t2 = std::thread(&ActiveHandler::receiver, this, c2, c1);
    inputHandler(c1, c2);
    t1.join();
    t2.join();
}

#include <iostream>
// #include<unistd.h>
#include "chat_app.hpp"
using namespace std;

message to_message(string msg)
{
    message res;

    res.hdr.type = DATA;
    res.hdr.time = time(NULL);
    res.body = msg;

    return res;
}

void displayMessage(message msg)
{
    cout << "TYPE: " << msg.hdr.type << "\n";
    cout << "TIME: " << asctime(localtime(&(msg.hdr.time)));
    cout << "BODY:\n"
         << msg.body << "\n";
}

int main(int argc, char *argv[])
{
    cout << "main.cpp\n";

    connection *con = NULL;
    connection *con2 = NULL;
    message m;
    try
    {
        con = new server_connection(8080);
        con2 = new client_connection(argv[2],8080);
        cout << con->to_name << ": \n";
        m = con->read_msg();
        displayMessage(m);
        con2->send_msg(m);

        cout << con2->to_name << ": \n";
        m = con2->read_msg();
        displayMessage(m);
        con->send_msg(m);

        cout << con->to_name << ": \n";
        m = con->read_msg();
        displayMessage(m);
        con2->send_msg(m);
        
        cout << con2->to_name << ": \n";
        m = con2->read_msg();
        displayMessage(m);
        con->send_msg(m);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    delete con;
    delete con2;

    return 0;
}
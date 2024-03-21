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

    try
    {
        for (int i = 1; i < argc; i++)
        {
            if (argv[i] == string("-s"))
            {
                con = new server_connection(8080);
                con->startSSL();

                displayMessage(con->read_msg());
                con->send_msg(to_message("hello how are you"));
                displayMessage(con->read_msg());
                con->send_msg(to_message("I am good"));
                displayMessage(con->read_msg());
                con->send_msg(to_message("Yeah, finally safe"));
                break;
            }
            if (argv[i] == string("-c"))
            {
                con = new client_connection("hostname",8080);
                con->startSSL();
                
                con->send_msg(to_message("hello"));
                displayMessage(con->read_msg());
                con->send_msg(to_message("I am fine, how about you?"));
                displayMessage(con->read_msg());
                con->send_msg(to_message("This is encrypted"));
                displayMessage(con->read_msg());
                break;
            }
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    delete con;

    return 0;
}
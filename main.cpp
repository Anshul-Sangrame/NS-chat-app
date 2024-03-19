#include <iostream>
// #include<unistd.h>
#include <connection.hpp>
using namespace std;

message to_message(string msg)
{
    message res;

    res.hdr.type = DATA;
    res.hdr.size = msg.size();
    res.hdr.time = time(NULL);
    res.body = msg;

    return res;
}

void displayMessage(message msg)
{
    cout << "TYPE: " << msg.hdr.type << "\n";
    cout << "TIME: " << asctime(localtime(&(msg.hdr.time))) << "\n";
    cout << "SIZE: " << msg.hdr.size << "\n";
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
                message msg = con->read();
                displayMessage(msg);
                con->send(to_message("CHAT_OK_REPLY"));
                con->startSSL();
                break;
            }
            if (argv[i] == string("-c"))
            {
                con = new client_connection(8080);
                con->send(to_message("CHAT_HELLO"));
                message msg = con->read();
                displayMessage(msg);
                // sleep(5);
                con->startSSL();
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
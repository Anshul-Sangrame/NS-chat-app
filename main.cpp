#include <iostream>
#include <connection.hpp>
using namespace std;

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
                cout << "TYPE: " << msg.hdr.type << "\n";
                cout << "TIME: " << asctime(localtime(&(msg.hdr.time)));
                cout << "SIZE: " << msg.hdr.size << "\n";
                cout << "BODY:\n"
                     << msg.body << "\n";
                string body = "hello world to you";
                con->send({.hdr = {.type = DATA, .size = body.size(), .time = time(NULL)}, .body = body});
                break;
            }
            if (argv[i] == string("-c"))
            {
                con = new client_connection(8080);
                string body = "hello world";
                con->send({.hdr = {.type = DATA, .size = body.size(), .time = time(NULL)}, .body = body});
                con->send({.hdr = {.type = DATA, .size = body.size(), .time = time(NULL)}, .body = body});
                message msg = con->read();
                cout << "TYPE: " << msg.hdr.type << "\n";
                cout << "TIME: " << asctime(localtime(&(msg.hdr.time))) << "\n";
                cout << "SIZE: " << msg.hdr.size << "\n";
                cout << "BODY:\n"
                     << msg.body << "\n";
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
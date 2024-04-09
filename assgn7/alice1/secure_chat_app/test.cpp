#include "chat_app.hpp"
#include <iostream>
using namespace std;

#define PORT 8080

connection *init_connection(bool is_server, uint16_t port, string hostname = "")
{
    if (is_server)
    {
        return new server_connection(port);
    }
    return new client_connection(hostname, port);
}

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
    connection *con = NULL;
    bool is_server;
    string hostname;
    try
    {
        if (argc < 2)
        {
            cerr << "Enter -s or -c hostname\n";
            return 0;
        }
        
        if (string(argv[1]) == "-s")
        {
            is_server = true;
        }
        else if (string(argv[1]) == "-c" && argc == 3)
        {
            is_server = false;
            hostname = argv[2];
        }
        else
        {
            cerr << "Enter -s or -c hostname\n";
            return 0;
        }

        con = init_connection(is_server, PORT, hostname);
        con->send_msg(to_message("hello bro"));
        displayMessage(con->read_msg());
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }
    delete con;
    try
    {
        con = init_connection(is_server, PORT, hostname);
        con->send_msg(to_message("hello bro"));
        displayMessage(con->read_msg());
    }
    catch(const std::exception& e)
    {
        std::cerr << e.what() << '\n';
    }

    // Clean up
    delete con;
    return 0;
}
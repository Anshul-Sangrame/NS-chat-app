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

int main(int argc, char *argv[])
{
    connection *con1 = NULL;
    connection *con2 = NULL;
    string hostname1;
    string hostname2;
    try
    {
        if (argc < 2 || string(argv[1]) != "-d")
        {
            cerr << "Enter -d for downgrade\n";
            return 0;
        }
        else if (argc < 4)
        {
            cerr << "Enter -d <servername> <clientname>\n";
            return 0;
        }
        else
        {
            hostname1 = string(argv[2]);
            hostname2 = string(argv[3]);
        }

        con1 = init_connection(1, PORT, hostname1);
        con2 = init_connection(0, PORT, hostname2);
        PassiveHandler h(con1, con2);
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
    }

    // Clean up
    delete con1;
    delete con2;
    return 0;
}
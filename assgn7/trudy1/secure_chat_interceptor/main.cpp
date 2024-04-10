#include "chat_app.hpp"
#include <iostream>
using namespace std;

#define PORT 8080

int main(int argc, char *argv[])
{
    // server <--- con1 ---> Trudy
    connection *con1 = NULL;
    //                       Trudy <--- con2 ---> client
    connection *con2 = NULL;

    try
    {
        if (argc < 4)
        {
            cerr << "Enter -d <server> <client>";
            return 0;
        }
        if (argv[1] != string("-d"))
        {
            cerr << "Enter -d <server> <client>";
            return 0;
        }
        con1 = new client_connection(argv[2],PORT);
        con2 = new server_connection(PORT);
        // Rajiv add passive handler here
        cerr << "reaching";
        PassiveHandler h(con1,con2);
        
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
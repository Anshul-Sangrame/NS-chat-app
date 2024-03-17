#include "connection.hpp"
#include <iostream>
#include <unistd.h>
#include <string.h>
#include <sstream>
using namespace std;
std::string to_text[] = {"DATA", "CONTROL"};

string messageToString(message msg)
{
    string txt =
        string("TYPE ") + to_text[msg.type] + "\n" +
        "SIZE " + to_string(msg.body.size()) + "\n" +
        "TIME " + to_string(msg.time) + "\n" +
        "#\n" + msg.body;
    return txt;
}

message stringToMessage(string s)
{
    message res;
    stringstream ss(s);
    string line;
    stringstream lineStream;
    string token;
    string value;
    // Get TYPE
    if (!getline(ss,line)) throw runtime_error("Invalid format");
    token = strtok(&line[0]," ");
    // if (!token) throw runtime_error("Invalid format");
    if (token != "TYPE") throw runtime_error("Invalid format");
    if (!(ss >> value)) throw runtime_error("Invalid format");
    if (value != "DATA" && value != "CONTROL") throw runtime_error("Invalid format");
    if (value == "DATA") res.type = DATA;
    if (value == "CONTROL") res.type = CONTROL;

    // Get SIZE
    if (!getline(ss,line)) throw runtime_error("Invalid format");
    if (!(ss >> token)) throw runtime_error("Invalid format");
    if (token != "SIZE") throw runtime_error("Invalid format");
    if (!(ss >> value)) throw runtime_error("Invalid format");
    try
    {
        stoi(value);
    }
    catch(const std::exception& e)
    {
        throw runtime_error("Invalid format");
    }

    // Get TIME
    if (!getline(ss,line)) throw runtime_error("Invalid format");
    if (!(ss >> token)) throw runtime_error("Invalid format");
    if (token != "TIME") throw runtime_error("Invalid format");
    if (!(ss >> value)) throw runtime_error("Invalid format");
    try
    {
        res.time = stoi(value);
    }
    catch(const std::exception& e)
    {
        throw runtime_error("Invalid format");
    }

    // Check for #
    if (!getline(ss,line)) throw runtime_error("Invalid format");
    if (!(ss >> token)) throw runtime_error("Invalid format");
    if (token != "#") throw runtime_error("Invalid format");
    
    // body
    res.body = ss.str();

    return {.type = DATA, .time = 90, .body = "jsfk"};
}

bool operator<(const message &a, const message &b)
{
    return (a.time < b.time);
}
bool operator>(const message &a, const message &b)
{
    return (a.time > b.time);
}
bool operator<=(const message &a, const message &b)
{
    return (a.time <= b.time);
}
bool operator>=(const message &a, const message &b)
{
    return (a.time >= b.time);
}

connection::connection()
{
    SSL = false;
}

void connection::send(message msg)
{
    string payload = messageToString(msg);
    if (!SSL)
        sendto(sockfd, payload.c_str(), payload.size(), MSG_CONFIRM, (const struct sockaddr *)&to_addr, len);
}

message connection::read()
{
    if (!SSL)
    {
        char buffer[BUFF_SIZE];
        int n;
        n = recvfrom(sockfd, (char *)buffer, BUFF_SIZE, MSG_WAITALL, (struct sockaddr *)&to_addr, &len);
        buffer[n] = '\0';
        cout << "TOTAL SIZE:" << n << "\n";
        cout << buffer << "\n";
        return stringToMessage(buffer);
    }
}

connection::~connection()
{
}

client_connection::client_connection(uint16_t port)
{
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }
    memset(&to_addr, 0, sizeof(to_addr));

    // Filling server information
    to_addr.sin_family = AF_INET;
    to_addr.sin_port = htons(port);
    to_addr.sin_addr.s_addr = INADDR_ANY;

    len = sizeof(to_addr);
}

void client_connection::startSSL()
{
    // Implement SSL stuff
}

client_connection::~client_connection()
{
    close(sockfd);
}

server_connection::server_connection(uint16_t port)
{
    // Creating socket file descriptor
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        throw runtime_error(string("socket creation failed: ") + strerror(errno));
    }

    memset(&my_addr, 0, sizeof(my_addr));
    memset(&to_addr, 0, sizeof(to_addr));

    // Filling server information
    my_addr.sin_family = AF_INET; // IPv4
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    // Bind the socket with the server address
    if (bind(sockfd, (const struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        throw runtime_error(string("bind failed: ") + strerror(errno));
    }

    len = sizeof(to_addr);
}

void server_connection::startSSL()
{
    // Implement SSL stuff
}

server_connection::~server_connection()
{
    // Destroy 
}

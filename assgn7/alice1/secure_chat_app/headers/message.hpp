#pragma once

#include <string>

#define BODY_SIZE 4096

#define DATA 0
#define CONTROL 1

struct message_header
{
    uint64_t type;
    int64_t time;
};

#define HEADER_SIZE sizeof(message_header)
#define BUFF_SIZE (HEADER_SIZE + BODY_SIZE)

struct message
{
    struct message_header hdr;
    std::string body;
};

// Can be used in priority queue or sorting
bool operator<(const message &a, const message &b);
bool operator>(const message &a, const message &b);
bool operator<=(const message &a, const message &b);
bool operator>=(const message &a, const message &b);

// Used to form message from user input string
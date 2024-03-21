#include "message.hpp"

// string to_text[] = {"DATA", "CONTROL"};

bool operator<(const message &a, const message &b)
{
    return (a.hdr.time < b.hdr.time);
}
bool operator>(const message &a, const message &b)
{
    return (a.hdr.time > b.hdr.time);
}
bool operator<=(const message &a, const message &b)
{
    return (a.hdr.time <= b.hdr.time);
}
bool operator>=(const message &a, const message &b)
{
    return (a.hdr.time >= b.hdr.time);
}
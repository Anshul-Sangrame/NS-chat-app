#include <iostream>
#include <cstring>
using namespace std;

struct hdr
{
    uint64_t a1;
    uint64_t a2;
};

int main()
{
    const char *buff = "hello";
    struct hdr st = {899,10};

    string res((char *)&st,sizeof(hdr));
    // res.append(buff,10);

    for (int i : res)
    {
        cout << i << " ";
    }
    cout << "\n";
    cout << res.size() << "\n";

    struct hdr fin = *(hdr *)res.c_str();;

    cout << fin.a1 << " " << fin.a2 << "\n";
}
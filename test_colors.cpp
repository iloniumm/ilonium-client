#include <iostream>
#include <string>
#include <cstring>

using namespace std;

string RemoveColors(const char* c, bool darkonly) {
    string ret;
    int len = strlen(c);
    bool removed = false;

    while (*c != '\0') {
        if (*c == '0' && len >= 2 && c[1] == 'x') {
            if (len >= 8 && darkonly) {
                // mock
            } else if (len >= 8) {
                c += 8;
                len -= 8;
                removed = true;
            } else {
                return RemoveColors(ret.c_str(), darkonly);
            }
        } else {
            ret += *(c++);
            len--;
        }
    }
    return removed ? RemoveColors(ret.c_str(), darkonly) : ret;
}

int main() {
    cout << "Bob0x -> " << RemoveColors("Bob0x", false) << endl;
    cout << "0xBob -> " << RemoveColors("0xBob", false) << endl;
    cout << "0x1111110xBob -> " << RemoveColors("0x1111110xBob", false) << endl;
    cout << "Bob0x1111110x -> " << RemoveColors("Bob0x1111110x", false) << endl;
    return 0;
}

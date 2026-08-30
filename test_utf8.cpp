#include <iostream>
#include <cstring>
#include <string>

// Recreate tString mock for the test
class tString {
public:
    std::string s;
    tString() {}
    tString & operator<<(char c) {
        s += c;
        return *this;
    }
    tString & operator<<(const char *str) {
        s += str;
        return *this;
    }
    int Len() const { return s.length() + 1; }
};

static tString Utf8ToCp1251(const char *utf8) {
    tString result;
    if (!utf8) return result;
    int len = strlen(utf8);
    for (int i = 0; i < len; ) {
        unsigned char c = (unsigned char)utf8[i];
        unsigned int codepoint = 0;
        int bytes = 0;
        if (c < 0x80) {
            codepoint = c;
            bytes = 1;
        } else if ((c & 0xE0) == 0xC0) {
            codepoint = c & 0x1F;
            bytes = 2;
        } else if ((c & 0xF0) == 0xE0) {
            codepoint = c & 0x0F;
            bytes = 3;
        } else if ((c & 0xF8) == 0xF0) {
            codepoint = c & 0x07;
            bytes = 4;
        } else {
            codepoint = c;
            bytes = 1;
        }
        
        if (i + bytes > len) {
            result << (char)c;
            i++;
            continue;
        }
        
        for (int j = 1; j < bytes; j++) {
            unsigned char next = (unsigned char)utf8[i + j];
            if ((next & 0xC0) == 0x80) {
                codepoint = (codepoint << 6) | (next & 0x3F);
            } else {
                codepoint = c;
                bytes = 1;
                break;
            }
        }
        
        i += bytes;
        
        if (codepoint < 128) {
            result << (char)codepoint;
        } else if (codepoint >= 0x0410 && codepoint <= 0x044F) {
            result << (char)(codepoint - 0x0410 + 0xC0);
        } else if (codepoint == 0x0401) {
            result << (char)0xA8;
        } else if (codepoint == 0x0451) {
            result << (char)0xB8;
        } else if (codepoint == 0x0402) {
            result << (char)0x80;
        } else if (codepoint == 0x0452) {
            result << (char)0x90;
        } else if (codepoint == 0x0403) {
            result << (char)0x81;
        } else if (codepoint == 0x0453) {
            result << (char)0x83;
        } else if (codepoint == 0x0404) {
            result << (char)0xAA;
        } else if (codepoint == 0x0454) {
            result << (char)0xBA;
        } else if (codepoint == 0x0405) {
            result << (char)0xBD;
        } else if (codepoint == 0x0455) {
            result << (char)0xBE;
        } else if (codepoint == 0x0406) {
            result << (char)0xB2;
        } else if (codepoint == 0x0456) {
            result << (char)0xB3;
        } else if (codepoint == 0x0407) {
            result << (char)0xAF;
        } else if (codepoint == 0x0457) {
            result << (char)0xBF;
        } else if (codepoint == 0x0408) {
            result << (char)0xA3;
        } else if (codepoint == 0x0458) {
            result << (char)0xBC;
        } else if (codepoint == 0x0409) {
            result << (char)0x8A;
        } else if (codepoint == 0x0459) {
            result << (char)0x9A;
        } else if (codepoint == 0x040A) {
            result << (char)0x8C;
        } else if (codepoint == 0x045A) {
            result << (char)0x9C;
        } else if (codepoint == 0x040B) {
            result << (char)0x8E;
        } else if (codepoint == 0x045B) {
            result << (char)0x9E;
        } else if (codepoint == 0x040C) {
            result << (char)0x8D;
        } else if (codepoint == 0x045C) {
            result << (char)0x9D;
        } else if (codepoint == 0x040E) {
            result << (char)0xA1;
        } else if (codepoint == 0x045E) {
            result << (char)0xA2;
        } else if (codepoint == 0x040F) {
            result << (char)0x8F;
        } else if (codepoint == 0x045F) {
            result << (char)0x9F;
        } else if (codepoint == 0x0490) {
            result << (char)0xA5;
        } else if (codepoint == 0x0491) {
            result << (char)0xB4;
        } else if (codepoint == 0x2014) {
            result << (char)0x97;
        } else if (codepoint == 0x2013) {
            result << (char)0x96;
        } else if (codepoint == 0x201E) {
            result << (char)0x84;
        } else if (codepoint == 0x201C) {
            result << (char)0x93;
        } else if (codepoint == 0x201D) {
            result << (char)0x94;
        } else if (codepoint == 0x2018) {
            result << (char)0x91;
        } else if (codepoint == 0x2019) {
            result << (char)0x92;
        } else if (codepoint == 0x201A) {
            result << (char)0x82;
        } else if (codepoint == 0x2039) {
            result << (char)0x8B;
        } else if (codepoint == 0x203A) {
            result << (char)0x9B;
        } else if (codepoint == 0x2022) {
            result << (char)0x95;
        } else if (codepoint == 0x2026) {
            result << (char)0x85;
        } else if (codepoint == 0x2020) {
            result << (char)0x86;
        } else if (codepoint == 0x2021) {
            result << (char)0x87;
        } else if (codepoint == 0x2122) {
            result << (char)0x99;
        } else if (codepoint == 0x2030) {
            result << (char)0x89;
        } else if (codepoint == 0x20AC) {
            result << (char)0x88;
        } else if (codepoint == 0x2116) {
            result << (char)0xB9;
        } else if (codepoint == 0x00A0) {
            result << (char)0xA0;
        } else if (codepoint >= 128 && codepoint < 256) {
            result << (char)codepoint;
        } else {
            result << '?';
        }
    }
    return result;
}

void print_bytes(const std::string &name, const tString &t) {
    std::cout << name << ": ";
    for (size_t i = 0; i < t.s.length(); i++) {
        unsigned char c = (unsigned char)t.s[i];
        std::cout << "0x" << std::hex << (int)c << " ";
    }
    std::cout << "\n";
}

static char TranslateQwertyToRussianCp1251(char c) {
    switch (c) {
        // Lowercase letters
        case 'q': return (char)0xE9; // й
        case 'w': return (char)0xF6; // ц
        case 'e': return (char)0xF3; // у
        case 'r': return (char)0xEA; // к
        case 't': return (char)0xE5; // е
        case 'y': return (char)0xED; // н
        case 'u': return (char)0xE3; // г
        case 'i': return (char)0xF8; // ш
        case 'o': return (char)0xF9; // щ
        case 'p': return (char)0xE7; // з
        case '[': return (char)0xF5; // х
        case ']': return (char)0xFA; // ъ
        case 'a': return (char)0xF4; // ф
        case 's': return (char)0xFB; // ы
        case 'd': return (char)0xE2; // в
        case 'f': return (char)0xE0; // а
        case 'g': return (char)0xEF; // п
        case 'h': return (char)0xF0; // р
        case 'j': return (char)0xEE; // о
        case 'k': return (char)0xEB; // л
        case 'l': return (char)0xE4; // д
        case ';': return (char)0xE6; // ж
        case '\'': return (char)0xFD; // э
        case 'z': return (char)0xFF; // я
        case 'x': return (char)0xF7; // ч
        case 'c': return (char)0xF1; // с
        case 'v': return (char)0xEC; // м
        case 'b': return (char)0xE8; // и
        case 'n': return (char)0xF2; // т
        case 'm': return (char)0xFC; // ь
        case ',': return (char)0xE1; // б
        case '.': return (char)0xFE; // ю
        case '/': return (char)0x2E; // .
        case '`': return (char)0xB8; // ё
        case '~': return (char)0xA8; // Ё

        // Uppercase letters
        case 'Q': return (char)0xC9; // Й
        case 'W': return (char)0xD6; // Ц
        case 'E': return (char)0xD3; // У
        case 'R': return (char)0xCA; // К
        case 'T': return (char)0xC5; // Е
        case 'Y': return (char)0xCD; // Н
        case 'U': return (char)0xC3; // Г
        case 'I': return (char)0xD8; // Ш
        case 'O': return (char)0xD9; // Щ
        case 'P': return (char)0xC7; // З
        case '{': return (char)0xD5; // Х
        case '}': return (char)0xDA; // Ъ
        case 'A': return (char)0xD4; // Ф
        case 'S': return (char)0xDB; // Ы
        case 'D': return (char)0xC2; // В
        case 'F': return (char)0xC0; // А
        case 'G': return (char)0xCF; // П
        case 'H': return (char)0xD0; // Р
        case 'J': return (char)0xCE; // О
        case 'K': return (char)0xCB; // Л
        case 'L': return (char)0xC4; // Д
        case ':': return (char)0xC6; // Ж
        case '"': return (char)0xDD; // Э
        case 'Z': return (char)0xDF; // Я
        case 'X': return (char)0xD7; // Ч
        case 'C': return (char)0xD1; // С
        case 'V': return (char)0xCC; // М
        case 'B': return (char)0xC8; // И
        case 'N': return (char)0xD2; // Т
        case 'M': return (char)0xDC; // Ь
        case '<': return (char)0xC1; // Б
        case '>': return (char)0xDE; // Ю
        case '?': return (char)0x2C; // ,

        // Other symbols mapped in Russian layout
        case '@': return (char)0x22; // "
        case '#': return (char)0xB9; // №
        case '$': return (char)0x3B; // ;
        case '^': return (char)0x3A; // :
        case '&': return (char)0x3F; // ?

        default: return c;
    }
}

int main() {
    // Check lowercase q to m
    std::string qwerty = "qwertyuiop[]asdfghjkl;'zxcvbnm,./`~";
    std::cout << "Translating QWERTY characters:\n";
    for (char c : qwerty) {
        unsigned char translated = (unsigned char)TranslateQwertyToRussianCp1251(c);
        std::cout << c << " -> 0x" << std::hex << (int)translated << "\n";
    }
    
    std::cout << "\nComparing with direct Utf8ToCp1251:\n";
    std::cout << "q mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('q') 
              << ", expected й: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("й").s[0] << "\n";
    std::cout << "w mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('w') 
              << ", expected ц: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("ц").s[0] << "\n";
    std::cout << "e mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('e') 
              << ", expected у: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("у").s[0] << "\n";
    std::cout << "r mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('r') 
              << ", expected к: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("к").s[0] << "\n";
    std::cout << "t mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('t') 
              << ", expected е: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("е").s[0] << "\n";
    std::cout << "y mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('y') 
              << ", expected н: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("н").s[0] << "\n";
    std::cout << "m mapped: 0x" << std::hex << (int)(unsigned char)TranslateQwertyToRussianCp1251('m') 
              << ", expected ь: 0x" << std::hex << (int)(unsigned char)Utf8ToCp1251("ь").s[0] << "\n";
    return 0;
}

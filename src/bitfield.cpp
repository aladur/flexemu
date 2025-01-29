#include <iostream>

int main(int /* argc */, char ** /* argv */)
{
    union u_t
    {
        unsigned char all;
        struct { bool first : 1; } bit;
    };

    u_t v{0};
    v.bit.first = true;
    std::cout << static_cast<unsigned>(v.all) << "\n";

    return 0;
}

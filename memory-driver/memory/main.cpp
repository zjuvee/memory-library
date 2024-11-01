#include "memory.h"

#include <thread>
#include <chrono>

using namespace std;

int main() {
    memory mem("cs2.exe");
    

    bool enabled = true;

    uint32_t STANDING = 65665;
    uint32_t CROUCHING = 65667;
    uint32_t PLUS_JUMP = 65537;
    uint32_t MINUS_JUMP = 256;

    uintptr_t dwLocalPlayerPawn = 0x1836BB8;
    uintptr_t dwForceJump = 0x182FBC0;
    uintptr_t m_fFlags = 0x3EC;
    
    uintptr_t client = mem.get_module("client.dll");
    uintptr_t ForceJump = client + dwForceJump;

    while (true)
    {
        if (enabled)
        {
            if (!enabled)
                break;

            uintptr_t localPlayer = mem.read<uintptr_t>(client + dwLocalPlayerPawn);
            uintptr_t flags = mem.read<uint32_t>(localPlayer + m_fFlags);

            if (GetAsyncKeyState(0x20) & 0x8000)
            {
                if (flags == STANDING || flags == CROUCHING)
                {
                    this_thread::sleep_for(chrono::milliseconds(15));
                    if (mem.write<uint32_t>(client + dwForceJump, PLUS_JUMP))
                        cout << "jump" << endl;
                }
                else
                {
                    if (mem.write<uint32_t>(client + dwForceJump, MINUS_JUMP))
                        cout << "reset" << endl;
                }
            }
            this_thread::sleep_for(chrono::milliseconds(12));
        }
    }

    return 0;
}

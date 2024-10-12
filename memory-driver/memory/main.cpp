#include "memory.h"

#include <thread>
#include <chrono>

using namespace std;

int main() {
    memory mem("cs2.exe");
	system("sc create kerneldriver type= kernel binPath= \"D:\\Repos\\gamehacking\\kernel-game\\x64\\Release\\kernel-game.sys\n");
	system("sc start kerneldriver");
	system("cls");
	mem.OpenDevice();

    const uint32_t STANDING = 65665;
    const uint32_t CROUCHING = 65667;
    const uint32_t PLUS_JUMP = 65537;
    const uint32_t MINUS_JUMP = 256;

    int dwLocalPlayerPawn = 0x1825138;
    int dwForceJump = 0x181E140;
    int m_fFlag = 0x3EC;

    
    uintptr_t client = mem.get_module("client.dll");
    uintptr_t ForceJump = client + dwForceJump;

    while (true)
    {
        uintptr_t LocalPlayer = mem.Read<uintptr_t>(client + dwLocalPlayerPawn);
        uint32_t fFlag = mem.Read<uint32_t>(LocalPlayer + m_fFlag);

        

        if (GetAsyncKeyState(0x20) & 0x8000)
        {
            if (fFlag == STANDING || fFlag == CROUCHING)
            {
                this_thread::sleep_for(chrono::milliseconds(1));
                mem.Write<uint32_t>(ForceJump, PLUS_JUMP);
            }
            else
            {
                mem.Write<uint32_t>(ForceJump, MINUS_JUMP);
            }
        }
        this_thread::sleep_for(chrono::milliseconds(1));
    }

	mem.CloseDevice();
    return 0;
}

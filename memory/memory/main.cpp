#include "memory.h"

using namespace std;

int main() {
    // ingresamos el proceso
    memory mem("explorer.exe");

    // definimos la direccion
    uintptr_t address = 0x7ff8c69ce813;

    // lee el string que se encuentra en la direccion de memoria
    string value = mem.ReadString(address);

    cout << "original string: " << value << endl;
    
    // modifica la direccion cambiando el string
    mem.WriteString(address, "penesemen");

    // volvemos a leer para verificar que haya cambiado
    value = mem.ReadString(address);
    
    cout << "new string: " << value << endl;

    return 0;
}

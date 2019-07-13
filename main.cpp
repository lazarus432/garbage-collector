#include "gc_pointer.h"
#include "LeakTester.h"

using namespace std;

int main() {
    Pointer<int> p = new int(20);
    p = new int(23);
    p = new int(30);

    return 0;
}
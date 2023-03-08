#include "sort_data.h"
#include <iostream>

int main() {
    SortData<unsigned> data{"../input/input.txt", SortData<unsigned>::MAX_LEN};
    data.gen();
    data.write();
}

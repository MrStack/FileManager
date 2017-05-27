#include <iostream>
#include <cstdlib>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "file_manage.hpp"

using namespace std;

int main()
{
    class File Test{};
    Test.ReadDir("/home/albertstack/Documents");

//    Test.ListDir("-a");
    Test.TimeFilter(FM_DAYS,0,-4);

    return 0;
}

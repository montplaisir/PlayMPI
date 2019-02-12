#include <iostream>

int main(int argc, char** argv)
{
    std::string cmd = "mpirun -np 5 -f ~/hostfile hello.exe";
    // Using system for now. popen might also be used.
    std::cout << "Launching command: " << std::endl << cmd << std::endl;
    system(cmd.c_str());
    return 0;
}

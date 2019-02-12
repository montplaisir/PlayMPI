#include <iostream>

int main(int argc, char** argv)
{
    std::string nbProcesses = "5";
    std::string hostfile    = "";
    std::string exec        = "hello.exe";
    std::string nbPoints    = "10";
    bool useMPI = true;

    if (argc >= 2)
    {
        std::string arg1(argv[1]);
        if ("-h" == arg1 || "-help" == arg1 || 2 == argc)
        {
            std::cout << "Usage: " << argv[0] << " -n <nb of processes> -f <hostfile> -p <nb of points to eval>" << std::endl;
            return 1;
        }
        // Not solid: order of arguments is strict. Only for proof of concept.
        std::string arg2(argv[2]);
        if ("-n" == arg1)
        {
            nbProcesses = arg2;
            std::cout << "nbProcesses = " << nbProcesses << std::endl;
            if ("0" == nbProcesses)
            {
                useMPI = false;
            }
        }
        if (argc >= 5)
        {
            std::string arg3(argv[3]);
            std::string arg4(argv[4]);
            if ("-f" == arg3)
            {
                hostfile = arg4;
            }
            if (argc >= 7)
            {
                std::string arg5(argv[5]);
                std::string arg6(argv[6]);
                if ("-p" == arg5)
                {
                    nbPoints = arg6;
                }
            }
        }
    }


    std::string cmd;
    if (useMPI)
    {
        // Construct command.
        // Number of processes
        cmd = "mpirun -np " + nbProcesses;
        // Name of the hostfile
        if (!hostfile.empty())
        {
            cmd += " -f " + hostfile;
        }
        // Name of the executable
        cmd += " " + exec;
        // Argument to the executable
        cmd += " " + nbPoints;
    }
    else
    {
        // Non-mpi call.
        cmd = exec + " " + nbPoints;
    }

    // Using system for now. popen might also be used.
    std::cout << "Launching command: " << std::endl << cmd << std::endl;
    system(cmd.c_str());

    return 0;
}

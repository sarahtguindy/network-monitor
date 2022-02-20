#include <iostream>
#include <string.h>
#include <vector>
#include <signal.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

using namespace std;

char socket_path[] = "/tmp/tmp1";
const int BUF_LEN = 512;
const int MAX_CLIENTS = 2;
bool is_running;
pid_t childPid[2];

// Function to handle shutdown on ctrl-C
static void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGINT:
        is_running = false;
        // Sends a shutdown message to each client and exits
        cout << "networkMonitor: signalHandler(" << signal << ") SIGINT" << endl;
        for (int i = 0; i < 2; i++)
        {
            cout << "networkMonitor: shutting down (" << childPid[i] << ")" << endl;
            kill(childPid[i], SIGINT);
        }
        break;
    default:
        cout << "signalHandler(" << signal << "): unknown" << endl;
    }
}

int main()
{
    int interfaces = 0;

    // Get the number of interfaces to monitor from user input
    cout << "How many interfaces do you want to monitor: ";
    cin >> interfaces;

    vector<string> interfaceNames;
    string intfName = "";

    // Get each interface name
    for (int i = 0; i < interfaces; i++)
    {
        cout << endl
             << "Enter interface " << i + 1 << ": ";
        cin >> intfName;
        interfaceNames.push_back(intfName);
    }

    // Prepare the socket
    struct sockaddr_un addr;
    char buf[BUF_LEN];
    int len;
    int master_fd, max_fd, cl[MAX_CLIENTS], rc;
    fd_set active_fd_set;
    fd_set read_fd_set;
    int ret;
    int numClients = 0;

    // Set up signal handler to terminate program gracefully
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    // Spawn child process
    bool isParent = true;
    for (int i = 0; i < interfaceNames.size() & isParent; ++i)
    {
        sleep(0.5);
        childPid[i] = fork();
        if (childPid[i] == 0)
        {
            isParent = false;
            execlp("./interfaceMonitor", "./interfaceMonitor", interfaceNames[i].c_str(), NULL);
            cout << "(" << getpid() << ") I should not get here!" << endl;
            cout << strerror(errno) << endl;
        }
    }

    // Create the socket
    memset(&addr, 0, sizeof(addr));
    if ((master_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        cout << "networkMonitor: " << strerror(errno) << endl;
        exit(-1);
    }

    // Set socket path to local socket file
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);
    unlink(socket_path);

    // Bind the socket to this local socket file
    if (bind(master_fd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        cout << "networkMonitor: " << strerror(errno) << endl;
        close(master_fd);
        exit(-1);
    }

    cout << "Waiting for the interfaces..." << endl;

    // Listen for a client to connect to this local socket file
    if (listen(master_fd, 5) == -1)
    {
        cout << "networkMonitor: " << strerror(errno) << endl;
        unlink(socket_path);
        close(master_fd);
        exit(-1);
    }

    FD_ZERO(&read_fd_set);
    FD_ZERO(&active_fd_set);
    FD_SET(master_fd, &active_fd_set);
    max_fd = master_fd;

    is_running = true;
    while (is_running)
    {
        // Block until an input arrives on one or more sockets
        read_fd_set = active_fd_set;
        ret = select(max_fd + 1, &read_fd_set, NULL, NULL, NULL);
        if (ret < 0)
        {
            cout << "networkMonitor: " << strerror(errno) << endl;
        }
        else
        {
            // Service all the sockets with input pending
            if (FD_ISSET(master_fd, &read_fd_set))
            {
                // Accept connection
                cl[numClients] = accept(master_fd, NULL, NULL);
                if (cl[numClients] < 0)
                {
                    cout << "server: " << strerror(errno) << endl;
                }
                else
                {
                    cout << "Server: incoming connection " << cl[numClients] << endl;

                    // Add connection to the set
                    FD_SET(cl[numClients], &active_fd_set);
                    // Clear buffer
                    memset(buf, 0, sizeof(buf));
                    // Read data from client
                    ret = read(cl[numClients], buf, BUF_LEN);
                    if (strcmp(buf, "Ready") == 0)
                    {
                        // Clear buffer
                        memset(buf, 0, sizeof(buf));
                        // Tell interfaceMonitor it is ready
                        len = sprintf(buf, "%s", "Monitor");
                        ret = write(cl[numClients], buf, len);
                    }
                    if (ret == -1)
                    {
                        cout << "server: " << strerror(errno) << endl;
                    }
                    if (max_fd < cl[numClients])
                    {
                        max_fd = cl[numClients];
                    }
                    ++numClients;
                }
            }
            // Case for data arriving on an already-connected socket
            else
            {
                for (int i = 0; i < numClients; ++i)
                {
                    // Find the client that sent the data
                    if (FD_ISSET(cl[i], &read_fd_set))
                    {
                        // Clear buffer
                        memset(buf, 0, sizeof(buf));
                        // Read data from client
                        ret = read(cl[i], buf, BUF_LEN);
                        if (strcmp(buf, "Link Down") == 0)
                        {
                            // Clear buffer
                            memset(buf, 0, sizeof(buf));
                            len = sprintf(buf, "%s", "Set Link Up");
                            ret = write(cl[i], buf, BUF_LEN);
                        }
                        if (ret == -1)
                        {
                            cout << "server: " << strerror(errno) << endl;
                        }
                    }
                }
            }
        }
    }

    // Request each client to quit
    for (int i = 0; i < numClients; ++i)
    {
        len = sprintf(buf, "Quit") + 1;
        ret = write(cl[i], buf, len);
        if (ret == -1)
        {
            cout << "networkMonitor: " << strerror(errno) << endl;
        }
        // Give the clients a change to quit
        sleep(1);
        // Remove the socket from the set of active sockets
        FD_CLR(cl[i], &active_fd_set);
        // Close the socket connection
        close(cl[i]);
    }

    // Close master socket, remove socket file and exit
    close(master_fd);
    unlink(socket_path);
    return 0;
}
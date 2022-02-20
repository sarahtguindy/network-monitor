#include <iostream>
#include <fstream>
#include <cstring>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "interfaceMonitor.h"

using namespace std;

char socket_path[] = "/tmp/tmp1";
const int BUF_LEN = 512;
bool is_running = true;

// Function to handle shutdown on ctrl-C
static void signalHandler(int signal)
{
    switch (signal)
    {
    case SIGINT:
        cout << "interfaceMonitor: signalHandler(" << signal << ") SIGINT" << endl;
        is_running = false;
        break;
    default:
        cout << "interfaceMonitor: signalHandler(" << signal << ") unknown" << endl;
    }
}

// Function that enables the client to write data
void writeMessage(int fd, string message)
{
    char buf[BUF_LEN];
    int len, ret;
    memset(buf, 0, sizeof(buf));
    len = sprintf(buf, "%s", message.c_str());
    ret = write(fd, buf, BUF_LEN);
}

// Function to set interface flags
int set_if_flags(char *ifname, short flags)
{
    int skfd = -1;
    struct ifreq ifr;
    int res = 0;
    ifr.ifr_flags = flags;
    strncpy(ifr.ifr_name, ifname, IFNAMSIZ);

    if ((skfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        printf("socket error: %s\n", strerror(errno));
        res = 1;
        goto out;
    }

    res = ioctl(skfd, SIOCSIFFLAGS, &ifr);

    if (res < 0)
    {
        printf("Interface '%s': Error: %s\n",
               ifname, strerror(errno));
    }
    else
    {
        printf("Interface '%s': Link Up\n", ifname);
    }

out:
    close(skfd);
    return res;
}

// Function to turn on network interface
int set_if_up(char *ifname, short flags)
{
    return set_if_flags(ifname, flags | IFF_UP);
}

int main(int argc, char *argv[])
{
    // Create and set interface from argument given
    char intf[BUF_LEN];
    strncpy(intf, argv[1], BUF_LEN);

    // Set up signal handler to terminate program gracefully
    struct sigaction action;
    action.sa_handler = signalHandler;
    sigemptyset(&action.sa_mask);
    action.sa_flags = 0;
    sigaction(SIGINT, &action, NULL);

    //Set up socket communications
    struct sockaddr_un addr;
    char buf[BUF_LEN];
    int len, ret;
    int fd, rc;

    memset(&addr, 0, sizeof(addr));

    // Create socket
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        cout << "interfaceMonitor (" << getpid() << "): " << strerror(errno) << endl;
        exit(-1);
    }
    addr.sun_family = AF_UNIX;

    // Set socket path to local socket file
    strncpy(addr.sun_path, socket_path, sizeof(addr.sun_path) - 1);

    // Connect to local socket
    if (connect(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout << "client (" << getpid() << "): " << strerror(errno) << endl;
        close(fd);
        exit(-1);
    }

    // Set a read timeout of one second
    struct timeval tv;
    tv.tv_sec = 1;
    tv.tv_usec = 0;
    setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof tv);

    // Informs networkMonitor that it is ready to start monitoring
    writeMessage(fd, "Ready");

    // Clear buffer
    memset(buf, 0, sizeof(buf));

    // Receives message from networkMonitor
    ret = read(fd, buf, BUF_LEN);
    if (strcmp(buf, "Monitor") == 0)
    {
        // Informs networkMonitor that it is monitoring
        writeMessage(fd, "Monitoring");
        Interface obj;
        obj.name = intf;

        is_running = true;
        while (is_running)
        {
            // Display interface statistics
            obj.displayIntfStats(intf);

            // If interface is down, informs networkMonitor and requests to link up interface
            if (obj.operstate.compare("down") == 0)
            {
                writeMessage(fd, "Link Down");

                memset(buf, 0, sizeof(buf));

                // Sets the interface link up again
                ret = read(fd, buf, BUF_LEN);
                if (strcmp(buf, "Set Link Up") == 0)
                {
                    cout << "interfaceMonitor: Set Link Up received" << endl;
                    set_if_up(intf, 1);
                }
            }
        }
    }

    // Close file descriptor, and remove the socket file before exiting
    close(fd);
    unlink(socket_path);
    return 0;
}
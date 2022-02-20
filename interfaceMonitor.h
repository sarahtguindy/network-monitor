#ifndef INTERFACEMONITOR_H
#define INTERFACEMONITOR_H

#include <iostream>
#include <fstream>
#include <string>
#include <unistd.h>

using namespace std;

class Interface
{
public:
    // Variables to store the statistics to be monitfored
    string name;
    string operstate;
    int carrier_up_count = 0;
    int carrier_down_count = 0;
    int rx_bytes = 0;
    int rx_dropped = 0;
    int rx_errors = 0;
    int rx_packets = 0;
    int tx_bytes = 0;
    int tx_dropped = 0;
    int tx_errors = 0;
    int tx_packets = 0;

    // Function to display the interface statistics
    void displayIntfStats(const char intf[])
    {
        char statPath[2 * 512];
        ifstream infile;

        // Get operstate
        sprintf(statPath, "/sys/class/net/%s/operstate", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> this->operstate;
            infile.close();
        }

        // Get carrier_up_count
        sprintf(statPath, "/sys/class/net/%s/carrier_up_count", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> carrier_up_count;
            infile.close();
        }

        // Get carrier_down_count
        sprintf(statPath, "/sys/class/net/%s/carrier_down_count", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> carrier_down_count;
            infile.close();
        }

        // Get rx_bytes
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_bytes", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> rx_bytes;
            infile.close();
        }

        // Get rx_dropped
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_dropped", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> rx_dropped;
            infile.close();
        }

        // Get rx_error
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_errors", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> rx_errors;
            infile.close();
        }

        // Get rx_packets
        sprintf(statPath, "/sys/class/net/%s/statistics/rx_packets", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> rx_packets;
            infile.close();
        }

        // Get tx_bytes
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_bytes", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> tx_bytes;
            infile.close();
        }

        // Get tx_dropped
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_dropped", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> tx_dropped;
            infile.close();
        }

        // Get tx_errors
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_errors", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> tx_errors;
            infile.close();
        }

        // Get tx_packets
        sprintf(statPath, "/sys/class/net/%s/statistics/tx_packets", intf);
        infile.open(statPath);
        if (infile.is_open())
        {
            infile >> tx_packets;
            infile.close();
        }

        // Display interface statistics
        cout << " Interface: " << intf << " state: " << operstate << " up_count: " << carrier_up_count << " down_count: " << carrier_down_count << endl;
        cout << " rx_bytes: " << rx_bytes << " rx_dropped: " << rx_dropped << " rx_errors: " << rx_errors << " rx_packets: " << rx_packets << endl;
        cout << " tx_bytes: " << tx_bytes << " tx_dropped: " << tx_dropped << " tx_errors: " << tx_errors << " tx_packets: " << tx_packets << endl
             << endl;

        sleep(1);
    }
};

#endif
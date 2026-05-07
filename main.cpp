#include <bits/stdc++.h>
#include "net/server.h"
#include "net/client.h"
using namespace std;
int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        cerr << "Usage: " << argv[0] << " [server|client]" << endl;
        return 1;
    }

    string mode(argv[1]);
    if (mode == "server")
    {
        net::runServer();
    }
    else if (mode == "client")
    {
        net::runClient();
    }
    else
    {
        cerr << "Invalid mode. Use 'server' or 'client'." << endl;
        return 1;
    }

    return 0;
}
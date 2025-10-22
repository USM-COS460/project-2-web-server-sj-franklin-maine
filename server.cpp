// Base of this code from https://www.tutorialspoint.com/cplusplus/cpp_socket_programming.htm
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/stat.h>
#include <fstream>
#include <cstdio>
#include <pthread.h>

#include <cstring>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <algorithm>
#include <sstream>

using namespace std;

struct arg_struct 
{
    int socket;
    string dir_path;
};

void *process_client(void *arguments) 
{
    struct arg_struct *args = (struct arg_struct *)arguments;
    int new_socket = args -> socket;
    string dir_path = args -> dir_path;
    char buffer[4096] = {0};
    bool has_input = true;
    string response;

    // Get the request from the user
    while (has_input)
    {
        int num_bytes = 0;
        // Clear the buffer before using it
        memset(buffer, 0, sizeof(buffer));
        num_bytes = read(new_socket, buffer, 4096);

        if (num_bytes <= 0)
        {
            // Error when < 0, maybe switch to error handling
            continue;
        }

        response.append(buffer, num_bytes);
        // Look for our termination sequence
        if (response.find("\r\n\r\n") != string::npos) 
        {
            has_input = false;
        }
    }

    // Actually process the request
    string input = response.substr(0, response.find("\n"));
    istringstream stream(input);
    string get, filename, version;
    if(stream >> get >> filename >> version)
    {
        // Valid request
        if(get.compare("GET") == 0 && version.rfind("HTTP/", 0) == 0)
        {
            string base_message = "HTTP/1.1 200 OK\nServer: server/1.0\nContent-Type: ";
            // Index request
            if(filename.compare("/") == 0 || filename.compare("/index.html") == 0)
            {
                cout << "-----------------------\nReceived request for index page\n" << endl;
                ifstream fin((dir_path + "/index.html").c_str(), ios::ate);
                if (fin) 
                {
                    string index_prefix_message = base_message + "text/html\nContent-Length: ";
                    char buffer[256];
                    sprintf(buffer, "%ld\n\n", static_cast<long>(fin.tellg()));
                    string message = index_prefix_message.append(buffer); // oof

                    fin.seekg(0, ios::beg);
                    ostringstream ostrm;
                    ostrm << fin.rdbuf();
                    message.append(ostrm.str());
                    send(new_socket, message.c_str(), message.size(), 0);
                }
                else 
                {
                    // Set error 404, file not found
                    string message = "HTTP/1.1 404 File not found\nServer: server/1.0";
                    cout << "Error message sent: 404 File not found\n";
                    send(new_socket, message.c_str(), message.size(), 0);
                }  
            } 
            else if(filename.substr(filename.find_last_of(".") + 1) == "css")
            {
                cout << "-----------------------\nReceived request for css file\n" << endl;
                ifstream fin((dir_path + filename).c_str(), ios::ate);
                if (fin) 
                {
                    string index_prefix_message = base_message + "text/css\nContent-Length: ";
                    char buffer[256];
                    sprintf(buffer, "%ld\n\n", static_cast<long>(fin.tellg()));
                    string message = index_prefix_message.append(buffer); // oof

                    fin.seekg(0, ios::beg);
                    ostringstream ostrm;
                    ostrm << fin.rdbuf();
                    message.append(ostrm.str());
                    send(new_socket, message.c_str(), message.size(), 0);
                }
                else 
                {
                    // Set error 404, file not found
                    string message = "HTTP/1.1 404 File not found\nServer: server/1.0";
                    cout << "Error message sent: 404 File not found in here 1\n";
                    send(new_socket, message.c_str(), message.size(), 0);
                }  
            }
            else if(filename.substr(filename.find_last_of(".") + 1) == "jpeg") 
            {
                cout << "-----------------------\nReceived request for image:" << filename << "\n\n";
                ifstream fin((dir_path + filename).c_str(), ios::binary | ios::ate);
                if (fin) 
                {
                    string index_prefix_message = base_message + "image/jpeg\nContent-Length: ";
                    char buffer[256];
                    sprintf(buffer, "%ld\n\n", static_cast<long>(fin.tellg()));
                    string message = index_prefix_message.append(buffer); // oof

                    fin.seekg(0, ios::beg);
                    ostringstream ostrm;
                    ostrm << fin.rdbuf();
                    message.append(ostrm.str());
                    send(new_socket, message.c_str(), message.size(), 0);
                }
                else 
                {
                    // Set error 404, file not found
                    string message = "HTTP/1.1 404 File not found\nServer: server/1.0";
                    cout << "Error message sent: 404 File not found in here 2\n";
                    send(new_socket, message.c_str(), message.size(), 0);
                }
            }
            else
            {
                // Set error 404, file not found
                string message = "HTTP/1.1 404 File not found\nServer: server/1.0";
                cout << "Error message sent: 404 File not found in here 3\n";
                send(new_socket, message.c_str(), message.size(), 0);
            }
        }
        else
        {
            // Set error 404, bad request
            string message = "HTTP/1.1 404 Unknown request type\nServer: server/1.0";
            cout << "Error message sent: 404 Unknown request\n";
            send(new_socket, message.c_str(), message.size(), 0);
        }
    }
    else
    {
        cout << "Error in GET request" << endl;
    }
    
    // Close socket
    close(new_socket);
}


int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        cout << "Three arguments required. Example: 'server [PORT_NUM] [DIR_NAME]'" << endl;
        return 1;
    }

    int port = stoi(argv[1]);
    if (port <= 0 || port > 65535)
    {
        cout << "Invalid port number (within range 1 and 65535)" << endl;
        return 1;
    }

    string dir_path = argv[2];

    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Attach socket to the port
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    // Bind
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen
    if (listen(server_fd, 3) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    while (true)
    {
        // Accept a connection
        new_socket =
            accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        struct arg_struct args;
        args.socket = new_socket;
        args.dir_path = dir_path;

        pthread_t tid;
        pthread_create(&tid, NULL, process_client, (void *)&args);
        pthread_detach(tid);
    }
    close(server_fd);
    return 0;
}
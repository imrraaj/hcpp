#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <zlib.h>
#include <vector>
#include <map>
#include <fstream>
#include <thread>

#ifndef HTTP_H
#define HTTP_H

#define MOD_GZIP_ZLIB_WINDOWSIZE 15
#define MOD_GZIP_ZLIB_CFACTOR 9
#define MOD_GZIP_ZLIB_BSIZE 8096

// Helper functions

std::vector<std::string> split(std::string s, std::string delimiter = " ")
{
    std::vector<std::string> result;
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos)
    {
        token = s.substr(0, pos);
        result.push_back(token);
        s.erase(0, pos + delimiter.length());
    }
    result.push_back(s);
    return result;
}

bool starts_with(std::string s, std::string prefix)
{
    return s.substr(0, prefix.size()) == prefix;
}

std::string compress_gzip(std::string s)
{
    z_stream zs;
    memset(&zs, 0, sizeof(zs));
    if (deflateInit2(&zs,
                     Z_BEST_COMPRESSION,
                     Z_DEFLATED,
                     MOD_GZIP_ZLIB_WINDOWSIZE + 16,
                     MOD_GZIP_ZLIB_CFACTOR,
                     Z_DEFAULT_STRATEGY) != Z_OK)
    {
        throw(std::runtime_error("deflateInit2 failed while compressing."));
    }

    zs.next_in = (Bytef *)s.data();
    zs.avail_in = s.size();

    int ret;
    char outbuffer[32768];
    std::string out_string;

    do
    {
        zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
        zs.avail_out = sizeof(outbuffer);

        ret = deflate(&zs, Z_FINISH);

        if (out_string.size() < zs.total_out)
        {
            out_string.append(outbuffer,
                              zs.total_out - out_string.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END)
    {
        std::cout << "Exception during zlib compression: (" << ret << ") " << zs.msg;
    }
    return out_string;
}

class Request
{
public:
    std::string method;
    std::string path;
    const std::string version = "HTTP/1.1";
    std::map<std::string, std::string> headers;
    std::string body;

    Request() {}
    Request(std::string method, std::string path, std::map<std::string, std::string> headers, std::string body)
    {
        this->method = method;
        this->path = path;
        this->headers = headers;
        this->body = body;
    }

    bool parse_request(char *buf)
    {
        printf("REQUEST: %s\n", buf);

        std::string req_str(buf);
        std::vector<std::string> headers_body_split = split(req_str, "\r\n\r\n");
        std::vector<std::string> req_headers = split(headers_body_split[0], "\r\n");
        std::vector<std::string> in_headers(req_headers.begin() + 1, req_headers.end());

        for (auto header : in_headers)
        {
            std::vector<std::string> header_split = split(header, ": ");
            this->headers[header_split[0]] = header_split[1];
        }
        std::vector<std::string> status_line_split = split(req_headers[0]);
        this->method = status_line_split[0];
        this->path = status_line_split[1];
        this->body = headers_body_split.size() > 1 ? headers_body_split[1] : "";
        return true;
    }
};

class Response
{
private:
    void send_response()
    {
        ::send(this->clientfd, this->status.c_str(), this->status.size(), 0);
        ::send(this->clientfd, "\r\n", 2, 0);
        for (auto header : this->headers)
        {
            std::string x = header.first + ": " + header.second;
            ::send(this->clientfd, x.c_str(), x.size(), 0);
            ::send(this->clientfd, "\r\n", 2, 0);
        }
        ::send(this->clientfd, "\r\n", 2, 0);
        ::send(this->clientfd, this->body.c_str(), this->body.size(), 0);
    }

public:
    std::string status;
    std::map<std::string, std::string> headers;
    std::string body;
    int clientfd;

    Response() {}
    Response(std::string status, std::map<std::string, std::string> headers, std::string body)
    {
        this->status = status;
        this->headers = headers;
        this->body = body;
    }

    void send(std::string body)
    {
        this->body = body;
        std::cout << "BODY ABOUT TO BEING SENT: " << this->body << std::endl;
        if (this->headers.find("Content-Type") == this->headers.end())
        {
            this->headers["Content-Type"] = "text/html";
        }
        if (this->headers.find("Content-Length") == this->headers.end())
        {
            this->headers["Content-Length"] = std::to_string(this->body.size());
        }

        this->send_response();
    }
};

class Server
{
public:
    int port;
    std::string directory;

    std::map<std::string, void (*)(Request, Response)> get_routes;
    std::map<std::string, void (*)(Request, Response)> post_routes;

    Server() {}

    void get(std::string path, void (*callback)(Request, Response))
    {
        get_routes[path] = callback;
    }

    void post(std::string path, void (*callback)(Request, Response))
    {
        post_routes[path] = callback;
    }

    void handle_request_loop(int clientfd, char *buf)
    {
        Request req;
        Response res;
        req.parse_request(buf);
        res.clientfd = clientfd;

        if (req.method == "GET")
        {
            if (get_routes.find(req.path) != get_routes.end())
            {
                res.status = "HTTP/1.1 200 OK";
                get_routes[req.path](req, res);
            }
            else
            {
                res.status = "HTTP/1.1 404 Not Found";
                res.send("");
            }
        }
        else if (req.method == "POST")
        {
            if (post_routes.find(req.path) != post_routes.end())
            {
                res.status = "HTTP/1.1 201 Created";
                post_routes[req.path](req, res);
            }
            else
            {
                res.status = "HTTP/1.1 404 Not Found";
                res.send("");
            }
        }
        close(clientfd);
    }

    void handle_client(int clientfd)
    {
        char req[1024];
        recv(clientfd, req, 1024, 0);
        handle_request_loop(clientfd, req);
    }

    // take port number and a callback as arguments
    void listen(int port, void (*callback)())
    {
        this->port = port;
        int server_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (server_fd < 0)
        {
            std::cerr << "Failed to create server socket\n";
            exit(1);
        }

        int reuse = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
        {
            std::cerr << "setsockopt failed\n";
            exit(1);
        }

        struct sockaddr_in server_addr;
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        if (::bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
        {
            std::cerr << "Failed to bind to port " << port << "\n";
            exit(1);
        }

        int connection_backlog = 5;
        if (::listen(server_fd, connection_backlog) != 0)
        {
            std::cerr << "listen failed\n";
            exit(1);
        }

        struct sockaddr_in client_addr;
        int client_addr_len = sizeof(client_addr);

        callback();

        while (1)
        {
            int clientfd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
            if (clientfd < 0)
            {
                std::cerr << "Failed to accept client connection\n";
                continue; // Handle the error and continue accepting new connections
            }
            std::thread client_thread(&Server::handle_client, this, clientfd);
            client_thread.detach(); // Detach the thread to allow it to run independently
        }
    }
};
#endif // HTTP_H
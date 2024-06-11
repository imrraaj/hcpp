#include <iostream>
#include <cstdlib>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <vector>
using namespace std;

vector<string> split(string s, string delimiter = " ")
{

  vector<string> result;
  size_t pos = 0;
  string token;
  while ((pos = s.find(delimiter)) != string::npos)
  {
    token = s.substr(0, pos);
    result.push_back(token);
    s.erase(0, pos + delimiter.length());
  }
  result.push_back(s);
  return result;
}

bool starts_with(string s, string prefix)
{
  return s.substr(0, prefix.size()) == prefix;
}

struct Request
{
  string method;
  string path;
  string version;
  vector<string> headers;
  string body;
};

struct Response
{
  string status;
  vector<string> headers;
  string body;
};

Request parse_request(char *buf)
{
  string req_str(buf);
  vector<string> headers_body_split = split(req_str, "\r\n\r\n");
  string body = headers_body_split.size() > 1 ? headers_body_split[1] : "";

  vector<string> req_headers = split(headers_body_split[0], "\r\n");
  vector<string> headers(req_headers.begin() + 1, req_headers.end());

  string status_line = req_headers[0];
  vector<string> status_line_split = split(status_line);
  string method = status_line_split[0];
  string path = status_line_split[1];
  string version = status_line_split[2];
  return Request{method, path, version, headers, body};
}

Response handle_request(Request req)
{
  Response res;
  if (req.path == "/")
  {
    res.status = "HTTP/1.1 200 OK";
    res.headers = {"Content-Type: text/plain", "Content-Length: 0"};
    res.body = "";
  }
  else if (starts_with(req.path, "/echo"))
  {
    string echo_param = "/echo/";
    string body = req.path.substr(echo_param.size());
    res.status = "HTTP/1.1 200 OK";
    res.headers = {"Content-Type: text/plain", "Content-Length: " + to_string(body.size())};
    res.body = body;
  }
  else if (starts_with(req.path, "/user-agent"))
  {
    res.status = "HTTP/1.1 200 OK";
    // find the user-agent header
    for (auto header : req.headers)
    {
      if (starts_with(header, "User-Agent: "))
      {
        cout << header << endl;
        const string user_agent_prefix = "User-Agent: ";
        res.body = header.substr(user_agent_prefix.size());
        res.headers = {"Content-Type: text/plain", "Content-Length: " + to_string(res.body.length())};
        break;
      }
    }
  }
  else
  {
    res.status = "HTTP/1.1 404 Not Found";
    res.headers = {"Content-Type: text/html", "Content-Length: 0"};
    res.body = "";
  }

  return res;
}

void send_response(int clientfd, Response res)
{

  // print the response

  cout << res.status << endl;
  for (auto header : res.headers)
  {
    cout << header << endl;
  }
  cout << res.body << endl;

  cout << "\n\n\n\n";

  send(clientfd, res.status.c_str(), res.status.size(), 0);
  send(clientfd, "\r\n", 2, 0);
  for (auto header : res.headers)
  {
    send(clientfd, header.c_str(), header.size(), 0);
    send(clientfd, "\r\n", 2, 0);
  }
  send(clientfd, "\r\n", 2, 0);
  send(clientfd, res.body.c_str(), res.body.size(), 0);
}

int main(int argc, char **argv)
{
  // Flush after every std::cout / std::cerr
  std::cout << std::unitbuf;
  std::cerr << std::unitbuf;

  // You can use print statements as follows for debugging, they'll be visible when running tests.
  // std::cout << "Logs from your program will appear here!\n";

  // Uncomment this block to pass the first stage
  //
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0)
  {
    std::cerr << "Failed to create server socket\n";
    return 1;
  }
  //
  // // Since the tester restarts your program quite often, setting SO_REUSEADDR
  // // ensures that we don't run into 'Address already in use' errors
  int reuse = 1;
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
  {
    std::cerr << "setsockopt failed\n";
    return 1;
  }
  //
  struct sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(4221);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
  {
    std::cerr << "Failed to bind to port 4221\n";
    return 1;
  }

  int connection_backlog = 5;
  if (listen(server_fd, connection_backlog) != 0)
  {
    std::cerr << "listen failed\n";
    return 1;
  }

  struct sockaddr_in client_addr;
  int client_addr_len = sizeof(client_addr);

  std::cout << "Waiting for a client to connect...\n";

  int clientfd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);

  char req[1024];
  recv(clientfd, req, 1024, 0);
  Request request = parse_request(req);

  // std::cout << "Status Line: " << status_line << "\n";

  // for (auto header : headers)
  // {
  //   std::cout << "Header: " << header << "\n";
  // }

  // std::cout << "Body: " << body << "\n";

  Response res = handle_request(request);
  send_response(clientfd, res);

  close(server_fd);

  return 0;
}

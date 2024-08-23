// #pragma once
// #include "hcpp.h"

// std::vector<std::string> split(std::string s, std::string delimiter = " ")
// {
// 	std::vector<std::string> result;
// 	size_t pos = 0;
// 	std::string token;
// 	while ((pos = s.find(delimiter)) != std::string::npos)
// 	{
// 		token = s.substr(0, pos);
// 		result.push_back(token);
// 		s.erase(0, pos + delimiter.length());
// 	}
// 	result.push_back(s);
// 	return result;
// }

// bool starts_with(std::string s, std::string prefix)
// {
// 	return s.substr(0, prefix.size()) == prefix;
// }

// std::string compress_gzip(std::string s)
// {
// 	z_stream zs;
// 	memset(&zs, 0, sizeof(zs));
// 	if (deflateInit2(&zs,
// 					 Z_BEST_COMPRESSION,
// 					 Z_DEFLATED,
// 					 MOD_GZIP_ZLIB_WINDOWSIZE + 16,
// 					 MOD_GZIP_ZLIB_CFACTOR,
// 					 Z_DEFAULT_STRATEGY) != Z_OK)
// 	{
// 		throw(std::runtime_error("deflateInit2 failed while compressing."));
// 	}

// 	zs.next_in = (Bytef *)s.data();
// 	zs.avail_in = s.size();

// 	int ret;
// 	char outbuffer[32768];
// 	std::std::string outstd::string;

// 	do
// 	{
// 		zs.next_out = reinterpret_cast<Bytef *>(outbuffer);
// 		zs.avail_out = sizeof(outbuffer);

// 		ret = deflate(&zs, Z_FINISH);

// 		if (outstd::string.size() < zs.total_out)
// 		{
// 			outstd::string.append(outbuffer,
// 							 zs.total_out - outstd::string.size());
// 		}
// 	} while (ret == Z_OK);

// 	deflateEnd(&zs);

// 	if (ret != Z_STREAM_END)
// 	{
// 		cout << "Exception during zlib compression: (" << ret << ") " << zs.msg;
// 	}
// 	return outstd::string;
// }

// struct Request
// {
// 	std::string method;
// 	std::string path;
// 	std::string version;
// 	map<std::string, std::string> headers;
// 	std::string body;
// };

// struct Response
// {
// 	std::string status;
// 	map<std::string, std::string> headers;
// 	std::string body;
// };

// std::string directory = "";

// Request parse_request(char *buf)
// {
// 	std::string req_str(buf);
// 	std::vector<std::string> headers_body_split = split(req_str, "\r\n\r\n");
// 	std::string body = headers_body_split.size() > 1 ? headers_body_split[1] : "";

// 	std::vector<std::string> req_headers = split(headers_body_split[0], "\r\n");
// 	std::vector<std::string> in_headers(req_headers.begin() + 1, req_headers.end());
// 	map<std::string, std::string> headers;

// 	for (auto header : in_headers)
// 	{
// 		std::vector<std::string> header_split = split(header, ": ");
// 		headers[header_split[0]] = header_split[1];
// 	}

// 	std::string status_line = req_headers[0];
// 	std::vector<std::string> status_line_split = split(status_line);
// 	std::string method = status_line_split[0];
// 	std::string path = status_line_split[1];
// 	std::string version = status_line_split[2];
// 	return Request{method, path, version, headers, body};
// }

// Response handle_request(Request req)
// {
// 	Response res;
// 	if (req.method == "GET")
// 	{
// 		if (req.path == "/")
// 		{
// 			res.status = "HTTP/1.1 200 OK";
// 			res.headers = {{"Content-Type", "text/html"},
// 						   {"Content-Length", "0"}};
// 			res.body = "";
// 		}
// 		else if (starts_with(req.path, "/echo"))
// 		{
// 			std::string echo_param = "/echo/";
// 			std::string body = req.path.substr(echo_param.size());
// 			std::vector<std::string> content_encoding = split(req.headers["Accept-Encoding"], ", ");

// 			bool is_valid_encoding = false;
// 			for (auto &x : content_encoding)
// 			{
// 				if (x == "gzip")
// 				{
// 					is_valid_encoding = true;
// 					res.body = compress_gzip(body);
// 					res.headers = {{"Content-Type", "text/plain"},
// 								   {"Content-Length", to_std::string(res.body.size())},
// 								   {"Content-Encoding", "gzip"}};
// 				}
// 			}
// 			if (!is_valid_encoding)
// 			{
// 				res.body = body;
// 				res.headers = {{"Content-Type", "text/plain"},
// 							   {"Content-Length", to_std::string(res.body.size())}};
// 			}
// 			res.status = "HTTP/1.1 200 OK";
// 		}
// 		else if (starts_with(req.path, "/user-agent"))
// 		{
// 			res.status = "HTTP/1.1 200 OK";
// 			res.headers = {{"Content-Type", "text/plain"},
// 						   {"Content-Length", to_std::string(req.headers["User-Agent"].size())}};
// 			res.body = req.headers["User-Agent"];
// 		}

// 		else if (starts_with(req.path, "/files"))
// 		{
// 			std::string filename_param = "/files/";
// 			std::string filename = req.path.substr(filename_param.size());

// 			std::string file_path = directory + filename;
// 			ifstream file(file_path);
// 			if (file.is_open())
// 			{
// 				std::string file_contents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
// 				res.status = "HTTP/1.1 200 OK";
// 				res.headers = {{"Content-Type", "application/octet-stream"},
// 							   {"Content-Length", to_std::string(file_contents.size())}};
// 				res.body = file_contents;
// 			}
// 			else
// 			{
// 				res.status = "HTTP/1.1 404 Not Found";
// 				res.headers = {{"Content-Type", "text/html"},
// 							   {"Content-Length", "0"}};
// 				res.body = "";
// 			}
// 			file.close();
// 		}
// 		else
// 		{
// 			res.status = "HTTP/1.1 404 Not Found";
// 			res.headers = {{"Content-Type", "text/html"},
// 						   {"Content-Length", "0"}};
// 			res.body = "";
// 		}
// 	}

// 	else if (req.method == "POST")
// 	{
// 		if (starts_with(req.path, "/files"))
// 		{
// 			std::string filename_param = "/files/";
// 			std::string filename = req.path.substr(filename_param.size());
// 			std::string content_length = req.headers["Content-Length"];

// 			std::string file_path = directory + filename;
// 			ofstream file(file_path);

// 			if (file.is_open())
// 			{
// 				file << req.body.substr(0, stoi(content_length));
// 				file.close();
// 				res.status = "HTTP/1.1 201 Created";
// 				res.headers = {{"Content-Type", "text/html"},
// 							   {"Content-Length", "0"}};
// 				res.body = "";
// 			}
// 			else
// 			{
// 				res.status = "HTTP/1.1 404 Not Found";
// 				res.headers = {{"Content-Type", "text/html"},
// 							   {"Content-Length", "0"}};
// 				res.body = "";
// 			}
// 		}
// 	}
// 	return res;
// }

// void send_response(int clientfd, Response res)
// {
// 	cout << res.status << endl;
// 	for (auto header : res.headers)
// 	{
// 		cout << header.first << ": " << header.second << endl;
// 	}
// 	cout << res.body << endl;

// 	cout << "\n\n\n\n";

// 	send(clientfd, res.status.c_str(), res.status.size(), 0);
// 	send(clientfd, "\r\n", 2, 0);
// 	for (auto header : res.headers)
// 	{
// 		std::string x = header.first + ": " + header.second;
// 		send(clientfd, x.c_str(), x.size(), 0);
// 		send(clientfd, "\r\n", 2, 0);
// 	}
// 	send(clientfd, "\r\n", 2, 0);
// 	send(clientfd, res.body.c_str(), res.body.size(), 0);
// }

// int main(int argc, char **argv)
// {
// 	for (int i = 0; i < argc; i++)
// 	{
// 		if (std::string(argv[i]) == "--directory")
// 		{
// 			if (i + 1 < argc)
// 			{
// 				directory = std::string(argv[i + 1]);
// 			}
// 			else
// 			{
// 				cout << "Please provide a directory path" << endl;
// 				return 1;
// 			}
// 		}
// 	}

// 	// Flush after every std::cout / std::cerr
// 	std::cout << std::unitbuf;
// 	std::cerr << std::unitbuf;

// 	// You can use print statements as follows for debugging, they'll be visible when running tests.
// 	// std::cout << "Logs from your program will appear here!\n";

// 	// Uncomment this block to pass the first stage
// 	//
// 	int server_fd = socket(AF_INET, SOCK_STREAM, 0);
// 	if (server_fd < 0)
// 	{
// 		std::cerr << "Failed to create server socket\n";
// 		return 1;
// 	}
// 	//
// 	// // Since the tester restarts your program quite often, setting SO_REUSEADDR
// 	// // ensures that we don't run into 'Address already in use' errors
// 	int reuse = 1;
// 	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
// 	{
// 		std::cerr << "setsockopt failed\n";
// 		return 1;
// 	}
// 	//
// 	struct sockaddr_in server_addr;
// 	server_addr.sin_family = AF_INET;
// 	server_addr.sin_addr.s_addr = INADDR_ANY;
// 	server_addr.sin_port = htons(4221);

// 	if (::bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
// 	{
// 		std::cerr << "Failed to bind to port 4221\n";
// 		return 1;
// 	}

// 	int connection_backlog = 5;
// 	if (listen(server_fd, connection_backlog) != 0)
// 	{
// 		std::cerr << "listen failed\n";
// 		return 1;
// 	}

// 	struct sockaddr_in client_addr;
// 	int client_addr_len = sizeof(client_addr);

// 	std::cout << "Waiting for a client to connect...\n";

// 	while (1)
// 	{
// 		int clientfd = accept(server_fd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_len);
// 		char req[1024];
// 		recv(clientfd, req, 1024, 0);
// 		Request request = parse_request(req);
// 		Response res = handle_request(request);
// 		send_response(clientfd, res);
// 	}
// 	close(server_fd);

// 	return 0;
// }


// #endif // HTTP_H
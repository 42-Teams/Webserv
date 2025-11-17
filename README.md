# Webserv - HTTP Server

A custom HTTP/1.1 web server built from scratch in C++ to handle multiple client connections efficiently.

## Project Overview

Webserv is a lightweight HTTP server implementation inspired by NGINX behavior. This project demonstrates low-level network programming concepts and HTTP protocol implementation without relying on external web server libraries.

## Key Features

- **HTTP/1.1 Protocol Support**: Full implementation of HTTP/1.1 specifications
- **Multiple HTTP Methods**: GET, POST, and DELETE request handling
- **Concurrent Connections**: Non-blocking I/O using select() for handling multiple simultaneous client connections
- **Static File Serving**: Host static websites with automatic MIME type detection
- **File Upload**: Support for file uploads via POST requests with chunked transfer encoding
- **CGI Support**: Execute CGI scripts (PHP, Python, etc.) for dynamic content
- **Custom Error Pages**: Configurable error pages for different HTTP status codes
- **Directory Listing**: Automatic index generation for directories when enabled
- **Configuration System**: TOML-based configuration files for flexible server setup
- **Keep-Alive Connections**: Persistent HTTP connections support
- **Connection Timeout**: Automatic cleanup of inactive connections after 20 seconds

## Technical Details

- **Language**: C++11
- **I/O Multiplexing**: select() system call
- **Non-blocking Sockets**: Using fcntl() with O_NONBLOCK flag
- **Compilation**: Custom Makefile with strict compiler flags (-Wall -Werror -Wextra)
- **Architecture**: Event-driven server design with ServerManager class

## Project Structure

```
.
├── main.cpp                 # Entry point
├── ServerManager.cpp        # Core server loop and connection management
├── server_utils.cpp         # Socket setup and utility functions
├── webserv.hpp             # Main header file
├── Request.hpp/cpp         # HTTP request parsing
├── Response.hpp/cpp        # HTTP response generation
├── Cgi.hpp/cpp            # CGI execution handling
├── parsing/               # Configuration file parsing
├── conf/                  # Server configuration files
├── cgi-bin/              # CGI executables
└── webser_default/       # Default static website files
```

## Building the Project

```bash
make
```

This will compile the project with the following flags:
- `-Wall -Werror -Wextra`: Strict error checking
- `-std=c++11`: C++11 standard
- `-fsanitize=address`: Memory leak detection

To clean build files:
```bash
make clean    # Remove object files
make fclean   # Remove object files and executable
make re       # Clean rebuild
```

## Running the Server

Start the server with the default configuration:
```bash
./webserv
```

Or specify a custom configuration file:
```bash
./webserv path/to/config.toml
```

The default configuration runs on:
- Host: 127.0.0.1
- Port: 1024
- Root directory: ./webser_default/

## Configuration

The server uses TOML format for configuration. Example configuration:

```toml
[server]
host = 127.0.0.1
server_name = localhost
port = 1024
root = ./webser_default/
index = index.html

[location]
name = /
root = ./webser_default/
```

### Configuration Options

**Server Block:**
- `host`: IP address to bind to
- `server_name`: Server name for virtual hosting
- `port`: Port number(s) to listen on
- `root`: Root directory for serving files
- `index`: Default index file
- `body_size`: Maximum request body size
- `auto_index`: Enable/disable directory listing

**Location Block:**
- `name`: URL path pattern
- `root`: Directory for this location
- `methods`: Allowed HTTP methods (GET, POST, DELETE)
- `upload_path`: Directory for file uploads
- `upload_enable`: Enable/disable file uploads
- `cgi`: CGI script handlers (extension to interpreter mapping)
- `auto_index`: Enable directory listing
- `return`: HTTP redirection

## Features in Detail

### HTTP Methods

**GET**: Retrieve resources (files, directories, CGI output)
**POST**: Upload files, submit forms, execute CGI scripts
**DELETE**: Remove files and directories

### Non-blocking I/O

The server uses select() to monitor multiple file descriptors:
- Maintains separate read and write sets for client sockets
- Handles new connections without blocking existing ones
- Processes data as it becomes available

### CGI Execution

The server can execute CGI scripts to generate dynamic content:
- Supports PHP, Python, and other interpreters
- Passes HTTP headers as environment variables
- Handles POST data and query strings
- Captures and returns script output

### File Uploads

Supports two upload methods:
- **Content-Length**: Fixed-size uploads
- **Chunked Transfer Encoding**: Variable-size uploads

### Error Handling

Custom error pages for common HTTP status codes:
- 404 Not Found
- 500 Internal Server Error
- 403 Forbidden
- 405 Method Not Allowed
- And more

## Development Timeline

- **Start Date**: December 2023
- **Completion Date**: February 2024
- **Development Period**: Winter 2024 (3 months)

## Team Collaboration

This was a team project demonstrating:
- Collaborative C++ development
- Code review and integration
- Socket programming and network concepts
- HTTP protocol implementation
- System-level programming skills

## Technologies Used

- C++11
- POSIX Sockets API
- select() for I/O multiplexing
- Non-blocking I/O
- CGI (Common Gateway Interface)
- HTTP/1.1 Protocol
- TOML configuration parsing

## Testing

The server can be tested with:
- Web browsers (Chrome, Firefox, Safari)
- Command-line tools (curl, wget)
- HTTP testing tools (Postman)
- Siege or ab for load testing

Example curl commands:
```bash
# GET request
curl http://localhost:1024/

# POST file upload
curl -X POST -F "file=@test.txt" http://localhost:1024/upload

# DELETE request
curl -X DELETE http://localhost:1024/file.txt
```

## Limitations

- Not intended for production use
- Limited to HTTP/1.1 (no HTTP/2 or HTTP/3)
- No HTTPS/TLS support
- No compression support
- Basic authentication not implemented

## Learning Outcomes

This project provided hands-on experience with:
- TCP/IP networking fundamentals
- HTTP protocol details and implementation
- Socket programming in C++
- Event-driven architecture design
- Concurrent connection handling
- Configuration file parsing
- Process management for CGI
- Memory management and resource cleanup

## License

Educational project - 42 School curriculum

## Authors

- ael-maar
- mkhairou
- azaghlou

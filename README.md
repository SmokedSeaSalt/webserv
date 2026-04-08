*This project has been created as part of the 42 curriculum by egrisel, mvan-rij.*

## Description

This project is a HTTP/1.1 webserver written in c++23.
It has support for the following CGI types: xx, xx, xx.
It supports the GET, POST, and DELETE methods.

The following things can be configured:
- Interface:port pair
- Default error page
- Request body maximum allowed size
- Accepted HTTP methods per route
- HTTP redirection
- Default file location
- Enable/disable directory listing
- Default file when resource is a directory
- Enable/disable clients uploading files
- Clients uploading file location
- Supported CGI types

## Instructions

- ```git clone```
- ```cd webserv```
- ```make```
- ```./webserv [config_file]```

## Resources
- https://www.rfc-editor.org/rfc/rfc9112.html
- https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/
- https://beej.us/guide/bgnet/html/
- https://towardsdev.com/cpp-23-std-expected-tutorial-modern-error-handling-guide-2026-9494ebb44e81
- send(): https://cplusplus.com/forum/general/266899/
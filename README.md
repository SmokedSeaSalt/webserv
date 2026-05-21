*This project has been created as part of the 42 curriculum by egrisel, mvan-rij.*

## Description

This project is a HTTP/1.1 webserver written in c++23.
It has support for the following CGI types: xx, xx, xx.
It supports the GET, HEAD, POST, and DELETE methods.
It supports mutliple client connections and is non blocking with the use of epoll()

The following things can be configured in the config file:
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
- ```./webserv <config_file>```
- ```./webserv -h``` to check out additional running options

TODO check rules for duplicates and upload store things
### config file layout
- If you want to custmize your own .conf file, follow the same structure as the example.conf provided. All options are present in the example.conf file.
- Locations with a return (redirect) must only contain return.
- Locations with 'upload_store' must contain only an 'upload_store' and a 'methods' with POST
- Duplicate location path prefixes are invalid
- duplicate server listens are invalid

## Resources
- HTTP 1.1: https://www.rfc-editor.org/rfc/rfc9112.html
- HTTP semantics: https://www.rfc-editor.org/rfc/rfc9110.html
- URI syntax: https://www.rfc-editor.org/rfc/rfc3986.html
- CGI rfc: https://datatracker.ietf.org/doc/html/rfc3875
- https://www.geeksforgeeks.org/cpp/socket-programming-in-cpp/
- https://beej.us/guide/bgnet/html/
- https://towardsdev.com/cpp-23-std-expected-tutorial-modern-error-handling-guide-2026-9494ebb44e81
- send(): https://cplusplus.com/forum/general/266899/

## AI usage
- Test writing
- Explaining/finding information in documentation
- Explaining functions when documentation is vague or insufficient
- Help track down bugs
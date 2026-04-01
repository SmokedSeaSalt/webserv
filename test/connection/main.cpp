#include <Server.hpp>

int main()
{
	Server server("example.txt");
	server.setup();
	server.connection_loop();
	return 0;
}

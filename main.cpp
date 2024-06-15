#include "server.h"

int main(int argc, char const *argv[])
{
  if(argc <= 1)
  {
    std::cout << "port" <<std::endl;
    return 0;
  } 
  unsigned short port = atoi (argv [1]);

  asio::io_context context;
  Server server {context, port};
  server.accept();
  context.run();
}
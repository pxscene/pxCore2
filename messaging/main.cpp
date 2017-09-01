#include "rtIpAddress.h"
#include "rtSocket.h"

#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[])
{

#if 0
  rtTcpListener listener;
  rtError e = listener.start();
  std::cout << "start:" << rtStrError(e) << std::endl;
  std::cout << "local:" << listener.localEndPoint().toString() << std::endl;

  while (true)
  {
    rtTcpClient* client = listener.acceptClient();
  }
#endif
  char req[] = { "GET /index.html\r\n\r\n" };

  rtSocket* soc = new rtSocket();
  rtError e = soc->connect("172.217.12.196", 80);

  rtTcpClient client(soc);
  std::cout << "connect:" << rtStrError(e) << std::endl;
  std::cout << "remote :" << client.remoteEndPoint().toString() << std::endl;
  std::cout << "local  :" << client.localEndPoint().toString() << std::endl;
  client.send(req, sizeof(req));

  char buff[64];
}


int
TestOne(char* argv[])
{
  rtError  err;
  rtSocket soc;

  int port = strtol(argv[1], NULL, 10);

  // rtError e = soc.connect(rtIpEndPoint(rtIpAddress::fromString("172.217.12.196"), 80));
  // std::cout << "connect:" << rtStrError(e) << std::endl;
  err = soc.bind(rtIpEndPoint(rtIpAddress::fromString("127.0.0.1"), port));
  std::cout << "bind:" << rtStrError(err) << std::endl;

  err = soc.listen();
  std::cout << "listen:" << rtStrError(err) << std::endl;


  #if 0
  rtSocket s2 = soc.accept();
  err = rtErrorGetLastError();
  std::cout << "accept:" << rtStrError(err) << std::endl;
  std::cout << "remote:" << s2.remoteEndPoint().toString() << std::endl;
  std::cout << "local :" << s2.localEndPoint().toString() << std::endl;
  #endif

  return 0;
}

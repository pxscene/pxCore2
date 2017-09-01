#include "rtIpAddress.h"
#include "rtSocket.h"

#include <iostream>
#include <stdlib.h>

int main(int argc, char* argv[])
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

 
  rtSocket s2 = soc.accept();
  err = rtErrorGetLastError();
  std::cout << "accept:" << rtStrError(err) << std::endl;
  std::cout << "remote:" << s2.remoteEndPoint().toString() << std::endl;
  std::cout << "local :" << s2.localEndPoint().toString() << std::endl;
  return 0;
}

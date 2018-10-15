#ifndef BUFIO_H
#define BUFIO_H

#include <sys/types.h>
#include "def.h"

#ifdef WINDOWS
#include <Winsock2.h>
#endif

class BufIo
{
 private:
  char *b;
  size_t p;
  size_t n;

  int f_socket_remote_closed;

  ssize_t _realloc_buffer();
  ssize_t _SEND(SOCKET socket,char *buf,size_t len);
  ssize_t _RECV(SOCKET socket,char *buf,size_t len);
  
 public:
  BufIo();
  ~BufIo() { if(b){ delete []b; b = (char*) 0;} }


  void Reset(){ p = 0; f_socket_remote_closed = 0;}

  void Close(){
    if( b ){ delete []b; b = (char*) 0; }
    p = n = 0;
  }

  size_t Count() const { return p; } //�����������ֽ���
  size_t LeftSize() const { return (n - p); }

  ssize_t PickUp(size_t len); //�Ƴ�������ǰlen���ֽ�

  ssize_t FeedIn(SOCKET sk); //��sk�����ݵ�����ֱ����ʱ�����ݿɶ��򻺳�����
  ssize_t FlushOut(SOCKET sk); //������������д��socket
  ssize_t Put(SOCKET sk,char *buf,size_t len); //��buf������ӵ�����
  ssize_t PutFlush(SOCKET sk,char *buf,size_t len);

  char *BasePointer(){ return b; }
  char *CurrentPointer() { return ( b + p); }
};

#endif

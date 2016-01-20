#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <strings.h>

#include "comm.h"




int  saferead(int sd, unsigned char buf[], int len) {
  unsigned char *p = buf;
  int r = 0;
  while (r < len) {
    int l = read(sd, p, len);
    if (l < 0) {
      perror("read");
      exit(1);
    }
    if (l == 0)
      return r;
    r += l;
    p += l;
  }
  return r;
}

int  safewrite(int sd, unsigned char buf[], int len) {
  unsigned char *p = buf;
  int r = 0;
  while (r < len) {
    int l = write(sd, p, len);
    if (l < 0) {
      perror("read");
      exit(1);
    }
    r += l;
    p += l;
  }
  return r;
}


int createClient(char *ip, int port) {
  int sd = socket(AF_INET, SOCK_STREAM, 0);
  if (sd < 0) {
    perror("socket");
    exit(1);
  }
  struct sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);
  inet_aton(ip, &sin.sin_addr);
  if (connect(sd, (struct sockaddr *)&sin, sizeof(sin)) < 0) {
    perror("connect");
    exit(1);
  }
  return sd;
}


int comm(int sd, unsigned char dat) {
  unsigned char buf[BLOCKSIZE];
  static int first=1;


  bzero(buf, BLOCKSIZE);
  if (!first)
    saferead(sd, buf, BLOCKSIZE);
  first = 0;
  safewrite(sd, buf, BLOCKSIZE);
}


/*
int startSlave(int fds[2]) {
  int dfds[2];
  if (pipe(dfds) < 0) {
    perror("pipe");
    exit(1);
  }
  int ufds[2];
  if (pipe(ufds) < 0) {
    perror("pipe");
    exit(1);
  }
  int pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }
  if (pid == 0) {
    close(dfds[1]);
    close(ufds[0]);
    dup2(dfds[0], 0);
    //if (dfds[0] != 0)
      //close(dfds[0]);
    dup2(ufds[1], 1);
    //if (ufds[1] != 1)
      //close(ufds[1]);
    execl("./gpg", "gpg", "-d", "-o", "aaa", "fout", NULL);
    perror("execl");
    exit(1);
  }
  //close(dfds[0]);
  //close(ufds[1]);
  fds[0] = dfds[0];
  fds[1] = ufds[1];
  return pid;
}
*/

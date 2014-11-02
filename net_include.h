#include <stdio.h>

#include <stdlib.h>

#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
#include <errno.h>

#define PORT	     10080 /* assigned address */
#define FCC          1
#define PACKET_SIZE  1200
#define MAX_MESS_LEN 1500
#define ARRAY_SIZE   16384
#define MAX_RTR      250

/* Initializer variables */
struct packet_structure {
  int type;
  int sequence;
  int received;
  int machine_index;
  int packet_index;
  int random_number;
  char data[PACKET_SIZE];
};

struct initializers {
  /* Sequence value of the most recent packet written to log */
  int sequence;
  int packets_to_send;

  int packet_index;
  int total_machines;
  int debug;
  int max_packets;
  char group[80];
  
  struct packet_structure *packet;
};

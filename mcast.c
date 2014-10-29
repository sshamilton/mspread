/* This is our implementation of the reliable multicast protocol  */

#include "sp.h"
#include "net_include.h"
#include "unistd.h"
#include "stdlib.h"

static	char	User[80];
static  char    Spread_name[80];
static  char    Private_group[MAX_GROUP_NAME];
static  mailbox Mbox;
static	int	    Num_sent;
static	unsigned int	Previous_len;
static  int     To_exit = 0;
void setup(struct initializers *i) {
  /* Sets up all ports */
  /* and waits for the start_mcast message to start the actual process */
   int              mcast_addr;
   int              start = 0;
   int              bytes;
   int              num, c;
   struct		timeval timeout;
    unsigned char      ttl_val;
   char logfilename[10];
   snprintf(logfilename, 10, "%d.out", i->machine_index);
   i->written_seq = -1;
   i->logfile = fopen(logfilename, "w");

};



int write_log(struct initializers *i) {
  /* writes to log for all received data */
     fprintf(i->logfile, "%2d, %8d, %8d\n", i->packet->machine_index, 
					    i->packet->sequence, 
					    i->packet->random_number);
}

void printpacket (struct packet_structure *p) {
  printf("Machine id: %d, Seq: %d, Rand: %di\n", p->machine_index, p->sequence, p->random_number);
}

struct packet_structure *generate_packet(struct initializers *i){
  /* Generates the next packet, and */
  int r = rand() % 1000000 + 1;
  struct packet_structure *p=malloc(sizeof(struct packet_structure));

  p->received=0; /* Packet sent is set to 0, so receiving machine can update */
  p->machine_index = i->machine_index;
  p->type = 1; /*packet data type */
  p->random_number=r;
  return p;
}
void receive_packet(struct initializers *i) {
  /* receiving data */
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  p = (struct packet_structure *)i->mess_buf;
  
}

void send_data(struct initializers *i){

}

int parseargs(int argc, char **argv, struct initializers *i)
{
    char               *at; /* position of @ symbol in 3rd arg */
    char               *compname; /* remote computer name */
    /*Ensure we got the right number of arguments */
    if(argc !=4) {
  
      printf("Usage: mcast <num_of_packets> <machine_index> <number of machines>\n");
        exit(0);
    }
    else {
        i->packets_to_send = atoi(argv[1]);
        i->machine_index = atoi(argv[2]);
        i->total_machines = atoi(argv[3]);
        return 1;
    }
}

static  void	Bye();
/* Message types: */
/* 1 = Data */
/* 3 = Machine id message */
/* 4 = Ready to begin message */
int main(int argc, char **argv)
{
	/* Variables */
  unsigned char      ttl_val;
  int                c, ret;
  strcpy(Private_group, "shamil22");

  struct timeval    timeout, start_time, end_time;;
  struct initializers *i=malloc(sizeof(struct initializers));
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  double time1, time2;
  sp_time test_timeout;
  i->debug = 0; /*Turn on for testing */

  parseargs(argc, argv, i);
  time1=start_time.tv_sec+(start_time.tv_usec/1000000.0); 
  i->packet_index = 0;
  setup(i); /*Setup ports and wait for start process */
  i->max_packets = FCC*6;
  gettimeofday(&start_time, NULL);
  time1=start_time.tv_sec+(start_time.tv_usec/1000000.0);
  ret = SP_connect_timeout( Spread_name, User, 0, 1, &Mbox, Private_group, test_timeout );
	if( ret != ACCEPT_SESSION ) 
	{
	  SP_error( ret );
	  Bye();
	}
   printf("User: connected to %s with private group %s\n", Spread_name, Private_group );
  E_init();
  while(0) {
    
    timeout.tv_sec = 0;
    timeout.tv_usec = 10000;
  }
  Bye();
  return (0);
}

static  void	Bye()
{
	To_exit = 1;

	printf("\nBye.\n");

	SP_disconnect( Mbox );

	exit( 0 );
}


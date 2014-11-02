/* Multicast tool utilizing the Spread toolkit  */
#define MAX_MEMBERS     100

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
static  int     sequence = 0;
static  void	Bye()
{
	To_exit = 1;
	printf("\nBye.\n");
	SP_disconnect( Mbox );
	exit( 0 );
}
static  int     packets_to_send;
static  int     machine_index;
static  int     total_machines;
static  FILE *logfile;
static  int completed [10];

void setup(struct initializers *i) {
  /* Sets up all ports */
  /* and waits for the start_mcast message to start the actual process */
   int              mcast_addr;
   int              start = 0;
   int              bytes;
   int              num, c, num_groups, r, ret;
   int              service_type = 0;
   int              responded=0;
   int16            mess_type;
   int              endian_mismatch=0;
   int				response[10];   
   struct		    timeval timeout;
   unsigned char      ttl_val;
   char             target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
   char             logfilename[10];
   char             sender[MAX_GROUP_NAME];
   char             groups[10][MAX_GROUP_NAME];
   char mess_buf[MAX_MESS_LEN];
   i->sequence = 0;
   sp_time test_timeout;
   struct packet_structure *p=malloc(sizeof(struct packet_structure));
   snprintf(logfilename, 10, "%d.out", machine_index);
    
   logfile = fopen(logfilename, "w");
   ret = SP_connect_timeout( Spread_name, User, 0, 1, &Mbox, Private_group, test_timeout );
   if( ret != ACCEPT_SESSION ) 
   {
	 SP_error( ret );
	 Bye();
   }
   printf("User: connected to %s with private group %s\n", Spread_name, Private_group );
   E_init();
   ret = SP_join(Mbox, i->group);
   if (machine_index == 1) {
    for (c=1; c <= i->total_machines; c++) {
       response[c] = 0;
       completed[c] = 0;
    }
     /* Collect up the users, and send start message when everyone is in the group */
    response[1]=1;
    while (responded < 1) {
 
	  ret = SP_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups, 
                &mess_type, &endian_mismatch, sizeof(mess_buf), mess_buf );
      p = (struct packet_structure *)mess_buf;
      if (p->type == 4) {
          printf("ret = %d Got machine id %d\n", ret, p->machine_index);
          /* Add this machine to the array and check to see if we are done */
          response[p->machine_index] = 1; 
          //if (i->debug) printf("Got response from %d\n", p->machine_index);
          r = 1;
          for (c=1; c <= i->total_machines; c++) {
                if (response[c] == 0) r =0; 
                }
          if (r==1) responded = 1;
      }
      else if (p->type == 3 && machine_index == 1)
      {
        completed[p->machine_index] = 1;
      }
    }
  /* Send start sending message to everyone */
    p->type=  2;
    ret= SP_multicast( Mbox, AGREED_MESS, i->group, 1, sizeof(struct packet_structure), (char *)p );
  }
  else {
    /*Send ready to begin message */
    p->type = 4;
    p->machine_index = machine_index;    
    
    ret= SP_multicast( Mbox, AGREED_MESS, i->group, 1, sizeof(struct packet_structure), (char *)p );
    printf("Join=%d, group %s\n", ret, i->group);
    if( ret < 0 ) 
    {
            SP_error( ret );
            Bye();
    } 
    else
    {
      printf("Sent: %d\n", ret);
     }
  }

};



int write_log(struct packet_structure *packet) {
  /* writes to log for all received data */
     fprintf(logfile, "%2d, %8d, %8d\n", packet->machine_index, 
					    packet->sequence, 
					    packet->random_number);
}

void printpacket (struct packet_structure *p) {
  printf("Machine id: %d, Seq: %d, Rand: %di\n", p->machine_index, p->sequence, p->random_number);
}

struct packet_structure *generate_packet(){
  /* Generates the next packet, and */
  int r = rand() % 1000000 + 1;
  struct packet_structure *p=malloc(sizeof(struct packet_structure));

  p->received=0; /* Packet sent is set to 0, so receiving machine can update */
  p->machine_index = machine_index;
  p->type = 1; /*packet data type */
  p->sequence = sequence;
  sequence++;
  p->random_number=r;
  return p;
}
void receive_packet() {
  /* receiving data */
  printf("Called receive\n");
  int c, ret, num_groups;
  int r=1;
  int16            mess_type;
  int              endian_mismatch=0;
  char mess_buf[MAX_MESS_LEN];
  char             target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
  char             sender[MAX_GROUP_NAME];
  int              service_type = 0;
  char              group[80];
  struct packet_structure *packet;
  SP_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups, 
                &mess_type, &endian_mismatch, sizeof(mess_buf), mess_buf );
  packet = (struct packet_structure *)mess_buf;
  if (packet->type == 1) /*Data packet, write to log */
  {
    write_log(packet);
  } 
  else if (packet->type == 3 && machine_index == 1)
  {  printf("Checking for termination \n");
    completed[packet->machine_index] = 1;
    for (c=1; c <= total_machines; c++) {
      if (completed[c] == 0) r =0; 
      printf("%d = %d\n", c, completed[c]);
    }
    if (r==1) {
    /*All machines complete.  Send termination */
      packet->type = 5; printf("Complete sending termination\n");
      ret= SP_multicast( Mbox, AGREED_MESS, group, 1, sizeof(struct packet_structure), (char *)packet );
    }
  }
  else
  {
    printf("Got packet type %d, mid=%d\n", packet->type, packet->machine_index);
    
  }
}

void send_data(){
  int sp = FCC;
  int c, ret, num_groups;
  int16 mess_type;
  int endian_mismatch=0;
  char              group[80];
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  if (packets_to_send < FCC) sp = packets_to_send;
  printf("Sending data\n");
  for (c=1; c <= sp; c++)
  {
     p = generate_packet();
     ret= SP_multicast( Mbox, AGREED_MESS, group, 1, sizeof(struct packet_structure), (char *)p );
       
  }
  packets_to_send = packets_to_send - sp;
  
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
        packets_to_send = atoi(argv[1]);
        machine_index = atoi(argv[2]);
        total_machines = atoi(argv[3]);
        return 1;
    }
}


/* Message types: */
/* 1 = Data */
/* 2 = Start sending message */
/* 3 = Complete */
/* 4 = Ready to begin message */
/* 5 = Terminate */
int main(int argc, char **argv)
{
	/* Variables */
  unsigned char      ttl_val;
  int                c, ret, num_groups;
  int complete = 0;
  int				response[10];
  char             sender[MAX_GROUP_NAME];
  int              service_type = 0;
  int               rts = 0;
  char mess_buf[MAX_MESS_LEN];
  char              group[80];
  struct timeval    timeout, start_time, end_time;
  struct initializers *i=malloc(sizeof(struct initializers));
  struct packet_structure *p=malloc(sizeof(struct packet_structure));
  double starttime1, starttime2;
  sp_time test_timeout;
  int16            mess_type;
  int              endian_mismatch=0;
  char             target_groups[MAX_MEMBERS][MAX_GROUP_NAME];
  i->debug = 0; /*Turn on for testing */
  strcpy(i->group, "shamil22");
  parseargs(argc, argv, i);
  i->packet_index = 0;
  setup(i); /*Setup ports and wait for start process */
  i->max_packets = 1;
     
  printf("Waiting to start.\n");
  while(!rts) { 
    ret = SP_receive( Mbox, &service_type, sender, 100, &num_groups, target_groups, 
                &mess_type, &endian_mismatch, sizeof(mess_buf), mess_buf );
    if (ret > 0) {
      i->packet = (struct packet_structure *)mess_buf; 
      if (i->packet->type == 2) {
         rts =1;
      }
    }
  }
  printf("Begin!\n");
  gettimeofday(&start_time, NULL);          
  starttime1=start_time.tv_sec+(start_time.tv_usec/1000000.0);
  E_attach_fd( Mbox, READ_FD, send_data, 0, NULL, HIGH_PRIORITY );
  E_attach_fd( Mbox, READ_FD, receive_packet, 0, NULL, HIGH_PRIORITY );
  printf("Handling events.\n");
  E_handle_events();


  if (i->packets_to_send == 0) /*we completed */
  {
    printf("Finished sending\n");
    p->type=3;
    p->machine_index = machine_index;
    ret= SP_multicast( Mbox, AGREED_MESS, group, 1, sizeof(struct packet_structure), (char *)p );
    i->packets_to_send = -1;
  }    
  
  printf("Complete\n");
  gettimeofday(&end_time, NULL);          
  starttime2=end_time.tv_sec+(end_time.tv_usec/1000000.0);
  printf("%.6lf seconds elapsed\n", starttime2-starttime1);
  fclose(logfile);
  Bye();
  return (0);
}



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/*******************************************************************************
 * void getusername(char*)
 * 
 * Gets the user's name from standard input
 * Args: an empty char array of 10 elements
 ******************************************************************************/
void getusername(char * input)
{
  printf("Please enter a 10 character usename: ");
  scanf("%s", input);
}
/*******************************************************************************
 * struct addrinfo * create_address_info(char*, char*)
 * 
 * creates a pointer to an address info linked list with a address and port
 * Args: two strings: the address and port number
 * Returns: An address info linked list
 ******************************************************************************/
struct addrinfo * create_address_info(char * input_addr, char * port)
{
  // create status indicator and structures for address info
  int status;
  struct addrinfo hints, *res;

  // clear out all fields of hints and then set them up to use IPv4 TCP
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;

  // if the status indicator is not 0, we have an error, print it
  // otherwise, return the address information
  if((status = getaddrinfo(input_addr, port, &hints, &res)) != 0){
    fprintf(stderr,
        "getaddrinfo error: %s\nDid you enter the correct IP/Port?\n",
        gai_strerror(status));
    exit(1);
  }
  
  return res;
}

/*******************************************************************************
 * int create_socket(struct addrinfo *)
 * 
 * Creates a socket from an address info linked list
 * Args: The address info linked list
 * Returns: a socket file descriptor
 ******************************************************************************/
int create_socket(struct addrinfo * res){
  int sockfd;
  // if the socket file descriptor is -1, exit, otherwise return it
  if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1){
    fprintf(stderr, "Error in creating socket\n");
    exit(1);
  }
  return sockfd;
}

/*******************************************************************************
 * void connect_socket(int, struct addrinfo *)
 * 
 * Connects the socket to the address specified in the address info linked list
 * Args: a socket file descriptor and an address info linked list
 ******************************************************************************/
void connect_socket(int sockfd, struct addrinfo * res){
  int status;
  // if the status indicator is -1, we were unable to connect
  // otherwise connect the socket to the address information
  if ((status = connect(sockfd, res->ai_addr, res->ai_addrlen)) == -1){
    fprintf(stderr, "Error in connecting socket\n");
    exit(1);
  }
}

/*******************************************************************************
 * void chat(int, char*, char*)
 * 
 * Initiates a chat between two hosts connected by a socket. The client (this
 * executable) initiates the converstion.
 * Args: a socket file descriptor, a username and a peer's name
 ******************************************************************************/
void chat(int sockfd, char * username, char * servername){
  // create buffers for input and output
  char input[503];
  char output[501];
  // clear out those buffers 
  memset(input,0,sizeof(input));
  memset(output,0,sizeof(output));
  // initialize error detectors
  int num_bytes_sent = 0;
  int status;
  // clear out newlines from standard input
  fgets(input, 500, stdin);
  while(1){
    // grab the input from the user
    printf("%s> ", username);
    fgets(input, 502, stdin);
    // if the string is \quit, we close the connection
    if (strcmp(input, "\\quit\n") == 0){
      break;
    }
    // else, we send information to the peer
    num_bytes_sent = send(sockfd, input, strlen(input) ,0);
    // if there was an error, exit
    if(num_bytes_sent == -1){
        fprintf(stderr, "Error when sendind data to host\n");
        exit(1);
    }
    // grab the message from the peer
    status = recv(sockfd, output, 500, 0);
    // if there was an error receiving, exit
    if (status == -1){
      fprintf(stderr, "Error when receiving data from host\n");
      exit(1);
    }
    else if (status == 0){ // the server closed the connection
      printf("Connection closed by server\n");
      break;
    }
    else{ // the message was ok, print it
      printf("%s> %s\n", servername, output);
    }
    // clear out the input and output buffers for the next iteration
    memset(input,0,sizeof(input));
    memset(output,0,sizeof(output));
  }
  // if we break, we close the connection
  close(sockfd);
  printf("Closed Connection\n");
}

/*******************************************************************************
 * void handshake(int, char*, char*)
 * 
 * intiates an exhange of information between the client and the server. Allows
 * the server to have the client's name and for the client to have the server's
 * name
 * Args: the user's name and an empty array to store the server's name
 ******************************************************************************/
void handshake(int sockfd, char * username, char * servername){
  // send our username to the peer
  int sendingcode = send(sockfd, username, strlen(username), 0);
  // grab the username of our peer and place into servername
  int receivingcode = recv(sockfd, servername, 10, 0);
}

/*******************************************************************************
 * int main(int, char*)
 * 
 * main method. must be run with command line arguments:
 *         '<executablename> <hostname> <portnumber>'
 ******************************************************************************/
int main(int argc, char *argv[]){
  // if we have too few or too many args, exit and print error
  if(argc != 3){
    fprintf(stderr, "Invalid number of arguments\n");
    exit(1);
  }
  // create a username buffer and get the username from user
  char uname[10];
  getusername(uname);
  // create address information from the arguments passed in by
  // user
  struct addrinfo * res = create_address_info(argv[1], argv[2]);
  // create a socket from address information
  int sockfd = create_socket(res);
  // connect the socket and the address information
  connect_socket(sockfd, res);
  // create a peername buffer and exhange username and peername
  // with peer
  char sname[10];
  handshake(sockfd, uname, sname);
  // chat with peer
  chat(sockfd, uname, sname);
  // free up the space from address information
  freeaddrinfo(res);
}

// By Kianoush Ranjbar


#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <math.h> // needed for sqrt() and pow() functions


#define STDIN 0
#define CONFIG_FILE "config.file"
#define BUF_SIZE 1024


int global_location; // myLocation
int sendersLoc; 



int main(int argc, char *argv[])
{
  int sd, newsd; /* socket descriptor */
  int i;  /* loop variable */
  struct sockaddr_in server_address; /* my address */
  struct sockaddr_in from_address;  /* address of sender */
  char bufferReceived[1000]; // used in recvfrom
  char serverIP[20]; 
  int portNumber; // get this from command line
  int configPortNumber, location = 0; 
  int rc; // always need to check return codes!
  socklen_t fromLength;
  int flags = 0; // used for recvfrom
  fd_set socketFDS; // the socket descriptor set
  int maxSD; // tells the OS how many sockets are set
  

   int rows; // rows = 4 for test
   int cols; // cols = 3 for test
   
   printf("Enter numbers of rows: ");
   scanf("%d", &rows);
   getchar();      // so it doesnt interfere with select function

   printf("Enter number of columns: ");
   scanf("%d", &cols);
   getchar();

   
    
   printf("There are %d rows and %d columns for the grid\n", rows, cols);
 
   int grid[rows][cols];

    // Initialize the grid with sequential numbers
    int num = 1;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            grid[i][j] = num;
            num++;
        }
    }
  
        
    
  //--------------------------------------------------------------------
    // display config file stuff
    
        FILE *config_file = fopen(CONFIG_FILE, "r");   // if config file is specified as parameter, open it
    if (config_file == NULL) {
        perror("Error opening config file");
        exit(EXIT_FAILURE);
    }
  
	
	    // first display my location 
    while (fscanf(config_file, "%s %d %d", serverIP, &configPortNumber, &location) == 3) {
        
        char str[10];
        sprintf(str, "%d", configPortNumber);
        
        if (strstr(str, argv[1])){
            
        printf("My Location: %d\n", location);
	global_location = location;      // save my location in a global variable so we can include in the buffer message

        }
    }
    
        rewind(config_file);

	
    // then print contents of the file
    while (fscanf(config_file, "%s %d %d", serverIP, &portNumber, &location) == 3){
				printf("IPaddresses '%s', port '%d', location '%d'\n", serverIP, portNumber, location );
	}
		
	 fclose(config_file);    // close the config file
  
  //---------------------------------------------------------------------
  /* first, decide if we have the right number of parameters */
  if (argc < 2){
    printf("Error: enter <Port Number> as parameter\n");
    exit (1);
  }

//------------------------------------------------------------------------
  sd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */

  /* always check for errors */
  if (sd == -1){ /* means some kind of error occured */
    perror ("Error creating the socket");
    exit(1); /* just leave if wrong number entered */
  }

  /* now fill in the address data structure we use to sendto the server */
  for (i=0;i<strlen(argv[1]); i++){
    if (!isdigit(argv[1][i]))
      {
	printf ("Error: the port number must be a numerical integer\n");
	exit(1);
      }
  }
//------------------------------------------------------------------------
// make ip address
    
  portNumber = strtol(argv[1], NULL, 10); /* many ways to do this */

  if ((portNumber > 65535) || (portNumber < 0)){
    printf ("Error: you entered an invalid port number out of the range of 0-65535\n");
    exit (1);
  }

  fromLength = sizeof(struct sockaddr_in);

  server_address.sin_family = AF_INET; /* use AF_INET addresses */
  server_address.sin_port = htons(portNumber); /* convert port number */
  server_address.sin_addr.s_addr = INADDR_ANY; /* any adapter */
  
//------------------------------------------------------------------------
  /* the next step is to bind to the address */
  rc = bind (sd, (struct sockaddr *)&server_address,
	     sizeof(struct sockaddr ));
  
  if (rc < 0){
    perror("Error binding to the socket");
    exit (1);
  }

//------------------------------------------------------------------------

    
  while(1){ 
    
    
    memset (bufferReceived, 0, 1000); // zero out the buffers in C

	FD_ZERO(&socketFDS);// NEW                                 
    FD_SET(sd, &socketFDS); //NEW - sets the bit for the initial sd socket
    FD_SET(STDIN, &socketFDS); // NEW tell it you will look at STDIN too
      
    if (STDIN > sd) { // figure out what the max sd is. biggest number
        maxSD = STDIN;
    }
    else {
        maxSD = sd;
    }

      
      printf("\nEnter a message: ");
      fflush(stdout); // flush the output buffer

      rc = select(maxSD + 1, &socketFDS, NULL, NULL, NULL); // NEW block until something arrives
      printf("\n\nselect popped\n");


  //------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------
      
  
     if (FD_ISSET(STDIN, &socketFDS)){ // means i received something from the keyboard. 
     
         
    
		// prepare message to send to all servers in config file
		char buf[BUF_SIZE];
		memset (buf, '\0', 100);
		fgets(buf, BUF_SIZE, stdin);
		size_t len = strlen(buf);
		
		if(len > 0 && buf[len-1] == '\n'){
			buf[len-1] = '\0';
		}
		printf("\n");

		    if (!strcmp(buf, "STOP")){
      printf ("you asked me to end, so i will end\n");
      exit (1);
    }
      
      
       
    // now send the message to all servers in config file  
    int numServers = 0;     // increment through severs in config file
    int addSendersLoc = 1;  // we only need to append the senders location to the buffer message one time
		
    fopen(CONFIG_FILE, "r");

    while (fscanf(config_file, "%s %d %d", serverIP, &configPortNumber, &location) == 3){   // starting with the first line of the config file 
        numServers++;   // these will incrmenet each line of the config file
        
        newsd = socket(AF_INET, SOCK_DGRAM, 0); /* create a socket */
        if (newsd == -1){
            printf("Error creating socket\n");
            exit(1);
        }
        
		// set the destination and port address
        memset(&server_address, 0, sizeof(server_address));
        server_address.sin_family = AF_INET; /* use AF_INET addresses */
        server_address.sin_port = htons(configPortNumber); /* convert port number */
        server_address.sin_addr.s_addr = inet_addr(serverIP); /* convert IP addr */
        

	//-------------------------------------------
	char loc[] = " SendersLocation:";
        char global_loc[4];
        sprintf(global_loc, "%d", global_location);     // convert the senders location to a string so we can append it
        
        if(addSendersLoc && strlen(buf) > 0){  // if the buf is not empty, we will use this to append senders loc to all strings only once
            
            
            strcat(loc, global_loc);
            strcat(buf, loc);
            
            addSendersLoc = 0;
        }
     //-------------------------------------------
        
		 printf("Sending to location: %d, on port: %d, message: '%s'\n", location, configPortNumber, buf);         //print what we are sending to server
         sendto(newsd, buf, strlen(buf), 0, (struct sockaddr *) &server_address, sizeof(server_address)); // send it


         close(newsd);      // close the sockets
    }
            fclose(config_file);    // close the config file
                
    
    }
  
          
      
  //------------------------------------------------------------------------------------------------------------------
  //------------------------------------------------------------------------------------------------------------------
  
      if (FD_ISSET(sd, &socketFDS)){   // if we get something from the network


      rc = recvfrom(sd, bufferReceived, sizeof(bufferReceived), flags,
(struct sockaddr *)&from_address, &fromLength);
      printf ("I received %d bytes from the network\n",rc);
	  
	      
            // convert myLocation to string to we can include it at the end of the message
        char my_loc[] = " MyLocation:";
        char my_loc_str[4];
        sprintf(my_loc_str, "%d", global_location);    

        strcat(my_loc, my_loc_str);
        strcat(bufferReceived, my_loc);
          
          
    
      //------------------------
          
    // grab the sendersLocation from the buffered message and use it to find coordinates to do distance calculated
        char* locationString = strstr(bufferReceived, "SendersLocation:");

    if (locationString != NULL) {
        // advance the pointer to the beginning of the value
        locationString += strlen("SendersLocation:");

        // extract the value as an integer
        int sendersLocation = atoi(locationString);
        sendersLoc = sendersLocation;   // put it in a global var so I can use it outside of this if statement
    }
     
      
           
       // find my location's coordinates on the grid to calucalte distance between sender   
             int x1, y1, x2, y2;
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            if (grid[i][j] == global_location) {  // find coordinates of myLocation
                x2 = i;
                y2 = j;
            }
            if (grid[i][j] == sendersLoc) {     // find coordinates of sendersLocation
                x1 = i;
                y1 = j;
              
            }
        }
    } 
          
    // Calculate the Euclidean distance between the positions of myLocation and sendersLocation   
          
    int row_dist = abs(x2 - x1);
    int col_dist = abs(y2 - y1);
    double total_dist = sqrt(pow(row_dist, 2) + pow(col_dist, 2));
    int rounded_down = (int) floor(total_dist);

    printf("Distance between you and the sender is %d\n", rounded_down);    

	   //------------------------    
	     
          
      

    
	  char *version_string = "version:4";	
	  
if (strstr(bufferReceived, argv[1]) && strstr(bufferReceived, version_string)) {  // only print the string if version3 and port number is included in message

    
    //-----------------------
       if (rounded_down > 2) {
    printf("***Darn, this was for me, but it was too far for me to hear\n");
}     
     //----------------------   
    
    
  char *ptr;  // creat pointer to received message so we can parse it properly
  ptr = strtok(bufferReceived, " ");
  
	printf("--------------------------------------------------\n");
    printf("name:value\n");      
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
    // this is where we tokenize and print the string
  while (ptr !=NULL){
    

    int len = strcspn(ptr, "\"");
                      
    if (ptr[len] == '\"'){
        char quote[1000];
        strcpy(quote, ptr);
        ptr = strtok(NULL, " ");
        
        while (ptr != NULL && ptr[strcspn(ptr, "\"")] != '\"'){
            strcat(quote, " ");
            strcat(quote, ptr);
            ptr = strtok(NULL, " ");
        }
        if (ptr != NULL) {
            strcat(quote, " ");
            strcat(quote, ptr);
        printf ("'%s'\n",quote);

        }
    }
      
  else {

       printf ("'%s'\n",ptr);

    }
        ptr = strtok(NULL, " ");


    }
			printf("--------------------------------------------------\n");

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

}
      else{     // if port number is not in message, print this
        printf("--------------------------------------------------\n");
          printf("Recieved a message not destined for me!\n");
        printf("--------------------------------------------------\n");

      }
	  
	  
	   // reset vars used to make distance calculations
          x2 = 0;
          y2 = 0;
          x1 = 0;
          y1 = 0;
          rounded_down = 0;
          total_dist = 0;
          row_dist = 0;
          col_dist = 0;
          
	  
    } // end of network input
   
	  
  } // end of program while loop    

	return 0;    

} // end of main()

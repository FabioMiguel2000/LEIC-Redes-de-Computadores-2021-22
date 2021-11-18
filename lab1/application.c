#include "application.h"
#include "msg.h"
#include "data_stuffing.h"


struct applicationLayer applicationLayer;
struct linkLayer linkLayer;
extern int timeout,timeoutCount;

void parseArgs(int argc, char** argv){
    if(argc == 2 && strcmp("/dev/ttyS10", argv[1])==0){
        applicationLayer.status = TRANSMITTER;
        initLinkLayer(argv[1]);
        return;
    }
    if(argc == 2 && strcmp("/dev/ttyS11", argv[1])==0){
        applicationLayer.status = RECEIVER;
        initLinkLayer(argv[1]);
        return;
    }
    logUsage();
    exit(-1);
}

int llopen(int type){
    int fd;
    struct termios oldtio,newtio;
    fd = open(linkLayer.port, O_RDWR | O_NOCTTY| O_NONBLOCK);
    if (fd <0) {
        logError("Function llopen(), could not open port!\n");
        exit(-1); 
    }

    if ( tcgetattr(fd,&oldtio) == -1) { /* save current port settings */
        logError("Function llopen(), error on tcgetattr()!\n");
        exit(-1);
    }

    bzero(&newtio, sizeof(newtio));
    newtio.c_cflag = BAUDRATE | CS8 | CLOCAL | CREAD;
    newtio.c_iflag = IGNPAR;
    newtio.c_oflag = 0;

    /* set input mode (non-canonical, no echo,...) */
    newtio.c_lflag = 0;

    newtio.c_cc[VTIME]    = 0;   /* inter-character timer unused */
    newtio.c_cc[VMIN]     = 5;   /* blocking read until 5 chars received */
    
    /* VTIME e VMIN devem ser alterados de forma a proteger com um temporizador a 
    leitura do(s) pr�ximo(s) caracter(es)*/
    tcflush(fd, TCIOFLUSH);

    if ( tcsetattr(fd,TCSANOW,&newtio) == -1) {
        logError("Function llopen(), error on tcgetattr()!\n");
        exit(-1);
    }
    logSuccess("New termios structure set\n");
    return fd;
}

void initLinkLayer(char *port){
    strcpy(linkLayer.port, port);
    linkLayer.baudRate = BAUDRATE;
    linkLayer.sequenceNumber = 0x00;
    linkLayer.timeout = TIME_OUT;
    linkLayer.numTransmissions = TIME_OUT_CHANCES;
}

int transmitter(){
    int fd, res;
    char buf[MAX_SIZE];
    char msg[MAX_SIZE];
    fd = llopen(applicationLayer.status);
    if(fd< 0){
        logError(">>>Could not open transmitter serial port\n");
        exit(-1);
    }
    logSuccess("Transmitter ready!\n");
    buf[0] = FLAG;
    buf[1] = A_CERR;
    buf[2] = C_SET;
    buf[3] = BCC(A_CERR, C_SET);      //Will be trated as null character (\0) if = 0x00
    buf[4] = FLAG;
    
    signal(SIGALRM,timeoutHandler);
    
    timeout=0;
    timeoutCount=0;
    stateMachine_st stateMachine;
    stateMachine.currState=START;

    alarm(TIME_OUT);          // 3 seconds timout
    res = write(fd,buf, 5);   //Sends the data to the receiver
    sprintf(msg, "%d bytes written\n", res);
    logInfo(msg);

    while (stateMachine.currState!=STOP) {       /* loop for input */
      if(timeout){
        if(timeoutCount>=3){
          logError("TIMEOUT, UA not received!\n");
          exit(-1);
        }
        res = write(fd,buf, 5); //SENDS DATA TO RECEIVER AGAIN
        timeout=0;
        stateMachine.currState=START;
        alarm(TIME_OUT);
      }


      res = read(fd,buf,1);   /* returns after 1 char have been input */
      buf[res]=0;               /* so we can printf... */

      if(res != -1){
        sprintf(msg, "Received from Receiver:%#x:%d\n", buf[0], res);
        logInfo(msg);
      }
      updateStateMachine(&stateMachine, buf, applicationLayer.status);
    }
    return 0;
}

int prepareFrameI(){
    
}

int receiver(){
    int fd, res;
    char buf[MAX_SIZE];
    char msg[MAX_SIZE];
    fd = llopen(applicationLayer.status);
    if(fd< 0){
        logError(">>>Could not open receiver serial port\n");
        exit(-1);
    }
    logSuccess("Receiver ready!\n");
    stateMachine_st stateMachine;
    stateMachine.currState=START;
    while (stateMachine.currState!=STOP) {       /* loop for input */
      res = read(fd,buf,1);   /* returns after 1 char have been input */
      buf[res]=0;               /* so we can printf... */

    if(res != -1){
        sprintf(msg, "Received from Transmitter:%#x:%d\n", buf[0], res);
        logInfo(msg);
    }
      updateStateMachine(&stateMachine, buf, applicationLayer.status);
    }
    buf[0] = FLAG;
    buf[1] = A_CERR;
    buf[2] = C_UA;
    buf[3] = BCC(A_CERR, C_UA);
    buf[4] = FLAG;
    
    res = write(fd,buf, 5);  //Sends it back to the sender
    
    printf("Receiver: %d bytes written\n", res);
    // for(int i=0;i<5;i++){
    //   printf("Receiver Buffer:%#x\n",buf[i]);
    // }


    // waiting to receive frame I
    


    return 0;
}



int main(int argc, char** argv){
    /*parseArgs(argc, argv);
    if(applicationLayer.status == RECEIVER){
        receiver();
    }
    else if (applicationLayer.status == TRANSMITTER)
    {
        transmitter();
    }
    return 0;*/
    
  unsigned char data[]={0xaa,0x42,ESCAPE,0x3e,FLAG,0x11};
  int sqnum = 1;
  int frame_size;
  unsigned char* frame;

  frame=build_info_frame(data, sizeof(data), sqnum, &frame_size);
  

  printf("\n\nDados originais:\n");
  for(int i=0; i<sizeof(data);i++)
    printf("%02X ",data[i]);

  printf("\n\nFrame:\n");
  for(int i=0; i<frame_size;i++)
    printf("%02X ",frame[i]);  


  int stuff_size;
  unsigned char* stuffed=stuffing(frame, sizeof(data)+6,&stuff_size);
  printf("\n\nStuffed data:\n");
  for(int i=0; i<stuff_size;i++)
    printf("%02X ",stuffed[i]);

  unsigned char* unstuffed;
  int data_size;
  unstuffed=unstuffing(stuffed, stuff_size,&data_size);

  printf("\n\nUnStuffed data:\n");
  for(int i=0; i<data_size;i++)
    printf("%02X ",unstuffed[i]);

  return 0;

    
}
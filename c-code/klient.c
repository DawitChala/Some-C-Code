#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include "pgmread.h"
#include "send_packet.h"
#include "klient.h"
#include <dirent.h>
#include <libgen.h>
#include "pgmread.c"
#include "send_packet.c"

struct filname **listOfFileNames;
int antall = 0;
char *inputip;
int port;
char *inputfilnavn;
float tapsprosent;

void finnStr() {

  DIR *d;
  struct dirent *dir;

  d = opendir("big_set");
  if (d){
    int teller = 0;

    while ((dir = readdir(d)) != NULL){
      if(strcmp(dir->d_name,".")!=0&&(strcmp(dir->d_name,"..")!=0)){
        antall++;
      }
    }

    closedir(d);
    }
}
void relase(){
  int i;
  for (i = 0; i<antall ;i++) {
    free(listOfFileNames[i]);
  }
  free(listOfFileNames);
}
void error(int rc,char *messege){
  if(rc==-1){
    perror(messege);
    exit(EXIT_FAILURE);
  }
}
void readFromFile(){
  listOfFileNames = malloc(sizeof(struct filname*)*antall);
  memset(listOfFileNames,'\0',8*antall);
  int teller = 0;
  char navn[30];
  navn[0] = '\0';
  char buf;
  int count = 0;
  FILE *fp;
  int ny = 0;
  int rc = 1;

  fp = fopen("list_of_filenames.txt", "r");
  if (fp == NULL){
      printf("Could not open file %s","list_of_filenames.txt");
      return ;
  }

  while (rc){

      rc = fread(&buf, sizeof(unsigned char), 1, fp);


      if (buf == '\0' || buf == '\n' || buf == '\r') {
        ny = 1;



      }
      else if(ny == 1){
        struct filname *A = malloc(sizeof(struct filname));
        navn[teller] = '\0';

        teller = 0;
        memcpy(A->navn,navn,30);


        listOfFileNames[count] = A;
        count++;
        ny = 0;
        memset(navn,'\0', 30);
        navn[teller] = buf;
        teller++;
      }
      else{
        navn[teller] = buf;
        teller++;
      }
    }

    struct filname *A = malloc(sizeof(struct filname));
    navn[teller] = '\0';
    strcpy(A->navn,navn);
    listOfFileNames[count] = A;
    fclose(fp);



}
void makePack(int indeks){


  FILE *fil;


    char *navn = listOfFileNames[indeks]->navn;
    char buf[150];
    memset(buf,'\0',150);
    char bildedata[15000];
    memset(bildedata,'\0',15000);
    fil = fopen(navn,"r");
    if(!fil){
        printf("feilet \n" );
        return;
    }
    while(fgets(buf,150,fil)){
        buf[strlen(buf)] = '\0';
        strcat(bildedata,buf);

    }
    bildedata[strlen(bildedata)]='\0';
    fclose(fil);
    char buffer[1500];
    memset(buffer,'\0',1500);

    char *baseNavn;
    baseNavn = basename(navn);

    unsigned char tall = indeks;
    buffer[4] = tall;
    buffer[5] = 0;
    buffer[6]= 0x1;
    buffer[7] = 0x7f;
    int i;
    unsigned long lengde_filnavn = strlen(baseNavn)+1;

    buffer[12] = (lengde_filnavn >> 24) & 0xFF;
    buffer[13] = (lengde_filnavn >> 16) & 0xFF;
    buffer[14] = (lengde_filnavn >> 8) & 0xFF;
    buffer[15] = lengde_filnavn & 0xFF;
    buffer[16] = '\0';

    i = 17;
    int count = 0;
    for (i = i ; i < lengde_filnavn+17; i++) {
      buffer[i] = baseNavn[count];
      count ++;
    }
    count = 0;
    for(i = i; i < strlen(bildedata)+lengde_filnavn+17;i++){

      buffer[i] = bildedata[count];
      count++;
    }
    unsigned long value = strlen(bildedata)+17+lengde_filnavn;
    buffer[0] = (value >> 24) & 0xFF;
    buffer[1] = (value >> 16) & 0xFF;
    buffer[2] = (value >> 8) & 0xFF;
    buffer[3] = value & 0xFF;
    buffer[lengde_filnavn]='\0';
    sende(buffer);


    bildedata[0]='\0';
    buf[0]='\0';
}
void sende(char *data){

    int rc = -1;
    int socc = -1;
    char *info = data;
    int tall = 0;

    if(data[6]&&0x1){
      tall = 1500;
    }
    else{
      tall=8;
    }
    struct in_addr ipadresse;
    struct sockaddr_in adresse;
    socc = socket(AF_INET, SOCK_DGRAM, 0);


    inet_pton(AF_INET, inputip, &ipadresse);

    adresse.sin_family = AF_INET;
    adresse.sin_port = htons(port);
    adresse.sin_addr = ipadresse;

    rc = send_packet(socc,info,tall,0,(struct sockaddr *)&adresse,sizeof(struct sockaddr_in));
    error(rc, "send_packet");
    close(socc);

}
void slide(){
  int seqNumber = 0;
  int mottatt = 0;
  int vindu = 7;
  fd_set read_set;
  struct timeval tv;
  tv.tv_sec = 2;
  tv.tv_usec = 0;
  char buf[8];
  buf[0] = '\0';
  int socc = 0;
  int rc = 0 ;
  struct in_addr ipadress;
  struct sockaddr_in adress;

  unsigned int senderAddrSize = sizeof(adress);
  /* vil lytte på IPv4-adress til localhost */
  inet_pton(AF_INET, "0.0.0.0", &ipadress);
  set_loss_probability(tapsprosent);

  /* vil lytte med IPv4 på port 1234 på localhost */
  adress.sin_family = AF_INET;
  adress.sin_port = htons(1234);
  adress.sin_addr = ipadress;

  /* vil ha en socket som tar imot datagrammer med IPv4 */
  socc = socket(AF_INET, SOCK_DGRAM, 0);
  error(socc, "socket");


  /* socketen skal lytte på adressen vi har satt (localhost, port 1234) */
  rc = bind(socc, (struct sockaddr *)&adress, sizeof(struct sockaddr_in));

  error(rc, "bind");

  while(1){
    FD_ZERO(&read_set);
    FD_SET(socc, &read_set);

    while(seqNumber  < vindu && seqNumber - 1 < antall) {
        makePack(seqNumber);
        seqNumber++;

    }
    //printf("seqnr:%d vindu:%d motatt:%d \n",seqNumber,vindu,mottatt);

    while(seqNumber == vindu || seqNumber - 1 == antall){
      socklen_t addrlen = sizeof(struct sockaddr_in);
      rc = select(FD_SETSIZE, &read_set, NULL, NULL, &tv);
      //printf("seqnr:%d vindu:%d motatt:%d\n",seqNumber,vindu,mottatt);
      if(FD_ISSET(socc, &read_set)) {
        rc = recvfrom(socc, buf, 8, 0,
                       (struct sockaddr*)&adress, &addrlen);

        printf("Fikk ack for pakke nr %d\n",(int) buf[4]);
        if(mottatt == buf[4]-1){
          mottatt++;
          vindu++;
          if(buf[4] == antall-1){
            sendterminering();
            return;
          }
        }
          }
        else{

          seqNumber = mottatt;

        }
    }

    if(mottatt > antall){
      break;
    }
  }


}
void sendterminering(){


  unsigned long value = 8;
  char terminering[8];
  sprintf(terminering, "%d", 8);

  terminering[4] = 0;
  terminering[5] = 0;
  terminering[6]= 0x4;
  terminering[7] = 0x7;
  printf("sender terminering\n");
  sende(terminering);

}
int main(int argc, char *argv[]) {
  inputip = argv[1];
  port = atoi(argv[2]);
  inputfilnavn = argv[3];
  tapsprosent = (float) atoi(argv[4])/100;

  finnStr();
  readFromFile();
  slide();
  relase();

  return 0;
}

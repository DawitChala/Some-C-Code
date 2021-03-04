#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "klient.h"
#include "send_packet.h"
#include "send_packet.c"
#include "pgmread.c"
#include "server.h"
#include <dirent.h>
#include "libgen.h"

struct filname **listOfFileNames;
struct Image **listOfPictureStruct;
int number_of_packets = 0;
char *file_to_write_to;
char *direktorat;
int portnr;
int seqNumber = 0;
int expected = 0;

#define BUFSIZE 325

void freeImages(){
  int i;
  for(i=0;i<number_of_packets;i++){
    free(listOfFileNames[i]);
    Image_free(listOfPictureStruct[i]);
  }
  free(listOfFileNames);
  free(listOfPictureStruct);

}
void finnStr() {
  DIR *d;
  struct dirent *dir;
  d = opendir(direktorat);
  if (d){
    int teller = 0;
    while ((dir = readdir(d)) != NULL){
      //printf("%s\n", dir->d_name);
      if(strcmp(dir->d_name,".")!=0&&(strcmp(dir->d_name,"..")!=0)){
        number_of_packets++;
      }
    }
    closedir(d);
    }
}

void error(int rc,char *messege){
  if(rc==-1){
    perror(messege);
    exit(EXIT_FAILURE);
  }
}

void sammenligne(struct Image *bilde, char *fil){
  int number_of_packetsMatcher=0;
  int i;
  for (i = 0;i  < number_of_packets; i++) {
    if(Image_compare(bilde,listOfPictureStruct[i])==1){
      printf("fant en match %s=%s\n",listOfFileNames[i]->navn,fil);
      FILE *fp;
      char str[strlen(fil)+strlen(listOfFileNames[i]->navn)+6];

      str[0]='\0';
      strcat(str,"<");
      strcat(str,fil);
      strcat(str,">");
      strcat(str,"<");
      strcat(str,listOfFileNames[i]->navn);
      strcat(str,">");

      str[strlen(fil)+strlen(listOfFileNames[i]->navn)+4] = '\n';
      str[strlen(fil)+strlen(listOfFileNames[i]->navn)+5] = '\0';
      Image_free(bilde);
      printf("str %s\n",str );
      fp = fopen( file_to_write_to ,"a");
      fwrite(str , 1 , strlen(str) , fp );


      fclose(fp);
      return;
    }
  }
  FILE *fp;
  char str[strlen(fil)+strlen("UNKNOWN")+6];
  str[0]='\0';
  strcat(str,"<");
  strcat(str,fil);
  strcat(str,">");
  strcat(str,"<");
  strcat(str,"UNKNOWN");
  strcat(str,">");

  str[strlen(fil)+strlen(listOfFileNames[i]->navn)+4] = '\n';
  str[strlen(fil)+strlen(listOfFileNames[i]->navn)+5] = '\0';

  printf("str %s\n",str );
  fp = fopen( file_to_write_to ,"a");
  fwrite(str , 1 , strlen(str) , fp );
  fclose(fp);
}

int finnFilnavn(){
  listOfFileNames = malloc(sizeof(struct filname*)*number_of_packets);

  DIR *d;
  struct dirent *dir;
  d = opendir(direktorat);
  if (d){
    int teller = 0;
    while ((dir = readdir(d)) != NULL){
      //printf("%s\n", dir->d_name);
      if(strcmp(dir->d_name,".")!=0&&(strcmp(dir->d_name,"..")!=0)){
        struct filname *filnavn = malloc(sizeof(struct filname));
        memcpy(filnavn->navn,dir->d_name,30);
        listOfFileNames[teller] = filnavn;
        teller++;
      }
    }
    closedir(d);
    }
    return(0);
}

void hentFilStruct(){
  listOfPictureStruct = malloc(sizeof(struct Image*)*number_of_packets);
  memset(listOfPictureStruct,'\0',8*number_of_packets);
  char filnavnet[40];
  filnavnet[0] = '\0';
  int i;
  for (i = 0; i < number_of_packets; i++) {
    strcat(filnavnet,direktorat);
    strcat(filnavnet,"/");
    strcat(filnavnet,listOfFileNames[i]->navn);

    FILE *fil;
    char buf[150];
    buf[0] = '\0';
    char bildedata[15000];
    bildedata[0] = '\0';
    fil = fopen(filnavnet,"r");
    if(!fil){
        printf("feilet \n" );
        return;
    }
    while(fgets(buf,150,fil)){
        strcat(bildedata,buf);
    }
    fclose(fil);
    bildedata[strlen(bildedata)]= '\0';
    struct Image *bilde = Image_create(bildedata);
    listOfPictureStruct[i] = bilde;
    filnavnet[0] = '\0';
  }

}

void sendeAck(){
  int so, rc;

  struct in_addr ipadresse;
  struct sockaddr_in adresse;
  so = socket(AF_INET, SOCK_DGRAM, 0);

  inet_pton(AF_INET, "0.0.0.0", &ipadresse);

  adresse.sin_family = AF_INET;
  adresse.sin_port = htons(1234);
  adresse.sin_addr = ipadresse;

  char buffer[8];
  printf("sender ack %d\n",expected );
  unsigned long value = 8;
  buffer[0] = (value >> 24) & 0xFF;
  buffer[1] = (value >> 16) & 0xFF;
  buffer[2] = (value >> 8) & 0xFF;
  buffer[3] = value & 0xFF;

  unsigned char tall = expected;
  buffer[4] = tall;
  buffer[5] = 0;
  buffer[6]= 0x2;
  buffer[7] = 0x7;
  rc = sendto(so,buffer,8,0,(struct sockaddr *)&adresse,sizeof(struct sockaddr_in));
  error(rc, "sendt ack");
  printf("Sendte ack %d\n", expected);
  close(so);
}
void motta(){
  int so, rc;
  struct in_addr ipadresse;
  struct sockaddr_in adresse;
  unsigned int SenderAddrSize = sizeof (adresse);

  inet_pton(AF_INET, "0.0.0.0", &ipadresse);

  adresse.sin_family = AF_INET;
  adresse.sin_port = htons(portnr);
  adresse.sin_addr = ipadresse;

  so = socket(AF_INET, SOCK_DGRAM, 0);
  error(so,"socket");

  rc = bind(so, (struct sockaddr *)&adresse, sizeof(struct sockaddr_in));
  error(rc,"bind");

  unsigned char buf[1500];
  while(1){
    printf("mottar pakke \n" );
    rc = recvfrom(so,buf,sizeof(buf), 0,(struct sockaddr *)&adresse, &SenderAddrSize);

    int file_name_as_string = (buf[12] << 24) | (buf[13] << 16) | (buf[14] << 8) | buf[15];
    int hele = (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
    int lengde = hele - file_name_as_string -17;
    char bilde[1500];

    if(buf[6]&0x4){
      printf("Motatt terminering \n");
      close(so);
      return;
    }

    memcpy(bilde,buf+file_name_as_string+17,lengde);

    char filnanv[20];
    memcpy(filnanv,buf+17,file_name_as_string);

    if(expected >= buf[4]){
      printf("tall som kom inn %hhu\n",buf[4] );
      if(buf[4]==expected){
        printf("kom hit\n" );
        struct Image *img = Image_create(bilde);
        printf("expected %d\n",expected );
        sendeAck();
        expected++;
        sammenligne(img,filnanv);
      }

    }

    memset(buf,0,1500);
    buf[0]='\0';

  }
  close(so);
}

int main(int argc, char *argv[]) {

  file_to_write_to = argv[3];
  direktorat = argv[2];
  portnr = atoi(argv[1]);

  finnStr();
  finnFilnavn();
  hentFilStruct();
  motta();
  freeImages();
  return 0;
}

#ifndef HEADER_FILE
#define HEADER_FILE

struct pakke{
  unsigned int length;
  unsigned char seqnr;
  unsigned char flags;
  unsigned char ack;
  unsigned char unused;
  unsigned char data[320];

};
struct filname{
  char navn[30];
};
void sende(char *data);
void relase();
void relasePakker();
void error(int rc,char *messege);
void readFromFile();
void checkMatch();
void makePack();
void sende(char *data);
void mottaAck();
void makePack(int indeks);
void slide();
void sendterminering();
#endif

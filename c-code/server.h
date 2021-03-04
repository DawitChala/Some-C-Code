#ifndef HEADER_FILE
#define HEADER_FILE


struct filname{
  char navn[30];
};
void error(int rc,char *messege);
void sammenligne(struct Image *bilde, char *fil);
void finnFilnavn();
void hentFilStruct();
void motta();
void sendeAck();
int main(int argc, char *argv[]);
void finnStr();


#endif

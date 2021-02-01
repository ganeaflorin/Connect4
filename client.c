#define ROWS 6
#define COLUMNS 7
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <string.h>
typedef enum
{
  F,
  T
} boolean;
void print_matrix(int matrix[ROWS][COLUMNS], int turn, char scor[256])
{
  fflush(stdout);
  printf("\n");
  fflush(stdout);
  printf("A B C D E F G\n\n");
  fflush(stdout);
  for (int i = 0; i < ROWS; i++)
  {
    for (int j = 0; j < COLUMNS; j++)
      if (matrix[i][j] == 0)
        printf("_ ");
      else if (matrix[i][j] == 1)
      {
        if (turn == 1)
          printf("G ");
        else
          printf("R ");
      }
      else if (matrix[i][j] == 2)
      {
        if (turn == 1)
          printf("R ");
        else
          printf("G ");
      }
    printf("\n");
    fflush(stdout);
  }
  printf("%s\n", scor);
  fflush(stdout);
}

void create_matrix(int sd, int matrix[ROWS][COLUMNS], char scor[256])
{
  for (int i = 0; i < ROWS; i++)
    for (int j = 0; j < COLUMNS; j++)
    {
      int x;
      read(sd, &x, sizeof(int));
      matrix[i][j] = x;
    }
    
    int length;
    read(sd, &length, sizeof(int));
    read(sd, scor, length);
}
void send_input(int sd)
{
  char input[256];
  bzero(input, 256);
  printf("Introduceti coloana: ");
  fflush(stdout);

  scanf("%s", input);
  fflush(stdin);
  int length = strlen(input);

  write(sd, &length, sizeof(int));
  write(sd, input, length);
}
void receive_input_check(int sd)
{
  char buff[256];
  int length = -1;
  bzero(buff, 256);
  read(sd, &length, sizeof(int));
  read(sd, buff, length);
  if (strcmp(buff, "Input corect.") != 0)
    printf("%s", buff);
}
void play_again(int sd)
{
  char input[256], play_again_check[256];
  bzero(input, 256);
  bzero(play_again_check,256);
  scanf("%s", input);
  fflush(stdin);
  if (strcmp(input, "play") == 0)
  {
    int send_input = 1;
    write(sd, &send_input, sizeof(int));
  }
  else if (strcmp(input, "quit") == 0)
  {
    int send_input = 0;
    write(sd, &send_input, sizeof(int));
    // close(sd);
    // exit(0);
  }
  else
  {
    printf("Comanda gresita! Va rugam incercati din nou.");
    // fflush(stdout);
    play_again(sd);
  }
  int length;
  read(sd, &length, sizeof(int));
  read(sd, play_again_check, length);
  printf("%s", play_again_check);
  fflush(stdout);
  sleep(2);
  if(strcmp(play_again_check, "Veti fi deconectat.") == 0)
    { 
      close(sd);
      exit(0);
    }
    else if(strcmp(play_again_check, "Oponentul a refuzat sa mai joace. Veti fi deconectat.") == 0)
    { 
      close(sd);
      exit(0);
    }
  fflush(stdout);
  printf("\nIn continuare veti juca cu aceeasi culoare.");
}
int switch_turn(int turn)
{
  if (turn == 1)
    return 2;
  return 1;
}
void game_finished_message(int check_game_finished, int turn2, char score[256])
{
  char mesaj[256] = "Daca doriti sa mai jucati, introduceti comanda \"play\". \nDaca nu doriti sa mai jucati, introduceti \"quit\" \n ";

  if (check_game_finished == -1)
  {
    printf("Egalitate! \nNoul scor este: %s\n", score);
    printf("%s", mesaj);
  }
  else
  {
    if (check_game_finished != turn2)
    {
      printf("Ai castigat!\nNoul scor este: %s\n", score);
      fflush(stdout);
    }
    else
    {
      printf("Ai pierdut!\nNoul scor este: %s\n", score);
      fflush(stdout);
    }
    printf("%s", mesaj);
    fflush(stdout);
  }
}
/* codul de eroare returnat de anumite apeluri */
extern int errno;

/* portul de conectare la server*/
int port;

int main(int argc, char *argv[])
{
  int sd;                    // descriptorul de socket
  struct sockaddr_in server; // structura folosita pentru conectare
  // int coloana;               // mesajul trimis

  /* exista toate argumentele in linia de comanda? */
  if (argc != 3)
  {
    printf("[client] Sintaxa: %s <adresa_server> <port>\n", argv[0]);
    return -1;
  }

  /* stabilim portul */
  port = atoi(argv[2]);

  /* cream socketul */
  if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("[client] Eroare la socket().\n");
    return errno;
  }

  /* umplem structura folosita pentru realizarea conexiunii cu serverul */
  /* familia socket-ului */
  server.sin_family = AF_INET;
  /* adresa IP a serverului */
  server.sin_addr.s_addr = inet_addr(argv[1]);
  /* portul de conectare */
  server.sin_port = htons(port);

  /* ne conectam la server */
  if (connect(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
  {
    perror("[client]Eroare la connect().\n");
    return errno;
  }
char size_mess;
char mesaj_prim[256];
printf("Bine ati venit! Va rugam sa va autentificati.\n");
printf("Daca nu aveti un cont, acesta va fi inregistrat automat.\n");
do 
  //INTRODUCERE NICKNAME
 { char nickname[256], password[256];
    bzero(mesaj_prim, 256);
  printf("Va rugam introduceti un username.\n");
  scanf("%s", nickname);
  fflush(stdin);

  //SEND NICKNAME
  int size_nickname = strlen(nickname);
  write(sd, &size_nickname, sizeof(int));
  write(sd, nickname, size_nickname);

  //SEND PASSWORD
  printf("Va rugam introduceti parola. \n ");
  scanf("%s", password);
  fflush(stdin);
  int size_password = strlen(password);
  write(sd, &size_password, sizeof(int));
  write(sd, password, size_password);
  read(sd, &size_mess, sizeof(int));
  read(sd, mesaj_prim, size_mess);
  printf("%s\n", mesaj_prim);
  } while(strcmp(mesaj_prim, "Parola incorecta, va rugam reincercati!")==0);
  printf("Asteptati un oponent.\n");
  //OPONENT GASIT:
  int size_buff;
  char mesaj_primit[256];
  bzero(mesaj_primit, 256);
  read(sd, &size_buff, sizeof(int));
  read(sd, mesaj_primit, size_buff);
  printf("%s\n", mesaj_primit);

  //MESAJ CINE INCEPE
  bzero(mesaj_primit, 256);
  read(sd, &size_buff, sizeof(int));
  read(sd, mesaj_primit, size_buff);
  printf("%s\n", mesaj_primit);
  int turn = 0, turn_copy = 0;
  int matrix[ROWS][COLUMNS] = {0};
  //turn copy pt print

  read(sd, &turn_copy, sizeof(int));
  read(sd, &turn, sizeof(int));
  int turn2 = turn;
  char scor[256]="";
  bzero(scor, 256);
  //turn_copy = clientul care a inceput jocul, folosit pt print
  //turn2 = tura originala a clientului acesta, sa vedem daca e tura lui
  //atunci cand citim "turn" din server
  fflush(stdout);
  int x;
  while (1)
  { 
    fflush(stdout);
    int check_game_finished = -2;
    read(sd, &check_game_finished, sizeof(int));
    create_matrix(sd, matrix, scor);
    print_matrix(matrix, turn_copy, scor);
    if (check_game_finished == 0) // daca nu s-a terminat
    {

      read(sd, &turn, sizeof(int)); //vedem cine muta

      fflush(stdout);
      if (turn != turn2) //daca nu muta clientul curent isi asteapta tura
      {
        printf("Va rugam asteptati-va randul.");
        fflush(stdout);
        continue;
      }
      
      //daca muta clientul curent, citim un input si il trimitem
      send_input(sd);          //trimitem coloana
      receive_input_check(sd); //primim raspunsul daca inputul e bun

    }
    else //daca jocul s-a terminat
    {
      //afisam daca a castigat sau a pierdut
      game_finished_message(check_game_finished, turn2, scor);
      play_again(sd);
        
    }
  }
  close(sd);
}
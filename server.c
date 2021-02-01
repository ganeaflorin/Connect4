#define ROWS 6
#define COLUMNS 7

#include <sys/socket.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <ctype.h>
#include <stdbool.h>
#include <time.h>
#include <sqlite3.h>
int create_DB(const char *path)
{
    sqlite3 *DB;
    int exit = 0;
    exit = sqlite3_open(path, &DB);
    // sqlite3_close(DB);
    return 0;
}
int create_table(const char *path)
{
    sqlite3 *DB;
    char *sql = "CREATE TABLE IF NOT EXISTS LOGIN("
                "ID INTEGER PRIMARY KEY AUTOINCREMENT, "
                "USERNAME   TEXT NOT NULL, "
                "PASSWORD   TEXT NOT NULL );";
    int exit = 0;
    exit = sqlite3_open(path, &DB);
    char *error;
    exit = sqlite3_exec(DB, sql, NULL, 0, &error);
    if (exit != SQLITE_OK)
    {
        printf("Error Create Table\n");
        sqlite3_free(error);
    }
    else
        printf("Table created successfully\n");
    // sqlite3_close(DB);
}
int callback(void *not_used, int argc, char **argv, char **az_col_name)
{
    for (int i = 0; i < argc; i++)
    {
        printf(" %s\n", argv[i]);
    }
    printf("\n");
}
int insert_data(const char *path, char username[256], char password[256])
{
    sqlite3 *DB;
    char *error;
    int exit = sqlite3_open(path, &DB);
    char *sql;
    //const char *sql = "INSERT INTO LOGIN (USERNAME, PASSWORD) VALUES ( 'florinescu','boss');";
    sprintf(sql, "INSERT INTO LOGIN (USERNAME, PASSWORD) VALUES ('%s', '%s')", username, password);
    exit = sqlite3_exec(DB, sql, NULL, 0, &error);
    if (exit != SQLITE_OK)
    {
        perror("insert");
        sqlite3_free(error);
    }
    else
        printf("insert successfully\n");
    // sqlite3_close(DB);
}
int select_all_data(sqlite3 *db)
{
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "select * from LOGIN", -1, &stmt, NULL);
    while (sqlite3_step(stmt) != SQLITE_DONE)
    {
        int i;
        int num_cols = sqlite3_column_count(stmt);

        for (i = 0; i < num_cols; i++)
        {
            switch (sqlite3_column_type(stmt, i))
            {
            case (SQLITE3_TEXT):
                printf("%s, ", sqlite3_column_text(stmt, i));
                break;
            case (SQLITE_INTEGER):
                printf("%d, ", sqlite3_column_int(stmt, i));
                break;
            case (SQLITE_FLOAT):
                printf("%g, ", sqlite3_column_double(stmt, i));
                break;
            default:
                break;
            }
        }
        printf("\n");
        sqlite3_close(db);
    }

    sqlite3_finalize(stmt);
}
int username_check(sqlite3 *db, const char *path, char username[256], char password[256])
{
    int ok = -2;
    //ok==1 user logat
    //ok==0 username corect, parola gresita
    //ok==-2 usernameul nu exista, il inregistram
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, "select * from LOGIN", -1, &stmt, NULL);
    //BD are atributele ID (col. 0), Username(1) si Password(2)
    while (sqlite3_step(stmt) != SQLITE_DONE) //schimbam randul
    {   //daca gasim usernameul pe a 2-a coloana
        if (strcmp(sqlite3_column_text(stmt, 1), username) == 0) 
        { //daca parola corespunde
            if (strcmp(sqlite3_column_text(stmt, 2), password) == 0) 
            //daca am gasit usernameul si pe acelasi rand gasimt parola
                ok = 1;
            //daca am gasit usernameul dar parola nu corespunde
            else
                ok = 0;
            break;
        }
    }
    //daca nu am gasit usernameul ok ramane -2.
    
    sqlite3_close(db); //inchidem BD
    return ok;
}

void send_message_username_check(int fd, int ok)
{
    char register_user[256] = "V-ati inregistrat cu succes!";
    char login_failed[256] = "Parola incorecta, va rugam reincercati!";
    char login_success[256] = "V-ati logat cu succes!";
    if (ok == -2)
        send_message(fd, register_user);


    else if (ok == 1)
        send_message(fd, login_success);
    else
        send_message(fd, login_failed);
}
typedef enum
{
    F,
    T
} boolean;
void send_message(int fd, char buff[256])
{
    int size = strlen(buff);
    write(fd, &size, sizeof(int));
    write(fd, buff, size);
}
void print_matrix(int matrix[ROWS][COLUMNS], int turn)
{
    printf("\n");
    printf("A B C D E F G\n\n");
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
    }
}
/* portul folosit */
int game_finished(int matrix[ROWS][COLUMNS])
{
    //-1 DACA E DRAW
    //1 DACA CASTIGA CLIENT1
    //2 DACA CASTIGA CLIENT2
    //0 DACA NU S-A TERMINAT
    bool is_matrix_full = T;
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLUMNS; j++)
            if (matrix[i][j] == 0)
                is_matrix_full = F;

    if (is_matrix_full == T)
        return -1; //daca matricea e plina cu valori != 0 returnam -1 semnaland egalitatea

    for (int i = 0; i < ROWS; i++) //verificam pe orizontala
    {
        int current_element = 0;
        int number_elements = 1;
        for (int j = 0; j < COLUMNS; j++)
        {
            if (current_element != matrix[i][j])
            {
                current_element = matrix[i][j];
                number_elements = 1;
            }
            else
                number_elements++;

            if (number_elements == 4 && current_element != 0) //daca avem 4 elemente la rand diferite de 0
            {
                return current_element; //returnam 1 sau 2 in functie de castigator
                break;
            }
        }
    }
    for (int i = 0; i < COLUMNS; i++) //verificam pe verticala
    {
        int current_element = 0;
        int number_elements = 1;
        for (int j = 0; j < ROWS; j++)
        {
            if (current_element != matrix[j][i])
            {
                current_element = matrix[j][i];
                number_elements = 1;
            }
            else
                number_elements++;

            if (number_elements == 4 && current_element != 0) //daca avem 4 elemente la rand diferite de 0
            {
                return current_element; //returnam 1 sau 2 in functie de castigator
                break;
            }
        }
    }
    for (int i = 0; i < ROWS / 2; i++)
    {
        for (int j = 0; j < COLUMNS; j++)
        {
            if ((matrix[i][j] == 1 && matrix[i + 1][j + 1] == 1 && matrix[i + 2][j + 2] == 1 && matrix[i + 3][j + 3] == 1) ||
                (matrix[i][j] == 2 && matrix[i + 1][j + 1] == 2 && matrix[i + 2][j + 2] == 2 && matrix[i + 3][j + 3] == 2) ||
                (matrix[i][j] == 1 && matrix[i + 1][j - 1] == 1 && matrix[i + 2][j - 2] == 1 && matrix[i + 3][j - 3] == 1) ||
                (matrix[i][j] == 2 && matrix[i + 1][j - 1] == 2 && matrix[i + 2][j - 2] == 2 && matrix[i + 3][j - 3] == 2))
                return matrix[i][j];
        }
    }
    return 0;
}

void send_matrix(int client, int matrix[ROWS][COLUMNS], int score[2], char nick1[256], char nick2[256])
{
    for (int i = 0; i < ROWS; i++)
        for (int j = 0; j < COLUMNS; j++)
        {
            int x = matrix[i][j];
            write(client, &x, sizeof(int));
        }

    char scor[256];
    bzero(scor, 256);

    sprintf(scor, "%s %d - %d %s", nick1, score[1], score[0], nick2);
    fflush(stdout);
    send_message(client, scor);
}
#define PORT 4000

extern int errno; /* eroarea returnata de unele apeluri */

/* functie de convertire a adresei IP a clientului in sir de caractere */
char *conv_addr(struct sockaddr_in address)
{
    static char str[25];
    char port[7];

    /* adresa IP a clientului */
    strcpy(str, inet_ntoa(address.sin_addr));
    /* portul utilizat de client */
    bzero(port, 7);
    sprintf(port, ":%d", ntohs(address.sin_port));
    strcat(str, port);
    return (str);
}
int create_socket(int sd)
{
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
    {
        perror("[server] Eroare la socket().\n");
        return errno;
    }

    return sd;
}
//pregatim structurile de date
struct sockaddr_in server_structure(struct sockaddr_in server)
{
    bzero(&server, sizeof(server));

    /* umplem structura folosita de server */
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);
    return server;
}
void read_string(int fd, char buffer[256])
{
    int size = 0;
    char buff[256] = "";
    if (read(fd, &size, sizeof(int)) < 0)
        perror("eroare read sizeof_nickname");

    if (read(fd, buff, size) < 0)
        perror("eroare citit nickname");
    strcpy(buffer, buff);
}

int client_to_start(int a[2], int rand)
{
    char msg1[256] = "Ati primit culoarea rosie prin randomizare. Veti incepe jocul.";
    char msg2[256] = "Ati primit culoarea galbena prin randomizare. Oponentul va incepe jocul.";

    if (rand == 0)
    {
        send_message(a[0], msg1);
        send_message(a[1], msg2);
        return 1; //primul client va incepe jocul
    }
    else
    {
        send_message(a[0], msg2);
        send_message(a[1], msg1);
        return 2; //al 2-lea client va incepe jocul
    }
}
void send_initial_turn(int client, int client1, int turn_copy)
{
    if (turn_copy == 1)
    {
        int turn_temp = 2;
        write(client, &turn_copy, sizeof(int));
        write(client1, &turn_temp, sizeof(int));
    }
    else
    {
        int turn_temp = 1;
        write(client, &turn_temp, sizeof(int));
        write(client1, &turn_copy, sizeof(int));
    }
}
int check_input_format(char buff[256], int matrix[ROWS][COLUMNS])
{
    int coloana = -1;
    char ch = tolower(buff[0]);
    if (strlen(buff) > 1)
        return -1;

    else if (!(ch >= 'a' && ch <= 'g'))

        return -2;

    switch (ch)
    {
    case 'a':
        coloana = 0;
        break;
    case 'b':
        coloana = 1;
        break;
    case 'c':
        coloana = 2;
        break;
    case 'd':
        coloana = 3;
        break;
    case 'e':
        coloana = 4;
        break;
    case 'f':
        coloana = 5;
        break;
    case 'g':
        coloana = 6;
        break;
    }
    if (matrix[0][coloana] != 0)
        return -3;
    return coloana;
}
int switch_turn(int turn)
{
    if (turn == 1)
        return 2;
    return 1;
}
int turn_client(int client, int client1, int turn)
{
    if (turn == 1)
        return client;
    return client1;
}
void read_message(int client, int client1, int turn, char buff[256])
{
    bzero(buff, 256);
    int size_string = -1;

    read(turn_client(client, client1, turn), &size_string, sizeof(int));
    read(turn_client(client, client1, turn), buff, size_string);
}
int read_input(int client, int client1, int turn, int matrix[ROWS][COLUMNS])
{
    int length;
    char buff[256];
    bzero(buff, 256);
    //mesaje pe care le trimitem clientului
    char mesaj1[256] = "Va rugam introduceti o litera de la A la G.";
    char mesaj2[256] = "Va rugam introduceti un singur caracter.";
    char mesaj3[256] = "Input corect.";
    char mesaj4[256] = "Coloana este plina. Va rugam introduceti alta coloana";
//citim de la clientul a carui tura este inputul
    read(turn_client(client, client1, turn), &length, sizeof(int));  
    read(turn_client(client, client1, turn), buff, length);
    //verificam corectitudinea inputului si ii trimitem un mesaj corespunzator
    int a = check_input_format(buff, matrix);
    
    if (a == -2)
        send_message(turn_client(client, client1, turn), mesaj1);
    else if (a == -1)
        send_message(turn_client(client, client1, turn), mesaj2);
    else if (a >= 0)
        send_message(turn_client(client, client1, turn), mesaj3);
    else if (a == -3)
        send_message(turn_client(client, client1, turn), mesaj4);
    return a;
    fflush(stdout);
}
int change_matrix(int matrix[ROWS][COLUMNS], int column, int turn)
{
    int ok = 0; //presupun ca e full coloana
    for (int i = ROWS - 1; i >= 0; i--)
    {
        if (matrix[i][column] == 0)
        {
            matrix[i][column] = turn;
            ok = 1; //am reusit sa punem
            break;
        }
    }
    return ok;
}

void check_play_again(int client, int client1, int matrix[ROWS][COLUMNS])
{

    int play_again_c1 = -1, play_again_c2 = -1;
    read(client, &play_again_c1, sizeof(int));
    read(client1, &play_again_c2, sizeof(int));
    char mesaj[256] = "Oponentul a refuzat sa mai joace. Veti fi deconectat.";
    char mesaj2[256] = "Veti fi deconectat.";
    char mesaj3[256] = "Oponentul este de acord. Un nou joc va incepe imediat.";
    //daca vor ambii sa joace:

    if (play_again_c1 == 1 && play_again_c2 == 1)
    {
        send_message(client, mesaj3);
        send_message(client1, mesaj3);
        fflush(stdout);
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLUMNS; j++)
                matrix[i][j] = 0;
    }
    else if (play_again_c1 == 1 && play_again_c2 != 1)
    {
        send_message(client, mesaj);
        send_message(client1, mesaj2);
    }
    else if (play_again_c1 != 1 && play_again_c2 == 1)
    {
        send_message(client1, mesaj);
        send_message(client, mesaj2);
    }

    else
    {
        send_message(client, mesaj2);
        send_message(client1, mesaj2);
    }
}
void score_update(int game_result, int score[2])
{
    if (game_result == -1)
    {
        score[0]++;
        score[1]++;
    }
    if (game_result == 1)
        score[0]++;
    if (game_result == 2)
        score[1]++;
}

int main()
{
    int score[2] = {0}; /* variabila pentru contorizarea scorului */
    int check_game_finished = 0;
    struct sockaddr_in server; /* structurile pentru server si clienti */
    struct sockaddr_in from;
    // fd_set readfds;    /* multimea descriptorilor de citire */
    // fd_set actfds;     /* multimea descriptorilor activi */
    // struct timeval tv; /* structura de timp pentru select() */
    int sd, client;    /* descriptori de socket */
    int optval = 1;    /* optiune folosita pentru setsockopt()*/
    int fd;            /* descriptor folosit pentru 
				   parcurgerea listelor de descriptori */
    int nfds;          /* numarul maxim de descriptori */
    int len;           /* lungimea structurii sockaddr_in */
    int numar_clienti = 0;
    int fd_clienti[2] = {0}, client1;
    char nickname_c1[256], nickname_c2[256];
    char password_c1[256], password_c2[256];
    char welcome[256] = "Va rugam asteptati pana gasim un oponent.\n";
    char opponent_found[256] = "A fost gasit un oponent. Meciul va incepe imediat.\n";
    pid_t pid;

    const char *path = "LOGIN.db";
    sqlite3 *db;

    create_DB(path);
    create_table(path);
    sqlite3_open(path, &db);
    select_all_data(db);
    if (db == NULL)
    {
        printf("Failed to open DB\n");
        return 1;
    }
    printf("Astept client la port 4000\n");
    sd = create_socket(sd);
    /*setam pentru socket optiunea SO_REUSEADDR */
    setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    server = server_structure(server);

    if (bind(sd, (struct sockaddr *)&server, sizeof(struct sockaddr)) == -1)
    {
        perror("[server] Eroare la bind().\n");
        return errno;
    }
    if (listen(sd, 5) == -1)
    {
        perror("[server] Eroare la listen().\n");
        return errno;
    }
    while (1)
    {
        srand(time(NULL));
        int who_starts = rand() % 2;
        client = accept(sd, (struct sockaddr *)&from, &len);
        if (client < 0)
        {
            perror("[server] Eroare la accept().\n");
            continue;
        }

        fd_clienti[0] = client; //adaugam clientul nou in multimea de clienti, care sunt maxim 2;
        // numar_clienti++;

        client1 = accept(sd, (struct sockaddr *)&from, &len);
        if (client1 < 0)
        {
            perror("[server] Eroare la accept().\n");
            continue;
        }
        fd_clienti[1] = client1;
        // numar_clienti++;
        if ((pid = fork()) < 0)
        {
            perror("eroare fork");
            exit(errno);
        }
        if (pid == 0) //child
        {
            int login_check = -3;
            // close(sd);
            printf("S-au conectat 2 clienti cu descriptorii: %d, %d\n", fd_clienti[0], fd_clienti[1]);
            do //CITIRE DATE LOGIN PRIMUL CLIENT  
            {
                read_string(client, nickname_c1); //citim usernameul primului client
                read_string(client, password_c1); //citim parola acestuia
                login_check = username_check(db, path, nickname_c1, password_c1); //verificam in baza de date
                if(login_check == -2) //daca nu exista in baza de date il adaugam
                  insert_data(path, nickname_c1, password_c1);

                send_message_username_check(client, login_check); //trimitem clientului daca logarea a esuat sau a reusit
            } while (login_check == 0);

            do //CITIRE DATE LOGIN AL 2-LEA CLIENT
            {
                read_string(client1, nickname_c2);
                read_string(client1, password_c2);
                login_check = username_check(db, path, nickname_c2, password_c2);
                if(login_check == -2)
                  insert_data(path, nickname_c2, password_c2);
                printf("LOGIN CHECK: %d \n", login_check);
                send_message_username_check(client1, login_check);
            } while (login_check == 0);

            send_message(client, opponent_found);
            send_message(client1, opponent_found);
            //VEDEM CARE CLIENT INCEPE
            int turn = client_to_start(fd_clienti, who_starts);

            int turn_copy = turn;
            write(client, &turn_copy, sizeof(int));
            write(client1, &turn_copy, sizeof(int));
            int matrix[ROWS][COLUMNS] = {0};

            send_initial_turn(client, client1, turn);

            fflush(stdout);
            while (1)
            {   //VERIFICAM DACA S-A TERMINAT JOCUL
                int check_game_finished = game_finished(matrix);
                write(client, &check_game_finished, sizeof(int));
                write(client1, &check_game_finished, sizeof(int));
                score_update(check_game_finished, score);
                //TRIMITEM MATRICEA SI SCORUL
                send_matrix(client, matrix, score, nickname_c1, nickname_c2);

                send_matrix(client1, matrix, score, nickname_c1, nickname_c2);

                // print_matrix(matrix, turn_copy);

                if (check_game_finished == 0) //daca nu s-a terminat
                {

                    write(client, &turn, sizeof(int));
                    write(client1, &turn, sizeof(int));

                    int input = read_input(client, client1, turn, matrix);
                    if (input >= 0) //daca inputul e valid
                    {
                        turn = switch_turn(turn); //schimbam tura

                        change_matrix(matrix, input, turn); //facem mutarea
                    }
                }
                else //daca s-a terminat
                    check_play_again(client, client1, matrix);
            }

            close(client);
            close(client1);
            fd_clienti[0] = 0;
            fd_clienti[1] = 0;
            exit(0);
        }
        else //parinte
        {

            close(client);
            close(client1);
        }
    }
}
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdbool.h> 
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <time.h>
#include <pthread.h>
// write before read

//#include <sqlite3.h>

#define MAXLINE 1024

#define MAX_THREAD_NUM 100

#define C_REGISTER 0
#define C_LOGIN 1
#define C_LOGOUT 2
#define C_GAMERULE 3
#define C_STARTGAME 4
#define C_GUESS 5
#define C_EXIT 6
#define C_INVALID 7

#define R_REGISTER_SUCCESS 0
#define R_REGISTER_ERR_USR 1
#define R_REGISTER_ERR_EMAIL 2
#define R_REGISTER_ERR_USAGE 3

#define R_LOGIN_SUCCESS 0
#define R_LOGIN_ERR_LOGOUT 1
#define R_LOGIN_ERR_USR 2
#define R_LOGIN_ERR_PASSWORD 3
#define R_LOGIN_ERR_USAGE 4

#define R_LOGOUT_SUCCESS 0
#define R_LOGOUT_ERR_LOGIN 1
#define R_LOGOUT_ERR_USAGE 2

#define R_GAMERULE_SUCCESS 0
#define R_GAMERULE_ERR_USAGE 1

#define R_STARTGAME_SUCCESS 0
#define R_STARTGAME_SUCCESS_RIGGED 1
#define R_STARTGAME_ERR_LOGIN 2
#define R_STARTGAME_ERR_USAGE 3

#define R_GUESS_VALID 0     // Valid guess but not the answer
#define R_GUESS_INVALID 1
#define R_GUESS_WIN 2
#define R_GUESS_LOSE 3

#define R_EXIT_SUCCESS 0
#define R_EXIT_ERR_USAGE 1

#define R_INVALID_COMMAND 0

const char* M_REGISTER_SUCCESS = "Register successfully.\n";
const char* M_REGISTER_ERR_USR = "Username is already used.\n";
const char* M_REGISTER_ERR_EMAIL = "Email is already used.\n";
const char* M_REGISTER_ERR_USAGE = "Usage: register <username> <email> <password>\n";

const char* M_LOGIN_SUCCESS = "Welcome, ";      // strcat with username and ".\n" for full message
const char* M_LOGIN_ERR_LOGOUT = "Please logout first.\n";
const char* M_LOGIN_ERR_USR = "Username not found.\n";
const char* M_LOGIN_ERR_PASSWORD = "Password not correct.\n";
const char* M_LOGIN_ERR_USAGE = "Usage: login <username> <password>\n";

const char* M_LOGOUT_SUCCESS = "Bye, ";         // strcat with username and ".\n" for full message
const char* M_LOGOUT_ERR_LOGIN = "Please login first.\n";
const char* M_LOGOUT_ERR_USAGE = "Usage: logout\n";

const char* M_GAMERULE_SUCCESS = "1. Each question is a 4-digit secret number.\n2. After each guess, you will get a hint with the following information:\n2.1 The number of \"A\", which are digits in the guess that are in the correct position.\n2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\nThe hint will be formatted as \"xAyB\".\n3. 5 chances for each question.\n";
const char* M_GAMERULE_ERR_USAGE = "Usage: game-rule\n";

const char* M_STARTGAME_SUCCESS = "Please typing a 4-digit number:\n";
const char* M_STARTGAME_ERR_LOGIN = "Please login first.\n";
const char* M_STARTGAME_ERR_USAGE = "Usage: start-game\n";

const char* M_GUESS_VALID = "XAXB\n";                      // xAxB
const char* M_GUESS_INVALID = "Your guess should be a 4-digit number.\n";
const char* M_GUESS_WIN = "You got the answer!\n";
const char* M_GUESS_LOSE = "You lose the game!\n";

const char* M_EXIT_SUCCESS = "\0";
const char* M_EXIT_ERR_USAGE = "Usage: exit\n";

const char* M_INVALID_COMMAND = "Invalid command.\n";

struct routineParams{
    int clientSocketFd;
    int availableThreadIdx;
};

struct udpTopParams{
    struct sockaddr_in serverAddress;
};

struct udpRoutineParams{
    int uServerSocketFd;
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLength;
    int availableThreadIdx;
    char msg[MAXLINE];
};

pthread_rwlock_t rwLock;
FILE* userFilePtr;

pthread_t newThread[100];
pthread_t recyclingThread;
int clientSocketFds[100];
int threadAvailable[100];
int threadEnded[100] = {0};
struct  routineParams rParamsArr[100];
struct udpRoutineParams uParamsArr[100];
struct udpTopParams udpTopParam;

int splitParams(char* msg, char params[][MAXLINE]){
    char cpyMsg[MAXLINE];
    int comType, resultCode;
    char* ptr;
    char* delim = " \n";
    int nParams = 0; 

    strcpy(cpyMsg, msg);
    ptr = strtok(cpyMsg, delim);
    //strcpy(params[0], ptr);
    //strcat(params[0], "\0");
    //nParams++;
    //printf("%s\n", params[0]);
    while(ptr != NULL){
        strcpy(params[nParams], ptr);
        strcat(params[nParams], "\0");
        //printf("%s\n", params[nParams]);
        nParams++;
        ptr = strtok(NULL, delim);
        //printf("%d\n", nParams);
    }
    return nParams;
}

int min(int a, int b){
    if(a >= b){
        return b;
    }else{
        return a;
    }
}

// This function returns 10*A + B
int getAB(char* ans, char* guess){
    int A=0, B=0;
    //int ansOccur[10] = {0};
    //int guessOccur[10] = {0};
    //int minOccur[10] = {0};
    int numUsed[10] = {0};
    int posCounted[4] = {0};  // If posCounted = 1: A if posCounted = 2: this pos in B is counted

    /*for(int it=0; it<4; it++){
        ansOccur[ans[it]-48]++;
        guessOccur[guess[it]-48]++;
    }
    for(int it=0; it<10; it++){
        minOccur[it] = min(ansOccur[it], guessOccur[it]);
    }*/

    for(int it=0; it<4; it++){
        if(ans[it] == guess[it]){
            //numUsed[ans[it]]++;
            posCounted[it] = 1;
            //printf("A pos: %d\n", it);
            A++;
        }
    }

    for(int it=0; it<4; it++){
        char tgt = ans[it];
        if(posCounted[it] == 1){
            continue;
        }
        for(int it2=0; it2<4; it2++){
            if(it != it2 && posCounted[it2]==0 && ans[it]==guess[it2]){
                posCounted[it2] = 2;
                //printf("B pos(ans): %d\t B pos(guess): %d\n", it, it2);
                B++;
                break;
            }
        }
    }

    /*for(int it=0; it<4; it++){
        char tgt = ans[it];
        for(int it2 =0; it2<4; it2++){
            if(it==it2 && ans[it]==guess[it2] && numUsed[ans[it]] < minOccur[ans[it]]){
                A++;
                numUsed[ans[it]]++;
                break;
            }else if(it!=it2 && ans[it]==guess[it2] && numUsed[ans[it]] < minOccur[ans[it]]){
                B++;
                numUsed[ans[it]]++;
                break;
            }
        }
    }*/
    return 10*A+B;
}


int regReadFile(FILE* userFilePtr, char* userName, char* email){
    char lineUserName[MAXLINE];
    char lineEmail[MAXLINE];
    char linePassword[MAXLINE];
    size_t len = 0;
    int retval = R_REGISTER_SUCCESS;   
    fseek(userFilePtr, 0, SEEK_SET);
    while(EOF != fscanf(userFilePtr, "%s %s %s\n", lineUserName, lineEmail, linePassword)){
        // Check if username and email exists
        if(!strcmp(userName, lineUserName)){
            retval = R_REGISTER_ERR_USR;
            break;
        }else if(!strcmp(email, lineEmail)){
            retval = R_REGISTER_ERR_EMAIL;
        }
    }
    return retval;
}


int logReadFile(FILE* userFilePtr, char* userName, char* password){
    char lineUserName[MAXLINE];
    char lineEmail[MAXLINE];
    char linePassword[MAXLINE];
    size_t len = 0;
    int retval = R_LOGIN_ERR_USR;   
    fseek(userFilePtr, 0, SEEK_SET);
     while(EOF != fscanf(userFilePtr, "%s %s %s\n", lineUserName, lineEmail, linePassword)){
        // Check if username and email exists
        //printf("username: %s\n", lineUserName);
        if(!strcmp(userName, lineUserName)){
            if(strcmp(password, linePassword) != 0){
                retval = R_LOGIN_ERR_PASSWORD;
            }else{
                retval = R_LOGIN_SUCCESS;
            }
        }
    }
    return retval;
}


int myRegister(char* userName, char* email, char* password){
    int readRet;
    char userInfo[MAXLINE];
    bzero(userInfo, sizeof(userInfo));
    strcpy(userInfo, userName);
    strcat(userInfo, " ");
    strcat(userInfo, email);
    strcat(userInfo, " ");
    strcat(userInfo, password);
    //pthread_rwlock_wrlock(&rwlock);
    readRet = regReadFile(userFilePtr, userName, email);
    // Write 
    if(readRet == R_REGISTER_SUCCESS){
        fprintf(userFilePtr, "%s\n", userInfo);
    }
    //pthread_rwlock_unlock(&rwlock);
    return readRet;
}

int login(char* userName, char* password){
    int readRet;
    //pthread_rwlock_rdlock(&rwlock);
    readRet = logReadFile(userFilePtr, userName, password);
    // Read
    //fscanf(userFilePtr, "%s %s ")
    //pthread_rwlock_unlock(&rwlock);
    return readRet;
}

// Make this a command processing function with void return value (Take modified variables' addresses as input)
void handleCommand(char* msg, int nParams, char params[][MAXLINE], int* loggedInPtr, int* inGamePtr, char* ans, char* userName){
    
    int comType, resultCode;
    
    if(*inGamePtr != 0){
        // Handle guesses
        comType = C_GUESS;
        
        if(nParams != 1){
            resultCode = R_GUESS_INVALID;
            strcpy(msg, M_GUESS_INVALID);
            return;
        }else{

            int validNum = 1;   // 1 if valid, 0 if not

            for(int it=0; it<4; it++){
                if(params[0][it] < 48 || params[0][it] > 57){
                    validNum = 0;
                }
            }
            if(params[0][4] != 0){
                validNum = 0;
            }
            
            if(validNum == 0){
                resultCode = R_GUESS_INVALID;
                strcpy(msg, M_GUESS_INVALID);
                return;
            }else if(*inGamePtr < 5){
                // Calculate A and B
                char ANum, BNum;
                int ABRet = getAB(ans, params[0]);
                ANum = (char)(ABRet/10 + 48);
                BNum = (char)(ABRet%10 + 48);
                //printf("Anum: %d\n", ANum);
                //printf("Bnum: %d\n", BNum);

                if(!strcmp(params[0], ans)){
                    //printf("ans: %s\n", ans);
                    //printf("params[0]: %s \n", params[0]);
                    resultCode = R_GUESS_WIN;
                    strcpy(msg, M_GUESS_WIN);
                    //printf("%s", msg);
                    *inGamePtr = 0;
                    return;
                }else{
                    //printf("ans: %s\n", ans);
                    //printf("params[0]: %s \n", params[0]);
                    resultCode = R_GUESS_VALID;
                    strcpy(msg, M_GUESS_VALID);
                    msg[0] = ANum;
                    msg[2] = BNum;
                    //printf("%s", msg);
                    *inGamePtr += 1;
                    return;
                }
            }else{
                if(!strcmp(params[0], ans)){
                    //printf("ans: %s\n", ans);
                    //printf("params[0]: %s \n", params[0]);
                    resultCode = R_GUESS_WIN;
                    strcpy(msg, M_GUESS_WIN);
                    //printf("%s", msg);
                    *inGamePtr = 0;
                    return;
                }else{
                    //printf("ans: %s\n", ans);
                    //printf("params[0]: %s \n", params[0]);
                    //printf("Game Lost !!!!!\n");
                    resultCode = R_GUESS_LOSE;
                    strcpy(msg, M_GUESS_LOSE);
                    //printf("%s", msg);
                    *inGamePtr = 0;
                    return;
                }

            }
        }

    }else if(!strcmp(params[0], "register")){
        
        comType = C_REGISTER;
        //printf("Entered register command handling...\n");
        if(nParams != 4){
            //printf("Wrong number of params!\n");
            resultCode = R_REGISTER_ERR_USAGE;
            strcpy(msg, M_REGISTER_ERR_USAGE);
            //printf("%s\n", msg);
        }else{
            int regRet;
            // Check if user and email already exist in database

            regRet = myRegister(params[1], params[2], params[3]);

            if(regRet == R_REGISTER_ERR_USR){
                resultCode = R_REGISTER_ERR_USR;
                strcpy(msg, M_REGISTER_ERR_USR);
            }else if(regRet == R_REGISTER_ERR_EMAIL){
                resultCode = R_REGISTER_ERR_EMAIL;
                strcpy(msg, M_REGISTER_ERR_EMAIL);
            }else if(regRet == R_REGISTER_SUCCESS){
                resultCode = R_REGISTER_SUCCESS;
                strcpy(msg, M_REGISTER_SUCCESS);
            }
        }

    }else if(!strcmp(params[0], "login")){

        comType = C_LOGIN;

        if(nParams != 3){
            resultCode = R_LOGIN_ERR_USAGE;
            strcpy(msg, M_LOGIN_ERR_USAGE);
        }else{
            int logRet;
            // Check if user in DB
            
            logRet = login(params[1], params[2]);
            
            if(*loggedInPtr != 0){
                resultCode = R_LOGIN_ERR_LOGOUT;
                strcpy(msg, M_LOGIN_ERR_LOGOUT);
            }else if(logRet == R_LOGIN_ERR_USR){
                resultCode = R_LOGIN_ERR_USR;
                strcpy(msg, M_LOGIN_ERR_USR);
            }else if(logRet == R_LOGIN_ERR_PASSWORD){
                resultCode = R_LOGIN_ERR_PASSWORD;
                strcpy(msg, M_LOGIN_ERR_PASSWORD);
            }else{
                resultCode = R_LOGIN_SUCCESS;
                *loggedInPtr = 1;
                strcpy(userName, params[1]);
                strcpy(msg, M_LOGIN_SUCCESS);
                strcat(msg, userName);
                strcat(msg, ".\n");
            }
        }


    }else if(!strcmp(params[0], "logout")){
        
        comType = C_LOGOUT;
        
        if(nParams != 1){
            resultCode = R_LOGOUT_ERR_USAGE;
            strcpy(msg, M_LOGOUT_ERR_USAGE);
        }else if(*loggedInPtr == 0){
            resultCode = R_LOGOUT_ERR_LOGIN;
            strcpy(msg, M_LOGOUT_ERR_LOGIN);
        }else{
            resultCode = R_LOGOUT_SUCCESS;
            strcpy(msg, M_LOGOUT_SUCCESS);
            strcat(msg, userName);
            strcat(msg, ".\n");
            *loggedInPtr = 0;
            bzero(userName, sizeof(userName));
        }

    }else if(!strcmp(params[0], "game-rule")){
        
        comType = C_GAMERULE;

        if(nParams != 1){
            resultCode = R_GAMERULE_ERR_USAGE;
            strcpy(msg, M_GAMERULE_ERR_USAGE);
        }else{
            resultCode = R_GAMERULE_SUCCESS;
            strcpy(msg, M_GAMERULE_SUCCESS);
        }

    }else if(!strcmp(params[0], "start-game")){

        comType = C_STARTGAME;
        if(*loggedInPtr == 0){
            resultCode = R_STARTGAME_ERR_LOGIN;
            strcpy(msg, M_STARTGAME_ERR_LOGIN);
        }else if(nParams == 1){
            // Generate answer with a random 4-digit number and start game
            char randomAns[5];
            bzero(randomAns, sizeof(randomAns));
            srand(time(NULL));
            for(int it=0; it<4; it++){
                int min = 48;
                int max = 57;
                int x = rand() % (max - min + 1) + min;
                randomAns[it] = x;
            }
            randomAns[4] = 0;
            bzero(ans, sizeof(ans));
            resultCode = R_STARTGAME_SUCCESS;
            strcpy(msg, M_STARTGAME_SUCCESS);
            strcpy(ans, randomAns);
            *inGamePtr = 1;

        }else if(nParams == 2){
            // Generate answer based on the params[1] and start game
            // Check if second parameter is 4 digit number
            int validNum = 1;   // 1 if valid, 0 if not
            
            for(int it=0; it<4; it++){
                if(params[1][it] < 48 || params[1][it] > 57){
                    validNum = 0;
                }
            }

            if(!validNum){
                resultCode = R_STARTGAME_ERR_USAGE;
                strcpy(msg, M_STARTGAME_ERR_USAGE);
            }else{
                resultCode = R_STARTGAME_SUCCESS_RIGGED;
                bzero(ans, sizeof(ans));
                strcpy(msg, M_STARTGAME_SUCCESS);
                strcpy(ans, params[1]);
                *inGamePtr = 1;
            }

        }else{
            resultCode = R_STARTGAME_ERR_USAGE;
            strcpy(msg, M_STARTGAME_ERR_USAGE);
        }

    }else if(!strcmp(params[0], "exit")){
        if(nParams != 1){
            resultCode = R_EXIT_ERR_USAGE;
            strcpy(msg, M_EXIT_ERR_USAGE);
        }else{
            resultCode = R_EXIT_SUCCESS;
            strcpy(msg, M_EXIT_SUCCESS);
        }

    }else{
        comType = C_INVALID;
        resultCode = R_INVALID_COMMAND;
        strcpy(msg, M_INVALID_COMMAND);
    }
    strcat(msg, "\0");

}


void* routine(void* param){
    char msg[MAXLINE];
    char params[5][MAXLINE];
    int nParams;
    int loggedIn = 0;  // 1 if logged in, 0 if not  
    int inGame = 0;    // 1 or above if in game, 0 if not, indicates number of guesses made if > 0
    char ans[5] = "1234";
    char userName[MAXLINE];

    struct routineParams* paramPtr = param;

    strcpy(msg, "*****Welcome to Game 1A2B*****\n");
    write(paramPtr->clientSocketFd, msg, strlen(msg)+1);

    // TCP server
    // TCP echo
    while(1){

        //for(int it = 0; it<5; it++){
        //    bzero(params[it], sizeof(params[it]));
        //}

        bzero(msg, sizeof(msg));
        read(paramPtr->clientSocketFd, msg, sizeof(msg));

        if(!strcmp(msg, "exit\n")){
            // Logout
            loggedIn = 0;
            bzero(userName, sizeof(userName));
            break;
        }

        nParams = splitParams(msg, params);
        //printf("number of params: %d\n", nParams);
        /*for(int it=0; it<nParams; it++){
            printf("%s\n", params[it]);
        }*/
        bzero(msg, sizeof(msg));
        handleCommand(msg, nParams, params, &loggedIn, &inGame, ans, userName);
        //printf("Message after funcion ret: %s", msg);
        write(paramPtr->clientSocketFd, msg, sizeof(msg));
        //printf("Sent: \"%s\"\n", msg);
    }

    close(paramPtr->clientSocketFd);
    printf("tcp get msg: exit\n");
    threadEnded[paramPtr->availableThreadIdx] = 1;
}


void* udpProcessAndSendRoutine(void* param){
    char msg[MAXLINE];
    int nParams;
    char params[5][MAXLINE];
    struct udpRoutineParams* paramPtr = param;
    int iDummy = 0;
    char cDummy[MAXLINE] = "asfdiuf";
    int sentSize = 0;

    //printf("message handling thread entered...\n");
    strcpy(msg, paramPtr->msg);
    nParams = splitParams(msg, params);
    bzero(msg, sizeof(msg));
    handleCommand(msg, nParams, params, &iDummy, &iDummy, cDummy, cDummy);
    //printf("check2\n");
    //printf("uServerSockFd: %d\n", paramPtr->uServerSocketFd);
    sentSize = sendto(paramPtr->uServerSocketFd, msg, strlen(msg) + 1, 0, (struct sockaddr*)&(paramPtr->clientAddress), paramPtr->clientAddressLength);
    //printf("sent to client: %s\n", msg);
    //printf("sent size: %d\n", sentSize);
    threadEnded[paramPtr->availableThreadIdx] = 1;
}


void* udpRoutine(void* param){
    int uServerSocketFd;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int reuseOption = 1;
    int rMsgLength;
    char msg[MAXLINE];
    char passMsg[MAXLINE];
    struct udpTopParams* paramPtr = param;

    serverAddress = paramPtr->serverAddress;

    //printf("Udp top routine entered...\n");

    // UDP socket creation
    uServerSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(uServerSocketFd == -1){
        printf("Failed to create UDP socket\n");
        exit(1);
    }else{
        //printf("socket : %d\n", uServerSocketFd);
    }
    setsockopt(uServerSocketFd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseOption, sizeof(reuseOption));
    
    // UDP binding
    if(bind(uServerSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0){
        printf("Failed to bind\n");
        exit(1);
    }else{
        printf("UDP server is running\n");
    }
    while(1){
        bzero(&clientAddress, sizeof(clientAddress));
        rMsgLength = recvfrom(uServerSocketFd, msg, sizeof(msg),0, (struct sockaddr*)&clientAddress, &clientAddressLength);
        //printf("cliAddressLength: %d\n", clientAddressLength);
        msg[rMsgLength] = '\0';
        char buf[MAXLINE];
        inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, buf, sizeof(buf));
        //printf("%s\n", buf);
        strcpy(passMsg, msg);
        for(int i=0; i<100; i++){
            if(threadAvailable[i] == 1){ 
                threadAvailable[i] = 0;
                uParamsArr[i].clientAddress = clientAddress;
                uParamsArr[i].clientAddressLength = clientAddressLength;
                strcpy(uParamsArr[i].msg, passMsg);
                //printf("check\n");
                uParamsArr[i].uServerSocketFd = uServerSocketFd;
                pthread_create(&newThread[i], NULL, &udpProcessAndSendRoutine, &uParamsArr[i]);
                break;
            }
        }
    }
    
}

void* recycleThreads(){
    // Join terminated threads
    for(int k=0; k<100; k++){
        if(threadEnded[k] == 1){
            threadEnded[k] = 0;
            threadAvailable[k] = 1;
            // pthread_join
            pthread_join(newThread[k], NULL);
        }
    }
}



int main(int argc, char* argv[]){

    int serverSocketFd, clientSocketFd;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int reuseOption = 1;

    for(int i=0; i<100; i++){
        threadAvailable[i] = 1;
    }

    // Set server address
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddress.sin_port = htons(atoi(argv[1]));

    // Open user file
    if(fopen("userFile.txt", "rw+") == NULL){
        // Do task when file does not exists
        userFilePtr = fopen("userFile.txt", "w+");
    }
    // Do task when file does exists
    userFilePtr = fopen("userFile.txt", "rw+");

    // Thread recycling thread
    pthread_create(&recyclingThread, NULL, &recycleThreads, NULL);


    // UDP recv thread
    udpTopParam.serverAddress = serverAddress;
    pthread_create(&newThread[0], NULL, &udpRoutine, &udpTopParam);
    threadAvailable[0] = 0;


    // TCP socket creation
    serverSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocketFd == -1){
        printf("Failed to create socket\n");
        exit(1);
    }else{
        //printf("socket : %d\n", serverSocketFd);
    }
    setsockopt(serverSocketFd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseOption, sizeof(reuseOption));

    // TCP socket binding
    if(bind(serverSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0){
        printf("Failed to bind\n");
        exit(1);
    }else{
        //printf("bound\n");
    }


    // TCP listening
    if (listen(serverSocketFd, 100) != 0){
        printf("Failed to listen\n");
        exit(1);
    }else{
        printf("TCP server is running\n");
    }

    // TCP accepting
    while(1){
        bzero(&clientAddress, sizeof(clientAddress));
        clientSocketFd = accept(serverSocketFd, (struct sockaddr*)&clientAddress, &clientAddressLength);
        //printf("%d\n", clientAddressLength);
        char buf[MAXLINE];
        inet_ntop(AF_INET, &clientAddress.sin_addr.s_addr, buf, sizeof(buf));
        //printf("clientAddress: %s\n", buf);
        if(clientSocketFd == -1){
            printf("Failed to accept client\n");
            continue;
        }else{
            printf("New connection.\n");
            // pthread_create
            // Find an available thread
            for(int i=0; i<100; i++){
                if(threadAvailable[i] == 1){
                    threadAvailable[i] = 0;
                    clientSocketFds[i] = clientSocketFd;
                    rParamsArr[i].clientSocketFd = clientSocketFd;
                    rParamsArr[i].availableThreadIdx = i;
                    pthread_create(&newThread[i], NULL, &routine, &rParamsArr[i]);
                    break;
                }
            }   
        }

    }

    //close
    close(clientSocketFd);
    printf("Client socket closed\n");
    close(serverSocketFd);
    printf("Server socket closed\n");
    return 0;
}



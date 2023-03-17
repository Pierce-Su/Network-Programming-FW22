#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentialsProvider.h>


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

//using namespace Aws;


// Define my server number
#define MY_SERV_NO 2


// Definition of constant values
#define MAXLINE 1024
#define MAX_THREAD_NUM 200
#define MAX_INVITATIONS 1000
#define MAX_PLAYERS 100
#define MAX_USERS 1000
#define MAX_ROOMS 100


// Definition of command types 
#define C_REGISTER 0
#define C_LOGIN 1
#define C_LOGOUT 2
#define C_GAMERULE 3
#define C_CREATE_PUBLIC_ROOM 4
#define C_CREATE_PRIVATE_ROOM 5
#define C_LIST_ROOMS 6
#define C_LIST_USERS 7
#define C_JOIN_ROOM 8
#define C_INVITE 9
#define C_LIST_INVITATIONS 10
#define C_ACCEPT 11
#define C_LEAVE_ROOM 12
#define C_START_GAME 13
#define C_GUESS 14
#define C_EXIT 15
#define C_STATUS 16
#define C_INVALID 17


// Definition of command results
#define R_REGISTER_SUCCESS 0
#define R_REGISTER_ERROR_USERNAME_EMAIL 1
#define R_REGISTER_ERROR_USAGE 2

#define R_LOGIN_SUCCESS 0
#define R_LOGIN_ERROR_USERNAME 1
#define R_LOGIN_ERROR_LOGGED_IN_SELF 2
#define R_LOGIN_ERROR_LOGGED_IN_OTHER 3
#define R_LOGIN_ERROR_PASSWORD 4
#define R_LOGIN_ERROR_USAGE 5

#define R_LOGOUT_SUCCESS 0
#define R_LOGOUT_ERROR_LOGGED_OUT 1
#define R_LOGOUT_ERROR_IN_ROOM 2
#define R_LOGOUT_ERROR_USAGE 3

#define R_CREATE_PUBLIC_ROOM_SUCCESS 0
#define R_CREATE_PUBLIC_ROOM_ERROR_LOGGED_OUT 1
#define R_CREATE_PUBLIC_ROOM_ERROR_IN_ROOM 2
#define R_CREATE_PUBLIC_ROOM_ERROR_ID_USED 3
#define R_CREATE_PUBLIC_ROOM_ERROR_USAGE 4

#define R_CREATE_PRIVATE_ROOM_SUCCESS 0
#define R_CREATE_PRIVATE_ROOM_ERROR_LOGGED_OUT 1
#define R_CREATE_PRIVATE_ROOM_ERROR_IN_ROOM 2
#define R_CREATE_PRIVATE_ROOM_ERROR_ID_USED 3
#define R_CREATE_PRIVATE_ROOM_ERROR_USAGE 4

#define R_LIST_ROOMS_SUCCESS_NO_ROOM 0
#define R_LIST_ROOMS_SUCCESS_EXIST_ROOMS 1
#define R_LIST_ROOMS_ERROR_USAGE 2

#define R_LIST_USERS_SUCCESS_NO_USER 0
#define R_LIST_USERS_SUCCESS_EXIST_USERS 1
#define R_LIST_USERS_ERROR_USAGE 2

#define R_JOIN_ROOM_SUCCESS 0
#define R_JOIN_ROOM_ERROR_LOGGED_OUT 1
#define R_JOIN_ROOM_ERROR_IN_ROOM 2
#define R_JOIN_ROOM_ERROR_INVALID_ID 3
#define R_JOIN_ROOM_ERROR_PRIVATE 4
#define R_JOIN_ROOM_ERROR_USAGE 5
#define R_JOIN_ROOM_ERROR_IN_GAME 6

#define R_INVITE_SUCCESS 0
#define R_INVITE_ERROR_LOGGED_OUT 1
#define R_INVITE_ERROR_NOT_IN_ROOM 2
#define R_INVITE_ERROR_NOT_ROOM_ADMIN 3
#define R_INVITE_ERROR_INVITEE_NOT_LOGGED_IN 4
#define R_INVITE_ERROR_USAGE 5

#define R_LIST_INVITATIONS_SUCCESS_NO_INVITATION 0
#define R_LIST_INVITATIONS_SUCCESS_EXIST_INVITATIONS 1
#define R_LIST_INVITATIONS_ERROR_USAGE 2
#define R_LIST_INVITATIONS_ERROR_LOGGED_OUT 3

#define R_ACCEPT_SUCCESS 0
#define R_ACCEPT_ERROR_LOGGED_OUT 1
#define R_ACCEPT_ERROR_IN_ROOM 2
#define R_ACCEPT_ERROR_INVALID_INVITATION 3
#define R_ACCEPT_ERROR_WRONG_CODE 4
#define R_ACCEPT_ERROR_GAME_STARTED 5
#define R_ACCEPT_ERROR_USAGE 6

#define R_LEAVE_ROOM_SUCCESS_ADMIN 0
#define R_LEAVE_ROOM_SUCCESS_MEMBER_IN_GAME 1
#define R_LEAVE_ROOM_SUCCESS_MEMBER_NOT_IN_GAME 2
#define R_LEAVE_ROOM_ERROR_LOGGED_OUT 3
#define R_LEAVE_ROOM_ERROR_NOT_IN_ROOM 4

#define R_START_GAME_SUCCESS 0
#define R_START_GAME_ERROR_LOGGED_OUT 1
#define R_START_GAME_ERROR_NOT_IN_ROOM 2
#define R_START_GAME_ERROR_INVALID_GUESS_NUMBER 3
#define R_START_GAME_ERROR_USAGE 4
#define R_START_GAME_ERROR_NOT_MANAGER 5
#define R_START_GAME_ERROR_IN_GAME 6

#define R_GUESS_SUCCESS_WRONG 0
#define R_GUESS_SUCCCESS_GAME_OVER 1
#define R_GUESS_SUCCESS_BINGO 2
#define R_GUESS_ERROR_INVALID_GUESS 3
#define R_GUESS_ERROR_USAGE 4
#define R_GUESS_ERROR_LOGGED_OUT 5
#define R_GUESS_ERROR_NOT_IN_ROOM 6
#define R_GUESS_ERROR_NOT_IN_GAME_IS_MANAGER 7
#define R_GUESS_ERROR_NOT_IN_GAME_NOT_MANAGER 8
#define R_GUESS_ERROR_NOT_MY_TURN 9

#define R_STATUS_SUCCESS 0
#define R_STATUS_ERROR 1

#define R_EXIT_SUCCESS 0
#define R_EXIT_ERROR_USAGE 1

#define R_INVALID_COMMAND -1


const int my_serv_no = MY_SERV_NO;
// const Aws::String myBucketName = "genghisswan";
// const Aws::String myObjectName = "server_status02.txt";
// const Aws::String otherObjectName_A = "server_status01.txt";
// const Aws::String otherObjectName_B = "server_status03.txt";
// const std::string myObjectContent = "0\n";
// Aws::S3::S3Client s3Client;

// 加入 Mutex
pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;


// Definition of messages according to command results
const char* M_REGISTER_SUCCESS = "Register Successfully\n";
const char* M_REGISTER_ERROR_USERNAME_EMAIL = "Username or Email is already used\n";
const char* M_REGISTER_ERROR_USAGE = "Usage: register <username> <email> <user password>\n";

const char* M_LOGIN_SUCCESS = "Welcome, ";      // "Welcome, <username>\n"
const char* M_LOGIN_ERROR_USERNAME = "Username does not exist\n";
const char* M_LOGIN_ERROR_LOGGED_IN_SELF = "You already logged in as ";     // "You already logged in as <username>\n"
const char* M_LOGIN_ERROR_LOGGED_IN_OTHER = "Someone already logged in as ";        // "Someone already logged in as <username>\n"
const char* M_LOGIN_ERROR_PASSWORD = "Wrong password\n";
const char* M_LOGIN_ERROR_USAGE = "Usage: login <username> <password>\n";

const char* M_LOGOUT_SUCCESS = "Goodbye, ";     // "Goodbye, <username>\n"
const char* M_LOGOUT_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_LOGOUT_ERROR_IN_ROOM = "You are already in game room ";   // "You are already in game room <game room id>, please leave game room\n"
const char* M_LOGOUT_ERROR_USAGE = "Usage: logout\n";

const char* M_CREATE_PUBLIC_ROOM_SUCCESS  = "You create public game room ";     // "You create public game room <game room id>\n"
const char* M_CREATE_PUBLIC_ROOM_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_CREATE_PUBLIC_ROOM_ERROR_IN_ROOM = "You are already in game room ";    // "You are already in game room <game room id>, please leave game room\n"
const char* M_CREATE_PUBLIC_ROOM_ERROR_ID_USED = "Game room ID is used, choose another one\n";
const char* M_CREATE_PUBLIC_ROOM_ERROR_USAGE = "Usage: create public room <game room id>\n";        

const char* M_CREATE_PRIVATE_ROOM_SUCCESS = "You create private game room ";        // "You create private game room <game room id>\n"
const char* M_CREATE_PRIVATE_ROOM_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_CREATE_PRIVATE_ROOM_ERROR_IN_ROOM = "You are already in game room ";      // "You are already in game room <game room id>, please leave game room\n"
const char* M_CREATE_PRIVATE_ROOM_ERROR_ID_USED = "Game room ID is used, choose another one\n";
const char* M_CREATE_PRIVATE_ROOM_ERROR_USAGE = "Usage: create private room <game room id>\n";        

const char* M_LIST_ROOMS_SUCCESS_NO_ROOM = "List Game Rooms\nNo Rooms\n";
const char* M_LIST_ROOMS_SUCCESS_EXIST_ROOMS = "List Game Rooms\n";     // "List Game Rooms\n<game room id>. (<Public or Private>) Game Room <game room id> <has started playing\n or is open for players\n>"
const char* M_LIST_ROOMS_ERROR_USAGE = "Usage: list rooms\n";     

const char* M_LIST_USERS_SUCCESS_NO_USER = "List Users\nNo Users\n";
const char* M_LIST_USERS_SUCCESS_EXIST_USERS = "List Users\n";      // "List Users\n<alphabetic order>. <username><email> <online status>\n"
const char* M_LIST_USERS_ERROR_USAGE = "Usage: list users\n";

const char* M_JOIN_ROOM_SUCCESS_SELF = "You join game room ";       // "You join game room <game room id>\n"
const char* M_JOIN_ROOM_SUCCESS_BROADCAST = "Welcome, ";        // "Welcome, <username> to game\n"
const char* M_JOIN_ROOM_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_JOIN_ROOM_ERROR_IN_ROOM = "You are already in game room ";    // "You are already in game room <game room id>, please leave game room\n"
const char* M_JOIN_ROOM_ERROR_INVALID_ID = "Game room ";    // "Game room <game room id> is not exist\n"
const char* M_JOIN_ROOM_ERROR_PRIVATE = "Game room is private, please join game by invitation code\n";
const char* M_JOIN_ROOM_ERROR_USAGE = "Usage: join room <game room id>\n";
const char* M_JOIN_ROOM_ERROR_IN_GAME = "Game has started, you can't join now\n";

const char* M_INVITE_SUCCESS_SELF = "You send invitation to ";      // "You send invitation to <invitee name><<invitee email>>\n"
const char* M_INVITE_SUCCESS_RECEIVER = "You receive invitation from ";     // "You receive invitation from <inviter name><<inviter email>>\n"
const char* M_INVITE_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_INVITE_ERROR_NOT_IN_ROOM = "You did not join any game room\n";
const char* M_INVITE_ERROR_NOT_ROOM_ADMIN = "You are not private game room manager\n";
const char* M_INVITE_ERROR_INVITEE_NOT_LOGGED_IN = "Invitee not logged in\n";
const char* M_INVITE_ERROR_USAGE = "Usage: invite <invitee email>\n";

const char* M_LIST_INVITATIONS_SUCCESS_NO_INVITATION = "List invitations\nNo Invitations\n";
const char* M_LIST_INVITATIONS_SUCCESS_EXIST_INVITATIONS = "List invitations\n";        // "List invitations\n<room id order>. <inviter name><<inviter email>> invite you to join game room <game room id>, invitation code is <invitation code>\n"
const char* M_LIST_INVITATIONS_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_LIST_INVITATIONS_ERROR_USAGE = "Usage: list invitations\n";

const char* M_ACCEPT_SUCCESS_SELF = "You join game room ";      // "You join game room <game room id>\n"
const char* M_ACCEPT_SUCCESS_BROADCAST = "Welcome, ";       // "Welcome, <username> to game\n"
const char* M_ACCEPT_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_ACCEPT_ERROR_IN_ROOM = "You are already in game room ";       // "You are already in game room <game room id>, please leave game room\n"
const char* M_ACCEPT_ERROR_INVALID_INVITATION = "Invitation not exist\n";
const char* M_ACCEPT_ERROR_WRONG_CODE = "Your invitation code is incorrect\n";
const char* M_ACCEPT_ERROR_GAME_STARTED = "Game has started, you can't join now\n";
const char* M_ACCEPT_ERROR_USAGE = "Usage: accept <invitee email> <invitation code>\n";

const char* M_LEAVE_ROOM_SUCCESS_ADMIN_SELF = "You leave game room ";       // "You leave game room <game room id>\n"
const char* M_LEAVE_ROOM_SUCCESS_ADMIN_BROADCAST = "Game room manager leave game room ";        // "Game room manager leave game room <game room id>, you are forced to leave too\n"
const char* M_LEAVE_ROOM_SUCCESS_MEMBER_IN_GAME_SELF = "You leave game room ";        // "You leave game room <game room id>, game ends\n"
const char* M_LEAVE_ROOM_SUCCESS_MEMBER_IN_GAME_BROADCAST = "";     // "<username> leave game room <game room id>, game ends\n"
const char* M_LEAVE_ROOM_SUCCESS_MEMBER_NOT_IN_GAME_SELF = "You leave game room ";       // "You leave game room <game room id>, game ends\n"
const char* M_LEAVE_ROOM_SUCCESS_MEMBER_NOT_IN_GAME_BROADCAST = "";     // "<username> leave game room <game room id>\n"
const char* M_LEAVE_ROOM_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_LEAVE_ROOM_ERROR_NOT_IN_ROOM = "You did not join any game room\n";

// M_START_GAME_SUCCESS self or broadcast?
const char* M_START_GAME_SUCCESS = "Game start! Current player is ";        // "Game start! Current player is <current player name>\n"
const char* M_START_GAME_ERROR_LOGGED_OUT = "You are not logged in\n";      
const char* M_START_GAME_ERROR_NOT_IN_ROOM = "You did not join any game room\n";
const char* M_START_GAME_ERROR_INVALID_GUESS_NUMBER = "Please enter 4 digit number with leading zero\n";
const char* M_START_GAME_ERROR_USAGE = "Usage: start game <number of rounds> <guess number>\n";
const char* M_START_GAME_ERROR_NOT_MANAGER = "You are not game room manager, you can't start game\n";
const char* M_START_GAME_ERROR_IN_GAME = "Game has started, you can't start again\n";

const char* M_GUESS_SUCCESS_WRONG = "";     // "<current player name> guess '<guess number>' and got '<guess result>'\n"
const char* M_GUESS_SUCCCESS_GAME_OVER = "";        // "<current player name> guess '<guess number>' and got '<guess result>'\nGame ends, no one wins\n"
const char* M_GUESS_SUCCESS_BINGO = "";     // "<current player name> guess '<guess number>' and got Bingo!!! <current player name> wins the game, game ends\n"
const char* M_GUESS_ERROR_INVALID_GUESS_NUMBER = "Please enter 4 digit number with leading zero\n";
const char* M_GUESS_ERROR_USAGE = "Usage: guess <guess number>\n";
const char* M_GUESS_ERROR_LOGGED_OUT = "You are not logged in\n";
const char* M_GUESS_ERROR_NOT_IN_ROOM = "You did not join any game room\n";
const char* M_GUESS_ERROR_NOT_IN_GAME_IS_MANAGER = "You are game room manager, please start game first\n";
const char* M_GUESS_ERROR_NOT_IN_GAME_NOT_MANAGER = "Game has not started yet\n";
const char* M_GUESS_ERROR_NOT_MY_TURN = "Please wait..., current player is ";

const char* M_EXIT_SUCCESS = "";
const char* M_EXIT_ERROR_USAGE = "Usage: exit\n";

const char* M_INVALID_COMMAND = "Invalid command\n";


// Structure definitions
// Two usages: to describe a connection's user and to describe a regiestered user
struct userInfo{            
    
    char userName[MAXLINE];     // Default: bzero
    char email[MAXLINE];        // Default: bzero
    char password[MAXLINE];     // Default: bzero
    int isLoggedIn;             // Default: -1 (-1 if logged out, 1 if logged in)
    int clinetSocketFd;         // Default: -1 (Records which file descriptor connection is logged in as this user)
    int in_room;                 // Default: -1 (-1 if not in room, Num if in roomArr[Num])

};

struct roomInfo{

    struct userInfo* adminUserInfoPtr;              // Default: NULL (Pointer to admin user)
    struct userInfo* playersPtrArr[MAX_PLAYERS];    // Default: array of NULL (Array of pointers to userInfo)
    int nPlayers;                                   // Default: 0 (Number of players in room)
    uint32_t roomId;                                // Default: 0
    uint32_t invitationCode;                        // Default: 0
    int isPublic;                                   // Default: 1 (-1 if private, 1 if public)
    int inGame;                                     // Default: -1 (-1 if not in game, 1 if in game)
    char answer[5];                                 // Default: bzero
    int isActive;                                   // Default: -1 (Set to 1 if room is created, set to -1 if room is abandoned)
    int curGuesserArrIdx;                           // Default: 0 (Increments as players guess)
    int rounds;                                     // Default: 0
    int curRound;                                   // Default: 0

};

struct invitationInfo{

    struct userInfo* inviterPtr;    // Default: NULL (Pointer to inviter user)
    struct userInfo* inviteePtr;    // Default: NULL (Pointer to invitee user)
    uint32_t invitationCode;        // Default: 0 
    int roomArrIdx;                 // Default: -1
    int isValid;                    // Default: -1 (Set to 1 if invitation is made, set to -1 if room is abandoned)    

};

struct  tcpConnectionInfo{
    
    int clientSocketFd;                         // Default: The fd of this connection
    struct userInfo* tcpConnectionUserInfoPtr;  // Default: NULL (Pointer to user that this connection is logged in as)

};


// Thread routine params
struct tcpClientRoutineParams{

    int tcpClientSocketFd;  // The fd of this connection
    int threadIdx;          // Index of the thread that is being used in the thread pool

};

struct udpMasterRoutineParams{

    struct sockaddr_in serverAddress;   // The address info of this server

};

struct udpSlaveRoutineParams{

    int udpServerSocketFd;              // The fd of the UDP server
    struct sockaddr_in clientAddress;   // The address of the client to send a message to
    socklen_t clientAddressLength;      // The length of the client address
    int threadIdx;                      // Index of the thread that is being used in the thread pool
    char msg[MAXLINE];                  // Message from the UDP client

};


// mutex locks
pthread_rwlock_t usersArrLock;
pthread_rwlock_t invitationsArrLock;
pthread_rwlock_t roomsArrLock;


// Thread related variables
pthread_t threadPool[MAX_THREAD_NUM];
pthread_t udpMasterThread;
pthread_t recyclerThread;


// Description of thread availability
int threadAvailable[MAX_THREAD_NUM];
int threadEnded[MAX_THREAD_NUM] = {0};


// Thread routine parameters based on thread utility
struct tcpClientRoutineParams tcpClientParamsArr[MAX_THREAD_NUM];
struct udpMasterRoutineParams udpMasterParams;
struct udpSlaveRoutineParams udpSlaveParamsArr[MAX_THREAD_NUM];


// Users (Static since there is no way to unregister a user)
int userCount = 0;
struct userInfo usersArr[MAX_USERS];


// Rooms (Dynamic since rooms can be removed)
int roomCount = 0;
struct roomInfo roomsArr[MAX_ROOMS];


// Invitations (Dynamic since invitations can be removed?)
int invitationCount = 0;
struct invitationInfo invitationArr[MAX_INVITATIONS];


// Function definitions
int min(int a, int b){
    if(a >= b){
        return b;
    }else{
        return a;
    }
}

int splitParams(char* msg, char params[][MAXLINE]){
    char cpyMsg[MAXLINE];
    char* ptr;
    const char* delim = " \n";
    int nParams = 0; 

    strcpy(cpyMsg, msg);
    ptr = strtok(cpyMsg, delim);
    while(ptr != NULL){
        strcpy(params[nParams], ptr);
        strcat(params[nParams], "\0");
        nParams++;
        ptr = strtok(NULL, delim);
    }
    return nParams;
}

// This function returns 10*A + B
int getAB(char* ans, char* guess){
    int A=0, B=0;
    int posCounted[4] = {0};  // If posCounted = 1: A if posCounted = 2: this pos in B is counted

    for(int it=0; it<4; it++){
        if(ans[it] == guess[it]){
            posCounted[it] = 1;
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
                B++;
                break;
            }
        }
    }
    return 10*A+B;
}

void modifyS3File(int add){


    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {

        pthread_mutex_lock(&fileMutex);
        const Aws::String myBucketName = "genghisswan";
        const Aws::String myObjectName = "server_status01.txt";
        const Aws::String otherObjectName_A = "server_status02.txt";
        const Aws::String otherObjectName_B = "server_status03.txt";
        const std::string myObjectContent = "0\n";
        Aws::S3::S3Client s3Client;
        std::ostringstream ss;
        int bytes;
        // 建立從 S3 下載 Object 的請求
        Aws::S3::Model::GetObjectRequest getObjectRequest;


        getObjectRequest.SetBucket(myBucketName);
        getObjectRequest.SetKey(myObjectName);
        

        // 從 S3 下載 Object
        Aws::S3::Model::GetObjectOutcome getObjectOutcome = s3Client.GetObject(getObjectRequest);
        std::streambuf* body;
        char fileContent[MAXLINE];
        if(getObjectOutcome.IsSuccess()) {
        bzero(fileContent, sizeof(fileContent));
        std::cout << "下載成功，資料內容：" << std::endl;
        body = getObjectOutcome.GetResult().GetBody().rdbuf();
        bytes = getObjectOutcome.GetResult().GetContentLength();
        body->sgetn(fileContent, bytes);
        printf("%s", fileContent);
        //std::cout << body << std::endl;
        } else {
        std::cout << "下載失敗：" << getObjectOutcome.GetError().GetExceptionName() << " "
                    << getObjectOutcome.GetError().GetMessage() << std::endl;
        }

        //ss << getObjectOutcome.GetResult().GetBody().rdbuf();
        //bytes = getObjectOutcome.GetResult().GetContentLength();
        //std::streambuf* body = getObjectOutcome.GetResult().GetBody().rdbuf();
        //std::string fileString = ss.str();
        //std::cout << "File string: " << fileString << std::endl;
        //char fileContent[MAXLINE];
        //bzero(fileContent, sizeof(fileContent));
        //std::cout << "body: " << body << std::endl;
        //body->sgetn(fileContent, bytes);
        /*int nParams = 0;
        char fileSplit[3][MAXLINE];
        for(int i=0; i<3; i++){
            bzero(fileSplit[i], sizeof(fileSplit[i]));
        }
        bzero(fileContent, sizeof(fileContent));
        strcpy(fileContent, fileString.c_str());
        printf("fileContent: %s\n", fileContent);
        nParams =  splitParams(fileContent, fileSplit);
        printf("fileSplit0: %s\n", fileSplit[0]);
        bzero(fileContent, sizeof(fileContent));
        strcpy(fileContent, fileSplit[0]);
        printf("fileContent: %s\n", fileContent);*/
        printf("fileContent: %s\n", fileContent);
        
        int curNum = atoi(fileContent);
        if(add > 0){
            curNum = curNum + 1;
        }else{
            curNum = curNum - 1;
        }
        char strNum[MAXLINE];
        bzero(strNum, sizeof(strNum));
        sprintf(strNum, "%d", curNum);
        printf("printed server1: %s\n", strNum);
        strcat(strNum, "\n");

        const std::string upObjectContent(strNum);

        // 建立從 S3 upload Object 的請求
        Aws::S3::Model::PutObjectRequest putObjectRequest;
        putObjectRequest.SetBucket(myBucketName);
        putObjectRequest.SetKey(myObjectName);

        const std::shared_ptr<Aws::IOStream> inputStream = Aws::MakeShared<Aws::StringStream>("");
        *inputStream << upObjectContent.c_str();

        // 設定上傳至 S3 的資料輸入串流
        putObjectRequest.SetBody(inputStream);

        // 將 Object 上傳至 S3
        Aws::S3::Model::PutObjectOutcome putObjectOutcome = s3Client.PutObject(putObjectRequest);

        if(putObjectOutcome.IsSuccess()) {
        std::cout << "上傳成功" << std::endl;
        } else {
        std::cout << "上傳失敗：" << putObjectOutcome.GetError().GetExceptionName()
                    << " " << putObjectOutcome.GetError().GetMessage() << std::endl;
        }
        pthread_mutex_unlock(&fileMutex);

    }
    Aws::ShutdownAPI(options);

    return;

}

void initializeS3File(){

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {   
        const Aws::String myBucketName = "genghisswan";
        const Aws::String myObjectName = "server_status01.txt";
        const Aws::String otherObjectName_A = "server_status02.txt";
        const Aws::String otherObjectName_B = "server_status03.txt";
        const std::string myObjectContent = "0\n";
        Aws::S3::S3Client s3Client;

        // 建立從 S3 upload Object 的請求
        Aws::S3::Model::PutObjectRequest putObjectRequest;
        putObjectRequest.SetBucket(myBucketName);
        putObjectRequest.SetKey(myObjectName);

        const std::shared_ptr<Aws::IOStream> inputStream = Aws::MakeShared<Aws::StringStream>("");
        *inputStream << myObjectContent.c_str();

        // 設定上傳至 S3 的資料輸入串流
        putObjectRequest.SetBody(inputStream);

        // 將 Object 上傳至 S3
        Aws::S3::Model::PutObjectOutcome putObjectOutcome = s3Client.PutObject(putObjectRequest);

        if(putObjectOutcome.IsSuccess()) {
        std::cout << "上傳成功" << std::endl;
        } else {
        std::cout << "上傳失敗：" << putObjectOutcome.GetError().GetExceptionName()
                    << " " << putObjectOutcome.GetError().GetMessage() << std::endl;
        }

    }
    Aws::ShutdownAPI(options);

}

int myRegister(char* userName, char* email, char* password){

    pthread_rwlock_wrlock(&usersArrLock);

    for(int i=0; i<userCount; i++){
        if(!strcmp(usersArr[i].userName, userName) || !strcmp(usersArr[i].email, email)){
            pthread_rwlock_unlock(&usersArrLock);
            return R_REGISTER_ERROR_USERNAME_EMAIL;
        }
    }

    strcpy(usersArr[userCount].userName, userName);
    strcpy(usersArr[userCount].email, email);
    strcpy(usersArr[userCount].password, password);
    usersArr[userCount].clinetSocketFd = -1;
    usersArr[userCount].isLoggedIn = -1;
    userCount++;
    pthread_rwlock_unlock(&usersArrLock);
    return R_REGISTER_SUCCESS;

}

int myLogin(char* username, char* password, struct tcpConnectionInfo* connectionInfoPtr){
    
    int retval = R_LOGIN_ERROR_USERNAME;
    int found = -1;

    pthread_rwlock_wrlock(&usersArrLock);
    for(int i=0; i<MAX_USERS; i++){
        if(!strcmp(usersArr[i].userName, username)){
            found = 1;
            break;
        }
    }
    pthread_rwlock_unlock(&usersArrLock);

    if(found == -1){
        retval = R_LOGIN_ERROR_USERNAME;
        return retval;
    }

    if(connectionInfoPtr->tcpConnectionUserInfoPtr != NULL){
        retval = R_LOGIN_ERROR_LOGGED_IN_SELF;
        return retval;
    }
    
    pthread_rwlock_wrlock(&usersArrLock);
    for(int i=0; i<userCount; i++){
        if(!strcmp(usersArr[i].userName, username)){
            if(!strcmp(usersArr[i].password, password)){
                if(usersArr[i].isLoggedIn > 0){
                    retval = R_LOGIN_ERROR_LOGGED_IN_OTHER;
                    pthread_rwlock_unlock(&usersArrLock);
                    return retval;
                }else{
                    retval = R_LOGIN_SUCCESS;
                    connectionInfoPtr->tcpConnectionUserInfoPtr = &usersArr[i];
                    usersArr[i].clinetSocketFd = connectionInfoPtr->clientSocketFd;
                    usersArr[i].isLoggedIn = 1;
                    usersArr[i].in_room = -1;
                    pthread_rwlock_unlock(&usersArrLock);

                    modifyS3File(1);

                    return retval;
                }
            }else{
                retval = R_LOGIN_ERROR_PASSWORD;
                pthread_rwlock_unlock(&usersArrLock);
                return retval;
            }
        }else{
            continue;
        }
    }
    retval = R_LOGIN_ERROR_USERNAME;
    pthread_rwlock_unlock(&usersArrLock);
    return retval;
}

int myLogout(char* userNameStr, struct tcpConnectionInfo* connectionInfoPtr){
    
    int retval = R_LOGOUT_ERROR_LOGGED_OUT;
    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_LOGOUT_ERROR_LOGGED_OUT;
        return retval;
    }else if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room >= 0){
        retval = R_LOGOUT_ERROR_IN_ROOM;
        return retval;
    }else{
        retval = R_LOGOUT_SUCCESS;
        connectionInfoPtr->tcpConnectionUserInfoPtr->clinetSocketFd = -1;
        connectionInfoPtr->tcpConnectionUserInfoPtr->isLoggedIn = -1;
        connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = -1;
        strcpy(userNameStr, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
        connectionInfoPtr->tcpConnectionUserInfoPtr = NULL;

        modifyS3File(-1);

        return retval;
    }
}

int myCreateRoom(int isPublic, uint32_t roomId, uint32_t invitationCode, struct tcpConnectionInfo* connectionInfoPtr){

    int retval = -1;

    if(isPublic > 0){
        // Create a Public Room
        if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
            retval = R_CREATE_PUBLIC_ROOM_ERROR_LOGGED_OUT;
            return retval;
        }else if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room >= 0){
            retval = R_CREATE_PUBLIC_ROOM_ERROR_IN_ROOM;
            return retval;
        }

        pthread_rwlock_wrlock(&roomsArrLock);
        for(int i=0; i<MAX_ROOMS; i++){
            if(roomsArr[i].roomId == roomId && roomsArr[i].isActive > 0){
                retval = R_CREATE_PUBLIC_ROOM_ERROR_ID_USED;
                pthread_rwlock_unlock(&roomsArrLock);
                return retval;
            }else{
                continue;
            }
        }
        for(int i=0; i<MAX_ROOMS; i++){
            if(roomsArr[i].isActive < 0){
                roomsArr[i].adminUserInfoPtr = connectionInfoPtr->tcpConnectionUserInfoPtr;
                for(int j=0; j<MAX_PLAYERS; j++){
                    roomsArr[i].playersPtrArr[j] = NULL;
                }
                roomsArr[i].playersPtrArr[0] = connectionInfoPtr->tcpConnectionUserInfoPtr;
                roomsArr[i].curGuesserArrIdx = 0;
                roomsArr[i].isPublic = 1;
                roomsArr[i].nPlayers = 1;
                roomsArr[i].roomId = roomId;
                roomsArr[i].invitationCode = invitationCode;
                roomsArr[i].inGame = -1;
                roomsArr[i].isActive = 1;
                roomsArr[i].curRound = 0;
                roomsArr[i].rounds = 0;
                bzero(roomsArr[i].answer, sizeof(roomsArr[i].answer));
                connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = i;
                retval = R_CREATE_PUBLIC_ROOM_SUCCESS;
                pthread_rwlock_unlock(&roomsArrLock);
                return retval;
            }
        }
        retval = -1;
        pthread_rwlock_unlock(&roomsArrLock);
        return retval;
    }else{
        // Create a Private Room
        if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
            retval = R_CREATE_PRIVATE_ROOM_ERROR_LOGGED_OUT;
            return retval;
        }else if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room >= 0){
            retval = R_CREATE_PRIVATE_ROOM_ERROR_IN_ROOM;
            return retval;
        }

        pthread_rwlock_wrlock(&roomsArrLock);
        for(int i=0; i<MAX_ROOMS; i++){
            if(roomsArr[i].roomId == roomId && roomsArr[i].isActive > 0){
                retval = R_CREATE_PRIVATE_ROOM_ERROR_ID_USED;
                pthread_rwlock_unlock(&roomsArrLock);
                return retval;
            }else{
                continue;
            }
        }
        for(int i=0; i<MAX_ROOMS; i++){
            if(roomsArr[i].isActive < 0){
                roomsArr[i].adminUserInfoPtr = connectionInfoPtr->tcpConnectionUserInfoPtr;
                for(int j=0; j<MAX_PLAYERS; j++){
                    roomsArr[i].playersPtrArr[j] = NULL;
                }
                roomsArr[i].playersPtrArr[0] = connectionInfoPtr->tcpConnectionUserInfoPtr;
                roomsArr[i].curGuesserArrIdx = 0;
                roomsArr[i].isPublic = -1;
                roomsArr[i].nPlayers = 1;
                roomsArr[i].roomId = roomId;
                roomsArr[i].invitationCode = invitationCode;
                roomsArr[i].inGame = -1;
                roomsArr[i].isActive = 1;
                roomsArr[i].curRound = 0;
                roomsArr[i].rounds = 0;
                bzero(roomsArr[i].answer, sizeof(roomsArr[i].answer));
                connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = i;
                retval = R_CREATE_PRIVATE_ROOM_SUCCESS;
                pthread_rwlock_unlock(&roomsArrLock);
                return retval;
            }
        }
        retval = -1;
        pthread_rwlock_unlock(&roomsArrLock);
        return retval;
    }
}

int myJoinRoom(uint32_t roomId, struct tcpConnectionInfo* connectionInfoPtr){
    
    int retval = R_JOIN_ROOM_ERROR_INVALID_ID;

    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_JOIN_ROOM_ERROR_LOGGED_OUT;
        return retval;
    }else if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room >= 0){
        retval = R_JOIN_ROOM_ERROR_IN_ROOM;
        return retval;
    }else{
        for(int i=0; i<MAX_ROOMS; i++){
            if(roomId == roomsArr[i].roomId && roomsArr[i].isActive){
                if(roomsArr[i].isPublic < 0){
                    retval = R_JOIN_ROOM_ERROR_PRIVATE;
                    return retval;
                }else if(roomsArr[i].inGame > 0){
                    retval = R_JOIN_ROOM_ERROR_IN_GAME;
                    return retval;
                }else{
                    retval = R_JOIN_ROOM_SUCCESS;
                    if(roomsArr[i].nPlayers == MAX_PLAYERS){
                        retval = -1;
                        return retval;
                    }
                    pthread_rwlock_wrlock(&roomsArrLock);
                    char toOthers[MAXLINE];
                    bzero(toOthers, sizeof(toOthers));
                    strcpy(toOthers, M_ACCEPT_SUCCESS_BROADCAST);
                    strcat(toOthers, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
                    strcat(toOthers, " to game!\n");
                    for(int j=0; j<roomsArr[i].nPlayers; j++){
                        write(roomsArr[i].playersPtrArr[j]->clinetSocketFd, toOthers, strlen(toOthers));
                        //write(roomsArr[i].playersPtrArr[j]->clinetSocketFd, toOthers, sizeof(toOthers));
                    }
                    roomsArr[i].playersPtrArr[roomsArr[i].nPlayers] = connectionInfoPtr->tcpConnectionUserInfoPtr;
                    roomsArr[i].nPlayers++;
                    connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = i;
                    pthread_rwlock_unlock(&roomsArrLock);
                    return retval;
                }
            }
        }
        retval = R_JOIN_ROOM_ERROR_INVALID_ID;
        return retval;
    }
}

int myInvite(char* inviteeUserName, char* inviteeEmail, struct tcpConnectionInfo* connectionInfoPtr){

    int retval = R_INVITE_ERROR_INVITEE_NOT_LOGGED_IN;
    int dup = -1;

    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_INVITE_ERROR_LOGGED_OUT;
        return retval;
    }

    if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room < 0){
        retval = R_INVITE_ERROR_NOT_IN_ROOM;
        return retval;
    }
    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].adminUserInfoPtr != connectionInfoPtr->tcpConnectionUserInfoPtr || roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].isPublic > 0){
        retval = R_INVITE_ERROR_NOT_ROOM_ADMIN;
        return retval;
    }

    for(int i=0; i<MAX_INVITATIONS; i++){
        if(invitationArr[i].isValid > 0 && invitationArr[i].inviterPtr == connectionInfoPtr->tcpConnectionUserInfoPtr && !strcmp(invitationArr[i].inviteePtr->email, inviteeEmail) && connectionInfoPtr->tcpConnectionUserInfoPtr->in_room == invitationArr[i].roomArrIdx){
            dup = 1;
        }
    }

    for(int i=0; i<MAX_USERS; i++){
        if(!strcmp(usersArr[i].email, inviteeEmail)){
            if(usersArr[i].isLoggedIn < 0){
                retval = R_INVITE_ERROR_INVITEE_NOT_LOGGED_IN;
                return retval;
            }else{
                char toInvitee[MAXLINE];
                bzero(toInvitee, sizeof(toInvitee));
                retval = R_INVITE_SUCCESS;
                if(dup == -1){

                    pthread_rwlock_wrlock(&invitationsArrLock);
                    for(int j=0; j<MAX_INVITATIONS; j++){
                        if(invitationArr[j].isValid < 0){
                            invitationArr[j].inviterPtr = connectionInfoPtr->tcpConnectionUserInfoPtr;
                            invitationArr[j].inviteePtr = &usersArr[i];
                            invitationArr[j].invitationCode = roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].invitationCode;
                            invitationArr[j].isValid = 1;
                            invitationArr[j].roomArrIdx = connectionInfoPtr->tcpConnectionUserInfoPtr->in_room;
                            break;
                        }
                    }
                    pthread_rwlock_unlock(&invitationsArrLock);
                }
                strcpy(inviteeUserName, usersArr[i].userName);
                strcpy(toInvitee, "You receive invitation from ");
                strcat(toInvitee, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
                strcat(toInvitee, "<");
                strcat(toInvitee, connectionInfoPtr->tcpConnectionUserInfoPtr->email);
                strcat(toInvitee, ">\n");
                write(usersArr[i].clinetSocketFd, toInvitee, strlen(toInvitee));
                //write(usersArr[i].clinetSocketFd, toInvitee, sizeof(toInvitee));
                return retval;
            }
        }
    }
    retval = R_INVITE_ERROR_INVITEE_NOT_LOGGED_IN;
    return retval;
}

int myAccept(char* inviterEmail, uint32_t invitationCode, struct tcpConnectionInfo* connectionInfoPtr){

    int retval = R_ACCEPT_ERROR_LOGGED_OUT;

    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_ACCEPT_ERROR_LOGGED_OUT;
        return retval;
    }

    if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room >= 0){
        retval = R_ACCEPT_ERROR_IN_ROOM;
        return retval;
    }
    for(int i=0; i<MAX_INVITATIONS; i++){
        if(invitationArr[i].isValid < 0){
            continue;
        }
        if(!strcmp(invitationArr[i].inviterPtr->email, inviterEmail) && invitationArr[i].inviteePtr == connectionInfoPtr->tcpConnectionUserInfoPtr && invitationArr[i].isValid > 0){
            /*if(roomsArr[invitationArr[i].roomArrIdx].isActive < 0){
                invitationArr[i].isValid = -1;
                break;
            }*/
            if(invitationArr[i].invitationCode == invitationCode){
                if(roomsArr[invitationArr[i].roomArrIdx].inGame > 0){
                    retval = R_ACCEPT_ERROR_GAME_STARTED;
                    return retval;
                }else{
                    retval = R_ACCEPT_SUCCESS;
                    char toOthers[MAXLINE];
                    bzero(toOthers, sizeof(toOthers));
                    char myRoomIdStr[MAXLINE];
                    strcpy(toOthers, M_ACCEPT_SUCCESS_BROADCAST);
                    strcat(toOthers, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
                    strcat(toOthers, " to game!\n");
                    for(int j=0; j<roomsArr[invitationArr[i].roomArrIdx].nPlayers; j++){
                        //write(roomsArr[invitationArr[i].roomArrIdx].playersPtrArr[j]->clinetSocketFd, toOthers, sizeof(toOthers));
                        write(roomsArr[invitationArr[i].roomArrIdx].playersPtrArr[j]->clinetSocketFd, toOthers, strlen(toOthers));
                    }
                    pthread_rwlock_wrlock(&roomsArrLock);
                    if(roomsArr[invitationArr[i].roomArrIdx].nPlayers < MAX_PLAYERS){
                        roomsArr[invitationArr[i].roomArrIdx].playersPtrArr[roomsArr[invitationArr[i].roomArrIdx].nPlayers] = connectionInfoPtr->tcpConnectionUserInfoPtr;
                        roomsArr[invitationArr[i].roomArrIdx].nPlayers++;
                        pthread_rwlock_unlock(&roomsArrLock);
                        //invitationArr[i].isValid = -1;      // ??????????????????????????????????????????????????
                        sprintf(myRoomIdStr, "%u", roomsArr[invitationArr[i].roomArrIdx].roomId);
                        connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = invitationArr[i].roomArrIdx;
                        return retval;
                    }else{
                        retval = -1;
                        return retval;
                    }
                }
            }else{
                retval = R_ACCEPT_ERROR_WRONG_CODE;
                return retval;
            }
        }
    }
    retval = R_ACCEPT_ERROR_INVALID_INVITATION;
    return retval;
}

int myLeaveRoom(char* roomIdStr, int calledFromExit, struct tcpConnectionInfo* connectionInfoPtr){

    int retval = R_LEAVE_ROOM_ERROR_LOGGED_OUT;

    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_LEAVE_ROOM_ERROR_LOGGED_OUT;
        return retval;
    }

    if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room < 0){
        retval = R_LEAVE_ROOM_ERROR_NOT_IN_ROOM;
        return retval;
    }

    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].adminUserInfoPtr == connectionInfoPtr->tcpConnectionUserInfoPtr){
        retval = R_LEAVE_ROOM_SUCCESS_ADMIN;
        char broadcastBuffer[MAXLINE];
        bzero(broadcastBuffer, sizeof(broadcastBuffer));
        strcpy(broadcastBuffer, M_LEAVE_ROOM_SUCCESS_ADMIN_BROADCAST);
        sprintf(roomIdStr, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
        strcat(broadcastBuffer, roomIdStr);
        strcat(broadcastBuffer, ", you are forced to leave too\n");
        if(calledFromExit < 0){
            for(int i=0; i<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; i++){
                if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i] != connectionInfoPtr->tcpConnectionUserInfoPtr){
                    roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i]->in_room = -1;
                    write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i]->clinetSocketFd, broadcastBuffer, strlen(broadcastBuffer));
                    //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i]->clinetSocketFd, broadcastBuffer, sizeof(broadcastBuffer));
                }
            }
        }
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].isActive = -1;
        pthread_rwlock_wrlock(&invitationsArrLock);
        for(int i=0; i<MAX_INVITATIONS; i++){
            if(invitationArr[i].roomArrIdx == connectionInfoPtr->tcpConnectionUserInfoPtr->in_room){
                invitationArr[i].isValid = -1;
            }
        }
        connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = -1;
        pthread_rwlock_unlock(&invitationsArrLock);
        return retval;
    }else if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame > 0){
        retval = R_LEAVE_ROOM_SUCCESS_MEMBER_IN_GAME;
        char broadcastBuffer[MAXLINE];
        bzero(broadcastBuffer, sizeof(broadcastBuffer));
        strcpy(broadcastBuffer, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
        strcat(broadcastBuffer, " leave game room ");
        sprintf(roomIdStr, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
        strcat(broadcastBuffer, roomIdStr);
        strcat(broadcastBuffer, ", game ends\n");
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame = -1;
        int k;
        int st = -1;
        pthread_rwlock_wrlock(&roomsArrLock);
        for(k=0; k<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; k++){
            if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k] == connectionInfoPtr->tcpConnectionUserInfoPtr){
                st = k;
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k] = NULL;
            }
            if(st > 0 && k < (roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers-1) && k >= st){
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k] = roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k+1];
            }
        }
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers-1] = NULL;
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers -= 1;
        pthread_rwlock_unlock(&roomsArrLock);
        if(calledFromExit < 0){
            for(int idx=0; idx<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; idx++){
                //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[idx]->clinetSocketFd, broadcastBuffer, sizeof(broadcastBuffer));
                write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[idx]->clinetSocketFd, broadcastBuffer, strlen(broadcastBuffer));
            }
        }
        connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = -1;
        return retval;
    }else{
        retval = R_LEAVE_ROOM_SUCCESS_MEMBER_NOT_IN_GAME;
        char broadcastBuffer[MAXLINE];
        bzero(broadcastBuffer, sizeof(broadcastBuffer));
        strcpy(broadcastBuffer, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
        strcat(broadcastBuffer, " leave game room ");
        sprintf(roomIdStr, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
        strcat(broadcastBuffer, roomIdStr);
        strcat(broadcastBuffer, "\n");
        int k;
        int st = -1;
        pthread_rwlock_wrlock(&roomsArrLock);
        for(k=0; k<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; k++){
            if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k] == connectionInfoPtr->tcpConnectionUserInfoPtr){
                st = k;
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k] = NULL;
            }
            if(st > 0 && k < (roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers-1) && k >= st){
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k] = roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[k+1];
            }
        }
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers-1] = NULL;
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers -= 1;
        pthread_rwlock_unlock(&roomsArrLock);
        if(calledFromExit < 0){
            for(int idx=0; idx<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; idx++){
                //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[idx]->clinetSocketFd, broadcastBuffer, sizeof(broadcastBuffer));
                write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[idx]->clinetSocketFd, broadcastBuffer, strlen(broadcastBuffer));
            }
        }
        connectionInfoPtr->tcpConnectionUserInfoPtr->in_room = -1;
        return retval;
    }
}

int myStartGame(int rounds, char* answer, struct tcpConnectionInfo* connectionInfoPtr){

    int retval = R_START_GAME_SUCCESS;
    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_START_GAME_ERROR_LOGGED_OUT;
        return retval;
    }
    if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room < 0){
        retval = R_START_GAME_ERROR_NOT_IN_ROOM;
        return retval;
    }
    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].adminUserInfoPtr != connectionInfoPtr->tcpConnectionUserInfoPtr){
        retval = R_START_GAME_ERROR_NOT_MANAGER;
        return retval;
    }
    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame > 0){
        retval = R_START_GAME_ERROR_IN_GAME;
        return retval;
    }
    //if(answer[0] != '0' && strlen(answer)!=4){
    if(strlen(answer)!=4){    
        retval = R_START_GAME_ERROR_INVALID_GUESS_NUMBER;
        return retval;
    }
    strcpy(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer, answer);
    roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx = 0;
    roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].rounds = rounds;
    roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curRound = 1;
    roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame = 1;

    char broadcastBuffer[MAXLINE];
    bzero(broadcastBuffer, sizeof(broadcastBuffer));
    strcpy(broadcastBuffer, M_START_GAME_SUCCESS);
    strcat(broadcastBuffer, roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx]->userName);
    strcat(broadcastBuffer, "\n");
    for(int i=0; i<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; i++){
        if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i] != connectionInfoPtr->tcpConnectionUserInfoPtr){
            //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i]->clinetSocketFd, broadcastBuffer, sizeof(broadcastBuffer));
            write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[i]->clinetSocketFd, broadcastBuffer, strlen(broadcastBuffer));
        }
    }

    retval = R_START_GAME_SUCCESS;
    return retval;
}

int myGuess(char* msgToSelf, char* guessAnswer, struct tcpConnectionInfo* connectionInfoPtr){
    

    int retval = -1;

    if(connectionInfoPtr->tcpConnectionUserInfoPtr == NULL){
        retval = R_GUESS_ERROR_LOGGED_OUT;
        return retval;
    }
    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame < 0){
        if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].adminUserInfoPtr != connectionInfoPtr->tcpConnectionUserInfoPtr){
            retval = R_GUESS_ERROR_NOT_IN_GAME_NOT_MANAGER;
            return retval;
        }else{
            retval = R_GUESS_ERROR_NOT_IN_GAME_IS_MANAGER;
            return retval;
        }
    }
    if(connectionInfoPtr->tcpConnectionUserInfoPtr->in_room < 0){
        retval = R_GUESS_ERROR_NOT_IN_ROOM;
        return retval;
    }
    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[0] == NULL){
    }
    if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx] != connectionInfoPtr->tcpConnectionUserInfoPtr){
        retval = R_GUESS_ERROR_NOT_MY_TURN;
        return retval;
    }
    if(strlen(guessAnswer)!=4){
        retval = R_GUESS_ERROR_INVALID_GUESS;
        return retval;
    }
    for(int i=0; i<strlen(guessAnswer); i++){
        if(guessAnswer[i]<48 || guessAnswer[i]>57){
            retval = R_GUESS_ERROR_INVALID_GUESS;
            return retval;
        }
    }

    if(!strcmp(guessAnswer, roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer)){
        retval = R_GUESS_SUCCESS_BINGO;
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx = 0;
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curRound = 0;
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame = -1;
        bzero(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer, sizeof(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer));
        roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].rounds = 0;
        char toOthers[MAXLINE];
        bzero(toOthers, sizeof(toOthers));
        strcpy(toOthers, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
        strcat(toOthers, " guess \'");
        strcat(toOthers, guessAnswer);
        strcat(toOthers, "\' and got Bingo!!! ");
        strcat(toOthers, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
        strcat(toOthers, " wins the game, game ends\n");
        strcpy(msgToSelf, toOthers);
        for(int j=0; j<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; j++){
            if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j] != connectionInfoPtr->tcpConnectionUserInfoPtr){
                //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j]->clinetSocketFd, toOthers, sizeof(toOthers));
                write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j]->clinetSocketFd, toOthers, strlen(toOthers));
            }
        }
        return retval;
    }else{

        if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curRound == roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].rounds && roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx == roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers-1){
            retval = R_GUESS_SUCCCESS_GAME_OVER;
            roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx = 0;
            roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curRound = 0;
            roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].inGame = -1;
            roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].rounds = 0;
            int ABRet = 0;
            char ANum, BNum;
            char ABString[MAXLINE];
            bzero(ABString, sizeof(ABString));
            strcpy(ABString, "xAxB");
            ABRet = getAB(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer, guessAnswer);
            bzero(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer, sizeof(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer));
            ANum = (char)(ABRet/10 + 48);
            BNum = (char)(ABRet%10 + 48);
            ABString[0] = ANum;
            ABString[2] = BNum;
            char toOthers[MAXLINE];
            bzero(toOthers, sizeof(toOthers));
            strcpy(toOthers, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
            strcat(toOthers, " guess \'");
            strcat(toOthers, guessAnswer);
            strcat(toOthers, "\' and got \'");
            strcat(toOthers, ABString);
            strcat(toOthers, "\'\nGame ends, no one wins\n");
            strcpy(msgToSelf, toOthers);
            for(int j=0; j<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; j++){
                if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j] != connectionInfoPtr->tcpConnectionUserInfoPtr){
                    //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j]->clinetSocketFd, toOthers, sizeof(toOthers));
                    write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j]->clinetSocketFd, toOthers, strlen(toOthers));
                }
            }
            return retval;
        }else{
            retval = R_GUESS_SUCCESS_WRONG;
            int ABRet = 0;
            char ANum, BNum;
            char ABString[MAXLINE];
            bzero(ABString, sizeof(ABString));
            strcpy(ABString, "xAxB");
            ABRet = getAB(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].answer, guessAnswer);
            ANum = (char)(ABRet/10 + 48);
            BNum = (char)(ABRet%10 + 48);
            ABString[0] = ANum;
            ABString[2] = BNum;
            char toOthers[MAXLINE];
            bzero(toOthers, sizeof(toOthers));
            strcpy(toOthers, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
            strcat(toOthers, " guess \'");
            strcat(toOthers, guessAnswer);
            strcat(toOthers, "\' and got \'");
            strcat(toOthers, ABString);
            strcat(toOthers, "\'\n");
            strcpy(msgToSelf, toOthers);
            for(int j=0; j<roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers; j++){
                if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j] != connectionInfoPtr->tcpConnectionUserInfoPtr){
                    write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j]->clinetSocketFd, toOthers, strlen(toOthers));
                    //write(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[j]->clinetSocketFd, toOthers, sizeof(toOthers));
                }
            }
            if(roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx == (roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].nPlayers-1)){
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx = 0;
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curRound ++;
            }else{
                roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx++;
            }
            return retval;
        }
    }
}

int myStatus(char* cpyStr){


    int retval = R_STATUS_SUCCESS;

    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {

        std::ostringstream ss1;
        // 建立從 S3 下載 Object 的請求
        Aws::S3::Model::GetObjectRequest getObjectRequest01;

        const Aws::String myBucketName = "genghisswan";
        const Aws::String myObjectName = "server_status01.txt";
        const Aws::String otherObjectName_A = "server_status02.txt";
        const Aws::String otherObjectName_B = "server_status03.txt";
        const std::string myObjectContent = "0\n";
        Aws::S3::S3Client s3Client;
        int bytes = 0;

        getObjectRequest01.SetBucket(myBucketName);
        getObjectRequest01.SetKey(myObjectName);
        

        // 從 S3 下載 Object
        Aws::S3::Model::GetObjectOutcome getObjectOutcome01 = s3Client.GetObject(getObjectRequest01);
        std::streambuf* body01;
        char fileContent[MAXLINE];
        bzero(fileContent, sizeof(fileContent));
        if(getObjectOutcome01.IsSuccess()) {
            std::cout << "下載成功，資料內容：" << std::endl;
            body01 = getObjectOutcome01.GetResult().GetBody().rdbuf();
            bytes = getObjectOutcome01.GetResult().GetContentLength();
            body01->sgetn(fileContent, bytes);
        } else {
        std::cout << "下載失敗：" << getObjectOutcome01.GetError().GetExceptionName() << " "
                    << getObjectOutcome01.GetError().GetMessage() << std::endl;
        }

        //ss1 << getObjectOutcome01.GetResult().GetBody().rdbuf();
        //std::string fileString1 = ss1.str();
        //char fileContent[MAXLINE];
        //int nParams = 0;
        //char fileSplit[3][MAXLINE];
        //for(int i=0; i<3; i++){
        //    bzero(fileSplit[i], sizeof(fileSplit[i]));
        //}
        //bzero(fileContent, sizeof(fileContent));
        //strcpy(fileContent, fileString1.c_str());
        //nParams =  splitParams(fileContent, fileSplit);
        //bzero(fileContent, sizeof(fileContent));
        //strcpy(fileContent, fileSplit[0]);
        
        //int nParams;
        //char fileLines[8][MAXLINE];
        //nParams = splitParams(fileContent, fileLines);

        strcpy(cpyStr, "Server1: ");
        strcat(cpyStr, fileContent);


        std::ostringstream ss2;
        // 建立從 S3 下載 Object 的請求
        Aws::S3::Model::GetObjectRequest getObjectRequest02;

        getObjectRequest02.SetBucket(myBucketName);
        getObjectRequest02.SetKey(otherObjectName_A);

        Aws::S3::Model::GetObjectOutcome getObjectOutcome02 = s3Client.GetObject(getObjectRequest02);
        std::streambuf* body02;
        bzero(fileContent, sizeof(fileContent));
        if(getObjectOutcome02.IsSuccess()) {
            std::cout << "下載成功，資料內容：" << std::endl;
            body02 = getObjectOutcome02.GetResult().GetBody().rdbuf();
            bytes = getObjectOutcome02.GetResult().GetContentLength();
            body02->sgetn(fileContent, bytes);
        } else {
        std::cout << "下載失敗：" << getObjectOutcome02.GetError().GetExceptionName() << " "
                    << getObjectOutcome02.GetError().GetMessage() << std::endl;
        }

        //ss2 << getObjectOutcome02.GetResult().GetBody().rdbuf();
        //std::string fileString2 = ss2.str();
        //for(int i=0; i<3; i++){
        //    bzero(fileSplit[i], sizeof(fileSplit[i]));
        // }
        // bzero(fileContent, sizeof(fileContent));
        // strcpy(fileContent, fileString2.c_str());
        // nParams =  splitParams(fileContent, fileSplit);
        // bzero(fileContent, sizeof(fileContent));
        // strcpy(fileContent, fileSplit[0]);

        strcat(cpyStr, "Server2: ");
        strcat(cpyStr, fileContent);
        

        std::ostringstream ss3;
        // 建立從 S3 下載 Object 的請求
        Aws::S3::Model::GetObjectRequest getObjectRequest03;

        getObjectRequest03.SetBucket(myBucketName);
        getObjectRequest03.SetKey(otherObjectName_B);

        Aws::S3::Model::GetObjectOutcome getObjectOutcome03 = s3Client.GetObject(getObjectRequest03);
        std::streambuf* body03;
        bzero(fileContent, sizeof(fileContent));
        if(getObjectOutcome03.IsSuccess()) {
            std::cout << "下載成功，資料內容：" << std::endl;
            body03 = getObjectOutcome03.GetResult().GetBody().rdbuf();
            bytes = getObjectOutcome03.GetResult().GetContentLength();
            body03->sgetn(fileContent, bytes);
        } else {
        std::cout << "下載失敗：" << getObjectOutcome03.GetError().GetExceptionName() << " "
                    << getObjectOutcome03.GetError().GetMessage() << std::endl;
        }

        /*ss3 << getObjectOutcome03.GetResult().GetBody().rdbuf();
        std::string fileString3 = ss3.str();
        for(int i=0; i<3; i++){
            bzero(fileSplit[i], sizeof(fileSplit[i]));
        }
        bzero(fileContent, sizeof(fileContent));
        strcpy(fileContent, fileString3.c_str());
        nParams =  splitParams(fileContent, fileSplit);
        bzero(fileContent, sizeof(fileContent));
        strcpy(fileContent, fileSplit[0]);*/


        strcat(cpyStr, "Server3: ");
        strcat(cpyStr, fileContent);

    }
    Aws::ShutdownAPI(options);


    return  retval;
}


void handleCommandUDP(char* msg, int nParams, char commandParams[][MAXLINE]){

    int commandType, commandResult;

    if(!strcmp(commandParams[0], "register")){
        
        commandType = C_REGISTER;

        if(nParams != 4){
            commandResult = R_REGISTER_ERROR_USAGE;
            strcpy(msg, M_REGISTER_ERROR_USAGE);
            return;
        }else{
            commandResult = myRegister(commandParams[1], commandParams[2], commandParams[3]);
            if(commandResult == R_REGISTER_ERROR_USERNAME_EMAIL){
                strcpy(msg, M_REGISTER_ERROR_USERNAME_EMAIL);
                return;
            }else{
                strcpy(msg, M_REGISTER_SUCCESS);
                return;
            }
        }
    }else if(!strcmp(commandParams[0], "list")){
        if(nParams != 2){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }else{
            if(!strcmp(commandParams[1], "rooms")){

                commandType = C_LIST_ROOMS;
                
                // Find valid rooms
                int printIdxOrder[MAX_ROOMS];
                int curCnt = 0;
                for(int i=0; i<MAX_ROOMS; i++){
                    if(roomsArr[i].isActive > 0){
                        printIdxOrder[curCnt] = i;
                        curCnt++;
                    }
                }

                // If no rooms exist
                if(curCnt == 0){
                    commandResult = R_LIST_ROOMS_SUCCESS_NO_ROOM;
                    strcpy(msg, M_LIST_ROOMS_SUCCESS_NO_ROOM);
                    return;
                }

                commandResult = R_LIST_ROOMS_SUCCESS_EXIST_ROOMS;

                // Sort printIdxOrder array in gameId ascending order
                for(int i=0; i<curCnt-1; i++){
                    for(int j=0; j<curCnt-i-1; j++){
                        if(roomsArr[printIdxOrder[j]].roomId > roomsArr[printIdxOrder[j+1]].roomId){
                            int tmp = printIdxOrder[j];
                            printIdxOrder[j] = printIdxOrder[j+1];
                            printIdxOrder[j+1] = tmp;
                        }
                    }
                }

                // Compose the message to send to client
                char listIdxStr[5];
                char roomIdStr[15];
                const char* publicStr = "Public";
                const char* privateStr = "Private";
                strcpy(msg, M_LIST_ROOMS_SUCCESS_EXIST_ROOMS);
                for(int i=0; i<curCnt; i++){
                    bzero(listIdxStr, sizeof(listIdxStr));
                    bzero(roomIdStr, sizeof(roomIdStr));

                    sprintf(listIdxStr, "%d", i+1);
                    strcat(msg, listIdxStr);
                    strcat(msg, ". (");
                    if(roomsArr[printIdxOrder[i]].isPublic > 0){
                        strcat(msg, publicStr);
                    }else{
                        strcat(msg, privateStr);
                    }
                    strcat(msg, ") Game Room ");
                    sprintf(roomIdStr, "%u", roomsArr[printIdxOrder[i]].roomId);
                    strcat(msg, roomIdStr);
                    if(roomsArr[printIdxOrder[i]].inGame < 0){
                        strcat(msg, " is open for players\n");
                    }else{
                        strcat(msg, "has started playing\n");
                    }
                }
                return;
            }else if(!strcmp(commandParams[1], "users")){
                
                commandType = C_LIST_USERS;

                // Find valid users
                int printIdxOrder[MAX_USERS];
                int curCnt = 0;
                for(int i=0; i<MAX_USERS; i++){
                    if(usersArr[i].userName[0] == 0){
                        break;
                    }
                    printIdxOrder[i] = i;
                    curCnt++;
                }

                if(curCnt == 0){
                    commandResult = R_LIST_USERS_SUCCESS_NO_USER;
                    strcpy(msg, M_LIST_USERS_SUCCESS_NO_USER);
                    return;
                }

                // Sort printIdxOrder array in username ascending order
                for(int i=0; i<curCnt-1; i++){
                    for(int j=0; j<curCnt-i-1; j++){
                        if(strcmp(usersArr[printIdxOrder[j]].userName, usersArr[printIdxOrder[j+1]].userName)>0){
                            int tmp = printIdxOrder[j];
                            printIdxOrder[j] = printIdxOrder[j+1];
                            printIdxOrder[j+1] = tmp;
                        }
                    }
                }

                // Compose the message to send to client
                char listIdxStr[5];
                strcpy(msg, M_LIST_USERS_SUCCESS_EXIST_USERS);
                for(int i=0; i<curCnt; i++){
                    bzero(listIdxStr, sizeof(listIdxStr));
                    sprintf(listIdxStr, "%d", i+1);
                    strcat(msg, listIdxStr);
                    strcat(msg, ". ");
                    strcat(msg, usersArr[printIdxOrder[i]].userName);
                    strcat(msg, "<");
                    strcat(msg, usersArr[printIdxOrder[i]].email);
                    strcat(msg, "> ");
                    if(usersArr[printIdxOrder[i]].isLoggedIn > 0){
                        strcat(msg, "Online\n");
                    }else{
                        strcat(msg, "Offline\n");
                    }
                }
                return;

            }else{
                commandType = C_INVALID;
                commandResult = R_INVALID_COMMAND;
                strcpy(msg, M_INVALID_COMMAND);
                return;
            }
        }
    }else{
        commandType = C_INVALID;
        commandResult = R_INVALID_COMMAND;
        strcpy(msg, M_INVALID_COMMAND);
        return;
    }
}

void handleCommandTCP(char* msg, int nParams, char commandParams[][MAXLINE], struct tcpConnectionInfo* connectionInfoPtr){

    int commandType, commandResult;

    if(!strcmp(commandParams[0], "login")){

        commandType = C_LOGIN;
        
        if(nParams != 3){
            commandResult = R_LOGIN_ERROR_USAGE;
            strcpy(msg, M_LOGIN_ERROR_USAGE);
            return;
        }

        commandResult = myLogin(commandParams[1], commandParams[2], connectionInfoPtr);

        if(commandResult == R_LOGIN_ERROR_USERNAME){
            strcpy(msg, M_LOGIN_ERROR_USERNAME);
            return;
        }else if(commandResult == R_LOGIN_ERROR_LOGGED_IN_SELF){
            strcpy(msg, M_LOGIN_ERROR_LOGGED_IN_SELF);
            strcat(msg, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
            strcat(msg, "\n");
            return;
        }else if(commandResult == R_LOGIN_ERROR_LOGGED_IN_OTHER){
            strcpy(msg, M_LOGIN_ERROR_LOGGED_IN_OTHER);
            strcat(msg, commandParams[1]);
            strcat(msg, "\n");
            return;
        }else if(commandResult == R_LOGIN_ERROR_PASSWORD){
            strcpy(msg, M_LOGIN_ERROR_PASSWORD);
            return;
        }else if(commandResult == R_LOGIN_SUCCESS){
            strcpy(msg, M_LOGIN_SUCCESS);
            strcat(msg, connectionInfoPtr->tcpConnectionUserInfoPtr->userName);
            strcat(msg, "\n");
            return;
        }else{
            strcpy(msg, "Function myLogin returned -1\n");
            return;
        }

    }else if(!strcmp(commandParams[0], "logout")){

        commandType = C_LOGOUT;
        char myUserNameBeforeLogout[MAXLINE];

        if(nParams != 1){
            commandResult = R_LOGOUT_ERROR_USAGE;
            strcpy(msg, M_LOGOUT_ERROR_USAGE);
            return;
        }

        commandResult = myLogout(myUserNameBeforeLogout, connectionInfoPtr);

        if(commandResult == R_LOGOUT_ERROR_LOGGED_OUT){
            strcpy(msg, M_LOGOUT_ERROR_LOGGED_OUT);
            return;
        }else if(commandResult == R_LOGOUT_ERROR_IN_ROOM){
            char rooomIdStr[MAXLINE];
            strcpy(msg, M_LOGOUT_ERROR_IN_ROOM);
            sprintf(rooomIdStr, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
            strcat(msg, rooomIdStr);
            strcat(msg, ", please leave game room\n");
            return;
        }else if(commandResult == R_LOGOUT_SUCCESS){
            strcpy(msg, M_LOGOUT_SUCCESS);
            strcat(msg, myUserNameBeforeLogout);
            strcat(msg, "\n");
            return;
        }else{
            strcpy(msg, "Function myLogout returned -1\n");
            return;
        }

    }else if(!strcmp(commandParams[0], "create")){

        if(nParams != 4 && nParams != 5){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        if(!strcmp(commandParams[1], "public") && !strcmp(commandParams[2], "room")){

            commandType = C_CREATE_PUBLIC_ROOM;
            if(nParams != 4){
                commandResult = R_CREATE_PUBLIC_ROOM_ERROR_USAGE;
                strcpy(msg, M_CREATE_PUBLIC_ROOM_ERROR_USAGE);
                return;
            }
            uint32_t roomId;
            //roomId = atoi(commandParams[3]);
            sscanf(commandParams[3], "%u", &roomId);
            commandResult = myCreateRoom(1, roomId, 0, connectionInfoPtr);
            if(commandResult == R_CREATE_PUBLIC_ROOM_ERROR_LOGGED_OUT){
                strcpy(msg, M_CREATE_PUBLIC_ROOM_ERROR_LOGGED_OUT);
                return;
            }else if(commandResult == R_CREATE_PUBLIC_ROOM_ERROR_IN_ROOM){
                char roomIdString[MAXLINE];
                strcpy(msg, M_CREATE_PUBLIC_ROOM_ERROR_IN_ROOM);
                sprintf(roomIdString, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
                strcat(msg, roomIdString);
                strcat(msg, ", please leave game room\n");
                return;
            }else if(commandResult == R_CREATE_PUBLIC_ROOM_ERROR_ID_USED){
                strcpy(msg, M_CREATE_PUBLIC_ROOM_ERROR_ID_USED);
                return;
            }else if(commandResult == R_CREATE_PUBLIC_ROOM_SUCCESS){
                strcpy(msg, M_CREATE_PUBLIC_ROOM_SUCCESS);
                strcat(msg, commandParams[3]);
                strcat(msg, "\n");
                return;
            }else{
                strcpy(msg, "Function myCreate public returned -1\n");
                return;
            }

        }else if(!strcmp(commandParams[1], "private") && !strcmp(commandParams[2], "room")){
            
            commandType = C_CREATE_PRIVATE_ROOM;
            if(nParams != 5){
                commandResult = R_CREATE_PRIVATE_ROOM_ERROR_USAGE;
                strcpy(msg, M_CREATE_PRIVATE_ROOM_ERROR_USAGE);
                return;
            }
            uint32_t roomId;
            uint32_t invitationCode;
            sscanf(commandParams[3], "%u", &roomId);
            sscanf(commandParams[4], "%u", &invitationCode);
            commandResult = myCreateRoom(-1, roomId, invitationCode, connectionInfoPtr);
            if(commandResult == R_CREATE_PRIVATE_ROOM_ERROR_LOGGED_OUT){
                strcpy(msg, M_CREATE_PRIVATE_ROOM_ERROR_LOGGED_OUT);
                return;
            }else if(commandResult == R_CREATE_PRIVATE_ROOM_ERROR_IN_ROOM){
                char roomIdString[MAXLINE];
                strcpy(msg, M_CREATE_PRIVATE_ROOM_ERROR_IN_ROOM);
                sprintf(roomIdString, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
                strcat(msg, roomIdString);
                strcat(msg, ", please leave game room\n");
                return;
            }else if(commandResult == R_CREATE_PRIVATE_ROOM_ERROR_ID_USED){
                strcpy(msg, M_CREATE_PRIVATE_ROOM_ERROR_ID_USED);
                return;
            }else if(commandResult == R_CREATE_PRIVATE_ROOM_SUCCESS){
                strcpy(msg, M_CREATE_PRIVATE_ROOM_SUCCESS);
                strcat(msg, commandParams[3]);
                strcat(msg, "\n");
                return;
            }else{
                strcpy(msg, "Function myCreate private returned -1\n");
                return;
            }

        }else{
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;

        }
    }else if(!strcmp(commandParams[0], "join")){
        
        if(nParams != 3){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        if(!strcmp(commandParams[1], "room")){
            
            commandType = C_JOIN_ROOM;
            uint32_t roomId;
            sscanf(commandParams[2], "%u", &roomId);
            commandResult = myJoinRoom(roomId, connectionInfoPtr);
            if(commandResult == R_JOIN_ROOM_ERROR_LOGGED_OUT){
                strcpy(msg, M_JOIN_ROOM_ERROR_LOGGED_OUT);
                return;
            }else if(commandResult == R_JOIN_ROOM_ERROR_IN_ROOM){
                char roomIdString[MAXLINE];
                strcpy(msg, M_JOIN_ROOM_ERROR_IN_ROOM);
                sprintf(roomIdString, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
                strcat(msg, roomIdString);
                strcat(msg, ", please leave game room\n");
                return;
            }else if(commandResult == R_JOIN_ROOM_ERROR_INVALID_ID){
                strcpy(msg, M_JOIN_ROOM_ERROR_INVALID_ID);
                strcat(msg, commandParams[2]);
                strcat(msg, " is not exist\n");
                return;
            }else if(commandResult == R_JOIN_ROOM_ERROR_PRIVATE){
                strcpy(msg, M_JOIN_ROOM_ERROR_PRIVATE);
                return;
            }else if(commandResult == R_JOIN_ROOM_ERROR_IN_GAME){
                strcpy(msg, M_JOIN_ROOM_ERROR_IN_GAME);
                return;
            }else if(commandResult == R_JOIN_ROOM_SUCCESS){
                char roomIdString[MAXLINE];
                strcpy(msg, M_JOIN_ROOM_SUCCESS_SELF);
                sprintf(roomIdString, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
                strcat(msg, roomIdString);
                strcat(msg, "\n");
                return;
            }else{
                strcpy(msg, "Function myJoinRoom returned -1\n");
                return;
            }

        }else{
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

    }else if(!strcmp(commandParams[0], "invite")){

        commandType = C_INVITE;
        char inviteeUserName[MAXLINE];

        if(nParams != 2){
            commandResult = R_INVITE_ERROR_USAGE;
            return;
        }

        commandResult = myInvite(inviteeUserName, commandParams[1], connectionInfoPtr);
        if(commandResult == R_INVITE_ERROR_LOGGED_OUT){
            strcpy(msg, M_INVITE_ERROR_LOGGED_OUT);
            return;
        }else if(commandResult == R_INVITE_ERROR_NOT_IN_ROOM){
            strcpy(msg, M_INVITE_ERROR_NOT_IN_ROOM);
            return;
        }else if(commandResult == R_INVITE_ERROR_NOT_ROOM_ADMIN){
            strcpy(msg, M_INVITE_ERROR_NOT_ROOM_ADMIN);
            return;
        }else if(commandResult == R_INVITE_ERROR_INVITEE_NOT_LOGGED_IN){
            strcpy(msg, M_INVITE_ERROR_INVITEE_NOT_LOGGED_IN);
            return;
        }else if(commandResult == R_INVITE_SUCCESS){
            strcpy(msg, M_INVITE_SUCCESS_SELF);
            strcat(msg, inviteeUserName);
            strcat(msg, "<");
            strcat(msg, commandParams[1]);
            strcat(msg, ">\n");
            return;
        }else{
            strcpy(msg, "Function myInvite returned -1\n");
            return;
        }

    }else if(!strcmp(commandParams[0], "list")){

        if(nParams != 2){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        if(!strcmp(commandParams[1], "invitations")){
            
            commandType = C_LIST_INVITATIONS;

            // Find valid invitations
            int printIdxOrder[MAX_INVITATIONS];
            int curCnt = 0;
            for(int i=0; i<MAX_INVITATIONS; i++){
                if(invitationArr[i].inviteePtr == connectionInfoPtr->tcpConnectionUserInfoPtr && invitationArr[i].isValid > 0){
                    printIdxOrder[curCnt] = i;
                    curCnt++;
                }
            }

            // If no rooms exist
            if(curCnt == 0){
                commandResult = R_LIST_INVITATIONS_SUCCESS_NO_INVITATION;
                strcpy(msg, M_LIST_INVITATIONS_SUCCESS_NO_INVITATION);
                return;
            }

            // Sort printIdxOrder array in gameId ascending order
            for(int i=0; i<curCnt-1; i++){
                for(int j=0; j<curCnt-i-1; j++){
                    if(roomsArr[invitationArr[printIdxOrder[j]].roomArrIdx].roomId > roomsArr[invitationArr[printIdxOrder[j+1]].roomArrIdx].roomId){
                        int tmp = printIdxOrder[j];
                        printIdxOrder[j] = printIdxOrder[j+1];
                        printIdxOrder[j+1] = tmp;
                    }
                }
            }

            // Compose the message to send to client
            char listIdxStr[5];
            char roomIdStr[15];
            char invitationCodeStr[15];
            strcpy(msg, M_LIST_INVITATIONS_SUCCESS_EXIST_INVITATIONS);
            for(int i=0; i<curCnt; i++){
                bzero(listIdxStr, sizeof(listIdxStr));
                bzero(roomIdStr, sizeof(roomIdStr));

                sprintf(listIdxStr, "%d", i+1);
                strcat(msg, listIdxStr);
                strcat(msg, ". ");
                strcat(msg, invitationArr[printIdxOrder[i]].inviterPtr->userName);
                strcat(msg, "<");
                strcat(msg, invitationArr[printIdxOrder[i]].inviterPtr->email);
                strcat(msg, "> invite you to join game room ");
                sprintf(roomIdStr, "%u", roomsArr[invitationArr[printIdxOrder[i]].roomArrIdx].roomId);
                strcat(msg, roomIdStr);
                strcat(msg, ", invitation code is ");
                sprintf(invitationCodeStr, "%u", invitationArr[printIdxOrder[i]].invitationCode);
                strcat(msg, invitationCodeStr);
                strcat(msg, "\n");
            }
            return;

        }else{
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

    }else if(!strcmp(commandParams[0], "accept")){

        commandType = C_ACCEPT;

        if(nParams != 3){
            commandResult = R_ACCEPT_ERROR_USAGE;
            strcpy(msg, M_ACCEPT_ERROR_USAGE);
            return;
        }
        uint32_t invitationCode;
        sscanf(commandParams[2], "%u", &invitationCode);
        commandResult = myAccept(commandParams[1], invitationCode, connectionInfoPtr);
        if(commandResult == R_ACCEPT_ERROR_LOGGED_OUT){
            strcpy(msg, M_ACCEPT_ERROR_LOGGED_OUT);
            return;
        }else if(commandResult == R_ACCEPT_ERROR_IN_ROOM){
            char roomIdStr[MAXLINE];
            strcpy(msg, M_ACCEPT_ERROR_IN_ROOM);
            sprintf(roomIdStr, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
            strcat(msg, roomIdStr);
            strcat(msg, ", please leave game room\n");
            return;
        }else if(commandResult == R_ACCEPT_ERROR_INVALID_INVITATION){
            strcpy(msg, M_ACCEPT_ERROR_INVALID_INVITATION);
            return;
        }else if(commandResult == R_ACCEPT_ERROR_WRONG_CODE){
            strcpy(msg, M_ACCEPT_ERROR_WRONG_CODE);
            return;
        }else if(commandResult == R_ACCEPT_ERROR_GAME_STARTED){
            strcpy(msg, M_ACCEPT_ERROR_GAME_STARTED);
            return;
        }else if(commandResult == R_ACCEPT_SUCCESS){
            char roomIdStr[MAXLINE];
            strcpy(msg, M_ACCEPT_SUCCESS_SELF);
            sprintf(roomIdStr, "%u", roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].roomId);
            strcat(msg, roomIdStr);
            strcat(msg, "\n");
            return;
        }else{
            strcpy(msg, "Function myAccept returned -1\n");
            return;
        }

    }else if(!strcmp(commandParams[0], "leave")){

        if(nParams != 2){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        if(!strcmp(commandParams[1], "room")){

            char roomIdStr[MAXLINE];
            commandResult = myLeaveRoom(roomIdStr, -1, connectionInfoPtr);
            
            if(commandResult == R_LEAVE_ROOM_ERROR_LOGGED_OUT){
                strcpy(msg, M_LEAVE_ROOM_ERROR_LOGGED_OUT);
                return;
            }else if(commandResult == R_LEAVE_ROOM_ERROR_NOT_IN_ROOM){
                strcpy(msg, M_LEAVE_ROOM_ERROR_NOT_IN_ROOM);
                return;
            }else if(commandResult == R_LEAVE_ROOM_SUCCESS_ADMIN){
                // in_room is already cleared
                strcpy(msg, M_LEAVE_ROOM_SUCCESS_ADMIN_SELF);
                strcat(msg, roomIdStr);
                strcat(msg, "\n");
                return;
            }else if(commandResult == R_LEAVE_ROOM_SUCCESS_MEMBER_IN_GAME){
                strcpy(msg, M_LEAVE_ROOM_SUCCESS_ADMIN_SELF);
                strcat(msg, roomIdStr);
                strcat(msg, ", game ends\n");
                return;
            }else if(commandResult == R_LEAVE_ROOM_SUCCESS_MEMBER_NOT_IN_GAME){
                strcpy(msg, M_LEAVE_ROOM_SUCCESS_ADMIN_SELF);
                strcat(msg, roomIdStr);
                strcat(msg, "\n");
                return;
            }else{
                strcpy(msg, "Function myLeaveRoom returned -1\n");
                return;
            }

        }else{
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

    }else if(!strcmp(commandParams[0], "start")){

        if(nParams != 4 && nParams != 3){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        if(!strcmp(commandParams[1], "game")){

            int rounds = atoi(commandParams[2]);
            commandType = C_START_GAME;
            if(nParams == 3){
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
                commandResult = myStartGame(rounds, randomAns, connectionInfoPtr);
            }
            if(nParams == 4){
                commandResult = myStartGame(rounds, commandParams[3], connectionInfoPtr);
            }
            if(commandResult == R_START_GAME_ERROR_LOGGED_OUT){
                strcpy(msg, M_START_GAME_ERROR_LOGGED_OUT);
                return;
            }else if(commandResult == R_START_GAME_ERROR_NOT_IN_ROOM){
                strcpy(msg, M_START_GAME_ERROR_NOT_IN_ROOM);
                return;
            }else if(commandResult == R_START_GAME_ERROR_NOT_MANAGER){
                strcpy(msg, M_START_GAME_ERROR_NOT_MANAGER);
                return;
            }else if(commandResult == R_START_GAME_ERROR_IN_GAME){
                strcpy(msg, M_START_GAME_ERROR_IN_GAME);
                return;
            }else if(commandResult == R_START_GAME_ERROR_INVALID_GUESS_NUMBER){
                strcpy(msg, M_START_GAME_ERROR_INVALID_GUESS_NUMBER);
                return;
            }else if(commandResult == R_START_GAME_SUCCESS){
                strcpy(msg, M_START_GAME_SUCCESS);
                strcat(msg, roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx]->userName);
                strcat(msg, "\n");
                return;
            }

        }else{
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

    }else if(!strcmp(commandParams[0], "guess")){

        if(nParams != 2){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        char msgToSelf[MAXLINE];
        commandResult = myGuess(msgToSelf, commandParams[1], connectionInfoPtr);

        if(commandResult == R_GUESS_ERROR_LOGGED_OUT){
            strcpy(msg, M_GUESS_ERROR_LOGGED_OUT);
            return;
        }else if(commandResult == R_GUESS_ERROR_NOT_IN_ROOM){
            strcpy(msg, M_GUESS_ERROR_NOT_IN_ROOM);
            return;
        }else if(commandResult == R_GUESS_ERROR_NOT_IN_GAME_IS_MANAGER){
            strcpy(msg, M_GUESS_ERROR_NOT_IN_GAME_IS_MANAGER);
            return;
        }else if(commandResult == R_GUESS_ERROR_NOT_IN_GAME_NOT_MANAGER){
            strcpy(msg, M_GUESS_ERROR_NOT_IN_GAME_NOT_MANAGER);
            return;
        }else if(commandResult == R_GUESS_ERROR_NOT_MY_TURN){
            strcpy(msg, M_GUESS_ERROR_NOT_MY_TURN);
            strcat(msg, roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].playersPtrArr[roomsArr[connectionInfoPtr->tcpConnectionUserInfoPtr->in_room].curGuesserArrIdx]->userName);
            strcat(msg, "\n");
            return;
        }else if(commandResult == R_GUESS_ERROR_INVALID_GUESS){
            strcpy(msg, M_GUESS_ERROR_INVALID_GUESS_NUMBER);
            return;
        }else if(commandResult == R_GUESS_SUCCESS_BINGO){
            strcpy(msg, msgToSelf);
            return;
        }else if(commandResult == R_GUESS_SUCCESS_WRONG){
            strcpy(msg, msgToSelf);
            return;
        }else if(commandResult == R_GUESS_SUCCCESS_GAME_OVER){
            strcpy(msg, msgToSelf);
            return;
        }else{
            strcpy(msg, "Function myGuess returned -1\n");
            return;
        }

    }else if(!strcmp(commandParams[0], "status")){
        if(nParams != 1){
            commandType = C_STATUS;
            commandResult = R_STATUS_ERROR;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }else{
            char locRetMsg[MAXLINE];
            commandType = C_STATUS;
            commandResult = myStatus(locRetMsg);
            strcpy(msg, locRetMsg);
            return;
        }
    }else if(!strcmp(commandParams[0], "exit")){

        if(nParams != 1){
            commandType = C_INVALID;
            commandResult = R_INVALID_COMMAND;
            strcpy(msg, M_INVALID_COMMAND);
            return;
        }

        char roomIdStr[MAXLINE];
        char userNameStr[MAXLINE];
        commandType = C_EXIT;
        commandResult = R_EXIT_SUCCESS;
        int leaveRoomResult = myLeaveRoom(roomIdStr, 1, connectionInfoPtr);
        int logoutResult = myLogout(userNameStr, connectionInfoPtr);
        return;

    }else{
        commandType = C_INVALID;
        commandResult = R_INVALID_COMMAND;
        strcpy(msg, M_INVALID_COMMAND);
        return;
    }

}


// Thread routine definitions
void* tcpClientRoutine(void* params){
    
    char msg[MAXLINE];
    char commandParams[8][MAXLINE];
    int nParams;
    struct tcpConnectionInfo myTcpConnectionInfo;
    struct tcpClientRoutineParams* paramsPtr = (tcpClientRoutineParams*)params;
    myTcpConnectionInfo.clientSocketFd = paramsPtr->tcpClientSocketFd;
    myTcpConnectionInfo.tcpConnectionUserInfoPtr = NULL;
    int breakSignal = -1;

    while(1){
        
        bzero(msg, sizeof(msg));
        read(myTcpConnectionInfo.clientSocketFd, msg, sizeof(msg));
        printf("%s", msg);

        if(!strcmp(msg, "exit\n") || !strcmp(msg, "\n") || !strcmp(msg, "") || !strcmp(msg, "-1")){
            breakSignal = 1;
        }
        nParams = splitParams(msg, commandParams);
        if(breakSignal > 0){
            nParams = 1;
            strcpy(commandParams[0], "exit");
        }
        bzero(msg, sizeof(msg));
        handleCommandTCP(msg, nParams, commandParams, &myTcpConnectionInfo);
        if(breakSignal > 0){
            break;
        }
        write(myTcpConnectionInfo.clientSocketFd, msg, strlen(msg));
        //write(myTcpConnectionInfo.clientSocketFd, msg, sizeof(msg));
        
    }

    close(paramsPtr->tcpClientSocketFd);
    printf("Client fd: %d disconnected !!!\n", paramsPtr->tcpClientSocketFd);
    threadEnded[paramsPtr->threadIdx] = 1;
    return NULL;
}


void* udpSlaveRoutine(void* params){

    char msg[MAXLINE];
    char commandParams[8][MAXLINE];
    int nParams;
    struct udpSlaveRoutineParams* paramsPtr = (udpSlaveRoutineParams*)params;
    int sentSize = 0;

    strcpy(msg, paramsPtr->msg);
    printf("%s", msg);
    nParams = splitParams(msg, commandParams);
    bzero(msg, sizeof(msg));
    // TODO: Handle UDP command
    handleCommandUDP(msg, nParams, commandParams);
    sentSize = sendto(paramsPtr->udpServerSocketFd, msg, strlen(msg), 0, (struct sockaddr*)&paramsPtr->clientAddress, paramsPtr->clientAddressLength);
    //sentSize = sendto(paramsPtr->udpServerSocketFd, msg, sizeof(msg), 0, (struct sockaddr*)&paramsPtr->clientAddress, paramsPtr->clientAddressLength);
    threadEnded[paramsPtr->threadIdx] = 1;

    return NULL;
}


void* udpMasterRoutine(void* params){

    int udpServerSocketFd;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int reuseOption = 1;
    int receivedMsgLen;
    char msg[MAXLINE];
    char passMsg[MAXLINE];
    struct udpMasterRoutineParams* paramsPtr = (udpMasterRoutineParams*)params;

    serverAddress = paramsPtr->serverAddress;

    // UDP socket creation
    udpServerSocketFd = socket(AF_INET, SOCK_DGRAM, 0);
    if(udpServerSocketFd == -1){
        printf("Failed to create UDP socket\n");
        exit(1);
    }else{
        //printf("socket : %d\n", uServerSocketFd);
    }
    setsockopt(udpServerSocketFd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseOption, sizeof(reuseOption));
    
    // UDP binding
    if(bind(udpServerSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0){
        printf("Failed to bind\n");
        exit(1);
    }else{
        printf("UDP server is running\n");
    }

    while(1){
        bzero(&clientAddress, sizeof(clientAddress));
        receivedMsgLen = recvfrom(udpServerSocketFd, msg, sizeof(msg), 0, (struct sockaddr*)&clientAddress, &clientAddressLength);
        msg[receivedMsgLen] = '\0';
        strcpy(passMsg, msg);

        for(int i=0; i<MAX_THREAD_NUM; i++){
            if(threadAvailable[i] == 1){
                threadAvailable[i] = 0;
                threadEnded[i] = 0;
                udpSlaveParamsArr[i].clientAddress = clientAddress;
                udpSlaveParamsArr[i].clientAddressLength = clientAddressLength;
                strcpy(udpSlaveParamsArr[i].msg, passMsg);
                udpSlaveParamsArr[i].udpServerSocketFd = udpServerSocketFd;
                udpSlaveParamsArr[i].threadIdx = i;
                pthread_create(&threadPool[i], NULL, &udpSlaveRoutine, &udpSlaveParamsArr[i]);
                break;
            }
        }
    }
    return NULL;
}


void* recyclerThreadRoutine(void* params){
    // Join terminated threads
    for(int k=0; k<MAX_THREAD_NUM; k++){
        if(threadEnded[k] == 1){
            threadEnded[k] = 0;
            threadAvailable[k] = 1;
            // pthread_join
            pthread_join(threadPool[k], NULL);
        }
    }
    return NULL;
}


int main(int argc, char* argv[]){

    int tcpServerSocketFd, tcpClientSocketFd;
    struct sockaddr_in serverAddress, clientAddress;
    socklen_t clientAddressLength = sizeof(clientAddress);
    int reuseOption = 1;

    initializeS3File();

    pthread_rwlock_init(&usersArrLock, NULL);
    pthread_rwlock_init(&roomsArrLock, NULL);
    pthread_rwlock_init(&invitationsArrLock, NULL);

    // Set all threads in threadpool to available
    for(int i=0; i<MAX_THREAD_NUM; i++){
        threadAvailable[i] = 1;
    }

    // Initialize User Array
    for(int i=0; i<MAX_USERS; i++){
        usersArr[i].clinetSocketFd = -1;
        bzero(usersArr[i].email, sizeof(usersArr[i].email));
        usersArr[i].in_room = -1;
        usersArr[i].isLoggedIn = -1;
        bzero(usersArr[i].password, sizeof(usersArr[i].password));
        bzero(usersArr[i].userName, sizeof(usersArr[i].userName));
    }

    // Initialize Room Array
    for(int i=0; i<MAX_ROOMS; i++){
        roomsArr[i].adminUserInfoPtr = NULL;
        bzero(roomsArr[i].answer, sizeof(roomsArr[i].answer));
        roomsArr[i].curGuesserArrIdx = 0;
        roomsArr[i].curRound = 0;
        roomsArr[i].inGame = -1;
        roomsArr[i].invitationCode = 0;
        roomsArr[i].isActive = -1;
        roomsArr[i].isPublic = 1;
        roomsArr[i].nPlayers = 0;
        for(int j=0; j<MAX_PLAYERS; j++){
            roomsArr[i].playersPtrArr[j] = NULL;
        }
        roomsArr[i].roomId = 0;
        roomsArr[i].rounds = 0;
    }

    // Initialize Invitation Array
    for(int i=0; i<MAX_INVITATIONS; i++){
        invitationArr[i].invitationCode = 0;
        invitationArr[i].inviteePtr = NULL;
        invitationArr[i].inviterPtr = NULL;
        invitationArr[i].isValid = -1;
        invitationArr[i].roomArrIdx = -1;
    }

    // Set server address
    bzero(&serverAddress, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = htonl(INADDR_ANY);
    //serverAddress.sin_port = htons(atoi(argv[1]));
    serverAddress.sin_port = htons(8888);

    // Thread recycling thread
    pthread_create(&recyclerThread, NULL, &recyclerThreadRoutine, NULL);

    // UDP master thread
    udpMasterParams.serverAddress = serverAddress;
    pthread_create(&udpMasterThread, NULL, &udpMasterRoutine, &udpMasterParams);

    // TCP socket creation
    tcpServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if(tcpServerSocketFd == -1){
        printf("Failed to create socket\n");
        exit(1);
    }else{
        //printf("socket : %d\n", serverSocketFd);
    }
    setsockopt(tcpServerSocketFd, SOL_SOCKET, SO_REUSEADDR, (void*)&reuseOption, sizeof(reuseOption));

    // TCP socket binding
    if(bind(tcpServerSocketFd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) != 0){
        printf("Failed to bind\n");
        exit(1);
    }else{
        //printf("bound\n");
    }

    // TCP listening
    if (listen(tcpServerSocketFd, 100) != 0){
        printf("Failed to listen\n");
        exit(1);
    }else{
        printf("TCP server is running\n");
    }

    while(1){
        printf("check\n");
        bzero(&clientAddress, sizeof(clientAddress));
        tcpClientSocketFd = accept(tcpServerSocketFd, (struct sockaddr*)&clientAddress, &clientAddressLength);
        if(tcpClientSocketFd == -1){
            printf("Failed to accept client\n");
            continue;
        }else{
            printf("New connection with Fd: %d\n", tcpClientSocketFd);
            for(int i=0; i<MAX_THREAD_NUM; i++){
                if(threadAvailable[i] == 1){
                    threadAvailable[i] = 0;
                    threadEnded[i] = 0;
                    tcpClientParamsArr[i].tcpClientSocketFd = tcpClientSocketFd;
                    tcpClientParamsArr[i].threadIdx = i;
                    pthread_create(&threadPool[i], NULL, &tcpClientRoutine, &tcpClientParamsArr[i]);
                    break;
                }
            }
        }
    }

    close(tcpServerSocketFd);
    return 0;

}
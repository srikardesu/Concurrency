#include "headerq3server.h"
using namespace std;
/////////////////////////////

//Regular bold text
#define BBLK "\e[1;30m"
#define BRED "\e[1;31m"
#define BGRN "\e[1;32m"
#define BYEL "\e[1;33m"
#define BBLU "\e[1;34m"
#define BMAG "\e[1;35m"
#define BCYN "\e[1;36m"
#define ANSI_RESET "\x1b[0m"

typedef long long LL;

#define pb push_back
#define debug(x) cout << #x << " : " << x << endl
#define part cout << "-----------------------------------" << endl;

///////////////////////////////
#define MAX_CLIENTS 10000
#define PORT_ARG 8001

const int initial_msg_len = 256;

////////////////////////////////////

const LL buff_sz = 1048576;
///////////////////////////////////////////////////
pair<string, int> read_string_from_socket(const int &fd, int bytes)
{
    std::string output;
    output.resize(bytes);

    int bytes_received = read(fd, &output[0], bytes - 1);
    debug(bytes_received);
    if (bytes_received <= 0)
    {
        cerr << "Failed to read data from socket. \n";
    }

    output[bytes_received] = 0;
    output.resize(bytes_received);
    // debug(output);
    return {output, bytes_received};
}

int send_string_on_socket(int fd, const string &s)
{
    // debug(s.length());
    int bytes_sent = write(fd, s.c_str(), s.length());
    if (bytes_sent < 0)
    {
        cerr << "Failed to SEND DATA via socket.\n";
    }

    return bytes_sent;
}

///////////////////////////////

void handle_connection(int client_socket_fd)
{
    // int client_socket_fd = *((int *)client_socket_fd_ptr);
    //####################################################

    int received_num, sent_num;

    /* read message from client */
    int ret_val = 1;

    while (true)
    {
        string cmd;
        tie(cmd, received_num) = read_string_from_socket(client_socket_fd, buff_sz);
        ret_val = received_num;
        // debug(ret_val);
        // printf("Read something\n");
        if (ret_val <= 0)
        {
            // perror("Error read()");
            printf("Server could not read msg sent from client\n");
            goto close_client_socket_ceremony;
        }
        cout << "Client sent : " << cmd << endl;
        if (cmd == "exit")
        {
            cout << "Exit pressed by client" << endl;
            goto close_client_socket_ceremony;
        }
        string msg_to_send_back = "Ack: " + cmd;

        ////////////////////////////////////////
        // "If the server write a message on the socket and then close it before the client's read. Will the client be able to read the message?"
        // Yes. The client will get the data that was sent before the FIN packet that closes the socket.

        int sent_to_client = send_string_on_socket(client_socket_fd, msg_to_send_back);
        // debug(sent_to_client);
        if (sent_to_client == -1)
        {
            perror("Error while writing to client. Seems socket has been closed");
            goto close_client_socket_ceremony;
        }
    }

close_client_socket_ceremony:
    close(client_socket_fd);
    printf(BRED "Disconnected from client" ANSI_RESET "\n");
    // return NULL;
}

void * handleworker (void * arg) {
    while(1) {
        S:
        // cout<<"Worker thread id: "<<gettid()<<"\n";                                                                                                                                                                                                                                          
        pthread_mutex_lock(&que_lock);

        // wait till atleast one client we get to process
        while(que.empty()) {
            pthread_cond_wait(&qempty,&que_lock);
        }

        // get the client to be processed
        int client_fd = que.front();
        // cout<<"hereeeee "<<client_fd<<"\n";
        que.pop();
        pthread_mutex_unlock(&que_lock);
        std::string output;
        int bytes = buff_sz;
        output.resize(bytes);

        // read from the client, the command

        int bytes_received = read(client_fd, &output[0], bytes - 1);
        //debug(bytes_received);
        if (bytes_received <= 0)
        {
            cerr << "Failed to read data from socket. \n";
            goto S;
        }

        // tokenising, strtok gave issues as its not thread safe thus strtok_r has been used

        output[bytes_received] = '\0';
        output.resize(bytes_received);
        char * arr = (char *)malloc(1000 * sizeof(char));
        strcpy(arr, output.c_str());
        int len = output.length();
        arr[len] = '\0';
        char *token = strtok_r(arr," ",&arr);
        char ** commands = (char ** )malloc(1000 * sizeof(char *));
        int no_of_args = 0;
        while (token != NULL) {
            commands[no_of_args] = (char *)malloc(500 * sizeof(char));
            strcpy(commands[no_of_args], token);
            no_of_args++;
            token = strtok_r(arr," ",&arr);
        }
        // cout<<"no_of_args: "<<no_of_args<<" command[0]: "<<commands[0]<<"\n"; 
        if(!no_of_args) {
            cout<<"No args :(\n";
            goto S;
        }
        commands[no_of_args]=NULL;

        // execute the commands
        // lock and unlock dictionary indices to simulate atomic operations.
        if(!strcmp(commands[1],"insert")) {
            if(no_of_args!=4) {
                cout<<"Invalid no of args (insert)\n";
                goto S;
            }
            int key = atoi(commands[2]);
            pthread_mutex_lock(dictmutex+key);
            if(dict[key]=="") {
                dict[key]=commands[3];
                send_string_on_socket(client_fd,"Insertion Successful");
            }
            else send_string_on_socket(client_fd,"Key already exists");
            pthread_mutex_unlock(dictmutex+key);
        }
        else if(!strcmp(commands[1],"delete")) {
            if(no_of_args!=3) {
                cout<<"Invalid no of args (delete)\n";
                goto S;
            }
            int key = atoi(commands[2]);
            pthread_mutex_lock(dictmutex+key);
            if(dict[key]!="") {
                dict[key]="";
                send_string_on_socket(client_fd,"Deletion successful");
            } 
            else send_string_on_socket(client_fd,"No such key exists");
            pthread_mutex_unlock(dictmutex+key);
        }
        else if(!strcmp(commands[1],"update")) {
            if(no_of_args!=4) {
                cout<<"Invalid no of args (update)\n";
                goto S;
            }
            int key = atoi(commands[2]);
            pthread_mutex_lock(dictmutex+key);
            if(dict[key]!="") {
                dict[key]=commands[3];
                send_string_on_socket(client_fd,dict[key]);
            } 
            else send_string_on_socket(client_fd,"No such key exists");
            pthread_mutex_unlock(dictmutex+key);
        }
        else if(!strcmp(commands[1],"concat")) {
            if(no_of_args!=4) {
                // cout<<"command is:\n";
                // for(int i=0;i<no_of_args;i++) {
                //     cout<<commands[i]<<" ";
                // }
                // cout<<"\n";
                cout<<"Invalid no of args (concat)\n";
                goto S;
            }
            int key_1 = atoi(commands[2]);
            int key_2 = atoi(commands[3]);
            pthread_mutex_lock(dictmutex+key_1); pthread_mutex_lock(dictmutex+key_2);
            if(dict[key_1]!="" && dict[key_2]!="") {
                string temp_1 = dict[key_1];
                string temp_2 = dict[key_2];
                dict[key_1]=dict[key_1]+temp_2;
                dict[key_2]=dict[key_2]+temp_1;
                send_string_on_socket(client_fd,dict[key_2]);
            } 
            else send_string_on_socket(client_fd,"Concat failed as at least one of the keys does not exist");
            pthread_mutex_unlock(dictmutex+key_1);pthread_mutex_unlock(dictmutex+key_2);
        }
        else if(!strcmp(commands[1],"fetch")) {
            if(no_of_args!=3) {
                cout<<"Invalid no of args (fetch)\n";
                goto S;
            }
            int key = atoi(commands[2]);
            pthread_mutex_lock(dictmutex+key);
            if(dict[key]!="") send_string_on_socket(client_fd,dict[key]);
            else send_string_on_socket(client_fd,"“Key does not exist");
            pthread_mutex_unlock(dictmutex+key);
        }
    }
    return NULL;                
} 

int main(int argc, char *argv[])
{
    int i, j, k, t, n;
    n = stoi(argv[1]);            // worker threads

    // init dict

    for(int i=0;i<101;i++) {
        dict[i]="";
        pthread_mutex_init(dictmutex + i, NULL);
    }     
    pthread_mutex_init(&que_lock,NULL);
    if (pthread_cond_init(&qempty, NULL) != 0) {                                    
        perror("pthread_cond_init() error");                                        
        exit(1);                                                                    
    }
    for(int i=0;i<n;i++) {
        // cout<<"spawning threads\n";
        pthread_create(serverthreads+i,NULL,handleworker,NULL);
    }
    int wel_socket_fd, client_socket_fd, port_number;
    socklen_t clilen;

    struct sockaddr_in serv_addr_obj, client_addr_obj;
    /////////////////////////////////////////////////////////////////////////
    /* create socket */
    /*
    The server program must have a special door—more precisely,
    a special socket—that welcomes some initial contact 
    from a client process running on an arbitrary host
    */
    //get welcoming socket
    //get ip,port
    /////////////////////////
    wel_socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (wel_socket_fd < 0)
    {
        perror("ERROR creating welcoming socket");
        exit(-1);
    }

    //////////////////////////////////////////////////////////////////////
    /* IP address can be anything (INADDR_ANY) */
    bzero((char *)&serv_addr_obj, sizeof(serv_addr_obj));
    port_number = PORT_ARG;
    serv_addr_obj.sin_family = AF_INET;
    // On the server side I understand that INADDR_ANY will bind the port to all available interfaces,
    serv_addr_obj.sin_addr.s_addr = INADDR_ANY;
    serv_addr_obj.sin_port = htons(port_number); //process specifies port

    /////////////////////////////////////////////////////////////////////////////////////////////////////////
    /* bind socket to this port number on this machine */
    /*When a socket is created with socket(2), it exists in a name space
       (address family) but has no address assigned to it.  bind() assigns
       the address specified by addr to the socket referred to by the file
       descriptor wel_sock_fd.  addrlen specifies the size, in bytes, of the
       address structure pointed to by addr.  */

    //CHECK WHY THE CASTING IS REQUIRED
    if (bind(wel_socket_fd, (struct sockaddr *)&serv_addr_obj, sizeof(serv_addr_obj)) < 0)
    {
        perror("Error on bind on welcome socket: ");
        exit(-1);
    }
    //////////////////////////////////////////////////////////////////////////////////////

    /* listen for incoming connection requests */

    listen(wel_socket_fd, MAX_CLIENTS);
    cout << "Server has started listening on the LISTEN PORT" << endl;
    clilen = sizeof(client_addr_obj);

    while (1)
    {
        /* accept a new request, create a client_socket_fd */
        /*
        During the three-way handshake, the client process knocks on the welcoming door
of the server process. When the server “hears” the knocking, it creates a new door—
more precisely, a new socket that is dedicated to that particular client. 
        */
        //accept is a blocking call
        printf("Waiting for a new client to request for a connection\n");
        client_socket_fd = accept(wel_socket_fd, (struct sockaddr *)&client_addr_obj, &clilen);
        if (client_socket_fd < 0)
        {
            perror("ERROR while accept() system call occurred in SERVER");
            exit(-1);
        }

        printf(BGRN "New client connected from port number %d and IP %s \n" ANSI_RESET, ntohs(client_addr_obj.sin_port), inet_ntoa(client_addr_obj.sin_addr));
        // push client fd into queue which we got from clients side
        pthread_mutex_lock(&que_lock);
        que.push(client_socket_fd);
        pthread_mutex_unlock(&que_lock);
        pthread_cond_signal(&qempty);
    }

    close(wel_socket_fd);

    // for(int i=0;i<n;i++) {
    //     pthread_join(serverthreads[i],NULL);
    // }
    return 0;
}

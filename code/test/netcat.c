#include "syscall.h"
#include "userlib.h"
#include "malloc.h"

#define MAX_ATTEMPS 5

int main(int argc, char** argv){  

    if (argc < 3){
        PutString("Usage: ");PutString(*argv);PutString(" <-l | <remote_addr>> <remote_port> <--read | --string> [<string_to_write>]\n");
        return -2;
    }
    
    int isServer = !strcmp(argv[1], "-l");
    OpenSocketId remotePoint;
    
    int global_attempts = 0;
    
    if (isServer)
        remotePoint = Socket(-1, -1, atoi(argv[2]));
    else
        remotePoint = Socket(atoi(argv[1]), atoi(argv[2]), -1);
        
    if (!remotePoint){
        PutString("Can't open the socket\n");
        return -1;
    }
        
    remote_peer_t client;
    if (isServer){
        if (!Accept(&client, -1, remotePoint)){
            PutString("Can't accept new peer\n");
            return -1;
        }
        PutString("Client has connected with address ");PutInt(client.addr);PutString(" and port ");PutInt(client.port);PutString("\n");
    }
    else {
        if (!Connect(10000, remotePoint)){
            PutString("Can't connect to server\n");
            return -1;
        }
    }
    
    if (!strcmp(argv[3], "--string")){
        int str_size = strlen(argv[4]);
        PutString("Sending (");PutInt(str_size);PutString("): ");
        //~ while (!Write((char*)&str_size, sizeof(int), remotePoint)){
            //~ if (++global_attempts == MAX_ATTEMPS){
                //~ PutString("Can't write after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
                //~ return -3;
            //~ }
        //~ }
        //~ while (!Write(argv[4], str_size, remotePoint)){
            //~ if (++global_attempts == MAX_ATTEMPS){
                //~ PutString("Can't write after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
                //~ return -3;
            //~ }
        //~ }
        if (!Write((char*)&str_size, sizeof(int), remotePoint)){
            PutString("Can't write after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
            return -3;
        }
        if (!Write(argv[4], str_size, remotePoint)){
            PutString("Can't write after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
            return -3;
        }
        PutString(argv[4]);PutString("\n");
    } else if (!strcmp(argv[3], "--file")){
        OpenFileId file = Open(argv[4]);
    
        if (!file){
            PutString("Cannot open ");PutString(argv[1]);PutString("\n");
            return -1;
        }
        
        int read;  
        
        file_info_t info;
        FileInfo(&info, file);
        char* buffer = malloc(sizeof(char)*info.size), *current = buffer;
        int remaining = info.size;
        
        if (!buffer){
            PutString("Can't allocate ");PutInt(info.size);PutString(" bytes\n");
            return -3;
        }
        
        while ((read = Read(current, remaining < 100 ? remaining : 100, file))){
            if (read < 0){
                switch (read) {
                case E_PERM:              
                    PutString("Can't read ");PutString(argv[1]);PutString(": Permission denied.\n");
                    return E_PERM;
                    break;
                default:              
                    PutString("Can't read ");PutString(argv[1]);PutString(": unknown error.\n");
                    return E_PERM;
                    break;
                }
                break;
            }
            if (read < 100)
                break;
            current += 100;
            remaining -= read;
        }
        
        Close(file);
        
        while (!Write((char*)&info.size, sizeof(int), remotePoint)){
            if (++global_attempts == MAX_ATTEMPS){
                PutString("Can't write after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
                return -3;
            }
        }
        while (!Write(buffer, info.size, remotePoint)){
            if (++global_attempts == MAX_ATTEMPS){
                PutString("Can't write after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
                return -3;
            }
        }
        free(buffer);
    } else {
        int str_size = 0;
        while (!Read((char*)&str_size, sizeof(int), remotePoint)){
            if (++global_attempts == MAX_ATTEMPS){
                PutString("Can't read after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
                return -3;
            }
        }
        if (!str_size){
                PutString("Received an empty string\n");
                return -3;
        }
        PutString("Received (");PutInt(str_size);PutString("): ");
        char* buffer = (char*)malloc(str_size + 1);
        if (!buffer){
            PutString("Can't allocate ");PutInt(str_size);PutString(" bytes\n");
            return -3;
        }
        while (!Read(buffer, str_size, remotePoint)){
            if (++global_attempts == MAX_ATTEMPS){
                PutString("Can't read after ");PutInt(MAX_ATTEMPS);PutString(" attempts\n");
                return -3;
            }
        }
        buffer[str_size] = '\0';
        PutString(buffer);PutString("\n");
        free(buffer);        
    }
    
    Close(remotePoint);
    return 0;
} 

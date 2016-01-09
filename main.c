#include "header.h"
main(int argc, char *argv[])
{
    if(argc<5)
    {
        printf("ERROR: Invalid Arguments!\n");
        printf("Arguments should be of the format => -t [topology-file-name] -i [routing-update-interval in seconds]\n");
        exit(ERROR);
    }
    else if(strcmp("-t",argv[1])==0 && strcmp("-i",argv[3])==0)
    {
        if(atoi(argv[4])<=0)
        {
            printf("ERROR: Invalid Arguments!\n");
            printf("Arguments should be of the format => -t [topology-file-name] -i [routing-update-interval in seconds]\n");
            exit(ERROR);
        }
        else
        {
            buildSocket(argv[2], argv[4]);
        }
    }
    else
    {
        printf("ERROR: Invalid Arguments!\n");
        printf("Arguments should be of the format => -t [topology-file-name] -i [routing-update-interval in seconds]\n");
        exit(ERROR);
    }
}

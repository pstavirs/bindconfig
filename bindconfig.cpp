#include "bindconfig.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

extern int doPrint;

void usage(char* progName)
{
   printf("usage: %s [class dev|client|proto|service] [comp <id>] "
           "[path <pathToken>] [enable|disable]\n", progName);
   exit(255);
}

int __cdecl main(int argc, char* argv[])
{
    int exitCode;
    bool show = true;
    CompClass cls = kCompClassDevice;
    char *compStr = NULL;
    char *pathStr = NULL;
    BindingStatus status;

    doPrint = 1;

    int i = 1;
    while (i < argc)
    {
        if (strcmp(argv[i], "class") == 0)
        {
            i++;
            if (i >= argc)
                usage(argv[0]);
            if (strcmp(argv[i], "dev") == 0)
                cls = kCompClassDevice;
            else if (strcmp(argv[i], "client") == 0)
                cls = kCompClassClient;
            else if (strcmp(argv[i], "proto") == 0)
                cls = kCompClassProtocol;
            else if (strcmp(argv[i], "service") == 0)
                cls = kCompClassService;
            else
                usage(argv[0]);
            i++;
        }
        else if (strcmp(argv[i], "comp") == 0)
        {
            i++;
            if (i >= argc)
                usage(argv[0]);
            compStr = argv[i];
            i++;
        }
        else if (strcmp(argv[i], "path") == 0)
        {
            i++;
            if (i >= argc)
                usage(argv[0]);
            pathStr = argv[i];
            i++;
        }
        else if (strcmp(argv[i], "enable") == 0)
        {
            show = false;
            status = kBindingEnabled;
            i++; 
        }
        else if (strcmp(argv[i], "disable") == 0)
        {
            show = false;
            status = kBindingDisabled;
            i++; 
        }
        else
            usage(argv[0]);
    }

    if (show)
    {
        status = bindingStatus(cls, compStr, pathStr);

        exitCode = (status == kBindingEnabled) ? 1 : 0;
    }
    else
    {
        if (setBindingStatus(cls, compStr, pathStr, status))
            exitCode = 0;
        else
            exitCode = 1;
    }

    return exitCode;
}

/****************************************************************************
			  Microsoft RPC Version 1.0
			 Copyright Microsoft Corp. 1992
				Hello Example

    FILE:	hellos.c
    USAGE:	hellos
    PURPOSE:	Server side of RPC distributed application hello
    FUNCTIONS:  main() - registers server as RPC server
    COMMENTS:
    This distributed application prints "hello, world" on the server.
    This version features a client that manages its connection to
    the server. It uses the binding handle hello_IfHandle that is defined
    in the generated header file hello.h.
****************************************************************************/
#include <stdlib.h>
#include <windows.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <rpc.h>    // RPC data structures and APIs
#include "hello.h"    // header file generated by MIDL compiler

void Usage(char * pszProgramName)
{
    fprintf(stderr, "Usage:  %s\n", pszProgramName);
    fprintf(stderr, " -p protocol_sequence\n");
    fprintf(stderr, " -n network_address\n");
    fprintf(stderr, " -e endpoint\n");
    fprintf(stderr, " -o options\n");
    fprintf(stderr, " -u uuid\n");
    exit(1);
}

HANDLE TerminateEvent;

int _CRTAPI1
main (argc, argv)
    int argc;
    char *argv[];
{
    RPC_STATUS status;
    unsigned char * pszUuid = "12345678-1234-1234-1234-123456789ABC";
    unsigned char * pszProtocolSequence = "ncacn_np";
    unsigned char * pszNetworkAddress   = NULL;
    unsigned char * pszEndpoint 	= "\\pipe\\hello";
    unsigned char * pszOptions          = NULL;
    unsigned char * pszStringBinding    = NULL;
    int i;
    DWORD WaitStatus;

    // allow the user to override settings with command line switches
    for (i = 1; i < argc; i++) {
	if ((*argv[i] == '-') || (*argv[i] == '/')) {
	    switch (tolower(*(argv[i]+1))) {
		case 'p':  // protocol sequence
		    pszProtocolSequence = argv[++i];
		    break;
		case 'n':  // network address
		    pszNetworkAddress = argv[++i];
		    break;
		case 'e':
		    pszEndpoint = argv[++i];
		    break;
		case 'o':
		    pszOptions = argv[++i];
		    break;
		case 'u':
		    pszUuid = argv[++i];
		    break;
		case 'h':
		case '?':
		default:
		    Usage(argv[0]);
	    }
	}
	else
	    Usage(argv[0]);
    }

    //
    // Create an event to wait on
    //

    TerminateEvent = CreateEvent( NULL,     // No security attributes
                                  TRUE,     // Must be manually reset
                                  FALSE,    // Initially not signaled
                                  NULL );   // No name

    if ( TerminateEvent == NULL ) {
        printf( "Couldn't CreateEvent %ld\n", GetLastError() );
        return 2;
    }


    status = RpcServerUseProtseqEp(pszProtocolSequence,
                                   1, // maximum concurrent calls
                                   pszEndpoint,
				   0);
    if (status) {
        printf("RpcServerUseProtseqEp returned 0x%x\n", status);
        exit(2);
    }

    status = RpcServerRegisterIf(hello_ServerIfHandle, 0, 0);
    if (status) {
        printf("RpcServerRegisterIf returned 0x%x\n", status);
        exit(2);
    }

    status = RpcServerRegisterAuthInfo( "HelloS", 10, NULL, NULL );
    if (status) {
        printf("RpcServerRegisterAuthInfo returned 0x%x\n", status);
        exit(2);
    }

    printf("Calling RpcServerListen\n");
    status = RpcServerListen(1,12345,1);
    if (status) {
        printf("RpcServerListen returned: 0x%x\n", status);
        exit(2);
    }

    WaitStatus = WaitForSingleObject( TerminateEvent, INFINITE );

    if ( WaitStatus != WAIT_OBJECT_0 ) {
        printf( "Couldn't WaitForSingleObject %ld %ld\n", WaitStatus, GetLastError() );
        return 2;
    }

    return 0;

} /* end main() */


// ====================================================================
//                MIDL allocate and free
// ====================================================================


void __RPC_FAR * __RPC_API MIDL_user_allocate(size_t len)
{
    return(malloc(len));
}

void __RPC_API MIDL_user_free(void __RPC_FAR * ptr)
{
    free(ptr);
}

/*
    PURPOSE:    Remote procedures that are linked with the server
		side of RPC distributed application
    FUNCTIONS:	HelloProc() - prints "hello, world" or other string
		sent by client to server
    COMMENTS:
    This version of the distributed application that prints
    "hello, world" (or other string) on the server features a client
    that manages its connection to the server. It uses the binding
    handle hello_IfHandle, defined in the file hello.h.
****************************************************************************/

void HelloProc(unsigned char * pszString)
{
    RPC_STATUS RpcStatus;

    RpcStatus = RpcImpersonateClient( NULL );

    if ( RpcStatus != RPC_S_OK ) {
        printf( "RpcImpersonateClient Failed %ld\n", RpcStatus );
    }
    printf("%s\n", pszString);

    RpcStatus = RpcRevertToSelf();

    if ( RpcStatus != RPC_S_OK ) {
        printf( "RpcRevertToSelf Failed %ld\n", RpcStatus );
    }


}

void Shutdown(void)
{
    RPC_STATUS status;

    printf("Calling RpcMgmtStopServerListening\n");
    status = RpcMgmtStopServerListening(NULL);
    if (status) {
        printf("RpcMgmtStopServerListening returned: 0x%x\n", status);
        exit(2);
    }

    printf("Calling RpcServerUnregisterIf\n");
    status = RpcServerUnregisterIf(NULL, NULL, FALSE);
    if (status) {
        printf("RpcServerUnregisterIf returned 0x%x\n", status);
        exit(2);
    }

    if ( !SetEvent( TerminateEvent) ) {
        printf( "Couldn't SetEvent %ld\n", GetLastError() );
    }

}

/* end hellos.c */

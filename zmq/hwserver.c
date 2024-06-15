//  Hello World server

#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int main (void)
{
    int rc;
    char msg[256];

    /* context */
    void *context = zmq_ctx_new ();

    /* socket */
    void *socket = zmq_socket (context, ZMQ_PULL);
    rc = zmq_bind (socket, "tcp://*:5555");
    assert (rc == 0);

    printf ("waiting for messages...\n");
    while (1) {
        int rc = zmq_recv (socket, msg, sizeof(msg)-1, ZMQ_DONTWAIT);
        if(rc>=0){
            msg[rc]='\0';
            printf ("Received a message (%d) [%s]\n", rc, msg);
        } else if (rc==EFSM) {
            printf ("EFSM error\n");
        } else {
            // printf ("rc = %d\n", rc);
        }
        // sleep (1);
    }

    zmq_close (socket);
    zmq_ctx_destroy (context);

    return 0;
}

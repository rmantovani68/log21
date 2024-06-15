//  Hello World client

#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <assert.h>

int main (void)
{
    int rc;
    char msg[256];

    printf ("Connecting to hello world server...\n");
    void *context = zmq_ctx_new ();
    void *socket = zmq_socket (context, ZMQ_PUSH);
    zmq_connect (socket, "tcp://localhost:1280");

    sprintf(msg,"ciao");
    printf ("Sending msg [%s]...\n", msg);
    zmq_send (socket, msg, strlen(msg), ZMQ_DONTWAIT);

    zmq_close (socket);
    zmq_ctx_destroy (context);
    return 0;
}

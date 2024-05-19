/*
 * Copyright (c) 2006-2023, Evlers Developers
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date         Author      Notes
 * 2024-02-28   Evlers      first implementation
 */

#include <string.h>
#include <nopoll.h>
#include <nopoll_decl.h>
#include <nopoll_private.h>


static noPollCtx *ctx = RT_NULL;


static void listener_on_message (noPollCtx * ctx, noPollConn * conn, noPollMsg * msg, noPollPtr  user_data)
{
    printf("Listener received (size: %d, ctx refs: %d): %s\n", 
                nopoll_msg_get_payload_size (msg),
                nopoll_ctx_ref_count(ctx),
                nopoll_msg_get_payload(msg));

    /* reply to the message */
    nopoll_conn_send_text (conn, "Message received", 16);

    return;
}

static void websocket_server_thread (void *parameter)
{
    /* initialize context */
	ctx = nopoll_ctx_new();

    /* create a listener to receive connections on port xxx */
    noPollConn *listener = nopoll_listener_new(ctx, "0.0.0.0", parameter);
    if (!nopoll_conn_is_ok(listener))
    {
        printf("Listener error, listener = %p, session = %d\n", listener, listener->session);
        goto __exit;
    }

    printf("Create a listener to receive connections on port %s\n", (char *)parameter);

    /* now set a handler that will be called when a message (fragment or not) is received */
    nopoll_ctx_set_on_msg(ctx, listener_on_message, NULL);

    /* now call to wait for the loop to notify events */
    nopoll_loop_wait (ctx, 0);

    /* close the listener */
	if (listener)
		nopoll_conn_close(listener);

    __exit:
    /* release context */
	if (ctx)
		nopoll_ctx_unref(ctx);

    ctx = NULL;

    printf("Exit listener\n");
}

static int nopoll_server (int argc, char **argv)
{
    if (argc < 2)
	{
        __using:
		printf("Using: nopoll_server <command>\n");
        printf("The following is the command description:\n");
        printf("\tstart\t\tProvides ports to start listening.\n");
        printf("\tstop\t\tStop listening.\n\n");
		return -1;
	}

    if (!strcmp(argv[1], "start"))
    {
        if (ctx == NULL)
        {
            if (argc < 3)
            {
                printf("Please provide port parameter!\n");
                return -1;
            }
            /* Create a thread listener */
            rt_thread_startup(rt_thread_create("websocket", websocket_server_thread, argv[2], 4096, 8, 10));
        }
        else
        {
            printf("Has already begun!\n");
        }
    }
    else if (!strcmp(argv[1], "stop"))
    {
        if (ctx != NULL)
        {
            nopoll_loop_stop(ctx);
        }
        else
        {
            printf("No start listener\n");
        }
    }
    else
        goto __using;

    return 0;
}
MSH_CMD_EXPORT(nopoll_server, websocket server test);

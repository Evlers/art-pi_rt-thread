# noPoll  
[noPoll](https://github.com/ASPLes/nopoll) is a OpenSource WebSocket implementation (RFC 6455) , written in ansi C , that allows building pure WebSocket solutions or to provide WebSocket support to existing TCP oriented applications.   

Here is a simple implementation for RT-Thread based on [noPoll](https://github.com/ASPLes/nopoll) , temporarily only support non-encrypted operation.  

# Quick Start  

There is a websocket server example at `examples/nopoll_server.c`.  
Listening port : 1234

Run the example at `MSH` as follows :
```
msh />nopoll_server start 1234
Create a listener to receive connections on port 1234
```

There is a websocket client example at `examples/nopoll_client.c`.  
Test server host : 127.0.0.1
Test server port : 1234

Run the example at `MSH` as follows :
```
msh />nopoll_client 127.0.0.1 1234
web socket connection ready!
sending content..
Listener received (size: 16, ctx refs: 3): Hello RT-Thread!
recv: Message received
Listener received (size: 1023, ctx refs: 3): **************
...
sendcnt = 1
...
...
```

# Notes  
## 1 Not defined `strdup` function   

If your compiler is not offered `strdup` function , you can implement it yourself as shown below :   

```
/* _strdup.c  */   
#include <rtthread.h>

#ifdef RT_USING_HEAP
char *strdup(const char *s)
{
	size_t len = strlen(s) + 1;
	char *tmp = (char *)rt_malloc(len);

	if(!tmp) return NULL;

	rt_memcpy(tmp, s, len);
	return tmp;
}
#endif
```
## 2 `FD_SETSIZE` too small  

Please config the project as shown below :   
The `RT-Thread Component/Device virtual file system/The maximal number of opened files` value need to  greater or equal to `RT-Thread Component/Network stack/light weight TCP/IP stack/The number of raw connection` value.  


# Reference  
1 WebSocket Official website : http://websocket.org/  
2 noPoll Official website : http://www.aspl.es/nopoll/  
3 noPoll GitHub repository : https://github.com/ASPLes/nopoll  
4 WebSocket test server : http://websocket.org/echo.html  

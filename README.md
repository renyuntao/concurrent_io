# concurrent_io
Some simple echo server program that implement with fork,select,poll,epoll and pthread.
#How to use these program?  
compile it with  
$ gcc echo_xxx.c  -o  server  
**particularly,if you want to compile echo_pthread_server.c,you should do as follows:**  
$ gcc echo_pthread_server.c -lpthread -o server  
then run as  
$ ./server    
and next you can use telnet to connect server as  
$ telnet localhost 8888  



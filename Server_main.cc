#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "server.h"
#include "thread_pool.h"

void option(int,char**,char*,int&,int&);

int main(int argc, char** argv)
{
	char ip[15]="0.0.0.0";
	int port=8000;
  int number_thread=50;
	option(argc,argv,ip,port,number_thread);

    ThreadPool* thread_poll=new ThreadPool();
    Server* ser=new Server(ip,port,thread_poll,100);
    ser->start();
    ser->Handle();
    ser->stop();
return 0;
}

void option(int argc,char** argv,char* ip,int &port,int &number_thread)
{
	 int opt;
   int digit_optind = 0;
   int option_index = 0;
   char optstring[] = "ipn";
   static struct option long_options[] = {
       {"ip", required_argument, NULL, 'i'},
       {"port",  required_argument,       NULL, 'p'},
       {"number-thread",  required_argument,       NULL, 'n'},
       {0, 0, 0, 0}
   };
 
   while ( (opt = getopt_long(argc, argv, optstring, long_options, &option_index)) != -1)
   {
   		if(opt=='i')
   			strcpy(ip,optarg);
   		else if(opt=='p')
   			port=atoi(optarg);
      else if(opt=='n')
        number_thread=atoi(optarg);

   }
   printf("ip:%s\n",ip );
   printf("port:%d\n",port );
   printf("number-thread:%d\n", number_thread);
}
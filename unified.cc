#include "unified.h"
#include<iostream>
#include <sstream>
#include <memory>
using std::string;
using std::unique_ptr;

std::map<int,struct sockaddr_in*> fd_addr_map_;

// static char* buff=new char[MAXLINE];
// static char* replay=new char[MAXLINE];
// static char* replay_data=new char[MAXLINE];
static char DirPath[10]="source";
static pthread_mutex_t file_index_mutex_=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t file_404_mutex_=PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t file_501_mutex_=PTHREAD_MUTEX_INITIALIZER;

static std::map<string, string> code_message_map_={
    {"404","Not Found"},
    {"501","Not Implemented"},
    {"200","OK"}
};



void Error(const char* message)
{
    printf(message,strerror(errno),errno);
     exit(0);
}

//获得一行
static int GetLine(char* buf,char* data)
{
    int len=0;
    int i=0;
    while(*(buf+i)!='\0')
    {
        if(*(buf+i)=='\r' && *(buf+i+1)=='\n')
        {
            len+=2;
            break;
        }
        i++;
        len++;
    }
 
    strncpy(data,buf,len);
    data[len]='\0';

    return len;
}

//形成应答报文
static void Reply(char* code,char* replay_data,char* replay)
{
    
    if(strcmp(code,"404")==0)
    {
        FILE* sendfp=nullptr;
        memset(replay_data,0,sizeof(replay_data));
        pthread_mutex_lock(&file_404_mutex_);
        sendfp=fopen("source/404.html","r");
        int length=fread(replay_data,sizeof(char),MAXLINE,sendfp);
        fclose(sendfp);
        pthread_mutex_unlock(&file_404_mutex_);
    }

    else if(strcmp(code,"501")==0)
    {
        FILE* sendfp=nullptr;
        memset(replay_data,0,sizeof(replay_data));
        pthread_mutex_lock(&file_501_mutex_);
        sendfp=fopen("source/501.html","r");
        int length=fread(replay_data,sizeof(char),MAXLINE,sendfp);
        fclose(sendfp);
         pthread_mutex_unlock(&file_501_mutex_);
    }

    std::stringstream ss;
    ss<<strlen(replay_data);
    string str(code);
    // printf("hahahah\n");
    memset(replay,0,sizeof(replay));
    // printf("hahahah\n");
    strcpy(replay,"HTTP/1.0");
    strcat(replay," ");
    strcat(replay,code);
    strcat(replay," ");
    strcat(replay,code_message_map_[str].c_str());
    strcat(replay,"\r\n");
    strcat(replay,"Server: JWebServer\r\n");
    strcat(replay,"Content-type:text/html;charset=utf-8\r\n");
    strcat(replay,"Content-length:");  strcat(replay,ss.str().c_str());
    strcat(replay,"\r\n\r\n");
    strcat(replay,replay_data);

    return;
}

//请求get请求
static void Deal_Get(char* path,char* replay_data,char* replay)
{
    FILE* sendfp=nullptr;
    
	char* Real_path=new char[NAME_MAX_SIZE]; 
    unique_ptr<char[]> up(Real_path);

	strcpy(Real_path,DirPath);
	strcat(Real_path,path);
	
    pthread_mutex_lock(&file_index_mutex_);
	sendfp=fopen(Real_path,"r");
	
	if(sendfp==nullptr)
    {
        pthread_mutex_unlock(&file_index_mutex_);
		Reply((char*)"404",replay_data,replay);
    }
	
	
	else{
         memset(replay_data,0,sizeof(replay_data));
	     int length=fread(replay_data,sizeof(char),MAXLINE,sendfp);
	    fclose(sendfp);
        pthread_mutex_unlock(&file_index_mutex_);
	 //printf("%s\n",replay_data);
	    Reply((char*)"200",replay_data,replay);
	}
	
	//delete [] Real_path;
	//delete [] replay_data;
    printf("Get Finish\n");
    return;
}

//获得Name和ID
static bool GetNameAndID(char* data,char* Name,char* ID)
{   
    char* tmp=new char[strlen(data)];
    unique_ptr<char[]> up(tmp);
    
    int i=0;
    while(data[i]!='\0' && data[i++]!='=');
    strncpy(tmp,data,i-1);
    tmp[i-1]='\0';
    if(strcmp(tmp,"Name")!=0) 
            return false;
        

    int j=i;
    while(data[j]!='\0' && data[j++]!='&');
    strncpy(Name,data+i,j-i-1);
    Name[j-i-1]='\0';

    i=j;
    while(data[i]!='\0' && data[i++]!='=');
    strncpy(tmp,data+j,i-j-1);
    tmp[i-j-1]='\0';
    if(strcmp(tmp,"ID")!=0)
        return false;

    
    strncpy(ID,data+i,strlen(data)-i);

    printf("Name:%s\n",Name );
    printf("ID:%s\n",ID );

    return true;
}

//获得post.html
static void GetPostHtml(char* Name,char* ID,char* replay_data)
{
    char post_1[NAME_MAX_SIZE]="<html>\n\n<head>\n<title>Post method</title>\n<link rel=\"icon\" href=\"data:;base64,=\">\n</head>\n\n<body bgcolor=ffffff>\nYour Name: ";
    char post_2[NAME_MAX_SIZE]="<br />\nID: ";
    char post_3[NAME_MAX_SIZE]="\n<hr><em>HTTP Web server</em>\n</body>\n</html>";

     memset(replay_data,0,sizeof(replay_data));
     strcpy(replay_data,post_1);
     strcat(replay_data,Name);
     strcat(replay_data,post_2);
     strcat(replay_data,ID);
     strcat(replay_data,post_3);
     printf("%s\n",replay_data);
}
//处理Post请求
static void Deal_Post(char* buff,char* path,char* replay_data,char* replay)
{   

    if(strcmp(path,"/Post_show")!=0)
    {
        Reply((char*)"404",replay_data,replay);
        return;
    }
    printf("Deal_Post\n");
    char* data=new char[MAXLINE];
    char* line=new char[MAXLINE];
    unique_ptr<char[]> up1[2]={
                                            unique_ptr<char[]>(data),
                                            unique_ptr<char[]>(line)
                                             };

    char* buf=&buff[0];
    int len;
    while(1)
    {
        len=GetLine(buf,line);
        buf+=len;
        if(strcmp(line,"\r\n")==0) break;
    }
	//data是请求数据
    GetLine(buf,data);
    printf("%s\n", data);

    char* Name=new char[strlen(data)];
    char* ID=new char[strlen(data)];
     unique_ptr<char[]> up2[2]={
                                            unique_ptr<char[]>(Name),
                                            unique_ptr<char[]>(ID)
                                             };

    if(!GetNameAndID(data,Name,ID))
    {
        printf("GetNameAndID Failed\n");
        Reply((char*)"404",replay_data,replay);
        return;
    }

    GetPostHtml(Name,ID,replay_data);
    Reply((char*)"200",replay_data,replay);

    return;
}


void Http_server(int connfd)
{
    printf("Http_server\n");
     char* replay=new char[MAXLINE];
     char* replay_data=new char[MAXLINE];
     char* buf=new char[MAXLINE];
    char* method=new char[METHOD_SIZE];
    char* path=new char[NAME_MAX_SIZE];
    char* buff=new char[MAXLINE];
     unique_ptr<char[]> up[6]={
                                    unique_ptr<char[]>(replay),
                                    unique_ptr<char[]>(replay_data),
                                    unique_ptr<char[]>(buf),
                                    unique_ptr<char[]>(method),
                                    unique_ptr<char[]>(path),
                                    unique_ptr<char[]>(buff)
                                    };
    
      int n = recv(connfd, buff, MAXLINE, 0);

     if(n==0)
        {
            struct sockaddr_in* clientaddr=fd_addr_map_[connfd];

            printf("Disconnect client:<%s,%d>\n",
                        inet_ntoa(clientaddr->sin_addr),
                        ntohs(clientaddr->sin_port)
                        );
            fd_addr_map_[connfd]=nullptr;
            return;
        }

    buff[n]='\0';
    printf("%s\n", buff);

    //获得请求方法
    int i;
    for(i=0;i<n&&buff[i]!=' ';i++);

    strncpy(method,buff,i);
    method[i]='\0';
    printf("%s\n", method);

    //获得请求路径
    int j;
    for(j=1;j<n&&buff[i+j]!=' ';j++);

    strncpy(path,buff+i+1,j);
    path[j-1]='\0';
    printf("%s\n", path);


    if(strcmp(method,"GET")==0)
        Deal_Get(path,replay_data,replay);
    else if(strcmp(method,"POST")==0)
        Deal_Post(buff,path,replay_data,replay);
    else
        Reply((char*)("501"),replay_data,replay);
		//delete [] replay_data;
	
    printf("\nconnfd:%d\n", connfd);
    //发送应答报文
     if(send(connfd,replay,strlen(replay),0)<0)
        printf("Send message Failed\n");

     close(connfd);

     struct sockaddr_in* clientaddr=fd_addr_map_[connfd];

            printf("Disconnect client:<%s,%d>\n",
                        inet_ntoa(clientaddr->sin_addr),
                        ntohs(clientaddr->sin_port)
                        );
    fd_addr_map_[connfd]=nullptr;

  //   delete [] path;
  //   delete [] method;
    // delete [] replay;
    // delete [] replay_data;
    // delete [] buf;
    return;
}

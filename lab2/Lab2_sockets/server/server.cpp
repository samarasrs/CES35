/* This is the server code */

//#include <unistd.h>
#include <string.h>
//#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
//#include <iostream>
#include <time.h>

//#include <pthread.h>

//para usar o inet_ntoa, include arpa/inet
//#include <arpa/inet.h>


#include <request.h>
#include <connection_thread.h>


#define SERVER_PORT 8080  /* arbitrary, but client & server must agree*/
//#define BUF_SIZE 4096  /* block transfer size */
#define QUEUE_SIZE 10
//#define GET "GET"


//using namespace std;
/*
typedef struct
{
    int sa;
    char * h_addr;
} connection_data;
*/

//void criarResp(long int status,int fd, string filename, char *respHeader, char *resp);
//void tratarSolic(string buf,long int &status, char *respHeader, char *resp);
//void * connectionHandle(void *arg);

int main(int argc, char *argv[])
{ 
   int s, b, l, fd, sa, bytes, on = 1;
   connection_data * connection;
   //char buf[BUF_SIZE];  /* buffer for outgoing file */
   struct sockaddr_in channel;  /* holds IP address */
   
   /* Build address structure to bind to socket. */
   memset(&channel, 0, sizeof(channel));
   
   /* zero channel */
   channel.sin_family = AF_INET;
   channel.sin_addr.s_addr = htonl(INADDR_ANY);
   channel.sin_port = htons(SERVER_PORT);
   
   /* Passive open. Wait for connection. */   
   s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP); /* create socket */   
   if (s < 0) {printf("socket call failed"); exit(-1);}
   
   setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (char *) &on, sizeof(on));
   
   b = bind(s, (struct sockaddr *) &channel, sizeof(channel));
   if (b < 0) {printf("bind failed"); exit(-1);} 
   
   l = listen(s, QUEUE_SIZE); /* specify queue size */
   if (l < 0) {printf("listen failed"); exit(-1);}
   
   /* Socket is now set up and bound. Wait for connection and process it. */
   pthread_t thread;
   while (1) {
      printf("Aguardando conexao.\n");
      connection = (connection_data*)malloc(sizeof(connection_data));
      connection->sa = accept(s, 0, 0); /* block for connection request */
      connection->h_addr = NULL;//h->h_addr;
      
    
      if (connection->sa < 0) {printf("accept failed"); exit(-1);}
      else printf("Conexao recebida.\n");
      

      
      if( pthread_create(&thread, NULL, connectionHandle, (void *)connection) != 0 )
           printf("Falha na criação da thread\n");
         
      else  
         pthread_detach(thread);    

   }   
}


/*
void criarResp(long int status,int fd, string filename, char *respHeader, char *resp)
{
   char aux[BUF_SIZE];
   time_t rawtime;
   time(&rawtime);
   char statusStr[20];
   int contentLen;
   

   if(status == 200)
   {
      strcpy(statusStr,"200 OK");      
      read(fd, aux, BUF_SIZE);
      contentLen = sprintf(resp,"%s",aux);
      close(fd);
   }
   else if(status == 404)
   {
      strcpy(statusStr,"404 Not Found");      
      contentLen = sprintf(resp,"<!DOCTYPE HTML PUBLIC>\n<html><head>\n<title>404 Not Found</title>\n</head><body>\n<h1>Not Found</h1>\n<p>The requested URL %s was not found on this server.</p>\n</body></html>\n",filename.c_str());

   }
   else
   {
      strcpy(statusStr,"400 Bad Request");      
      contentLen = sprintf(resp,"<!DOCTYPE HTML PUBLIC >\n<html><head>\n<title>400 Bad Request</title>\n</head><body>\n<h1>Bad Request</h1>\n<p>Your browser sent a request that this server could not understand.<br /></p>\n</body></html>\n");
   }   
   sprintf(respHeader,"HTTP/1.1 %s\r\nDate: %sServer: web-server\r\nContent-Length: %d\r\nContent-Type: text/html; charset=iso-8859-1\r\n\r\n",statusStr,ctime(&rawtime),contentLen);

}


void tratarSolic(string buf,long int &status, char *respHeader, char *resp)//,  char *h_addr)
{   
   //printf("BUFF: %s\n",buf.c_str() );
   status=200;
   int inicio,fim,fim2,fd;
   string filename;
  
   


   //observando se é GET
   string aux = buf.substr(0,3);
    
   if(strcmp(aux.c_str(),GET)!=0) status = 400;   
   //observando o host 
      //host do server
   char host[] = "localhost";
   struct hostent *h;
   h = gethostbyname(host);
   char addressHost[20];
   strcpy(addressHost,inet_ntoa(*(struct in_addr*)h->h_addr));
   printf("Host by addr %s\n",addressHost);
   
      //host do cliente
   inicio = buf.find("Host: ")+6;
   fim = buf.find("\r\n",inicio);
   fim2 = buf.find(":",inicio);
   if(fim2> 0 && fim2<fim)
      fim = fim2;
   aux = buf.substr(inicio,fim-inicio);

   
   struct hostent *hClient;

   hClient = gethostbyname(aux.c_str());

   char addressHostClient[20];
   strcpy(addressHostClient,inet_ntoa(*(struct in_addr*)hClient->h_addr));
   printf("Host do Cliente: %s\n",addressHostClient);

   if(strcmp(addressHostClient,addressHost)!=0) status = 400;
   //if(strcmp(aux.c_str(),h_name)!=0) status = 400;

   if(status==200)
   {
      //pegando filename
      inicio = 5;
      fim = buf.find(" ",inicio);      
      filename = buf.substr(inicio,fim-inicio);
      //tentando abrir filename
      fd = open(filename.c_str(), O_RDONLY);
      if (fd < 0)
      {
         printf("Arquivo %s invalido:open failed!\n",filename.c_str());
         status = 404;
      } 
   }
   
   criarResp(status,fd,filename, respHeader,resp);
   
   return;
}
/*
void * connectionHandle(void *arg)
{
   printf("Solicitação recebida: %d\n",(int) pthread_self() );

   char buf[BUF_SIZE];  // buffer for outgoing file 
   strcpy(buf," ");
   long int status=0;
   
   connection_data * conn = ((connection_data *)arg);
   read(conn->sa, buf, BUF_SIZE); // read file name from socket 
   char resp[BUF_SIZE],respHeader[BUF_SIZE];
   tratarSolic(buf,status,respHeader,resp);//,conn->h_addr);
   strcpy(buf,respHeader);
   strcat(buf,resp);
   write(conn->sa,buf,BUF_SIZE);
   printf("Resposta enviada. Status: %ld\n",status);
   
   close(conn->sa); // close connection  
   free(conn);
   pthread_exit(0);
         
}*/

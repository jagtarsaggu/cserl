    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <netdb.h>
    #include <stdio.h>
    #include <string.h>
    #include <stdlib.h>
    #include <unistd.h>
    #include <errno.h>
    #include <arpa/inet.h>
    #include <openssl/ssl.h>
    SSL_CTX *ssl_ctx;
    SSL *conn;
     
    int ReadHttpStatus(int sock){
        char c;
        char buff[1024]="",*ptr=buff+1;
        int bytes_received, status;
        printf("Begin Response ..\n");
        while(bytes_received = recv(sock, ptr, 1, 0)){
            if(bytes_received==-1){
                perror("ReadHttpStatus");
                exit(1);
            }
     
            if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
            ptr++;
        }
        *ptr=0;
        ptr=buff+1;
     
        sscanf(ptr,"%*s %d ", &status);
     
        printf("%s\n",ptr);
        printf("status=%d\n",status);
        printf("End Response ..\n");
        return (bytes_received>0)?status:0;
     
    }
    int ReadHttpsStatus(){
        char c;
        char buff[1024]="",*ptr=buff+1;
        int bytes_received, status;
        printf("Begin Response ..\n");
        while(bytes_received = SSL_read(conn, ptr, 1)){
            if(bytes_received==-1){
                perror("ReadHttpStatus");
                exit(1);
            }
     
            if((ptr[-1]=='\r')  && (*ptr=='\n' )) break;
            ptr++;
        }
        *ptr=0;
        ptr=buff+1;
     
        sscanf(ptr,"%*s %d ", &status);
     
        printf("%s\n",ptr);
        printf("status=%d\n",status);
        printf("End Response ..\n");
        return (bytes_received>0)?status:0;
     
    }
     
    //the only filed that it parsed is 'Content-Length' 
    int ParseHeader(int sock){
        char c;
        char buff[1024]="",*ptr=buff+4;
        int bytes_received, status;
        printf("Begin HEADER ..\n");
        while(bytes_received = recv(sock, ptr, 1, 0)){
            if(bytes_received==-1){
                perror("Parse Header");
                exit(1);
            }
     
            if(
                (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
                (ptr[-1]=='\r')  && (*ptr=='\n' )
            ) break;
            ptr++;
        }
     
        *ptr=0;
        ptr=buff+4;
        //printf("%s",ptr);
     
        if(bytes_received){
            ptr=strstr(ptr,"Content-Length:");
            if(ptr){
                sscanf(ptr,"%*s %d",&bytes_received);
            }else
                bytes_received=-1; //unknown size
     
           // printf("Content-Length: %d\n",bytes_received);
        }
        printf("End HEADER ..\n");
        return  bytes_received ;
     
    }
    int ParseHttpsHeader(){
        char c;
        char buff[1024]="",*ptr=buff+4;
        int bytes_received, status;
        printf("Begin HEADER ..\n");
        while(bytes_received = SSL_read(conn, ptr, 1)){
            if(bytes_received==-1){
                perror("Parse Header");
                exit(1);
            }
     
            if(
                (ptr[-3]=='\r')  && (ptr[-2]=='\n' ) &&
                (ptr[-1]=='\r')  && (*ptr=='\n' )
            ) break;
            ptr++;
        }
     
        *ptr=0;
        ptr=buff+4;
        //printf("%s",ptr);
     
        if(bytes_received){
            ptr=strstr(ptr,"Content-Length:");
            if(ptr){
                sscanf(ptr,"%*s %d",&bytes_received);
            }else
                bytes_received=-1; //unknown size
     
           // printf("Content-Length: %d\n",bytes_received);
        }
        printf("End HEADER ..\n");
        return  bytes_received ;
     
    }
    int main(int argc, char *args[]){
     
        char *URL;
    	URL = args[1];
    	char protocol[100];
    	char domain[156];
    	char path[100000];
    	char filename[10000];
    	int port_no=80;
    	// to be attained from URL or GET response.
    	int uri_scan_status = sscanf(URL, "%[^:]%*[:/]%[^/]/%s", protocol, domain, path);
        printf("Protocol: %s\n",protocol);
    	int end = strlen(path);
    	int i;
    	for(i=end-1;i>=0;i--)
    	{
    		if(path[i]=='/')
    		{
    			break;
    		}
    	}
    	i++;
    	int j=0;
    	for(;i<end;i++)
    	{
    		filename[j] = path[i];
    		j++;
    	}
    	filename[j]='\0';
        int sock, bytes_received;  
        char send_data[110000],recv_data[1024], *p;
        struct sockaddr_in server_addr;
        struct hostent *he;
     
        printf("Protocol: %s\n",protocol);
        if(strcmp(protocol,"https")==0)
        {
            printf("Protocol: %s\n",protocol);
            port_no = 443;
        }
     
        he = gethostbyname(domain);
        if (he == NULL){
           herror("gethostbyname");
           fprintf(stderr, "host doesn't exist or unreachable\n");
           exit(1);
        }
     
        if ((sock = socket(AF_INET, SOCK_STREAM, 0))== -1){
           perror("Socket");
           exit(1);
        }
        server_addr.sin_family = AF_INET;     
        server_addr.sin_port = htons(port_no);
        server_addr.sin_addr = *((struct in_addr *)he->h_addr);
        bzero(&(server_addr.sin_zero),8); 
     
        printf("Connecting ...\n");
        if (connect(sock, (struct sockaddr *)&server_addr,sizeof(struct sockaddr)) == -1){
           perror("Connect");
           exit(1); 
        }
     
        if(strcmp(protocol,"https")==0)
        {
            SSL_load_error_strings ();
            SSL_library_init ();
            ssl_ctx = SSL_CTX_new (SSLv23_client_method ());
            conn = SSL_new(ssl_ctx);
            SSL_set_fd(conn, sock);
            int err = SSL_connect(conn);
            if (err != 1)
                abort(); // handle error
        }
        printf("Sending data ...\n");
     
        snprintf(send_data, sizeof(send_data), "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", path, domain);
     
        if(strcmp(protocol,"https")==0)
        {
            if(SSL_write(conn,send_data,strlen(send_data))==-1){
                perror("SSL_Write");
                exit(2);
            }
            printf("data sent.\n");
            printf("Recieving data...\n\n");
     
            int contentlengh;
     
            int status = ReadHttpsStatus();
            if(status >= 300)
            {
                fprintf(stderr,"error in file either not found or redirect::--\n");
                exit(0);
            }
            if(status && (contentlengh=ParseHttpsHeader())){
     
                int bytes=0;
                FILE* fd=fopen(filename,"wb");
                printf("\n\nSaving data...\n\n");
     
                while(bytes_received = SSL_read(conn, recv_data, 1024)){
                    if(bytes_received<=0){
                        perror("recieve");
                        exit(3);
                    }
     
     
                    fwrite(recv_data,1,bytes_received,fd);
                    bytes+=bytes_received;
                    // printf("Bytes recieved: %d outof %d\n",bytes,contentlengh);
                    if(bytes==contentlengh)
                    break;
                }
                fclose(fd);
            }
            SSL_free(conn);
        }
        else
        {
            if(send(sock, send_data, strlen(send_data), 0)==-1){
                perror("send");
                exit(2); 
            }
            printf("Data sent.\n");  
     
            // fp=fopen("received_file","wb");
            printf("Recieving data...\n\n");
     
            int contentlengh;
     
            int status = ReadHttpStatus(sock);
            if(status >= 300)
            {
            	fprintf(stderr,"error in file either not found or redirect::--\n");
            	exit(0);
            }
            if(status && (contentlengh=ParseHeader(sock))){
     
                int bytes=0;
                FILE* fd=fopen(filename,"wb");
                printf("\n\nSaving data...\n\n");
     
                while(bytes_received = recv(sock, recv_data, 1024, 0)){
                    if(bytes_received<=0){
                        perror("recieve");
                        exit(3);
                    }
     
     
                    fwrite(recv_data,1,bytes_received,fd);
                    bytes+=bytes_received;
                    // printf("Bytes recieved: %d outof %d\n",bytes,contentlengh);
                    if(bytes==contentlengh)
                    break;
                }
                fclose(fd);
            }
        }
        close(sock);
        printf("\n\nDone.\n\n");
        return 0;
    }

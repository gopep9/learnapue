#ifndef PROXYTOOL_H
#define PROXYTOOL_H
struct ClientAndRemoteSocks{
    int clientSock;
    int remoteSock;
    bool stopConnect;
};

//读取socket一行
static int get_line(int sock, char *buf, int size)
{
    int i = 0;
    char c = '\0';
    int n;

    while ((i < size - 1) && (c != '\n'))
    {
        n = recv(sock, &c, 1, 0);
        /* DEBUG printf("%02X\n", c); */
        buf[i]=c;
        i++;
    }
    buf[i] = '\0';

    return(i);
}


//static int get_line(int sock, char *buf, int size)
//{
//    int i = 0;
//    char c = '\0';
//    int n;

//    while ((i < size - 1) && (c != '\n'))
//    {
//        n = recv(sock, &c, 1, 0);
//        /* DEBUG printf("%02X\n", c); */
//        if (n > 0)
//        {
//            if (c == '\r')
//            {
//                n = recv(sock, &c, 1, MSG_PEEK);
//                /* DEBUG printf("%02X\n", c); */
//                if ((n > 0) && (c == '\n'))
//                    recv(sock, &c, 1, 0);
//                else
//                    c = '\n';
//            }
//            buf[i] = c;
//            i++;
//        }
//        else
//            c = '\n';
//    }
//    buf[i] = '\0';

//    return(i);
//}




//根据网址和端口号获取socket
int Socket(const char *host, int clientPort)
{
    int sock;
    unsigned long inaddr;
    struct sockaddr_in ad;
    struct hostent *hp;

    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;

    inaddr = inet_addr(host);
    if (inaddr != INADDR_NONE)
        memcpy(&ad.sin_addr, &inaddr, sizeof(inaddr));
    else
    {
        hp = gethostbyname(host);
        if (hp == NULL)
            return -1;
        memcpy(&ad.sin_addr, hp->h_addr, hp->h_length);
    }
    ad.sin_port = htons(clientPort);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
        return sock;
    if (connect(sock, (struct sockaddr *)&ad, sizeof(ad)) < 0)
        return -1;
    return sock;
}

#endif // PROXYTOOL_H

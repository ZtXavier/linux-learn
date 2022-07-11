#include<signal.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<stdint.h>
#include<endian.h>
#include<byteswap.h>
#include<getopt.h>
#include<sys/time.h>
#include<arpa/inet.h>
// #include<infiniband/verbs.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>

#define MAX_POLL_CQ_TIMEOUT 2000
#define MSG "SEND operation"
#define RDMAMSGR "RDMA read operation"
#define RDMAMSGW "RDMA write operation"
#define MSG_SIZE (strlen(MSG) + 1)


#if _BYTE_ORDER == __LITTLE_ENDIAN
static inline uint64_t htonl(uint64_t x) {return bswap_64(x);}
static inline uint64_t ntonl(uint64_t x) {return bswap_64(x);}
#else 
#error _BYTE_ORDER is neither _LITTLE_ENDIAN nor _BIG_ENDIAN
#endif


struct config_t
{
    const char *dev_name;
    char *server_name;
    u_int32_t tcp_port;
    int ib_port;
    int gid_idx;
};

struct cm_con_data_t
{
    uint64_t addr;  // Buffer address
    uint32_t rkey;  // Remote key
    uint32_t qp_num; // QP number
    uint16_t lid;   //  LID of the IB port
    uint8_t gid[16];  // gid   
}_attribute_((packed));

struct resources
{
    struct ibv_device_attr device_attr;    // Device attrubutes
    struct ibv_prot_attr port_attr;
    struct cm_con_data_t remote_props;
    struct ibv_context *ib_ctx;
    struct ibv_pd *pd;
    struct ibv_cq *cq;
    struct ibv_qp *handle;
    struct ibv_mr *mr;
    char *buf;
    int sock;
};

struct config_t config={
    NULL,
    NULL,
    19875,
    1,
    -1
};


static int sock_connect(const char *servername,int port)
{
    struct addrinfo *resolved_addr = NULL;
    struct addrinfo *iterator;
    char service[6];
    int sockfd = -1;
    int listenfd = 0;
    int tmp;
    struct addrinfo hints = {
        .ai_flags = AI_PASSIVE,
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM
    };
    if(sprintf(service,"%d",port) < 0)
    {
        goto sock_connenct_exit;
    }
    sockfd = getaddrinfo(servername,service,&hints,&resolved_addr);
    if(sockfd < 0)
    {
        fprintf(stderr,"%s for %s:%d\n",gai_strerror(sockfd),servername,port);
        goto sock_connenct_exit;
    }
}


#include "lwip/sockets.h"
#include "lwip/netif.h"
#include "arch/cc.h"

/** default port number for "NetBIOS Name service */
#define NETBIOS_PORT     	137

/** size of a NetBIOS name */
#define NETBIOS_NAME_LEN 	16
#define NETBIOS_NAME 		"RTTHREAD"

/** The Time-To-Live for NetBIOS name responds (in seconds)
 * Default is 300000 seconds (3 days, 11 hours, 20 minutes) */
#define NETBIOS_NAME_TTL 	300000

/** NetBIOS header flags */
#define NETB_HFLAG_RESPONSE           0x8000U
#define NETB_HFLAG_OPCODE             0x7800U
#define NETB_HFLAG_OPCODE_NAME_QUERY  0x0000U
#define NETB_HFLAG_AUTHORATIVE        0x0400U
#define NETB_HFLAG_TRUNCATED          0x0200U
#define NETB_HFLAG_RECURS_DESIRED     0x0100U
#define NETB_HFLAG_RECURS_AVAILABLE   0x0080U
#define NETB_HFLAG_BROADCAST          0x0010U
#define NETB_HFLAG_REPLYCODE          0x0008U
#define NETB_HFLAG_REPLYCODE_NOERROR  0x0000U

/** NetBIOS name flags */
#define NETB_NFLAG_UNIQUE             0x8000U
#define NETB_NFLAG_NODETYPE           0x6000U
#define NETB_NFLAG_NODETYPE_HNODE     0x6000U
#define NETB_NFLAG_NODETYPE_MNODE     0x4000U
#define NETB_NFLAG_NODETYPE_PNODE     0x2000U
#define NETB_NFLAG_NODETYPE_BNODE     0x0000U

/* NetBIOS message header */
struct netbios_hdr
{
    rt_uint16_t trans_id;
    rt_uint16_t flags;
    rt_uint16_t questions;
    rt_uint16_t answerRRs;
    rt_uint16_t authorityRRs;
    rt_uint16_t additionalRRs;
};

/** NetBIOS message name part */
PACK_STRUCT_BEGIN
struct netbios_name_hdr
{
    rt_uint8_t  nametype;
    rt_uint8_t  encname[(NETBIOS_NAME_LEN*2)+1];
    rt_uint16_t type;
    rt_uint16_t cls;
    rt_uint32_t ttl;
    rt_uint16_t datalen;
    rt_uint16_t flags;

    struct ip_addr_packed addr;
}PACK_STRUCT_STRUCT;
PACK_STRUCT_END

/** NetBIOS message */
struct netbios_resp
{
    struct netbios_hdr      resp_hdr;
    struct netbios_name_hdr resp_name;
};

/** NetBIOS decoding name */
static int netbios_name_decoding(char *name_enc, char *name_dec, int name_dec_len)
{
    char *pname;
    char  cname;
    char  cnbname;
    int   index = 0;

    LWIP_UNUSED_ARG(name_dec_len);

    /* Start decoding netbios name. */
    pname  = name_enc;
    for (;;)
    {
        /* Every two characters of the first level-encoded name
         * turn into one character in the decoded name. */
        cname = *pname;
        if (cname == '\0')
            break;    /* no more characters */
        if (cname == '.')
            break;    /* scope ID follows */
        if (cname < 'A' || cname > 'Z')
        {
            /* Not legal. */
            return -1;
        }
        cname -= 'A';
        cnbname = cname << 4;
        pname++;

        cname = *pname;
        if (cname == '\0' || cname == '.')
        {
            /* No more characters in the name - but we're in
             * the middle of a pair.  Not legal. */
            return -1;
        }
        if (cname < 'A' || cname > 'Z')
        {
            /* Not legal. */
            return -1;
        }
        cname -= 'A';
        cnbname |= cname;
        pname++;

        /* Do we have room to store the character? */
        if (index < NETBIOS_NAME_LEN)
        {
            /* Yes - store the character. */
            name_dec[index++] = (cnbname!=' '?cnbname:'\0');
        }
    }

    return 0;
}

/** NetBIOS Name service recv callback */
static void netbios_handle(rt_uint8_t *netbios_pdu, int sock, const struct sockaddr *client_addr)
{
	rt_uint32_t addr_len;
    char   netbios_name[NETBIOS_NAME_LEN+1];
    struct netbios_hdr*      netbios_hdr      = (struct netbios_hdr*)netbios_pdu;
    struct netbios_name_hdr* netbios_name_hdr = (struct netbios_name_hdr*)(netbios_hdr+1);
	
    addr_len = sizeof(struct sockaddr);

    /* we only answer if we got a default interface */
    if (netif_default != NULL)
    {
        /* if the packet is a NetBIOS name query question */
        if (((netbios_hdr->flags & ntohs(NETB_HFLAG_OPCODE)) == ntohs(NETB_HFLAG_OPCODE_NAME_QUERY)) &&
                ((netbios_hdr->flags & ntohs(NETB_HFLAG_RESPONSE)) == 0) &&
                (netbios_hdr->questions == ntohs(1)))
        {
            /* decode the NetBIOS name */
            netbios_name_decoding( (char*)(netbios_name_hdr->encname), netbios_name, sizeof(netbios_name));

            /* if the packet is for us */
            if (rt_strcmp(netbios_name, NETBIOS_NAME) == 0)
            {
                struct netbios_resp _resp, *resp;

                resp = &_resp;
                /* prepare NetBIOS header response */
                resp->resp_hdr.trans_id      = netbios_hdr->trans_id;
                resp->resp_hdr.flags         = htons(NETB_HFLAG_RESPONSE |
                                                        NETB_HFLAG_OPCODE_NAME_QUERY |
                                                        NETB_HFLAG_AUTHORATIVE |
                                                        NETB_HFLAG_RECURS_DESIRED);
                resp->resp_hdr.questions     = 0;
                resp->resp_hdr.answerRRs     = htons(1);
                resp->resp_hdr.authorityRRs  = 0;
                resp->resp_hdr.additionalRRs = 0;

                /* prepare NetBIOS header datas */
                rt_memcpy( resp->resp_name.encname, netbios_name_hdr->encname, sizeof(netbios_name_hdr->encname));
                resp->resp_name.nametype     = netbios_name_hdr->nametype;
                resp->resp_name.type         = netbios_name_hdr->type;
                resp->resp_name.cls          = netbios_name_hdr->cls;
                resp->resp_name.ttl          = htonl(NETBIOS_NAME_TTL);
                resp->resp_name.datalen      = htons(sizeof(resp->resp_name.flags)+sizeof(resp->resp_name.addr));
                resp->resp_name.flags        = htons(NETB_NFLAG_NODETYPE_BNODE);
				resp->resp_name.addr.addr	 = netif_default->ip_addr.addr;

                /* send the NetBIOS response */
                lwip_sendto(sock, resp, sizeof(struct netbios_resp), 0,
                       client_addr, addr_len);
            }
        }
    }
}

#define BUF_SZ	128
void netbios_entry(void* parameter)
{
    int sock;
    rt_uint8_t *recv_data;
    rt_uint32_t addr_len, bytes_read;
    struct sockaddr_in server_addr, client_addr;

    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        rt_kprintf("create socket error\n");
        return;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(NETBIOS_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    rt_memset(&(server_addr.sin_zero),0, sizeof(server_addr.sin_zero));

    if (bind(sock,(struct sockaddr *)&server_addr,
             sizeof(struct sockaddr)) == -1)
    {
        rt_kprintf("bind error\n");
        return;
    }
    addr_len = sizeof(struct sockaddr);

    recv_data = (rt_uint8_t*)rt_malloc(BUF_SZ);
    if (recv_data == RT_NULL)
    {
        rt_kprintf("no memory yet\n");
        lwip_close(sock);
        return;
    }

    while (1)
    {
        /* recv on NetBIOS port */
        bytes_read = recvfrom(sock, recv_data, BUF_SZ, 0,
                              (struct sockaddr *)&client_addr, &addr_len);

		if (bytes_read >= sizeof(struct netbios_name_hdr))
        {
            /* handle on NetBIOS request */
			netbios_handle(recv_data, sock, (const struct sockaddr*)&client_addr);
        }
    }
}

void netbios_init(void)
{
    rt_thread_t tid;

    tid = rt_thread_create("netbios", netbios_entry, RT_NULL,
                           1024, RT_THREAD_PRIORITY_MAX/2, 5);
    if (tid != RT_NULL) rt_thread_startup(tid);
}

#ifdef RT_USING_FINSH
#include <finsh.h>
FINSH_FUNCTION_EXPORT(netbios_init, init netbios naming server);
#endif

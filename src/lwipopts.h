#ifndef __LWIPOPTS_H__
#define __LWIPOPTS_H__

#include <stdio.h>

/**
 * SYS_LIGHTWEIGHT_PROT==1: if you want inter-task protection for certain
 * critical regions during buffer allocation, deallocation and memory
 * allocation and deallocation.
 */
#define SYS_LIGHTWEIGHT_PROT    0

/**
 * NO_SYS==1: Provides VERY minimal functionality. Otherwise,
 * use lwIP facilities.
 */
#define NO_SYS                  1

/* ---------- Memory options ---------- */
/* MEM_ALIGNMENT: should be set to the alignment of the CPU for which
   lwIP is compiled. 4 byte alignment -> define MEM_ALIGNMENT to 4, 2
   byte alignment -> define MEM_ALIGNMENT to 2. */
#define MEM_ALIGNMENT           4

/* MEM_SIZE: the size of the heap memory. If the application will send
a lot of data that needs to be copied, this should be set high. */
#define MEM_SIZE                (8*1024)

/* MEMP_NUM_PBUF: the number of memp struct pbufs. If the application
   sends a lot of data out of ROM (or other static memory), this
   should be set high. */
#define MEMP_NUM_PBUF           10
/* MEMP_NUM_UDP_PCB: the number of UDP protocol control blocks. One
   per active UDP "connection". */
#define MEMP_NUM_UDP_PCB        3
/* MEMP_NUM_TCP_PCB: the number of simulatenously active TCP
   connections. */
#define MEMP_NUM_TCP_PCB        10
/* MEMP_NUM_TCP_PCB_LISTEN: the number of listening TCP
   connections. */
#define MEMP_NUM_TCP_PCB_LISTEN 6
/* MEMP_NUM_TCP_SEG: the number of simultaneously queued TCP
   segments. */
#define MEMP_NUM_TCP_SEG        12
/* MEMP_NUM_SYS_TIMEOUT: the number of simulateously active
   timeouts. */
#define MEMP_NUM_SYS_TIMEOUT    6


/* ---------- Pbuf options ---------- */
/* PBUF_POOL_SIZE: the number of buffers in the pbuf pool. */
#define PBUF_POOL_SIZE          10

/* PBUF_POOL_BUFSIZE: the size of each pbuf in the pbuf pool. */
#define PBUF_POOL_BUFSIZE       1500


/* ---------- TCP options ---------- */
#define LWIP_TCP                1
#define LWIP_TCP_KEEPALIVE      1
#define TCP_TTL                 255
#define TCP_LISTEN_BACKLOG      1

/* Controls if TCP should queue segments that arrive out of
   order. Define to 0 if your device is low on memory. */
#define TCP_QUEUE_OOSEQ         0

/* TCP Maximum segment size. */
#define TCP_MSS                 (1500 - 40)	  /* TCP_MSS = (Ethernet MTU - IP header size - TCP header size) */

/* TCP sender buffer space (bytes). */
#define TCP_SND_BUF             (2*TCP_MSS)

/* TCP sender buffer space (pbufs). This must be at least = 2 *
   TCP_SND_BUF/TCP_MSS for things to work. */
#define TCP_SND_QUEUELEN        (6*TCP_SND_BUF)/TCP_MSS

/* TCP receive window. */
#define TCP_WND                 (2*TCP_MSS)

#define SO_REUSE                1

/* ---------- ICMP options ---------- */
#define LWIP_ICMP               1

#define LWIP_AUTOIP             1

/* ---------- DHCP options ---------- */
/* Define LWIP_DHCP to 1 if you want DHCP configuration of
   interfaces. DHCP is not implemented in lwIP 0.5.1, however, so
   turning this on does currently not work. */
#define LWIP_DHCP               1
#define LWIP_DHCP_AUTOIP_COOP   1

/* ---------- UDP options ---------- */
#define LWIP_UDP                1
#define UDP_TTL                 255

#define LWIP_NETIF_STATUS_CALLBACK 0

/* ---------- Statistics options ---------- */
#define LWIP_STATS              0
#define LWIP_STATS_DISPLAY      0
#define LWIP_PROVIDE_ERRNO      0

#define LWIP_HTTPD_FILE_STATE      1
#define LWIP_HTTPD_DYNAMIC_HEADERS 1
#define LWIP_HTTPD_CUSTOM_FILES    1
#define LWIP_HTTPD_SSI             1
#define LWIP_HTTPD_SSI_INCLUDE_TAG 0
#define LWIP_HTTPD_USE_AUTH_BASIC  1
#define HTTP_IS_DATA_VOLATILE(hs) ((hs->file < (char *)0x20000000) ? 0 : TCP_WRITE_FLAG_COPY)
#define HTTPD_USE_CUSTUM_FSDATA    0
#define HTTPD_POLL_INTERVAL        1
/*
   --------------------------------------
   ---------- Checksum options ----------
   --------------------------------------
*/

//#define CHECKSUM_BY_HARDWARE

#ifdef CHECKSUM_BY_HARDWARE
  /* CHECKSUM_GEN_IP==0: Generate checksums by hardware for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 0
  /* CHECKSUM_GEN_UDP==0: Generate checksums by hardware for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                0
  /* CHECKSUM_GEN_TCP==0: Generate checksums by hardware for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                0
  /* CHECKSUM_CHECK_IP==0: Check checksums by hardware for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               0
  /* CHECKSUM_CHECK_UDP==0: Check checksums by hardware for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              0
  /* CHECKSUM_CHECK_TCP==0: Check checksums by hardware for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              0
  /* CHECKSUM_GEN_ICMP==0: Generate checksums by hardware for outgoing ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               0
#else
  /* CHECKSUM_GEN_IP==1: Generate checksums in software for outgoing IP packets.*/
  #define CHECKSUM_GEN_IP                 1
  /* CHECKSUM_GEN_UDP==1: Generate checksums in software for outgoing UDP packets.*/
  #define CHECKSUM_GEN_UDP                1
  /* CHECKSUM_GEN_TCP==1: Generate checksums in software for outgoing TCP packets.*/
  #define CHECKSUM_GEN_TCP                1
  /* CHECKSUM_CHECK_IP==1: Check checksums in software for incoming IP packets.*/
  #define CHECKSUM_CHECK_IP               1
  /* CHECKSUM_CHECK_UDP==1: Check checksums in software for incoming UDP packets.*/
  #define CHECKSUM_CHECK_UDP              1
  /* CHECKSUM_CHECK_TCP==1: Check checksums in software for incoming TCP packets.*/
  #define CHECKSUM_CHECK_TCP              1
  /* CHECKSUM_GEN_ICMP==1: Generate checksums in software for outgoing ICMP packets.*/
  #define CHECKSUM_GEN_ICMP               1
#endif


/*
   ----------------------------------------------
   ---------- Sequential layer options ----------
   ----------------------------------------------
*/
/**
 * LWIP_NETCONN==1: Enable Netconn API (require to use api_lib.c)
 */
#define LWIP_NETCONN                    0

/*
   ------------------------------------
   ---------- Socket options ----------
   ------------------------------------
*/
/**
 * LWIP_SOCKET==1: Enable Socket API (require to use sockets.c)
 */
#define LWIP_SOCKET                     0

#define LWIP_DEBUG
#define LWIP_PLATFORM_DIAG(x)           do {printf x;} while(0)
#define LWIP_PLATFORM_ASSERT(x)         do {printf(x);} while(0)

//#define PBUF_DEBUG                      LWIP_DBG_ON
#define ICMP_DEBUG                      LWIP_DBG_ON
//#define IGMP_DEBUG                      LWIP_DBG_ON
#define DHCP_DEBUG                      LWIP_DBG_ON
//#define TIMERS_DEBUG                    LWIP_DBG_ON
//#define INET_DEBUG                      LWIP_DBG_ON
//#define IP_DEBUG                        LWIP_DBG_ON
//#define RAW_DEBUG                       LWIP_DBG_ON
//#define SYS_DEBUG                       LWIP_DBG_ON
//#define TCP_DEBUG                       LWIP_DBG_ON
//#define UDP_DEBUG                       LWIP_DBG_ON
//#define ETHARP_DEBUG                    LWIP_DBG_ON

#endif /* __LWIPOPTS_H__ */

/*
 * This file is part of the sk-mstm32f107-example project.
 *
 * Copyright (C) 2016 Alexey Ryabov <6006l1k@gmail.com>
 *
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>

#include <string.h>

#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp_oe.h"

/* Network interface name. */
#define IFNAME0 'e'
#define IFNAME1 'n'

static uint8_t eth_frame[2000];
static bool link;

static err_t low_level_output(struct netif *netif, struct pbuf *p)
{
  uint32_t l = 0;
  struct pbuf *q;

#if ETH_PAD_SIZE
  pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

  for(q = p; q != NULL; q = q->next) {
    memcpy(&eth_frame[l], q->payload, q->len);
    l += q->len;
  }

  eth_tx(eth_frame, l);

#if ETH_PAD_SIZE
  pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

  LINK_STATS_INC(link.xmit);

  return ERR_OK;
}

static struct pbuf * low_level_input(struct netif *netif)
{
  struct pbuf *p, *q;
  uint32_t l = 0, len = 0;

  /* Receive packet to intermediate buffer and obtain the size of the packet */
  if (!eth_rx(eth_frame, &len, sizeof(eth_frame)))
    return NULL;

#if ETH_PAD_SIZE
  len += ETH_PAD_SIZE; /* allow room for Ethernet padding */
#endif

  /* We allocate a pbuf chain of pbufs from the pool. */
  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);

  if (p != NULL) {

#if ETH_PAD_SIZE
    pbuf_header(p, -ETH_PAD_SIZE); /* drop the padding word */
#endif

    /* We iterate over the pbuf chain until we have read the entire
     * packet into the pbuf. */
    for(q = p; q != NULL; q = q->next) {
      /* Read enough bytes to fill this pbuf in the chain. The
       * available data in the pbuf is given by the q->len
       * variable.
       */
        memcpy(q->payload, &eth_frame[l], q->len);
        l += q->len;
    }

#if ETH_PAD_SIZE
    pbuf_header(p, ETH_PAD_SIZE); /* reclaim the padding word */
#endif

    LINK_STATS_INC(link.recv);
  } else {
    LINK_STATS_INC(link.memerr);
    LINK_STATS_INC(link.drop);
  }

  return p;
}

static void ksz8721if_input(struct netif *netif)
{
  struct eth_hdr *ethhdr;
  struct pbuf *p;

  /* move received packet into a new pbuf */
  p = low_level_input(netif);
  /* no packet could be read, silently ignore this */
  if (p == NULL) return;
  /* points to packet payload, which starts with an Ethernet header */
  ethhdr = p->payload;

  switch (htons(ethhdr->type)) {
  /* IP or ARP packet? */
  case ETHTYPE_IP:
  case ETHTYPE_ARP:
#if PPPOE_SUPPORT
  /* PPPoE packet? */
  case ETHTYPE_PPPOEDISC:
  case ETHTYPE_PPPOE:
#endif /* PPPOE_SUPPORT */
    /* full packet send to tcpip_thread to process */
    if (netif->input(p, netif)!=ERR_OK)
     { LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_input: IP input error\n"));
       pbuf_free(p);
       p = NULL;
     }
    break;

  default:
    pbuf_free(p);
    p = NULL;
    break;
  }
}

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t ksz8721if_init(struct netif *netif)
{
  LWIP_ASSERT("netif != NULL", (netif != NULL));

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, LINK_SPEED_OF_YOUR_NETIF_IN_BPS);

  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* set MAC hardware address */
  netif->hwaddr[0] = 0x00;
  netif->hwaddr[1] = 0x01;
  netif->hwaddr[2] = 0x02;
  netif->hwaddr[3] = 0x03;
  netif->hwaddr[4] = 0x04;
  netif->hwaddr[5] = 0x05;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP;

  return ERR_OK;
}

/**
 * This function should be called from main loop.
 */
void ksz8721if_poll(int phy, struct netif * netif)
{
  bool link_isup;

  link_isup = phy_link_isup(phy);

  if (link_isup != link) {
    link = link_isup;
    if (link) {
      netif_set_link_up(netif);
    }
    else {
      netif_set_link_down(netif);
    }
  }

  ksz8721if_input(netif);
}

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

#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/usart.h>
#include <libopencm3/ethernet/mac.h>
#include <libopencm3/ethernet/phy.h>
#include <libopencm3/cm3/systick.h>

#include <netif/etharp.h>
#include <lwip/init.h>
#include <lwip/dhcp.h>
#include <lwip/ip_addr.h>
#include <lwip/timers.h>

#include "ksz8721if.h"

#include <errno.h>
#include <stdio.h>

#define PHY_ADDRESS PHY1

#define GPIO_ETH_MII_MDIO GPIO2     /* PA2 */
#define GPIO_ETH_MII_MDC GPIO1      /* PC1 */
#define GPIO_ETH_MII_TX_EN GPIO11   /* PB11 */
#define GPIO_ETH_MII_RX_ER GPIO10   /* PB10 */
#define GPIO_ETH_MII_TXD0 GPIO12    /* PB12 */
#define GPIO_ETH_MII_TXD1 GPIO13    /* PB13 */
#define GPIO_ETH_MII_TXD2 GPIO2     /* PC2 */
#define GPIO_ETH_MII_TXD3 GPIO8     /* PB8 */
#define GPIO_ETH_MII_TX_CLK GPIO3   /* PC3 */
#define GPIO_ETH_MII_CRS_WKUP GPIO0 /* PA0 */
#define GPIO_ETH_MII_PPS_OUT GPIO5  /* PB5 */

#define GPIO_ETH_MII_RE_RX_DV GPIO8 /* PD8 */
#define GPIO_ETH_MII_RE_RXD0 GPIO9  /* PD9 */
#define GPIO_ETH_MII_RE_RXD1 GPIO10 /* PD10 */
#define GPIO_ETH_MII_RE_RXD2 GPIO11 /* PD11 */
#define GPIO_ETH_MII_RE_RXD3 GPIO12 /* PD12 */

#define GPIO_ETH_RMII_MDIO GPIO2    /* PA2 */
#define GPIO_ETH_RMII_MDC GPIO1     /* PC1 */
#define GPIO_ETH_RMII_PPS_OUT GPIO5 /* PB5 */
#define GPIO_ETH_RMII_TX_EN GPIO11  /* PB11 */
#define GPIO_ETH_RMII_TXD0 GPIO12   /* PB12 */
#define GPIO_ETH_RMII_TXD1 GPIO13   /* PB13 */
#define GPIO_ETH_RMII_REF_CLK GPIO1 /* PA1 */
#define GPIO_ETH_RMII_CRS_DV GPIO7  /* PA7 */
#define GPIO_ETH_RMII_RXD0 GPIO4    /* PC4 */
#define GPIO_ETH_RMII_RXD1 GPIO5    /* PC5 */

#define GPIO_ETH_RMII_RE_CRS_DV GPIO8   /* PD8 */
#define GPIO_ETH_RMII_RE_RXD0 GPIO9     /* PD9 */
#define GPIO_ETH_RMII_RE_RXD1 GPIO10    /* PD10 */

#define GPIO_MCO GPIO8              /* PA8 */

#define ETH_TXBUFNB 2
#define ETH_RXBUFNB 2
#define ETH_TX_BUF_SIZE (1520 + 16)
#define ETH_RX_BUF_SIZE (1520 + 16)

uint8_t eth_descs[(ETH_TXBUFNB * ETH_TX_BUF_SIZE) + \
                  (ETH_RXBUFNB * ETH_RX_BUF_SIZE)];
struct netif netif;
uint32_t ticks;

int _write(int file, char * ptr, int len)
{
    int i;

    if (file == 1) {
        for (i = 0; i < len; i++) {
            if (ptr[i] == '\n')
                usart_send_blocking(USART2, '\r');
            usart_send_blocking(USART2, ptr[i]);
        }
        return i;
    }

    errno = EIO;
    return -1;
}

int _read(int file, char * ptr, int len)
{
    int i;

    if (file == 0) {
        for (i = 0; i < len; i++)
            ptr[i] = usart_recv_blocking(USART2);
        return i;
    }

    errno = EIO;
    return -1;
}

static void clock_setup(void)
{
    rcc_clock_setup_in_hse_25mhz_out_72mhz();

    rcc_periph_clock_enable(RCC_GPIOA);
    rcc_periph_clock_enable(RCC_GPIOB);
    rcc_periph_clock_enable(RCC_GPIOC);
    rcc_periph_clock_enable(RCC_GPIOD);
    rcc_periph_clock_enable(RCC_USART2);
    rcc_periph_clock_enable(RCC_AFIO);

    rcc_periph_clock_enable(RCC_ETHMAC);
    rcc_periph_clock_enable(RCC_ETHMACRX);
    rcc_periph_clock_enable(RCC_ETHMACTX);

    /* 50MHz for PHY */
    rcc_set_pll3_multiplication_factor(RCC_CFGR2_PLL3MUL_PLL3_CLK_MUL10);
    rcc_osc_on(RCC_PLL3);
    rcc_wait_for_osc_ready(RCC_PLL3);
    rcc_set_mco(RCC_CFGR_MCO_PLL3);

    systick_set_reload(rcc_ahb_frequency / 1000UL);
    systick_set_clocksource(STK_CSR_CLKSOURCE_AHB);
    systick_counter_enable();
    systick_interrupt_enable();

    /* GPIO Remap */
    gpio_primary_remap(AFIO_MAPR_SWJ_CFG_FULL_SWJ, AFIO_MAPR_ETH_REMAP |
            AFIO_MAPR_MII_RMII_SEL | AFIO_MAPR_USART2_REMAP);
}

static void eth_setup(void)
{
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_ETH_RMII_MDIO);
    gpio_set_mode(GPIOC, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_ETH_RMII_MDC);
    gpio_set_mode(GPIOB, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_ETH_RMII_TX_EN |
            GPIO_ETH_RMII_TXD0 | GPIO_ETH_RMII_TXD1);
    gpio_set_mode(GPIOA, GPIO_MODE_OUTPUT_50_MHZ,
            GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_MCO);
    gpio_set_mode(GPIOA, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, GPIO_ETH_RMII_REF_CLK);
    gpio_set_mode(GPIOD, GPIO_MODE_INPUT,
                GPIO_CNF_INPUT_FLOAT, GPIO_ETH_RMII_RE_CRS_DV |
                GPIO_ETH_RMII_RE_RXD0 | GPIO_ETH_RMII_RE_RXD1);

    rcc_periph_reset_pulse(RST_ETHMAC);

    eth_init(PHY_ADDRESS, ETH_CLK_060_100MHZ);
    eth_desc_init(eth_descs, ETH_TXBUFNB, ETH_RXBUFNB, ETH_TX_BUF_SIZE,
        ETH_RX_BUF_SIZE, false);
    eth_start();
}

static void usart2_setup(void)
{
    /* GPIOD: USART2 Tx */
    gpio_set_mode(GPIOD, GPIO_MODE_OUTPUT_50_MHZ,
        GPIO_CNF_OUTPUT_ALTFN_PUSHPULL, GPIO_USART2_RE_TX);

    /* Setup UART parameters. */
    usart_set_baudrate(USART2, 115200);
    usart_set_databits(USART2, 8);
    usart_set_stopbits(USART2, USART_STOPBITS_1);
    usart_set_parity(USART2, USART_PARITY_NONE);
    usart_set_flow_control(USART2, USART_FLOWCONTROL_NONE);
    usart_set_mode(USART2, USART_MODE_TX);

    /* Finally enable the UART. */
    usart_enable(USART2);

    /* Disable buffering */
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
}

int main(void)
{
    clock_setup();
    usart2_setup();
    eth_setup();
    lwip_init();

    printf("SK-MSTM32F107 Demo board\n");

    netif_add(&netif, NULL, NULL, NULL, NULL, &ksz8721if_init, &ethernet_input);
    netif_set_default(&netif);
    netif_set_up(&netif);
    dhcp_start(&netif);

    for(;;) {
        sys_check_timeouts();
        ksz8721if_poll(PHY_ADDRESS, &netif);
    }
}

uint32_t sys_now()
{
    return ticks;
}

void sys_tick_handler(void)
{
    ticks++;
}
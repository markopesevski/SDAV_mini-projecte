#include "stdio.h"
#include "xparameters.h"
#include "netif/xadapter.h"
#include "platform.h"
#include "xil_printf.h"
#include "lwipopts.h"
#include "webserver.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "lwip/tcp.h"
#include "lwip/tcp_impl.h"
#include "web_utils.h"
#include "sha1.h"
#include "base64.h"
#include "xspi.h"
#include "http_response.h"

#if LWIP_TCP_KEEPALIVE == 0
	//#define LWIP_TCP_KEEPALIVE 1
#endif
#define ETHERNET_MAC_ADDRESS	{0x00, 0x0a, 0x35, 0x00, 0x01, 0x02}

#if LWIP_DHCP==1
	extern volatile int dhcp_timoutcntr;
#endif

extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
extern volatile int fastestTmrFlag;
extern u8 WS_ok;
extern struct tcp_pcb * WSpcb;

/* the mac address of the board. this should be unique per board */
unsigned char mac_ethernet_address[] = ETHERNET_MAC_ADDRESS;
struct netif *netif, server_netif;
struct ip_addr ipaddr, netmask, gw; // gw = gateway
XSpi				SPI_instance;
u32 spi_ADC_reading = 0;
u8 spi_ok = 0;

int main(void)
{
	/* clears output */
	xil_printf("%c[2J",27);

	netif = &server_netif;

	if (init_platform() < 0)
	{
		xil_printf("ERROR initializing platform.\r\n");
		return -1;
	}

	if (init_spi(&SPI_instance) < 0)
	{
		xil_printf("ERROR initializing SPI.\r\n");
		return -1;
	}
	else
	{
		spi_ok = 1;
	}

	xil_printf("\r\n");
	xil_printf("----- SDAV - Marko Peshevski -----\r\n");

	lwip_init();

	/* initliaze IP addresses to be used */
	#if (LWIP_DHCP==0)
		IP4_ADDR(&ipaddr,  192, 168,   1, 10);
		IP4_ADDR(&netmask, 255, 255, 255,  0);
		IP4_ADDR(&gw,      192, 168,   1,  1);
		print_ip_settings(&ipaddr, &netmask, &gw);
	#elif (LWIP_DHCP==1)
		ipaddr.addr = 0;
		gw.addr = 0;
		netmask.addr = 0;
	#endif

	/* Add network interface to the netif_list, and set it as default */
	if (!xemac_add(netif, &ipaddr, &netmask, &gw, mac_ethernet_address, PLATFORM_EMAC_BASEADDR))
	{
		xil_printf("Error adding N/W interface\r\n");
		return -1;
	}

	/* set the default network interface (ethernet) */
	netif_set_default(netif);

	/* specify that the network if is up */
	netif_set_up(netif);

	/* now enable interrupts */
	platform_enable_interrupts();

	#if (LWIP_DHCP==1)
		/* Create a new DHCP client for this interface.
		 * Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
		 * the predefined regular intervals after starting the client.
		 */
		dhcp_start(netif);
		dhcp_timoutcntr = 24;

		xil_printf("Poking router for DHCP... ");
		while(((netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0))
		{
			xemacif_input(netif);
			if (TcpFastTmrFlag)
			{
				tcp_fasttmr();
				TcpFastTmrFlag = 0;
			}
			if (TcpSlowTmrFlag)
			{
				tcp_slowtmr();
				TcpSlowTmrFlag = 0;
			}
		}

		if (dhcp_timoutcntr <= 0)
		{
			if ((netif->ip_addr.addr) == 0)
			{
				xil_printf("Timeout\r\n");
				xil_printf("Trying to configure default IP of 192.168.1.10\r\n");
				IP4_ADDR(&(netif->ip_addr),	192,	168,	1,		10);
				IP4_ADDR(&(netif->netmask),	255,	255,	255,	0);
				IP4_ADDR(&(netif->gw),		192,	168,	1,		1);
			}
		}
		else
		{
			/* receive and process packets */
			xil_printf("OK\r\n");
			xil_printf("DHCP gave following configuration\r\n");
		}
		print_ip_settings(&(netif->ip_addr), &(netif->netmask), &(netif->gw));
	#endif

	/* start the application (web server) */
	xil_printf("Starting web app... ");
	if(start_web_application() != 0)
	{
		xil_printf("Following error occurred:\r\n");
	}
	else
	{
		xil_printf("Web app started\r\n");
		print_headers(&(netif->ip_addr));
	}

	while (1)
	{
		if (TcpFastTmrFlag)
		{
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag)
		{
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		if(fastestTmrFlag)
		{
			if(WS_ok > 0)
			{
		    	spi_ADC_reading = read_spi(&SPI_instance);
				updateWSWithWaterLevel(WSpcb, spi_ADC_reading);
			}
			fastestTmrFlag = 0;
		}

		xemacif_input(netif);
		transfer_web_data();
	}

	/* never reached */
	cleanup_platform();

	return 0;
}

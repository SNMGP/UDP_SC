
/******************************************************************************

  Filename:		udpapp.h
  Description:	UDP app for the WiShield 1.0

 ******************************************************************************

  TCP/IP stack and driver for the WiShield 1.0 wireless devices

  Copyright(c) 2009 Async Labs Inc. All rights reserved.

  This program is free software; you can redistribute it and/or modify it
  under the terms of version 2 of the GNU General Public License as
  published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful, but WITHOUT
  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
  more details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 59
  Temple Place - Suite 330, Boston, MA  02111-1307, USA.

  Contact Information:
  <asynclabs@asynclabs.com>

   Author               Date        Comment
  ---------------------------------------------------------------
   AsyncLabs			07/11/2009	Initial version

 *****************************************************************************/

#include "uip.h"
#include <string.h>
#include "udpapp.h"
#include "config.h"

#define STATE_INIT				0
#define STATE_LISTENING         1
#define STATE_HELLO_RECEIVED	2
#define STATE_NAME_RECEIVED		3

static struct udpapp_state s;

const int STELLA_bfsize = 256;
int STELLA_recvlen = 0;
int STELLA_sendlen = 0;
char STELLA_recv[256];
char STELLA_send[256];
int STELLA_i=0;

void dummy_app_appcall(void)
{
}

void udpapp_init(void)
{
	uip_ipaddr_t addr;
	struct uip_udp_conn *c;

	uip_ipaddr(&addr, 192,168,136,12);
	c = uip_udp_new(&addr, HTONS(12343));
	if(c != NULL) {
		uip_udp_bind(c, HTONS(12344));
	}

	s.state = STATE_INIT;

	PT_INIT(&s.pt);
}

static unsigned char parse_msg(void)
{
	if (memcmp(uip_appdata, "Hello", 5) == 0) {
                /*
                memset(STELLA_recv,'\0',sizeof(STELLA_recv));
                memcpy(STELLA_recv, uip_appdata, strlen(uip_appdata));
                STELLA_recvlen = strlen(STELLA_recv);*/
		return 1;
	}
	return 0;
}

static void send_request(void)
{
	char str[] = "Hello. What is your name?\n";
       
        memset(uip_appdata,NULL,strlen(uip_appdata));
	memcpy(uip_appdata, str, strlen(str));
	uip_send(uip_appdata, strlen(str));

        memset(STELLA_send,NULL,sizeof(STELLA_send));
        memcpy(STELLA_send, str, strlen(str));
        STELLA_sendlen = strlen(STELLA_send);
}

static void send_response(void)
{
	char i = 0;
	char str[] = "Hello ";

	while ( ( ((char*)uip_appdata)[i] != '\n') && i < 9) {
		s.inputbuf[i] = ((char*)uip_appdata)[i];
		i++;
	}
	s.inputbuf[i] = '\n';
        
        memset(uip_appdata,NULL,strlen(uip_appdata));
	memcpy(uip_appdata, str, 6);
	memcpy(uip_appdata+6, s.inputbuf, i+1);
	uip_send(uip_appdata, i+7);

        memset(STELLA_send,NULL,sizeof(STELLA_send));
        memcpy(STELLA_send, uip_appdata, i+7);
        STELLA_sendlen = strlen(STELLA_send);
}

static PT_THREAD(handle_connection(void))
{
	char str[] = "Hi~";
        
        PT_BEGIN(&s.pt);
	s.state = STATE_LISTENING;

        memset(uip_appdata,NULL,strlen(uip_appdata));
	do {/*
                uip_flags = UIP_NEWDATA;
                memset(uip_appdata,'\0',strlen(uip_appdata));
                memcpy(uip_appdata, str, strlen(str));
    	        uip_send(uip_appdata, strlen(str));
                memset(STELLA_send,'\0',sizeof(STELLA_send));
                memcpy(STELLA_send, uip_appdata, strlen(uip_appdata));
                STELLA_sendlen = strlen(STELLA_send);
                
		PT_WAIT_UNTIL(&s.pt, uip_newdata());
                
                
                uip_flags = "";*/
                PT_WAIT_UNTIL(&s.pt, uip_newdata());
                  
               
                //STELLA_i++;
		if(uip_newdata() && (parse_msg() || 1)) {
                        memset(STELLA_recv,NULL,sizeof(STELLA_recv));                
                        memcpy(STELLA_recv, uip_appdata, strlen(uip_appdata));
                        STELLA_recvlen = strlen(STELLA_recv);
                        
			s.state = STATE_HELLO_RECEIVED;
			uip_flags &= (~UIP_NEWDATA);
			break;
		}
	} while(s.state != STATE_HELLO_RECEIVED);

	do {
		send_request();
                STELLA_i++;
		PT_WAIT_UNTIL(&s.pt, uip_newdata());



		if(uip_newdata()) {
                        memset(STELLA_recv,NULL,sizeof(STELLA_recv));
                        memcpy(STELLA_recv, uip_appdata, strlen(uip_appdata));
                        STELLA_recvlen = strlen(STELLA_recv);
                        
			s.state = STATE_NAME_RECEIVED;
			uip_flags &= (~UIP_NEWDATA);
			break;
		}
	} while(s.state != STATE_NAME_RECEIVED);

	send_response();
        STELLA_i++;

	s.state = STATE_INIT;

	PT_END(&s.pt);
}

void udpapp_appcall(void)
{
	handle_connection();
}

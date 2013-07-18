
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
#include "timer.h"

#define STATE_INIT				0
#define STATE_LISTENING         1
#define STATE_HELLO_RECEIVED	2
#define STATE_NAME_RECEIVED		3
#define DELAY1 10//6000

static struct udpapp_state s;

struct timer timer;
const int STELLA_bfsize = 256;
int STELLA_recvlen = 0;
int STELLA_sendlen = 0;
char STELLA_recv[256];
char STELLA_send[256];
int STELLA_len = 0;
int STELLA_i = 0;

void dummy_app_appcall(void)
{
}

void udpapp_init(void)
{
	uip_ipaddr_t addr;
	struct uip_udp_conn *c;

	uip_ipaddr(&addr, 192,168,136,11);
	c = uip_udp_new(&addr, HTONS(12344));
	if(c != NULL) {
		uip_udp_bind(c, HTONS(12343));
	}

	s.state = STATE_INIT;

	PT_INIT(&s.pt);
}

static unsigned char parse_msg(void)
{
	if (memcmp(uip_appdata, "Hello", 5) == 0) {
                
                memset(STELLA_recv,'\0',sizeof(STELLA_recv));
                memcpy(STELLA_recv, uip_appdata, strlen(uip_appdata));
                STELLA_recvlen = strlen(STELLA_recv);
		return 1;
	}
	return 0;
}

static void send_request(void)
{
	char str[] = "Hello. What is your name?\n";

        memset(STELLA_send,'\0',sizeof(STELLA_send));
        memcpy(STELLA_send, str, strlen(str));
        STELLA_sendlen = strlen(STELLA_send);
        
	memcpy(uip_appdata, str, strlen(str));
	uip_send(uip_appdata, strlen(str));
}

static void send_response(void)
{
	char i = 0;
	char str[] = "Hello ";
/*
	while ( ( ((char*)uip_appdata)[i] != '\n') && i < 9) {
		s.inputbuf[i] = ((char*)uip_appdata)[i];
		i++;
	}
	s.inputbuf[i] = '\n';

	memcpy(uip_appdata, str, 6);
	memcpy(uip_appdata+6, s.inputbuf, i+1);
*/
        
        memset(STELLA_send,'\0',sizeof(STELLA_send));
        //memcpy(STELLA_send, uip_appdata, strlen(uip_appdata));
        //STELLA_sendlen = strlen(STELLA_send);
        
        while(( ((char*)uip_appdata)[i] != '\n') && i < 250){
                STELLA_send[i] = ((char*)uip_appdata)[i];
                i++;
        }
        STELLA_send[i] = '\n';
        STELLA_sendlen = strlen(STELLA_send);
        memcpy(uip_appdata, STELLA_send, i+1);

//	uip_send(uip_appdata, i+7);
        uip_send(uip_appdata, i+1);
}

static PT_THREAD(handle_connection(void))
{
	char str[] = "Hello";
        char str2[] ="End";
        char str3[] = "Hello2\n";
        char str4[] = "End2\n";
        int i = 0;
        PT_BEGIN(&s.pt);
	//s.state = STATE_LISTENING;

	do {    
               
                memset(uip_appdata,NULL,strlen(uip_appdata));
                //memcpy(uip_appdata, str, strlen(str));
                sprintf(uip_appdata,"%s %d\n",str,STELLA_i);
                //sprintf(uip_appdata,"%s\n",str);
                
    	        uip_send(uip_appdata, strlen(uip_appdata));
    
                memset(STELLA_send,NULL,sizeof(STELLA_send));
                memcpy(STELLA_send, uip_appdata, strlen(uip_appdata));
                STELLA_sendlen = strlen(STELLA_send);
                
                STELLA_i++;
		//PT_WAIT_UNTIL(&s.pt, uip_newdata());
                timer_set(&timer,DELAY1);
                PT_WAIT_UNTIL(&s.pt, timer_expired(&timer) || uip_newdata());
                if(uip_newdata())
                {   
                    memset(STELLA_recv,NULL,sizeof(STELLA_recv));                
                    memcpy(STELLA_recv, uip_appdata, strlen(uip_appdata));
                    STELLA_recvlen = strlen(STELLA_recv);
                    uip_flags = "";
                    break;
                }/*
                else
                {
                    
                timer_set(&timer,DELAY1);
                PT_WAIT_UNTIL(&s.pt, timer_expired(&timer));
                }*/
	} while(1);

                /*
                memset(STELLA_recv,'\0',sizeof(STELLA_recv));                
                memcpy(STELLA_recv, uip_appdata, strlen(uip_appdata));
                STELLA_recvlen = strlen(STELLA_recv);
                */
                //timer_set(&timer,DELAY1);
                //PT_WAIT_UNTIL(&s.pt, timer_expired(&timer));
                STELLA_len = strlen(uip_appdata);
                memset(uip_appdata,NULL,STELLA_len);
                sprintf(uip_appdata,"%s %d\n",str2,STELLA_i);
                //sprintf(uip_appdata,"%s\n",str2);
    	        uip_send(uip_appdata, strlen(uip_appdata));
    
                memset(STELLA_send,NULL,sizeof(STELLA_send));
                memcpy(STELLA_send, uip_appdata, strlen(uip_appdata));
                STELLA_sendlen = strlen(STELLA_send);
                STELLA_i++;


	s.state = STATE_INIT;

	PT_END(&s.pt);
}

void udpapp_appcall(void)
{
	handle_connection();
}


/*
 *  Hello World
 *
 *  Copyright (c) 2013 Realtek Semiconductor Corp.
 *
 *  This module is a confidential and proprietary property of RealTek and
 *  possession or use of this module requires written permission of RealTek.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h" 

#include "main.h"
#include "main_test.h"
#include "wifi_conf.h"
#include "wlan_intf.h"
#include "lwip_netconf.h"
#include "wifi_constants.h"

#include <platform/platform_stdlib.h>

//-----------uart--------------------//
#include "serial_api.h"
#define UART_TX    PA_7
#define UART_RX    PA_6   

   
#ifndef CONFIG_INIT_NET
#define CONFIG_INIT_NET             1
#endif
#ifndef CONFIG_INTERACTIVE_MODE
#define CONFIG_INTERACTIVE_MODE     1
#endif

// task size parameter
#define STACKSIZE                   (512 + 768)

// client send a http request need
#include "queue.h"

#include "sockets.h"
#define PORT "5000"
//#define HTTP_REQUEST_IP "140.116.226.227"
#define HTTP_REQUEST_IP "192.168.1.3"
#define STACKSIZE                   (512 + 768)
#define GET_INFO "m1"

#include "pwmout_api.h"   // mbed
#define PWM_1       PC_0 
#define PWM_2       PC_1 
#define PWM_3       PC_2 
#define PWM_PERIOD  20000
#define USE_FLOAT   0



int pwms[3]={0, 0, 0};
int steps[3]={PWM_PERIOD/4,PWM_PERIOD/4,0};
pwmout_t pwm_led[3];
PinName  pwm_led_pin[3] =  {PWM_1, PWM_2, PWM_3};

xSemaphoreHandle uart_rx_interrupt_sema = NULL;

TaskHandle_t xwifi_handle;
 xQueueHandle transfer_flag = NULL;
/*---------------queue_message--------------------*/
extern void *gettaskhandle(void);
TaskHandle_t *receive_handle;
void *getqueuehandle(void){
	return &transfer_flag;
}
 
/*---------------queue_message_end--------------------*/  

void uart_send_string(serial_t *sobj, char *pstr)
{
    unsigned int i=0;

    while (*(pstr+i) != 0) {
        serial_putc(sobj, *(pstr+i));
        i++;
    }
}



void init_thread(void *param)
{       
  
 
 //--------init pwm and set lock--------------------------------//
    int i,check_lock=0;
    pwmout_init(&pwm_led[2], pwm_led_pin[2]); 
    pwmout_period_us(&pwm_led[2], PWM_PERIOD); 
    
    if(check_lock==0){
    for(i=0;i<2;i++){
                   pwmout_pulsewidth_us(&pwm_led[2], pwms[2]);
                    
                    pwms[2] = PWM_PERIOD/10;
                   
                }                
                   pwms[2] = 0;
                   check_lock = 1;
    }
//--------init pwm and set lock end--------------------------------//  
  
//----------------set wifi parameters------------------------------//
#if CONFIG_INIT_NET
#if CONFIG_LWIP_LAYER
	/* Initilaize the LwIP stack */
	LwIP_Init();
#endif
#endif
       
        
#if CONFIG_WLAN
	//vTaskSuspendAll();
        wifi_on(RTW_MODE_STA);
        
#if CONFIG_AUTO_RECONNECT
        //setup reconnection flag
	wifi_set_autoreconnect(1);
#endif
	printf("\n\r%s(%d), Available heap 0x%x", __FUNCTION__, __LINE__, xPortGetFreeHeapSize());	
#endif

#if CONFIG_INTERACTIVE_MODE
 	/* Initial uart rx swmaphore*/
	vSemaphoreCreateBinary(uart_rx_interrupt_sema);
        printf("\n\r semaphore_debug_1");
	xSemaphoreTake(uart_rx_interrupt_sema, 1/portTICK_RATE_MS);
        printf("\n\r semaphore_debug_2");
        start_interactive_mode();
        printf("\n\r semaphore_debug_3");
#endif	

        /* Set Ameba wifi content to connect an Access point(MMNLAB) */
        unsigned long tick1 = xTaskGetTickCount();
	unsigned long tick2, tick3;
        
        static rtw_network_info_t wifi = {0};
        //char ssid[] = "MMNLAB";
        char ssid[] = "TOTOLINK N300RB";
        unsigned char password[] = "mmnlab41124a";
        int ret;
        
        strcpy((char *)wifi.ssid.val, (char*)ssid);
	wifi.ssid.len = strlen((char*)ssid);
        wifi.password = password;
	wifi.password_len = strlen((char*)password);
        wifi.security_type = RTW_SECURITY_WPA2_AES_PSK;
        wifi.key_id = -1;
        
        ret = wifi_connect((char*)wifi.ssid.val, wifi.security_type, (char*)wifi.password, wifi.ssid.len,
						wifi.password_len, wifi.key_id, NULL);
	if(ret!= RTW_SUCCESS){
		printf("\n\rERROR: Operation failed!\n");		
	}
        else{ printf("ap set ok!!\n");}  
        tick2 = xTaskGetTickCount();
	printf("\r\nConnected after %dms.\n", (tick2-tick1));
#if CONFIG_LWIP_LAYER
		/* Start DHCPClient */
		LwIP_DHCP(0, DHCP_START);
	tick3 = xTaskGetTickCount();
	printf("\r\n\nGot IP after %dms.\n", (tick3-tick1));
#endif
	printf("\n\r");
//----------------end set wifi parameters------------------------------//
 
//---------[Initlize]--------------------------------------------//
if(check_lock==1){        
    int lock = 1;
 /*   
    //--------init pwm--------------------------------//
    pwmout_init(&pwm_led[2], pwm_led_pin[2]); 
    pwmout_period_us(&pwm_led[2], PWM_PERIOD);        
*/        
        
//--------------------uart test-------------------------------------------------//
    char rc,get_info[4];
    char BUF[500];
    serial_t    sobj;
    int sock_local;
    
    
    // mbed uart test
    serial_init(&sobj,UART_TX,UART_RX);
    serial_baud(&sobj,38400);
    serial_format(&sobj, 8, ParityNone, 1);        
    
    
while(1){     
    
    memset(get_info , 0 , sizeof(get_info)); 
    memset(BUF , 0 , sizeof(strlen(BUF))); 
    //memset(rc , 0 , sizeof(strlen(rc)));    
    
    rc = serial_getc(&sobj);   
       
    //printf("%d\n",rc);
    if((rc=='1' || rc=='2') && lock == 1){    
        
          
        
        
        //strncpy(get_info,rc,1);    
        
//----------------------pwm_unlock-----------------------------------------
        
              for(i=0;i<2;i++){
                   pwmout_pulsewidth_us(&pwm_led[2], pwms[2]);
     
                
                    pwms[2] = PWM_PERIOD/25;
                    
                }
                
                    pwms[2] = 0;
                    lock = 0;
                               
    
    
//-----------------------------socket test---------------------
    
    
// in_port_t ; add this from sockets.h   
#if !defined(in_port_t) && !defined(IN_PORT_T_DEFINED)
      typedef u16_t in_port_t;
#endif
// in_port_t ; end   
    in_port_t port = atoi(PORT);
    
    struct sockaddr_in server;

    memset(&server, 0, sizeof(server));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

     printf("ok 1 \n");    
    //open a socket
    sock_local = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    
    int retVal = -1;
// inet_pton ; add this from sockets.h   
#define inet_pton(af,src,dst) \
    (((af) == AF_INET) ? inet_aton((src),(dst)) : 0)
// inet_pton ; end 
      
    retVal = inet_pton(AF_INET, HTTP_REQUEST_IP, &server.sin_addr.s_addr);
    printf("ok 2 \n");  
	//connect to web server 
    if ( connect( sock_local , (struct sockaddr*)&server, sizeof(server)) < 0)
    {
        perror("connect");
        exit(EXIT_FAILURE);
    }
    printf("ok 3 \n"); 
    //send a Request
    int nByte = send(sock_local, GET_INFO, strlen(GET_INFO), 0);
    //int nByte = send(sock_local, get_info, strlen(get_info), 0);
    if( nByte <= 0)
    {
        perror("send");
        exit(EXIT_FAILURE);
    }
    printf("ok 4 \n"); 
  
      
      size_t recived_len = 0;
      printf("ok 5 \n"); 
      
      while(recived_len = recv(sock_local, BUF, 255, 0)>0){
      
       
      printf("return bytes: %d\n",recived_len);
      printf("%s",BUF); 
      //uart_send_string(&sobj, BUF);     
      }
         if(recived_len < 0)
         {
            printf("\n Read Error \n");
         }else if(recived_len == 0){
         
            printf("\n Read Eof \n");
         }
         
    close(sock_local);       
    }

//---------------------pwm_lock-----------------------------------------      
      else if( (rc=='1' || rc=='2') && lock == 0){     
        
        
   
     
                 for(i=0;i<2;i++){
                   pwmout_pulsewidth_us(&pwm_led[2], pwms[2]);
                    
                    pwms[2] = PWM_PERIOD/10;
                   
                }                
                   pwms[2] = 0;             
                   lock = 1;
                  
                  
        
      }
          
//---------------socket end----------------------
   
        
       // printf("\n\r wifi init task delete ");
	// Kill init thread after all init tasks done 	
        //vTaskDelete(NULL);
    }
  }
}

void wlan_network()
{
	if(xTaskCreate(init_thread, ((const char*)"init"), STACKSIZE, NULL, tskIDLE_PRIORITY + 2, &xwifi_handle) != pdPASS)
		printf("\n\r%s xTaskCreate(init_thread) failed", __FUNCTION__);
}

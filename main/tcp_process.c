#include <string.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "tcp_server.h"
#include "tcp_process.h"

#include "config.h"
#include "options.h"

#include "contacts.h"
#include "relays.h"
#include "flags.h"

#include "temperature_dht.h"
#include "temperature_ds18b20.h"

#include <homekit/homekit.h>


static const char *TAG = "tcp_process";


int tcp_process(int sock, struct mea_config_s *mea_config, int8_t mode, char cmd, char *parameters, void *userdata)
{
   if(tcp_network_config(sock, mea_config, mode, cmd, parameters)) {
      return 1;
   }
   
   if(parameters) {
      switch(cmd) {
         case 'O': {
            int id, v, r1, r2;
            int n=sscanf(parameters, "%d%n/%d%n",&id,&r1,&v,&r2);
            if(n==2 && r2==strlen(parameters)) {
               relays_set(id, ((v == 0) ? 0 : 1));
               ESP_LOGI(TAG,"relay %d set to: %d",id,v);
               tcp_send_data(sock,OK_STATUS);
            }
            else if(n==1 && r1==strlen(parameters)) {
               char str[2];
               ESP_LOGI(TAG,"relay %d get",id);
               sprintf(str,"%d",relays_get(id));
               tcp_send_data(sock,str);
            }
            else {
               tcp_send_data(sock, BAD_REQUEST_STATUS);
            } 
            return 1;
         };
         case 'f':
         case 'i': {
            int id,r;
            int n=sscanf(parameters, "%d%n",&id,&r); 
            if(n==1 && r==strlen(parameters)) {
               ESP_LOGI(TAG,"input %d get",id);
               int8_t v;
               if(cmd=='I') {
                  v=contacts_get(id);
               }
               else {
                  v=flags_get(id);
               }
               if(v<0) {
                  ESP_LOGI(TAG,"KO");
                  tcp_send_data(sock,KO_STATUS);
               }
               else {
                  char s[2]="";
                  s[0] = '0' + ((v == 0) ? 0 : 1);
                  s[1] = 0;
                  tcp_send_data(sock,s);
               }
            }
            else {
               tcp_send_data(sock, BAD_REQUEST_STATUS);
            } 
            return 1;
         };
         case 'h': {
            int id,r,h;
            int n=sscanf(parameters, "%d%n",&id,&r); 
            if(n==1 && r==strlen(parameters) && id==0) {
               char str[5];
               ESP_LOGI(TAG,"humidity %d get",id);
               h=(int)temperature_dht_get_h();
               sprintf(str,"%d",h);
               tcp_send_data(sock,str);
            }
            else {
               tcp_send_data(sock, BAD_REQUEST_STATUS);
            }
            return 1;
         }
         case 't': {
            int id,r;
            char str[10];
            int n=sscanf(parameters, "%d%n",&id,&r); 
            if(n==1 && r==strlen(parameters) && (id==0 || id==1)) {
               ESP_LOGI(TAG,"temperature %d get",id);
               if(id==0) {
                  sprintf(str,"%d",(int)temperature_dht_get_t());
               }
               else {
                  sprintf(str,"%.1f",temperature_ds18b20_get());
               }
               tcp_send_data(sock,str);
            }
            else {
               tcp_send_data(sock, BAD_REQUEST_STATUS);
            }
            return 1;
         }
         case 'X': {
            int v,r;
            int n=sscanf(parameters, "%d%n",&v,&r); 
            if(n==1 && r==strlen(parameters) && (v==1 || v==2)) {
               ESP_LOGI(TAG,"reserve %d",v);
               setOption16("reserve",v);
               tcp_send_data(sock,OK_STATUS);
            }
            else {
               tcp_send_data(sock, BAD_REQUEST_STATUS);
            }
            return 1;
         }
      }
   }
   else {
      switch(cmd) {
         case 'x': {
            char str[8];
            ESP_LOGI(TAG,"display configuration options");
            sprintf(str,"%d",getOption16("reserve",1));
            tcp_send_data(sock,str);
         }
         return 1;
      }
   }

   return 0;
}

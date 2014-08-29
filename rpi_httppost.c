#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

#include "rpi_tcpclient.h"
#include "rpi_postmsg.h"

#define GET_FORMAT  "GET /dcs/alert?station_id=%s&status=%s&type=%s&data=%d HTTP/1.1\r\n"
#define POST "POST /dcs/?type=data HTTP/1.1\r\n"
#define HEADER "Content-Type: application/x-www-form-urlencoded; charset=UTF-8\r\n"
#define RWBUFFSIZE 1024

int dust_human_interval = 2;
int co2_interval = 1;

int *dust1 = NULL;
int *dust2 = NULL;
double *temp = NULL; 
double *humanity = NULL;
int *co2 = NULL;
int dust1Index = 0;
int dust2Index = 0;
int tempIndex = 0;
int humanityIndex = 0;
int co2Index = 0;

typedef enum {
	NORMAL =0,
	HIGH 
} CO2_STATE;

CO2_STATE co2_state = NORMAL;

extern int send_delay;
extern int co2_alarm_threshold;
extern char current_station[128];
extern char local_ip_address[24];
extern int local_ip_port;
static pthread_mutex_t threadlock;
rpi_tcp_client notify_client;

void calculateSensorData()
{
        int avg_dust1,total_dust1 = 0;	
        int avg_dust2,total_dust2 = 0;	
	double avg_temp,total_temp = 0.0;
	double avg_humanity,total_humanity = 0.0;
	int avg_co2,total_co2 = 0;

	int i;
	for(i = 0;i < dust1Index;i++){
		total_dust1 += dust1[i];
	}
	if(dust1Index != 0)
		avg_dust1 = total_dust1 / dust1Index;

	for(i = 0;i < dust2Index;i++){
		total_dust2 += dust2[i];
	}
	if(dust2Index != 0)
		avg_dust2 = total_dust2 / dust2Index;


	for(i = 0;i < tempIndex; i++){
		total_temp += temp[i];
	}
	if(tempIndex != 0)	
		avg_temp = total_temp / tempIndex;

	for(i = 0;i < humanityIndex; i++){
		total_humanity += humanity[i];
	}
	if(humanityIndex != 0)
		avg_humanity = total_humanity / humanityIndex;

	for(i = 0;i < co2Index; i++) {
		total_co2 += co2[i];
	}
	if(co2Index != 0)
		avg_co2 = total_co2 / co2Index;

    set_avg_sensor_data(avg_dust1,avg_dust2, avg_temp,avg_humanity,avg_co2);
}

void clearCacheData()
{
	int i;
	for(i = 0;i < send_delay / dust_human_interval;i++){
		dust1[i] = 0;
		dust2[i] = 0.0;
		temp[i] = 0.0; 
		humanity[i] = 0.0;
	}
	
	for(i = 0;i < send_delay / co2_interval; i++) {
		co2[i] = 0.0;
	}

	dust1Index = 0;
	dust2Index = 0;
	tempIndex = 0;
	humanityIndex = 0;
	co2Index = 0;
}

int update_record(const char *filepath){
    //printf("Update_record %s Sensor Data .....\n",filepath);
    if(filepath == NULL) return -1;
    FILE *fp = NULL;
    fp = fopen(filepath,"r");
    if(fp == NULL){
        printf("update_record don't open %s\n",filepath);
        exit(0);
    }
    char * tmp = NULL;
    char row_msg[BUFFSIZE];
    int count = 0;
	
    int sret = 0;

    while(!feof(fp)){
        memset(row_msg,0,BUFFSIZE);
        tmp = fgets(row_msg,BUFFSIZE,fp);
        if(tmp == NULL) break;
        count++;
        if(strstr(filepath,"Adafruit_DHT") && count == 3 && strstr(tmp,"Temp")){
			float tmpValue = 0.0;
			float tmpValue2 = 0.0;
			if((sret = sscanf(tmp,"Temp =  %f *C, Hum = %f %%",&tmpValue,&tmpValue2)) == 2){
				fprintf(stderr,"Temp:%f\t Hum: %f\n",tmpValue,tmpValue2);
				if(tmpValue != 0.0){
					temp[tempIndex++] = tmpValue; 
				}
				if(tmpValue2 != 0.0){
					humanity[humanityIndex++] = tmpValue2;
				}
			}
	    //printf("cat Adafruit_DHT.data > Adafruit_DHT_bak.data \n");
            //system("cat Adafruit_DHT.data > Adafruit_DHT_bak.data");
            //system("date \"+%Y_%m_%d %H:%M:%S\" >> Adafruit_DHT_bak.data");
            break;
        }else if(strstr(filepath,"dsm501a") && ( count == 5 || count == 14 )&& strstr(tmp,"particle")){
			int tmpValue = 0;
			if((sret = sscanf(tmp,"particle  = %d pcs/283ml",&tmpValue)) == 1){
				fprintf(stderr,count == 5 ? "dust 1:%d\t pcs/283ml\n":"dust 2:%d\t pcs/283ml\n",tmpValue);
				if(tmpValue != 0 && count == 5){
					dust1[dust1Index++] = tmpValue;
				}
				if(tmpValue != 0 && count == 14){
					dust2[dust2Index++] = tmpValue;
				}
			}
            //printf("cat dsm501a.data > dsm501a_bak.data \n");
            //system("cat dsm501a.data > dsm501a_bak.data");
            //system("date \"+%Y_%m_%d %H:%M:%S\" >> dsm501a_bak.data");
            //break;
        }
        else if(strstr(filepath,"ADS1115") && count == 2 && strstr(tmp,"PPM")){
			int tmpValue = 0;
			if((sret = sscanf(tmp,"PPM = %d ppm",&tmpValue)) == 1){
				fprintf(stderr,"co2 :%d ppm\n ",tmpValue);
				if(tmpValue != 0 ){
					co2[co2Index++] = tmpValue;
					char *respond = NULL;
					if(tmpValue > co2_alarm_threshold){
						if(co2_state != HIGH){
							co2_state = HIGH;
							http_get(&notify_client,&respond,co2_state,tmpValue);
						}
						fprintf(stderr,"alert!!!!!!!!!!!!!\n");
					}else{
						if(co2_state == HIGH){
							co2_state = NORMAL;	
							fprintf(stderr,"back to normal\n");
						}	
					}
				}
			}
            //printf("cat ADS1115.data > ADS1115_bak.data \n");
            //system("cat ADS1115.data > ADS1115_bak.data");
            //system("date \"+%Y_%m_%d %H:%M:%S\" >> ADS1115_bak.data");
            break;
        }
    }    
    if(fp) fclose(fp);
    return 0;
}

void sensor_data_thread(void){
    printf("Run Get Sensor Data thread..\n");
    for(;;){
        printf("Start Get Sensor Data....\n");

		//humanity/temp
        printf("Start Adafruit_DHT 22 4 > Adafruit_DHT.data....\n");
        system("./Adafruit_DHT 22 4 > Adafruit_DHT.data");
	sleep(dust_human_interval/2);
        pthread_mutex_lock(&threadlock);
        update_record("Adafruit_DHT.data");
        pthread_mutex_unlock(&threadlock);

		//dust 1.0
        printf("Start ./dsm501a 1 2 > dsm501a.data....\n");
        system("./dsm501a 1 2 > dsm501a.data");
	
		//dust 2.5
        printf("Start ./dsm501a 4 5 >> dsm501a.data....\n");
        system("./dsm501a 4 5 >> dsm501a.data");
      //system("cat dsm501a.data");
	sleep(dust_human_interval/2);
        pthread_mutex_lock(&threadlock);
        update_record("dsm501a.data");
        pthread_mutex_unlock(&threadlock);

    }
}

void co2_data_thread()
{
	printf("Run Get CO2 Sensor Data thread..\n");
	for(;;){
		//this is co2 sensor,need to put this sensor process to single thread
        //printf("ads1x15_ex_singleended > ADS1115.data....\n");
        system("./ads1x15_ex_singleended.py > ADS1115.data");
        sleep(co2_interval);
        pthread_mutex_lock(&threadlock);
        update_record("ADS1115.data");
        pthread_mutex_unlock(&threadlock);
	}
}

int http_post(rpi_tcp_client *pclient,char *request,char **response,ServerType type){
    char post[128],host[128],content_len[128],lpbuff[RWBUFFSIZE];
    char *ptmp;
    int len = 0;

    char *tmpIpAddr = type == Local?local_ip_address:IPSTR;
    int tmpPort = type == Local?local_ip_port:PORT;

    fprintf(stderr,"!!!!!servertype: %s\tlocal_ip_address: %s\n",type == Local?"Local":"AWS",tmpIpAddr);
    fprintf(stderr,"@@@@@servertype: %s\tlocal_ip_port: %d\n",type == Local?"Local":"AWS",tmpPort);
	
    if(rpi_tcpclient_create(pclient,tmpIpAddr,tmpPort) < 0)   return -1;
 
    if(rpi_tcpclient_conn(pclient) < 0)   return -2;

    memset(post, 0,128);
    sprintf(post,POST);

    memset(host, 0,128);
    sprintf(host,"Host: %s\r\n",tmpIpAddr);

    memset(content_len, 0,128);
    sprintf(content_len,"Content-Length: %d\n\n",(int)strlen(request));
    //send data
    memset(lpbuff, 0, RWBUFFSIZE);
    strcpy(lpbuff, post);
    strcat(lpbuff, host);
    strcat(lpbuff, HEADER);
    strcat(lpbuff, content_len);
    strcat(lpbuff, request);
    strcat(lpbuff, "\r\n\r\n");
    len = strlen(lpbuff);
    printf("http_post:len=%d \nrequest=%s\n",len,request);
    if(rpi_tcpclient_send(pclient,lpbuff,len) < 0){
        rpi_tcpclient_close(pclient);
        return -3;
    }
    int ret = rpi_tcpclient_recv(pclient,lpbuff,RWBUFFSIZE);
    if(ret <= 0){
        rpi_tcpclient_close(pclient);
        return -4;
    }
    //Response code |HTTP /1.1 200 OK|
    //Begin 10 start ,number 3
   if(lpbuff == NULL){
        printf("Response Data:lpbuff == NULL\n"); 
        return -5;
    }
    memset(post,0,sizeof(post));
    strncpy(post,lpbuff+9,3);
    if(atoi(post) != 200){
        printf("Response Data:%s\n",lpbuff);
        return atoi(post);
    }
    ptmp = (char*)strstr(lpbuff,"\r\n\r\n");
    if(ptmp == NULL){
        return -6;
    }
    ptmp += 4;
    len = strlen(ptmp)+1;
    *response = (char*)malloc(len);
    if(*response == NULL){
        return -7;
    }
    memset(*response,0,len);
    memcpy(*response,ptmp,len-1);
  //Begin head find content length ,if not find for not handle

    ptmp = (char*)strstr(lpbuff,"Content-Length:");
    if(ptmp != NULL){
        char *ptmp2;
        ptmp += 15;
        ptmp2 = (char*)strstr(ptmp,"\r\n");
        if(ptmp2 != NULL){
            memset(post,0,sizeof(post));
            strncpy(post,ptmp,ptmp2-ptmp);
            if(atoi(post) < len)
                (*response)[atoi(post)] = '\0';
        }
    }
    return 0;
}

int http_get(rpi_tcp_client *pclient,char **response,CO2_STATE co2State,int co2){
    char get[256],host[128],lpbuff[RWBUFFSIZE];
    int len = 0;
    char *tmpIpAddr = local_ip_address;
    int tmpPort = ALERT_PORT;

    if(rpi_tcpclient_create(pclient,tmpIpAddr,tmpPort) < 0)   return -1;
 
    if(rpi_tcpclient_conn(pclient) < 0)   return -2;

    //TODO: need to check normal/low state
    if(co2State == NORMAL)
	return -1;
    memset(get,0,256);
    sprintf(get,GET_FORMAT,current_station,"high","co2",co2); 
    fprintf(stderr,"Get Request: %s\n",get);

    memset(host, 0,128);
    sprintf(host,"Host: %s\r\n",tmpIpAddr);

    //send header data
    memset(lpbuff, 0, RWBUFFSIZE);
    strcpy(lpbuff, get);
    strcat(lpbuff, host);
    strcat(lpbuff, HEADER);
    strcat(lpbuff, "\r\n\r\n");
    len = strlen(lpbuff);
    printf("http_get:len=%d \n",len);
    if(rpi_tcpclient_send(pclient,lpbuff,len) < 0){
        rpi_tcpclient_close(pclient);
        return -3;
    }
    int ret = rpi_tcpclient_recv(pclient,lpbuff,RWBUFFSIZE);
    if(ret <= 0){
        rpi_tcpclient_close(pclient);
        return -4;
    }
    //Response code |HTTP /1.1 200 OK|
    //Begin 10 start ,number 3
    if(lpbuff == NULL){
        printf("Response Data:lpbuff == NULL\n"); 
        return -5;
    }
    memset(get,0,sizeof(get));
    strncpy(get,lpbuff+9,3);
    if(atoi(get) != 200){
        printf("Response Data:%s\n",lpbuff);
        return atoi(get);
    }
    return 0;
}

int main(){
	char *tmpValueb = NULL;
	get_local_data(&tmpValueb,Local);

	dust1 = calloc(send_delay / dust_human_interval,sizeof(int));
	dust2 = calloc(send_delay / dust_human_interval,sizeof(int));
	humanity = calloc(send_delay / dust_human_interval,sizeof(double));
	temp = calloc(send_delay / dust_human_interval,sizeof(double));
	co2 = calloc(send_delay / co2_interval,sizeof(int));

    char *response = NULL;
    char *post_text = NULL;
    int ret,error = 0;

    rpi_tcp_client aws_client;

    rpi_tcp_client local_client;

    pthread_t id;
    pthread_mutex_init(&threadlock, NULL);
    ret = pthread_create(&id,NULL,(void *)sensor_data_thread,NULL);
    if(ret != 0){
        fprintf(stderr,"Create sensor_data_thread error!\n");
        exit(0);
    }

    int ret2;
    pthread_t id2;
    ret2 = pthread_create(&id2,NULL,(void *)co2_data_thread,NULL);
    if(ret2 != 0){
	    fprintf(stderr,"Create co2_data_thread error!\n");
	    exit(0);
    }

    sleep(60);
   // sleep(10);
    for(;;){
       printf("Start get_post_text....\n");
       pthread_mutex_lock(&threadlock);
	calculateSensorData();
       if((error = get_post_text(&post_text,AWS)) == 0){
           printf("\nStart http_post !.....\n");
           if((error = http_post(&aws_client,post_text,&response,AWS)) < 0){
               printf("http_post error -%d\n",error);
           }else{
               printf("AWS Response : \n%s\n",response);
           }
	   
       }else{
           printf("get_post_text error -%d\n",error);
       }

       if((error = get_post_text(&post_text,Local)) == 0){
           if((error = http_post(&local_client,post_text,&response,Local)) < 0){
               printf("http_post error -%d\n",error);
           }else{
               printf("Local Response : \n%s\n",response);
           }
       }else{
           printf("get_post_text error -%d\n",error);
       }

       clearCacheData();
       pthread_mutex_unlock(&threadlock);
       if(post_text != NULL){
           free(post_text);
           post_text = NULL;
       }
       if(response != NULL){
           free(response);
           response = NULL;
       }
       printf("Wait for the next HTTP Post data....delay=%dMin\n",send_delay/60);
       sleep(send_delay);
    }
    return 0;
}

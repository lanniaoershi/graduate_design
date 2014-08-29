#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

#include "rpi_postmsg.h"
int co2_alarm_threshold = 0;
char current_station[128] = "";
char local_ip_address[24] ={0};
int  local_ip_port = 0;

int avg_co2 = 0.0;
int avg_dust1 = 0.0;
int avg_dust2 = 0.0;
double avg_temp = 0.0;
double avg_humanity = 0.0;

int send_delay = 600;
int co2_delay = 60;
int get_time(char *str_time){
    struct tm * ptm;
    long ts;
    int y,m,d,h,n,s;

    ts = time(NULL);
    ptm = localtime(&ts);
    y = ptm->tm_year +1900;
    m = ptm->tm_mon+1;
    d = ptm->tm_mday;
    h = ptm->tm_hour;
    n = ptm->tm_min;
    s = ptm->tm_sec;
    memset(str_time,0,sizeof(str_time));
    sprintf(str_time,"&time=%d-%d-%d+%d%%3A%d%%3A%d\n",y,m,d,h,n,s);
    return 0;
}

int get_local_data(char **local_data,ServerType type){
    FILE *fp = NULL;
    fp = fopen("./local_data.conf","r");
    if(fp == NULL){
        printf("don't open ./local_data.conf\n");
        exit(0);
    }
    char row_msg[BUFFSIZE];
    int len = 0;
    int row_num = 0;
    char del[8];
    char *tmp = NULL;
    char *ptmp = NULL;
    *local_data = NULL;
    while(!feof(fp)){
        memset(row_msg,0,BUFFSIZE);
        tmp = fgets(row_msg,BUFFSIZE,fp);
	if(type == AWS){
		if(row_num == 7){
			sprintf(tmp,"local_ip_address=%s ",IPSTR);
		}
		if(row_num == 8){
			sprintf(tmp,"local_ip_port=%d ",PORT);
		}
	}
        if(tmp == NULL) break;
        len += strlen(tmp);
        if(*local_data == NULL){
            *local_data = (char*)malloc(len);
            if(*local_data == NULL){
                printf("local_data malloc -2\n");
                return -2;
            }
        }else{
            *local_data = (char*)realloc(*local_data,len);
            if(*local_data == NULL){
                printf("local_data realloc -3\n");
                return -3;
            }
        }
        if(row_num > 0){
            strcat(*local_data,"&");

            strncat(*local_data,row_msg,strlen(row_msg)-1);
            ++row_num;
            if(row_num == 2){
		if(sscanf(row_msg,"station_id=%s",current_station) == 1){
			fprintf(stderr,"station_id: %s\n",current_station);
		}
                get_time(row_msg);
                len += strlen(row_msg);
                *local_data = (char*)realloc(*local_data,len);
                if(*local_data == NULL){
                    printf("local_data realloc -4\n");
                    return -4;
                }

                strncat(*local_data,row_msg,strlen(row_msg)-1);
            }else if(row_num == 4){
                ptmp = strchr(row_msg,'=');
                if(ptmp != NULL){
                    memset(del,0,sizeof(del));
                    if(strstr(row_msg,"interval_time")){
                        strncpy(del,ptmp+1,strlen(ptmp));
                        send_delay = atoi(del);
                        printf("send_delay=%d\n",send_delay);
                    }else{
                        printf("error don't find interval_time \n");
                    }
                }else{
                    printf("error row_num == 4 don't find \"=\"\n");
                }
            }else if(row_num == 6){
                ptmp = strchr(row_msg,'=');
                if(ptmp != NULL){
                    memset(del,0,sizeof(del));
                    if(strstr(row_msg,"co2_alarm_time")){
                       strncpy(del,ptmp+1,strlen(ptmp));
                       co2_delay = atoi(del);
                     //  printf("co2_alarm_time=%d\n",send_delay);
                    }else{
                        printf("error don't find co2_alarm_time\n");
                    }
                }else{
                    printf("error row_num > 5 don't find \"=\"\n");
                }
            }else if(row_num == 7){
		int tmpValue = 0;
		if(sscanf(row_msg,"co2_alarm_threshold=%d",&tmpValue) == 1){
			co2_alarm_threshold = tmpValue;
		}
	   }else if(row_num == 8){
		if(type == Local){
		   if(sscanf(row_msg,"local_ip_address=%s",local_ip_address) == 1){
			   fprintf(stderr,"local_ip_address: %s\n",local_ip_address);
		   }
		}
           }else if(row_num == 9){
		if(type == Local){
		   if(sscanf(row_msg,"local_ip_port=%d",&local_ip_port) == 1){
			   fprintf(stderr,"local_ip_port: %d\n",local_ip_port);
		   }
		}
           }
        }else{
            memset(*local_data,0,len);
            strncat(*local_data,row_msg,strlen(row_msg)-1);
	//	fprintf(stderr,"local_data:%s\n",row_msg);
	    //if(row_num == 0){
	//	sscanf(row_msg,"location=%s",current_station);
	//    	fprintf(stderr,"local_data:%s\n",current_station);
	//	}
            ++row_num;
        }
    }
    if(fp) fclose(fp);
    return 0;
}

int get_temp_hum(char *temp_hum){
/*
    char row_msg[BUFFSIZE];
    char temperature[8];
    char humidity[8];
    int count = 0;
    FILE *fp = NULL;
    fp = fopen("./Adafruit_DHT_bak.data","r");
    if(fp == NULL){
        printf("don't open Adafruit_DHT_bak.log\n");
        return -1;
    }
    int row_num = 0;
    char *tmp = NULL;
    memset(temperature,0,sizeof(temperature));
    memset(humidity,0,sizeof(humidity));
    while(!feof(fp)){
        memset(row_msg,0,BUFFSIZE);
        tmp = fgets(row_msg,BUFFSIZE,fp);
        if(tmp == NULL) break;
        ++row_num;
        if(row_num == 3){
            int i = 0;
            int j = 0;
            char *p = strchr(tmp,'=');
            for(;i < strlen(p);i++){
                if(isdigit(p[i])||p[i] == '.'){
                    temperature[j]=p[i];
                    if(temperature[j] == '.'){
                        temperature[++j]=p[++i];
                        break;
                    }
                    ++j;
                }
            }
            for(j=0,i++;i < strlen(p);i++){
                if(isdigit(p[i])||p[i] == '.'){
                    humidity[j]=p[i];
                    if(humidity[j] == '.'){
                        humidity[++j]=p[++i];
                        break;
                    }
                    ++j;
                }
            }
        }
    }
    memset(temp_hum,0,sizeof(temp_hum));
    if(strlen(temperature) > 0 && strlen(humidity) > 0){
        strcat(temp_hum,"&temperature%5Bsensor_id%5D=T_1&temperature%5Bvalue%5D=");
        strcat(temp_hum,temperature);
        strcat(temp_hum,"&humidity%5Bsensor_id%5D=H_1&humidity%5Bvalue%5D=");
        strcat(temp_hum,humidity);
    }else{
        printf("ERROR ! get_temp_hum function :Data not available temperature&humidity sensor\n");
    }

    if(fp) fclose(fp); 
*/
    sprintf(temp_hum,"&temperature%5Bsensor_id%5D=T_1&temperature%5Bvalue%5D=%.2f&humidity%5Bsensor_id%5D=H_1&humidity%5Bvalue%5D=%.2f",
		avg_temp,
		avg_humanity);

    //fprintf(stderr,"temp_humanity: %s\n",temp_hum);

    return 0;
}

int get_particle(char *particle){
/*
    char row_msg[BUFFSIZE];
    char particle_2_5[8];
    char particle_1[8];
    FILE *fp = NULL;
    fp = fopen("./dsm501a_bak.data","r");
    if(fp == NULL){
        printf("Can not open dsm501a_bak.data\n");
        return -1;
    }
    int row_num = 0;
    char *tmp = NULL;
    int i,j;
    memset(particle_2_5,0,sizeof(particle_2_5));
    memset(particle_1,0,sizeof(particle_1));
    while(!feof(fp)){
        memset(row_msg,0,BUFFSIZE);
        tmp = fgets(row_msg,BUFFSIZE,fp);
        if(tmp == NULL) break;
        ++row_num;
        if(row_num == 5){
            char *p = strchr(tmp,'=');
            for(i = 0,j = 0;i < strlen(p);i++){
                if(isdigit(p[i])){
                    particle_1[j]=p[i];
                    ++j;
                }else if(j > 0) break;
            }    
        }
        if(row_num == 14){
            char *p = strchr(tmp,'=');
            for(i = 0,j = 0;i < strlen(p);i++){
                if(isdigit(p[i])){
                    particle_2_5[j]=p[i];
                    ++j;
                }else if(j > 0) break;
            }
        }
    }
    memset(particle,0,sizeof(particle));
    if(strlen(particle_2_5) > 0 && strlen(particle_1) > 0){
        strcat(particle,"&particle_2_5%5Bsensor_id%5D=p2_1&particle_2_5%5Bvalue%5D=");
        strcat(particle,particle_2_5);
        strcat(particle,"&particle_1%5Bsensor_id%5D=p1_1&particle_1%5Bvalue%5D=");
        strcat(particle,particle_1);
    }else{
        printf("ERROR ! get_particle function :Data not available particle sensor\n");
    }
    if(fp) fclose(fp);
*/
    sprintf(particle,"&particle_2_5%5Bsensor_id%5D=p2_1&particle_2_5%5Bvalue%5D=%d&particle_1%5Bsensor_id%5D=p1_1&particle_1%5Bvalue%5D=%d",avg_dust2, avg_dust1);

    return 0;
}

int get_co2(char *str_co2){
/*
    char row_msg[BUFFSIZE];
    char co_2[8];	
    FILE *fp = NULL;
    fp = fopen("./ADS1115_bak.data","r");
    if(fp == NULL){
        printf("can not open ADS1115_bak.data\n");
        return -1;
    }
    int row_num = 0;
    char *tmp = NULL;
    int i,j;
    memset(co_2,0,sizeof(co_2));
	while(!feof(fp)){
        memset(row_msg,0,BUFFSIZE);
        tmp = fgets(row_msg,BUFFSIZE,fp);
        if(tmp == NULL) break;
        ++row_num;
        if(row_num == 2){
            char *p = strchr(tmp,'=');
            for(i = 0,j = 0;i < strlen(p);i++){
                if(isdigit(p[i])){
                    co_2[j]=p[i];
                    ++j;
                    //if(j==7) break;
                }else if (j > 0) break;
            }    
        }
	}

    memset(str_co2,0,sizeof(str_co2));
    if(strlen(co_2) > 0){
        strcat(str_co2,"&co%5Bsensor_id%5D=c_1&co%5Bvalue%5D=");
        strcat(str_co2,co_2);
        int del = send_delay;
        int co2 = atoi(co_2);
        //CO2 concentration is greater than 800ppm,the acceleration data is transmitted to one minute
       // if(co2 >= 800){
         //   send_delay = co2_delay;
         //   printf("get_co2=%s,send_delay=%d\n",co_2,send_delay);
      //  }else{
        //    send_delay = del;
       // }
    }else{
        printf("get_co2 don't find co_2 value!\n");
    }

	if(fp) fclose(fp);
*/
    sprintf(str_co2,"&co%5Bsensor_id%5D=c_1&co%5Bvalue%5D=%d",avg_co2);
    return 0;
}

void set_avg_sensor_data(int dust1,int dust2,double temp,double humanity,int co2){
	avg_dust1 = dust1;
	avg_dust2 = dust2;
	avg_temp = temp;	
	avg_humanity = humanity;
	avg_co2 = co2;

    fprintf(stderr,"avg_dust1:%d\tavg_dust2:%d\tavg_temp:%.2f\tavg_humanity:%.2f\tavg_co2:%d\n\n\n",avg_dust1,avg_dust2,avg_temp,avg_humanity,avg_co2);
}

int get_post_text(char **post_text,ServerType type){
    int len = 0;
    int error = 0;
    char *local_data = NULL;
    char temp_hum[128];
    char particle[128];
    char str_co2[128];
    //Get Locat Data....
    if(error = get_local_data(&local_data,type) < 0){
        printf("get_local_data error -%d\n",error);
        return -1;
    }else{
        len += strlen(local_data);
    }

    //Get temperature Data and  humidity Data....
    if(error = get_temp_hum(temp_hum) < 0){
        printf("get_temp_hum error -%d\n",error);
        return -2;
    }else{
        len += strlen(temp_hum);
    }
    //Get Particle Data....
    if(error = get_particle(particle) < 0){
        printf("get_particle error -%d\n",error);
        return -3;
    }else{
        len += strlen(particle);
    }
    //Get CO2 Data....
    if(error = get_co2(str_co2) < 0){
        printf("get_co2 error -%d\n",error);
        return -4;
    }else{
        len += strlen(str_co2);
    }

    *post_text = NULL;
    len += 1;
    *post_text = (char*)malloc(len);
    if(*post_text != NULL){
        memset(*post_text,0,len);
        strcpy(*post_text,local_data);
        strcat(*post_text,temp_hum);
        strcat(*post_text,particle);
        strcat(*post_text,str_co2);
    }else{
        printf("post_text malloc -4\n");
        if(local_data != NULL)
            free(local_data);
        return -4;
    }
    if(local_data != NULL){
        free(local_data);
    }
    return 0;
}

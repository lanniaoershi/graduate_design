/*************************************************************************
	> File Name: Main.c
	> Author: weiwei
	> Mail: lanniaoershi@163.com 
	> Created Time: Sun 23 Mar 2014 23:25:21 CST
 ************************************************************************/
#define BUFFSIZE 512
#include<stdio.h>
#include<string.h>
char  *get_temp_hum(char *temp_hum){

    char row_msg[BUFFSIZE];
    char temperature[8];
    char humidity[8];
    int count = 0;
    FILE *fp = NULL;
    fp = fopen("./TH.data","r");
    if(fp == NULL){
        printf("don't open Adafruit_DHT_bak.log\n");
        return "false";
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
        strcat(temp_hum,temperature);
        strcat(temp_hum,"&");
        strcat(temp_hum,humidity);
    }else{
        printf("Sensor not ready\n");
    }

    if(fp) fclose(fp); 


    printf("%s\n",temp_hum);

    return temp_hum;
}

void main()
{
	char temp_hum[100];
	char result[100];
	system("./Adafruit_DHT  22 4 > TH.data");
	strcpy(result,get_temp_hum(temp_hum));
    printf("%s\n",result);
}

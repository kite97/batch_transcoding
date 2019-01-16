#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#include "wave.h"

#define WAVE 1
#define ERROR 2

extern void encoding(int argc, char judge, char* file_name, char* address);

int num_soft_link = 0;

//通配符检测，用于通过文件名检测文件格式
int match(char *str1, char *str2)
{
    if(str1 == NULL || str2 == NULL)
        return 0 ;
    int len1 = strlen(str1);
    int len2 = strlen(str2);
    char *tmp_str = str2 ;
    int tmp_len = len2 ;
    int location = 0 ;
    int match_flag = 0 ;
    int i = 0 ;
    while(i<tmp_len) {
        if(tmp_str[i] == '*') {
            location = i+1 ;
            break ;
        }
        i++ ;
    }
    char *tmp_str1 = str1 ;
    char *tmp_str2 = str2 ;
    if(location) {
        location -= 1;
        if(!strncmp(str1, str2, location)) {
            tmp_len = len2 - location ;
            tmp_str1 += len1 ;
            tmp_str2 += len2 ;
            while(tmp_len && (*tmp_str1 == *tmp_str2)){
                tmp_str1 -- ;
                tmp_str2 -- ;
                tmp_len -- ;
            }
            match_flag = tmp_len?0:1 ;
        }
    }
    return match_flag ;
}

//判断文件类型
int type_check(wave_t *wav, char* file_name){
    if (match(file_name,"*.wav") || match(file_name,"*.WAV")){
        if(!strncasecmp(wav->riff.wave_id, "wave", 4)){
            return WAVE;
        }
        else{
            return ERROR;
        }
    }
    else{
        return ERROR;
    }
}

//遍历目录中的音频文件并分析
void ergodic_statistics(int argc, char judge, char* dirname){
    char* file_name = NULL;
    char address[128];
    DIR *dir_ptr;
    struct dirent *direntp;
    dir_ptr = opendir(dirname);
    if(dir_ptr == NULL){
        printf("Error: can not open %s\n",dirname);
    }
    while( direntp = readdir(dir_ptr) ){
        if(strcmp(direntp->d_name, "..") == 0 || strcmp(direntp->d_name,".") == 0){
            continue;
        }
        if(direntp->d_type == 4){
            file_name = direntp->d_name;
            sprintf(address,"%s/%s",dirname,file_name);
            ergodic_statistics(argc, judge, address);
        }
        else if(direntp->d_type == 10){
            file_name = direntp->d_name;
            num_soft_link++;
            sprintf(address,"%s/%s",dirname,file_name);
            printf("The address of soft-link %d is %s\n\n",num_soft_link,address);
        }
        else if(direntp->d_type == 8){
            file_name = direntp->d_name;
            printf("%s:\n",file_name);
            sprintf(address,"%s/%s",dirname,file_name);
            encoding(argc, judge, file_name, address);
        }
    }
}

//主程序
int main(int argc, char* argv[]) {
    if(argc == 1){
        printf("There is no file\n");
    }
    else{
        char judge;
        judge = argv[2][0];
        char * dirname;
        dirname = argv[1];
        ergodic_statistics(argc,judge,dirname);
        printf("Having %d soft-link\n",num_soft_link);
        putchar('\n');
    }
    return 0;
}

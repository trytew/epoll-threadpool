/******************************************************************************
 *Copyright(C),2010-2011,nicenelg@gmail.com
 *FileName: config_array.h
 *Author:nelg
 *Version:v1.0
 *Date:2018-12-11
 *Description: get config info
 *Others:
 *Function List:
******************************************************************************/

#include "config_array.h"

/*所有配置信息*/
static char ***CONFIG;

int 
conf_init(void)
{
    int    result=-1;
    char  *conf_file_buff=NULL, *file_end=NULL, *reset=NULL;
    int    file_length=0;
    char  *row=NULL, *r_end=NULL;
    int    rl=0;
    char  *sign=NULL, *s_start=NULL, *s_end=NULL;
    int    i=0, go_on=1, options=0, j=0;
    char  *options_reset=NULL;
    char **options_val=NULL;

    /*初始化配置信息全局变量*/
    CONFIG = (char ***)malloc(CONF_TOP_OPTIONS_MAX * sizeof(char **));

    /*获取内存空间放置配置文件信息*/
    conf_file_buff = (char *)malloc(CONF_FILE_MAX_SIZE);
    if(conf_file_buff == NULL) {
        /*内存分配错误*/
        return result;
    }
    bzero(conf_file_buff, CONF_FILE_MAX_SIZE);

    /*读取文件内容，放置到内存中*/
    result = fopt_get_file_content(conf_file_buff, CONF_FILE, 
                (uint64_t)CONF_FILE_MAX_SIZE);
    if(result != 0) {
        /*读取文件错误*/
        goto CONF_INIT_EXIT;
    }
    reset = conf_file_buff;
    /*计算文本长度*/
    file_length = strlen(conf_file_buff);
    file_end = conf_file_buff + (file_length - 1);
    /*单行读取*/
    while(conf_file_buff != file_end) {
        go_on = 1;
        r_end = strstr(conf_file_buff, "\n");
        if(r_end == NULL) {
            break;
        }
        /*复制当前行*/
        rl = r_end - conf_file_buff;
        row = (char *)malloc(rl * sizeof(char));
        memcpy(row, conf_file_buff, rl);
        /*指针偏移*/
        conf_file_buff = conf_point_offset(r_end, file_end);
        /*判断标志位*/
        if(row[0] == ';') {
            go_on = 0;
        }else if(!((s_start = strstr(row, "[")) && (s_end = strstr(row, "]")))){
            go_on = 0;
        }
        if(conf_file_buff == file_end) {
            free(row);
            row = NULL;
            break;
        }
        if(!go_on) {
            free(row);
            row = NULL;
            continue;
        }

        /*获取标志位*/
        sign = (char *)malloc((s_end-s_start-1) * sizeof(char));
        memcpy(sign, s_start+1, s_end -s_start-1);
        free(row);
        row = NULL;
        for(i=0;i<CONF_TOP_OPTIONS_MAX;i++) {
           if(strcmp(_CONF_OPTIONS[i],sign) != 0) {
               continue;
           }
           free(sign);
           sign = NULL;
           /*计算有多少个配置项*/
           options = 0;
           go_on = 1;
           options_reset = conf_file_buff;
           while(conf_file_buff != file_end) {
               r_end = strstr(conf_file_buff, "\n");
               if(r_end == NULL) {
                   r_end = file_end;        
               }
               if(r_end == NULL) {
                   break;
               }
               if(*conf_file_buff == ';') {
                   conf_file_buff = conf_point_offset(r_end, file_end);
                   continue;
               }
               rl = r_end - conf_file_buff;
               row = (char *)malloc(rl * sizeof(char));
               memcpy(row, conf_file_buff, rl);
               if(strstr(row, "=")) {
                   options++;
               }else if(strstr(row, "[") && strstr(row, "]")) {
                   go_on = 0;
               }
               free(row);
               row = NULL;
               conf_file_buff = conf_point_offset(r_end, file_end);
               if(!go_on) {
                   break;
               }
           }
           /*若没有配置项则跳出循环*/
           if(!options) {
               break;
           }
           /*重置内存地址，读取配置*/
           conf_file_buff = options_reset;
           /*分配内存空间*/
           CONFIG[i] = (char **)malloc(options * sizeof(char *));
           /*读取配置*/
           while(conf_file_buff != file_end) {
               go_on = 1;
               r_end = strstr(conf_file_buff, "\n");
               if(r_end == NULL) {
                   r_end = file_end;        
               }
               if(r_end == NULL) {
                   break;
               }
               if(*conf_file_buff == ';') {
                   conf_file_buff = conf_point_offset(r_end, file_end);
                   continue;
               }
               rl = r_end - conf_file_buff;
               row = (char *)malloc((rl+1) * sizeof(char));
               memcpy(row, conf_file_buff, rl);
               row[rl] = '\0';
               if(strstr(row, "=")) {
                   stropt_del_chr(row, (char)' ');
               }else if(strstr(row, "[") && strstr(row, "]")) {
                   go_on = -1;
               }else {
                   go_on = 0;
               }
               if(!go_on) {
                   free(row);
                   row = NULL;
                   conf_file_buff = conf_point_offset(r_end, file_end);
                   if(go_on == 0) {
                       continue;
                   } else {
                       break;
                   }
               }
               rl = strlen(row);
               s_end = strstr(row, "=");
               sign = (char *)malloc((s_end - row) * sizeof(char));
               memcpy(sign, row, (s_end - row));
               j=0;
               options_val = _CONF_OPTIONS_VAL[i];
               while(*options_val) {
                   if(strcmp(*options_val,sign) != 0) {
                       j++;
                       options_val++;
                       continue;
                   }
                   CONFIG[i][j] = (char *)malloc((rl-(s_end-row)+1) * 
                               sizeof(char));
                   memcpy(CONFIG[i][j], s_end+1, (rl-(s_end-row)+1));
                   break;
               }
               free(sign);
               sign = NULL;
               free(row);
               row = NULL;
               conf_file_buff = conf_point_offset(r_end, file_end);
           }
           break;
        }
        if(sign) {
            free(sign);
            sign = NULL;
        }
    }
    printf("%s\n", CONFIG[CONF_MYSQL][CONF_MYSQL_HOST]);
CONF_INIT_EXIT:
    free(reset);
    reset = NULL;
    conf_file_buff = NULL;
    return result;
}

int
conf_get_option_val(conf_val val, conf_opt config, conf_opt options)
{
    //*val = CONFIG[config][options];
    return 0;
}

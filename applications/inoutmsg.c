
#include <inoutmsg.h>

#define YGP_INOUTMSG_DEBUG
int testkey = 0;    //用于测试key的值，保留测试用
int iom1(char* in,int sizeofin,char* out,int sizeofout){
    int result = 0;
    int i;
    int j;

    char *CMD = NULL;
    char *key = NULL;
    char *setvalue = NULL;
    int getvalue=0;
    memset(out,0,sizeofout);


    int inlen = strlen(in);
    if (inlen >= sizeofin) {
        //应为要留\0,所以大于等于都为错误
        result = INLEN_ERR;
        goto __exit;
    }

    //开始解析
    //找"
    j = 0;
    for (i = 0; i < inlen; ++i) {
        if (*(in+i)=='\"') {
            if (j==0) {
                CMD = in+i+1;
            }else if (j==2) {
                key = in+i+1;
            }else if (j==5) {
                setvalue = in+i+2;
            }

            *(in+i) = '\0';
            j++;
        }

        if (*(in+i)=='{' || *(in+i)=='}' || *(in+i)==':') {
            *(in+i) = '\0';
        }
    }

#ifdef YGP_INOUTMSG_DEBUG
    rt_kprintf("CMD=%s\n",CMD);
    rt_kprintf("key=%s\n",key);
    rt_kprintf("setvalue=%s\n",setvalue);
#endif

    if (strcmp(CMD,"GET")==0) {
        if (strcmp(key,"key")==0) {
            testkey++;
            getvalue = testkey;
            sprintf(out,"{\"GET\":\"OK\",\"%s\":%d}",key,getvalue);
            return strlen(out);
        }else if (strcmp(key,"IPPORT")==0) {
            char IP[20] = {0};
            extern void getIP(char* IP);
            getIP(IP);
            sprintf(out,"{\"GET\":\"OK\",\"%s\":%s}",key,IP);
            return strlen(out);
        }else if (strcmp(key,"LED1")==0) {
            getvalue = (rt_pin_read(DOPIN1)+1)%2;
            sprintf(out,"{\"GET\":\"OK\",\"%s\":%d}",key,getvalue);
            return strlen(out);
        }else if (strcmp(key,"LED2")==0) {
            getvalue = (rt_pin_read(DOPIN2)+1)%2;
            sprintf(out,"{\"GET\":\"OK\",\"%s\":%d}",key,getvalue);
            return strlen(out);
        }else if (strcmp(key,"LED3")==0) {
            getvalue = (rt_pin_read(DOPIN3)+1)%2;
            sprintf(out,"{\"GET\":\"OK\",\"%s\":%d}",key,getvalue);
            return strlen(out);
        }else if (strcmp(key,"LED4")==0) {
            getvalue = (rt_pin_read(DOPIN4)+1)%2;
            sprintf(out,"{\"GET\":\"OK\",\"%s\":%d}",key,getvalue);
            return strlen(out);
        }else {
            result = KEY_ERR;
            goto __exit;
        }
    }else if (strcmp(CMD,"SET")==0) {
        //设定值校验 预留可有小数点和负号
        int svlen = strlen(setvalue);
        int pnum = 0;
        for (i = 0; i < svlen; ++i) {
            if (i==0) {
                if ((*(setvalue+i)<48 || *(setvalue+i)>57)&&(*(setvalue+i) != '-')) {
                    result = SETVALUE_ERR;
                    goto __exit;
                }
            }else {
                if ((*(setvalue+i)<48 || *(setvalue+i)>57)&&(*(setvalue+i) != '.')) {
                    result = SETVALUE_ERR;
                    goto __exit;
                }
            }

            if (*(setvalue+i) == '.') {
                //统计点的个数,至多只能有1个
                pnum++;
            }
        }
        if (pnum>1) {
            result = SETVALUE_ERR;
            goto __exit;
        }
        //rt_kprintf("setvalue=%s;\n",setvalue);
        char temp[64];
        sprintf(temp,"%s",setvalue);//这里存在字符串检索问题 需要中间变量过渡一下
        //rt_kprintf("temp=%s;\n",temp);
        int intsetvalue = atoi(temp);
        //rt_kprintf("intsetvalue=%d;\n",intsetvalue);

        if (strcmp(key,"key")==0) {
            //先设置
            testkey = intsetvalue;
            //后读取
            getvalue = testkey;
        }else if (strcmp(key,"LED1")==0) {
            //先设置
            rt_pin_write(DOPIN1, (intsetvalue+1)%2);
            //后读取
            getvalue = (rt_pin_read(DOPIN1)+1)%2;
        }else if (strcmp(key,"LED2")==0) {
            rt_pin_write(DOPIN2, (intsetvalue+1)%2);
            getvalue = (rt_pin_read(DOPIN2)+1)%2;
        }else if (strcmp(key,"LED3")==0) {
            rt_pin_write(DOPIN3, (intsetvalue+1)%2);
            getvalue = (rt_pin_read(DOPIN3)+1)%2;
        }else if (strcmp(key,"LED4")==0) {
            rt_pin_write(DOPIN4, (intsetvalue+1)%2);
            getvalue = (rt_pin_read(DOPIN4)+1)%2;
        }else {
            result = KEY_ERR;
            goto __exit;
        }

        sprintf(out,"{\"SET\":\"OK\",\"%s\":%d}",key,getvalue);
        return strlen(out);
    }else {
        result = CMD_ERR;
        goto __exit;
    }



__exit:

    sprintf(out,"{\"ERR\":%d}",result);
    return result;
}


char inmsg[128];
char outmsg[128];
void k001(int argc, char **argv){
    int result;

    result = iom1(argv[1],128,outmsg,128);
    rt_kprintf("result=%d;\n",result);
    rt_kprintf("outmsg=%s;\n",outmsg);
}
MSH_CMD_EXPORT(k001, k001);
//k001 {"GET":"key"}
//k001 {"SET":"key","key":350}
//k001 {"GET":"wendu"}
//k001 {"SET":"wendubaojin","wendubaojin":350}

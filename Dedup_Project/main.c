//
//  main.c
//  Dedup_Project
//
//  Created by Aditya Kotwal on 5/22/14.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define FNAME "/Users/akotwal/Desktop/Dedup_Project/duplicateFile"
#define verbose 1

// MD5 Libraries:
#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#  define SHA1 CC_SHA1
#else
#  include <openssl/md5.h>
#endif

#define MAX_FILE_NAME 1024
//#define MAX_BUF_LEN 4*1024
#define MAX_BUF_LEN 4
#define RDLEN MAX_BUF_LEN
#define CRYPT_LEN 20


int main(int argc, const char * argv[])
{
    MD5_CTX c;
    char fileName[MAX_FILE_NAME];
    strcpy(fileName,FNAME);
    FILE *ip,*op;
    if((ip = fopen(fileName,"rb"))==NULL){
        printf("File could not be opened, error, %d\n",errno);
    }
    if((op = fopen("/Users/akotwal/Desktop/Dedup_Project/outfile","wb"))==NULL){
        printf("File could not be opened, error, %d\n",errno);
    }
    
    char header[MAX_FILE_NAME];
    if(verbose){
        strcpy(header,"String, File Name, md5, offset, size \n");
        if(fwrite(header,1,strlen(header),op)!=strlen(header)){
            printf("Error in writing filename\n");
        }
    }
    char *buf = NULL;
    if((buf = malloc(4*1024)) == NULL){
        printf("Error in allocating memory");
    }
    char *buf_orig=buf;
    char out[16*2+1];
    unsigned long len;
    unsigned char final[16];
    unsigned long offset=0;
    unsigned long offsetCorrection=0;
    char *buf_read = buf;
    while((len=fread(buf,1,RDLEN,ip))!=0){
        MD5_Init(&c);
        offsetCorrection=len;
        
        while(len>0){
            if(len > 512){
                MD5_Update(&c, buf, 512);
                len -= 512;
                buf += 512;
            } else{
                MD5_Update(&c, buf, len);
                break;
            }
        }
        MD5_Final(final, &c);
        for (int n = 0; n < CRYPT_LEN; ++n) {
            snprintf(&(out[n*2]), CRYPT_LEN*2, "%02x",final[n]);
        }
        if(verbose){
            if(fwrite(buf_read,1,strlen(buf_read),op)!=strlen(buf_read)){
                printf("Error in writing buf\n");
            }
            if(fwrite(",",1,strlen(","),op)!=strlen(",")){
                printf("Error in writing ,\n");
            }
        }
        if(fwrite(fileName,1,strlen(fileName),op)!=strlen(fileName)){
            printf("Error in writing filename\n");
        }
        if(fwrite(",",1,strlen(","),op)!=strlen(",")){
            printf("Error in writing filename\n");
        }
        if(fwrite(out,1,strlen(out),op)!=strlen(out)){
            printf("Error in writing md5 value\n");
        }
        if(fwrite(",",1,strlen(","),op)!=strlen(",")){
            printf("Error in writing f ','\n");
        }
        char offset_str[MAX_BUF_LEN];
        sprintf(offset_str, "%lu",offset);
        if(fwrite(offset_str,1,strlen(offset_str),op)!=strlen(offset_str)){
            printf("Unable to write offest\n");
        }
        if(fwrite(",",1,strlen(","),op)!=strlen(",")){
            printf("Error in writing ','\n");
        }
        char offsetCorr_str[MAX_BUF_LEN];
        sprintf(offsetCorr_str, "%lu",offsetCorrection);
        if(fwrite(offsetCorr_str,1,strlen(offsetCorr_str),op)!=strlen(offsetCorr_str)){
            printf("Unable to write size\n");
        }
        if(fwrite("\n",1,strlen("\n"),op)!=strlen("\n")){
            printf("Error in writing newline\n");
        }
        offset += offsetCorrection;
        buf=buf_orig;
    }
    if(fclose(op)!=0){
        printf("Error in closing file, %d\n",errno);
    }
    if(fclose(ip)!=0){
        printf("Error in closing file, %d\n",errno);
    }
    
    return 0;
}

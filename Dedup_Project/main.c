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
#define verbose 0
#define fileNameRequired 0

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
#define CRYPT_LEN 16

// Error checking wrapper for fwrite
void FWRITE(char *buf,char *msg, FILE* stream){
    if(fwrite(buf,1,strlen(buf),stream)!=strlen(buf)){
        printf("Error in writing %s to file\n",msg);
    }
}
//Error checking wrapper for fopen
FILE* FOPEN(char *fileName, char* mode){
    FILE* fp = NULL;
    if((fp=fopen(fileName,mode))==NULL){
        printf("Failed to open file %s in %s mode\n",fileName,mode);
    }
    return fp;
}

int main(int argc, const char * argv[])
{
    MD5_CTX c;
    char fileName[MAX_FILE_NAME];
    strcpy(fileName,FNAME);
    FILE *ip,*op;
    char out[16*2+1];
    unsigned long len;
    unsigned char final[16];
    unsigned long offset=0;
    unsigned long offsetCorrection=0;
    char *buf = NULL, *buf_orig = NULL, *buf_read=NULL;
    
    // Opening input and output file
    ip = FOPEN(fileName,"rb");
    op = FOPEN("/Users/akotwal/Desktop/Dedup_Project/outfile","wb");
    
    char header[MAX_FILE_NAME];
    if(verbose){
        strcpy(header,"String, File Name, md5, offset, size \n");
        FWRITE(header, "header", op);
    }

    if((buf = malloc(4*1024)) == NULL){
        printf("Error in allocating memory");
    }
    buf_orig=buf;
    buf_read = buf;
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
        for (int n = 0; n < 16; ++n) {
            snprintf(&(out[n*2]), 16*2, "%02x",final[n]);
        }
        if(verbose){
            FWRITE(buf_read, "buf",op);
            FWRITE(",",",",op);
        }
        if(fileNameRequired){
            FWRITE(fileName, "fileName",op);
            FWRITE(",",",",op);
        }
        FWRITE(out,"md5value",op);
        FWRITE(",",",",op);

        char offset_str[MAX_BUF_LEN];
        sprintf(offset_str, "%lu",offset);
        FWRITE(offset_str,"offset",op);
        FWRITE(",",",",op);
        
        char offsetCorr_str[MAX_BUF_LEN];
        sprintf(offsetCorr_str, "%lu",offsetCorrection);
        FWRITE(offsetCorr_str,"size",op);
        FWRITE(",",",",op);
        
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

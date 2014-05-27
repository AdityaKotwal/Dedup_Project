

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

/************ MACROs **************/

#define MAX_FILE_NAME 1024
//#define MAX_BUF_LEN 4*1024
#define MAX_BUF_LEN 4
#define RDLEN MAX_BUF_LEN
#define CRYPT_LEN 16

/*********** WRAPPERS FOR ERROR CHECKING ***************/

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

void FCLOSE(FILE *fp){
    if(fclose(fp)!=0){
        printf("Error closing file, %d\n",errno);
    }
}

/************** ERROR CHECKING DONE ********************/

int main(int argc, const char * argv[])
{
    MD5_CTX c;
    char fileName[MAX_FILE_NAME];
    strcpy(fileName,FNAME);
    FILE *ip,*op;
  
    unsigned long len;
    unsigned char final[MD5_DIGEST_LENGTH];
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
        char out[16*2+1];
        strcpy(out,"");
        for(int i=0;i<MD5_DIGEST_LENGTH;i++){
            sprintf(out,"%s%02x",out,final[i]);
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
    free(buf);
    FCLOSE(op);
    FCLOSE(ip);
    return 0;
}

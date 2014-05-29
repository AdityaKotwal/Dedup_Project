
/* 
 * Author : Aditya Kotwal
 * Date of Start : 5/22/2014
 * Email : akotwal@vmware.com, adityako@andrew.cmu.edu, kotwal13aditya@gmail.com
 */

/*
 * Source code to compute md5 hashes of segment
 * sizes provided as parameters. The generated segments
 * are stored in a csv file which can be directly loaded
 * into the database.
 */

/***** RUNNING THIS CODE ********
 
 This code is not to be run directly. Rather, The
 script mainScript.sh calls this code with command
 line argument "1" or "2"
 
 
 ********************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#define SRC "/Users/akotwal/Desktop/Dedup_Project/Dedup_Project/testFile1"
#define DST "/Users/akotwal/Desktop/Dedup_Project/outfile"
#define verbose 0
#define fileNameRequired 0
#define MAX_COMBINED_HASH_LEN 1024*1024
// MD5 Libraries:
#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#else
#  include <openssl/md5.h>
#endif

/************ MACROs **************/

#define MAX_FILE_NAME 1024
//#define MAX_BUF_LEN 4*1024
#define MAX_BUF_LEN 4  // <- Smaller segment size for testing
#define RDLEN MAX_BUF_LEN

/********* Function Declarations *************/
void generate(char* source, char* destination);
void getMD5(const char *string, char *md5buf, long len);
void combine();

/******** Wrappers for error checking **********/
void FWRITE(char *buf,char *msg, FILE* stream);
FILE* FOPEN(char *fileName, char* mode);
void FCLOSE(FILE *fp);


int main(int argc, char * argv[])
{
    char source[MAX_FILE_NAME],destination[MAX_FILE_NAME];
    strcpy(source,SRC);
    strcpy(destination,DST);
    if(strcmp(argv[1],"1")==0){
        generate(source,destination);
    }
    else if(strcmp(argv[1],"2")==0){
        combine();
    }
    else{
        printf("Wrong input");
    }
    return 0;
}

/**************** FUNCTION DEFINITIONS *********************/


// Reads from new table which contains combined md5s and
// summed of sizes of adjacent blocks and generated new
// md5s
void combine(){
    FILE *ip= FOPEN("/tmp/file1", "rb");
    FILE *op= FOPEN("/tmp/nextLevel","w");
    char *line;
    char str[MAX_COMBINED_HASH_LEN];
    size_t maxLen=MAX_COMBINED_HASH_LEN;
    char md5str[MD5_DIGEST_LENGTH*2+1];
    while(getline(&line,&maxLen,ip)!=-1){
        char *token = strtok(line, ",");
        getMD5(token,md5str,strlen(token));
        token = strtok(NULL,",");
        sprintf(str,"%s,%s",md5str,token);
        FWRITE(str, "Entry row", op);
    }
    return;
}


// Generate text dump by scanning the target location
// defined in SRC macro and generate md5 hashes after
// dividing the file into 4kB segments. The csv output
// is dumped to the path passed in destination.

void generate(char* source, char* destination){
    FILE *ip,*op;
    ip = FOPEN(source,"rb");
    op = FOPEN(destination,"wb");
    unsigned long len;
    
    unsigned long offset=0;
    char *buf = NULL, *buf_orig = NULL, *buf_read=NULL;
    
    // Opening input and output file
    char header[MAX_FILE_NAME];
    if(verbose){
        strcpy(header,"String, File Name, md5, offset, size \n");
        FWRITE(header, "header", op);
    }
    
    if((buf = (char*)malloc(4*1024)) == NULL){
        printf("Error in allocating memory");
    }
    buf_orig=buf;
    buf_read = buf;
    while((len=fread(buf,1,RDLEN,ip))!=0){
        // Generate MD5 here
        char out[MD5_DIGEST_LENGTH*2+1];
        getMD5(buf,out,len);
        if(verbose){
            FWRITE(buf_read, "buf",op);
            FWRITE(",",",",op);
        }
        if(fileNameRequired){
            FWRITE(source, "fileName",op);
            FWRITE(",",",",op);
        }
        FWRITE(out,"md5value",op);
        FWRITE(",",",",op);
        
        char offset_str[MAX_BUF_LEN];
        sprintf(offset_str, "%lu",offset);
        FWRITE(offset_str,"offset",op);
        FWRITE(",",",",op);
        
        char size_str[MAX_BUF_LEN];
        sprintf(size_str, "%lu",len);
        FWRITE(size_str,"size",op);
        FWRITE("\n","New Line",op);
        
        offset += len;
    }
    free(buf);
    FCLOSE(op);
    FCLOSE(ip);
    
}

/************** GET MD5 ********************/
// This function takes in as parameters:
// 1) string : Character string who's md5 values has to be found
// 2) md5buf: A memory allocated budffer whereto the md5 hash would be written
// 3) len: Length of the string who's md5 value has to be found out
/********************************************/

void getMD5(const char *string, char *md5buf, long len){
    unsigned char final[MD5_DIGEST_LENGTH];
    MD5_CTX c;
    MD5_Init(&c);
    char *str = (char*) string;
    while(len>0){
        if(len > 512){
            MD5_Update(&c, str, 512);
            len -= 512;
            str += 512;
        } else{
            MD5_Update(&c, str, len);
            break;
        }
    }
    MD5_Final(final, &c);
    strcpy(md5buf,"");
    for(int i=0;i<MD5_DIGEST_LENGTH;i++){
        sprintf(md5buf,"%s%02x",md5buf,final[i]);
    }
    return;
}


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

// Error checking wrapper for fclose
void FCLOSE(FILE *fp){
    if(fclose(fp)!=0){
        printf("Error closing file, %d\n",errno);
    }
}

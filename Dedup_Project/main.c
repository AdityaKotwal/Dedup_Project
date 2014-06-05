
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
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <regex.h>
#include <err.h>
//#define SRC "/tempInFile"
#define DST "/Users/akotwal/Desktop/tempOutFile"
#define verbose 0
#define fileNameRequired 1
#define MAX_COMBINED_HASH_LEN 1024*1024
// MD5 Libraries:
#if defined(__APPLE__)
#  define COMMON_DIGEST_FOR_OPENSSL
#  include <CommonCrypto/CommonDigest.h>
#else
#  include <openssl/md5.h>
#endif

/************ MACROs **************/

#define MAX_FILE_NAME 1024*1024
#define MAX_BUF_LEN 4*1024
//#define MAX_BUF_LEN 4  // <- Smaller segment size for testing
#define RDLEN MAX_BUF_LEN

/********* Function Declarations *************/
void generate(char* source, char* destination);
void getMD5(const char *string, char *md5buf, long len);
void combine();
void doit(char *source);
void cleanFile(char *file);

/******** Wrappers for error checking **********/
void FWRITE(char *buf,char *msg, FILE* stream);
FILE* FOPEN(char *fileName, char* mode);
void FCLOSE(FILE *fp);

enum {
	WALK_OK = 0,
	WALK_BADPATTERN,
	WALK_NAMETOOLONG,
	WALK_BADIO,
};

#define WS_NONE		0
#define WS_RECURSIVE	(1 << 0)
#define WS_DEFAULT	WS_RECURSIVE
#define WS_FOLLOWLINK	(1 << 1)	/* follow symlinks */
#define WS_DOTFILES	(1 << 2)	/* per unix convention, .file is hidden */
#define WS_MATCHDIRS	(1 << 3)	/* if pattern is used on dir names too */

int walk_recur(char *dname, regex_t *reg, int spec, void (*hashFile)(char *file))
{
	struct dirent *dent;
	DIR *dir;
	struct stat st;
	char fn[FILENAME_MAX];
	int res = WALK_OK;
	int len = (int)strlen(dname);
	if (len >= FILENAME_MAX - 1)
		return WALK_NAMETOOLONG;

	strcpy(fn, dname);
	fn[len++] = '/';

	if (!(dir = opendir(dname))) {
		warn("can't open %s", dname);
		return WALK_BADIO;
	}

	errno = 0;
	while ((dent = readdir(dir))) {
		if (!(spec & WS_DOTFILES) && dent->d_name[0] == '.')
			continue;
		if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, ".."))
			continue;

		strncpy(fn + len, dent->d_name, FILENAME_MAX - len);

		if (lstat(fn, &st) == -1) {
			warn("Can't stat %s", fn);
			res = WALK_BADIO;
			continue;
		}

		/* don't follow symlink unless told so */
		if (S_ISLNK(st.st_mode) && !(spec & WS_FOLLOWLINK))
			continue;

		/* will be false for symlinked dirs */
		if (S_ISDIR(st.st_mode)) {
			/* recursively follow dirs */
			if ((spec & WS_RECURSIVE))
				walk_recur(fn, reg, spec,hashFile);

			if (!(spec & WS_MATCHDIRS)) continue;
		}

		//printf("%s\n",fn);
		if(S_ISREG(st.st_mode)){
			printf("Considering file %s\n",fn);
			hashFile(fn);
			//printf("ISRED IS %d \n",S_IFREG);
		}
		/* pattern match */
		if (!regexec(reg, fn, 0, 0, 0)){
		//	puts(fn);
		}
	}

	if (dir) closedir(dir);
	return res ? res : errno ? WALK_BADIO : WALK_OK;
}

int walk_dir(char *dname, char *pattern, int spec, void (*hashFile)(char *file))
{
	regex_t r;
	int res;
	if (regcomp(&r, pattern, REG_EXTENDED | REG_NOSUB))
		return WALK_BADPATTERN;
//	printf("%s",dname);
	res = walk_recur(dname, &r, spec,hashFile);
	regfree(&r);
    
	return res;
}

int main()
{
	cleanFile(DST);
	int r = walk_dir("/Users/akotwal/Documents", ".\\.c$", WS_DEFAULT|WS_MATCHDIRS,doit);
	switch(r) {
        case WALK_OK:		break;
        case WALK_BADIO:	err(1, "IO error");
        case WALK_BADPATTERN:	err(1, "Bad pattern");
        case WALK_NAMETOOLONG:	err(1, "Filename too long");
        default:
            err(1, "Unknown error?");
	}
	return 0;
}


void doit(char *source)
{
	printf("%s\n",source);
   	char destination[MAX_FILE_NAME];
    strcpy(destination,DST);
    FILE *fp=NULL;
    if((fp=fopen(source, "rb"))==NULL){
        return;
    }
    else{
        FCLOSE(fp);
    }
    generate(source,destination);
    return;
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


void cleanFile(char *file){
	FILE *fp=NULL;
	fp=FOPEN(file,"wb");
	FCLOSE(fp);
	return;
}

// Generate text dump by scanning the target location
// defined in SRC macro and generate md5 hashes after
// dividing the file into 4kB segments. The csv output
// is dumped to the path passed in destination.

void generate(char* source, char* destination){
    FILE *ip,*op;
	printf("File to work on : %s\n",source);
    ip = FOPEN(source,"rb");
    op = FOPEN(destination,"ab");
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
    int i;
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
    for(i=0;i<MD5_DIGEST_LENGTH;i++){
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

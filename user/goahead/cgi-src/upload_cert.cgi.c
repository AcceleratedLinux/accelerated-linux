
/*
 * Codes at here are heavily taken from upload.cgi.c which is for large file uploading , but 
 * in fact "upload_settings" only need few memory(~16k) so it is not necessary to follow 
 * upload.cgi.c at all.
 * 
 * chhung@Ralink TODO: code size.
 *  
 */

#include <unistd.h>	//for unlink
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#define RFC_ERROR "RFC1867 ...."

#define REFRESH_TIMEOUT		"60000"		/* 40000 = 40 secs*/

void *memmem(const void *buf, size_t buf_len, const void *byte_line, size_t byte_line_len)
{
    unsigned char *bl = (unsigned char *)byte_line;
    unsigned char *bf = (unsigned char *)buf;
    unsigned char *p  = bf;

    while (byte_line_len <= (buf_len - (p - bf))){
        unsigned int b = *bl & 0xff;
        if ((p = (unsigned char *) memchr(p, b, buf_len - (p - bf))) != NULL){
            if ( (memcmp(p, byte_line, byte_line_len)) == 0)
                return p;
            else
                p++;
        }else{
            break;
        }
    }
    return NULL;
}

#define MEM_SIZE	1024
#define MEM_HALT	512
int findStrInFile(char *filename, int offset, unsigned char *str, int str_len)
{
	int pos = 0, rc;
	FILE *fp;
	unsigned char mem[MEM_SIZE];

	if(str_len > MEM_HALT)
		return -1;
	if(offset <0)
		return -1;

	fp = fopen(filename, "rb");
	if(!fp)
		return -1;

	rewind(fp);
	fseek(fp, offset + pos, SEEK_SET);
	rc = fread(mem, 1, MEM_SIZE, fp);
	while(rc){
		unsigned char *mem_offset;
		mem_offset = (unsigned char*)memmem(mem, rc, str, str_len);
		if(mem_offset){
			fclose(fp);	//found it
			return (mem_offset - mem) + pos + offset;
		}

		if(rc == MEM_SIZE){
			pos += MEM_HALT;	// 8
		}else
			break;
		
		rewind(fp);
		fseek(fp, offset+pos, SEEK_SET);
		rc = fread(mem, 1, MEM_SIZE, fp);
	}

	fclose(fp);
	return -1;
}

/*
 *  ps. callee must free memory...
 */
void *getMemInFile(char *filename, int offset, int len)
{
    void *result;
    FILE *fp;
    if( (fp = fopen(filename, "r")) == NULL ){
        return NULL;
    }
	fseek(fp, offset, SEEK_SET);
    result = malloc(sizeof(unsigned char) * len );
	if(!result)
		return NULL;
    if( fread(result, 1, len, fp) != len){
        free(result);
        return NULL;
    }
    return result;
}
#if defined UPLOAD_WAPI_AS_CERT_SUPPORT || defined UPLOAD_WAPI_USER_CERT_SUPPORT
#define CMD_SIZE	1050
#define CERT_SIZE	1024
#elif defined UPLOAD_KEY_CERT_SUPPORT
#define CMD_SIZE	2070
#define CERT_SIZE	2048
#else
#define CMD_SIZE	4120
#define CERT_SIZE	4096
#endif
#define FILE_NAME_SIZE	128
int upload(char *filename, char *name, int offset, int len)
{
	char data;
	FILE *src;
	char tar_file_name[FILE_NAME_SIZE] = "/etc/";
	char cmd[CMD_SIZE];
	char tmpdata[CERT_SIZE];
	int i = 0;

	strcat(tar_file_name, name);	
	memset(cmd, 0, CMD_SIZE);
#if defined UPLOAD_KEY_CERT_SUPPORT
	sprintf(cmd, "nvram_set cert KeyCertFile %s", tar_file_name);
#elif defined UPLOAD_CACL_CERT_SUPPORT
	sprintf(cmd, "nvram_set cert CACLCertFile %s", tar_file_name);
#elif defined UPLOAD_WAPI_AS_CERT_SUPPORT
	sprintf(cmd, "nvram_set wapi ASCertFile %s", tar_file_name);
#elif defined UPLOAD_WAPI_USER_CERT_SUPPORT
	sprintf(cmd, "nvram_set wapi UserCertFile %s", tar_file_name);
#endif
	system(cmd);

	if(!( src = fopen(filename, "r"))){
		printf("open src file error<br />\n");
		return 0;
	}

	if( fseek(src, offset, SEEK_SET) == -1){
		printf("fseek error<br />\n");
		fclose(src);
		return 0;
	}

	memset(tmpdata, 0, CERT_SIZE);
	while( len > 0){
		if(! fread(&data, 1, 1, src))
			break;
		tmpdata[i] = data;
		len--;
		i++;
	}
	fclose(src);

	// flash write
	if (CERT_SIZE < strlen(tmpdata))
	{
		printf("Client Certificate is too big!<br />\n");
		fclose(src);
		return 0;
	}
	memset(cmd, 0, CMD_SIZE);
#if defined UPLOAD_KEY_CERT_SUPPORT
	sprintf(cmd, "nvram_set cert KeyCert \"%s\"", tmpdata);
#elif defined UPLOAD_CACL_CERT_SUPPORT
	sprintf(cmd, "nvram_set cert CACLCert \"%s\"", tmpdata);
#elif defined UPLOAD_WAPI_AS_CERT_SUPPORT
	sprintf(cmd, "nvram_set wapi ASCert \"%s\"", tmpdata);
#elif defined UPLOAD_WAPI_USER_CERT_SUPPORT
	sprintf(cmd, "nvram_set wapi UserCert \"%s\"", tmpdata);
#endif
	system(cmd);
	memset(cmd, 0, CMD_SIZE);
#if defined UPLOAD_KEY_CERT_SUPPORT || defined UPLOAD_CACL_CERT_SUPPORT
	sprintf(cmd, "ralink_init gen cert");
#elif defined UPLOAD_WAPI_AS_CERT_SUPPORT || defined UPLOAD_WAPI_USER_CERT_SUPPORT
	sprintf(cmd, "ralink_init gen wapi");
#endif
	system(cmd);

	return 1;
}

int main (int argc, char *argv[])
{
    int file_begin, file_end;
    int line_begin, line_end;
    char *boundary; 
    int boundary_len;
    char *filename = getenv("UPLOAD_FILENAME");

    printf(
"\
Server: %s\n\
Pragma: no-cache\n\
Content-type: text/html\n",
getenv("SERVER_SOFTWARE"));

    printf("\n\
<html>\n\
<head>\n\
<TITLE>Upload Certificate</TITLE>\n\
<link rel=stylesheet href=/style/normal_ws.css type=text/css>\n\
<meta http-equiv=\"content-type\" content=\"text/html; charset=utf-8\">\n\
</head>\n\
<body>\n<h1> Certificate Install</h1>\n");

    if(!filename){
        printf("failed, can't get env var.\n");
        return -1;
    }

    line_begin = 0;
    if((line_end = findStrInFile(filename, line_begin, "\r\n", 2)) == -1){
        printf("%s", RFC_ERROR);
        return -1;
    }
	boundary_len = line_end - line_begin;
    boundary = getMemInFile(filename, line_begin, boundary_len);

    // sth like this..
    // Content-Disposition: form-data; name="filename"; filename="\\192.168.3.171\tftpboot\a.out"
    //
    char *line, *semicolon, *user_filename;
    line_begin = line_end + 2;
    if((line_end = findStrInFile(filename, line_begin, "\r\n", 2)) == -1){
        printf("%s", RFC_ERROR);
        goto err;
    }
    line = getMemInFile(filename, line_begin, line_end - line_begin);
    if(strncasecmp(line, "content-disposition: form-data;", strlen("content-disposition: form-data;"))){
        printf("%s", RFC_ERROR);
        goto err;
    }
    semicolon = line + strlen("content-disposition: form-data;") + 1;
    if(! (semicolon = strchr(semicolon, ';'))  ){
        printf("dont support multi-field upload.\n");
        goto err;
    }
    user_filename = semicolon + 2;
    if( strncasecmp(user_filename, "filename=", strlen("filename="))  ){
        printf("%s", RFC_ERROR);
        goto err;
    }
    char tempname[FILE_NAME_SIZE];
    char *name = NULL;
    char c = 0x5c;

    user_filename += strlen("filename=");
    memset(tempname, 0, sizeof(tempname));
    strncpy(tempname, user_filename+1, strrchr(user_filename, '"') - 1 - strchr(user_filename, '"'));
    name = strrchr(tempname, c);
    if (name != NULL)
	    name += 1;
    else
	    name = tempname;

    //until now we dont care about what  true filename is.
    free(line);

    // We may check a string  "Content-Type: application/octet-stream" here
    // but if our firmware extension name is the same with other known name, 
    // the browser will define other content-type itself instead,
    line_begin = line_end + 2;
    if((line_end = findStrInFile(filename, line_begin, "\r\n", 2)) == -1){
        printf("%s", RFC_ERROR);
        goto err;
    }

    line_begin = line_end + 2;
    if((line_end = findStrInFile(filename, line_begin, "\r\n", 2)) == -1){
        printf("%s", RFC_ERROR);
        goto err;
    }

    file_begin = line_end + 2;

    if( (file_end = findStrInFile(filename, file_begin, boundary, boundary_len)) == -1){
        printf("%s", RFC_ERROR);
        goto err;
    }
    file_end -= 2;		// back 2 chars.(\r\n);

    if (!upload(filename, name, file_begin, file_end - file_begin))
    {
    	printf("Fail to copy into /etc\n");
    	printf("</body></html>\n");
        goto err;
    }

    printf("<script language=\"JavaScript\" type=\"text/javascript\">\n");
    printf("window.opener.location.reload();\n window.close();\n");
    printf("</script>\n");
    printf("</body>\n</html>\n");
    
err:
    free(boundary);
    exit(0);
}

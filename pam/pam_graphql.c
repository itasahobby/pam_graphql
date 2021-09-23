#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <curl/curl.h>

#define URL_TAG_STRING "url=" 
#define METHOD_TAG_STRING "method="
#define QUERY_TAG_STRING "query="
#define ERROR_RESPONSE_STRING "error"


typedef struct {
   char  url[256];
   char  method[256];
   char  query[1000];
} Args_t;

Args_t* parse_args(int argc, const char **argv)
{
	Args_t *args = NULL;
	int i;

	/* Allocating args with 0s */
	args = calloc( 1, sizeof( *args ) );
	if ( NULL == args ){
		return NULL;
	}

	/* Parsing parameters */
	for( i = 0 ; i < argc ; i++ ) {
		if( strncmp( argv[i], URL_TAG_STRING, sizeof(URL_TAG_STRING) - 1) == 0 ) {
			strncpy( args->url, argv[i] + ( sizeof(URL_TAG_STRING) - 1 ), sizeof(args->url) - 1 );
		} else if( strncmp(argv[i], METHOD_TAG_STRING, sizeof(METHOD_TAG_STRING) -1 ) == 0 ) {
			strncpy( args->method, argv[i] + ( sizeof(METHOD_TAG_STRING) -1 ), sizeof(args->method) - 1 );		
		} else if( strncmp(argv[i], QUERY_TAG_STRING, sizeof(QUERY_TAG_STRING) -1) == 0 ) {
			strncpy( args->query, argv[i] + ( sizeof(QUERY_TAG_STRING) -1 ), sizeof(args->query) -1 );
		}
	}

	/* If any empty parameter return NULL */
	if ('\0' == args->url[0] || '\0'== args->method[0] || '\0' == args->query[0]){
		return NULL;
	}

	return args;
}

typedef struct {
   char *response;
   size_t size;
} Memory_t;

size_t write_callback(void *data, size_t size, size_t nmemb, void *userp)
{
	size_t realsize = size * nmemb;
	Memory_t *mem = (Memory_t *)userp;
 
	char *ptr = realloc(mem->response, mem->size + realsize + 1);
	
	/* Out of memory */
	if(ptr == NULL)
		return 0;
 
	mem->response = ptr;
	memcpy(&(mem->response[mem->size]), data, realsize);
	mem->size += realsize;
	mem->response[mem->size] = 0;
 
	return realsize;
}


bool validate_response(char *response)
{
	/* Check if response contains any error (hence invalid credentials) */
	return NULL == strstr(response, ERROR_RESPONSE_STRING);
}

bool curl_req(Args_t* args)
{
	CURL *curl;
	CURLcode res;
	Memory_t chunk = { 0 };
	struct curl_slist *headers = NULL;
	
	curl = curl_easy_init();
	if(curl) {
		/* Setting curl options*/
		curl_easy_setopt(curl, CURLOPT_URL, args->url);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
		if( strncmp(args->method, "POST", sizeof(METHOD_TAG_STRING) -1 ) == 0 ){
			headers = curl_slist_append(headers, "Accept: application/json");
			headers = curl_slist_append(headers, "Content-Type: application/json");
			headers = curl_slist_append(headers, "charset: utf-8");
			curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "POST");
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args->query);
		}

		/* Make request */
		res = curl_easy_perform(curl);
		
		/* Always cleanup */
		curl_easy_cleanup(curl);

		/* Check for errros */
		if(res != CURLE_OK){
			return false;
		}

		return validate_response(chunk.response);
	}

	return false;

}

char* string_replace(char* source, size_t sourceSize, char* substring, const char *with)
{
	char *substring_source = strstr(source, substring);
	/* There is no such substring */
	if (NULL == substring_source) {
		return NULL;
	}
	/* Buffer is too small */
	if (sourceSize < strlen(source) + (strlen(with) - strlen(substring)) + 1) {
		return NULL;
	}

	memmove(
        substring_source + strlen(with),
        substring_source + strlen(substring),
        strlen(substring_source) - strlen(substring) + 1
    );

	memcpy(substring_source, with, strlen(with));
    return substring_source + strlen(with);
}


PAM_EXTERN int pam_sm_authenticate(pam_handle_t *handle, int flags, int argc, const char **argv)
{
	int pam_code;
	const char *username = NULL;
	const char *password = NULL;
	Args_t* args;


	/* Getting the username */
	pam_code = pam_get_user(handle, &username, "USERNAME: ");
	if (pam_code != PAM_SUCCESS) {
		return PAM_AUTH_ERR;
	}

	/* Getting the password */
	pam_code = pam_get_authtok(handle, PAM_AUTHTOK, &password, "Password: ");
	if (pam_code != PAM_SUCCESS) {
		return PAM_AUTH_ERR;
	}

	if ( NULL == (args = parse_args(argc, argv)) ){
		return PAM_AUTH_ERR;
	}

	string_replace(args->query,  sizeof(args->query), "%PLACEHOLDER1%", username);
	string_replace(args->query,  sizeof(args->query), "%PLACEHOLDER2%", password);

	if ( !curl_req(args) ) {
		memset(args, 0, sizeof(*args));
		free(args);
		return PAM_AUTH_ERR;
	}
	
	printf("");

	memset(args, 0, sizeof(*args));
	free(args);
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char *argv[]){
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh,int flags,int argc, const char *argv[]){
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh,int	flags, int argc, const char *argv[]){
	return PAM_SUCCESS;
}

PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh,int flags,int argc,const char *argv[]){
	return PAM_SERVICE_ERR;
}

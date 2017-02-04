/*
 * curl_util.h
 *
 *  Created on: Sep 4, 2014
 *      Author: cny
 */

#ifndef CURL_UTIL_H_
#define CURL_UTIL_H_
#include <curl/curl.h>
#include <string.h>
#include <stdlib.h>
#ifdef __cplusplus
extern "C" {
#endif
#if WIN32
#define CURL_UTIL_EXP __declspec(dllexport)
#else
#define CURL_UTIL_EXP
#endif
/* using default max response data length, when Content-Length not found */
#define MAX_RESPONSE_LEN 102400
#define MAX_HEAD_LEN 100
/* the CURL response head */
typedef struct CURLresh_ CURLresh;
struct CURLresh_ {
	char key[MAX_HEAD_LEN], val[MAX_HEAD_LEN];
	CURLresh *pre, *next;
};
/* the CURL HTTP result */
typedef struct CURLres_ CURLres;
struct CURLres_ {
	CURLcode ccode; //curl code
	int rcode; //HTTP response code.
	char ctype[MAX_HEAD_LEN]; //content type.
	int clen; //content length.
	char* rdata; //response data.
	CURLresh *h_start; //the response header link table start node.
	CURLresh *h_end; //the response header link table end node.
};
/* free the response CURLres */
CURL_UTIL_EXP void curl_free_res(CURLres** res);
/* get the head value by name */
CURL_UTIL_EXP CURLresh* curl_res_head(CURLres* res, const char* key);
/* print all head */
CURL_UTIL_EXP CURLresh* curl_sres_head(CURLres* res);
/* send GET HTTP request */
CURL_UTIL_EXP CURLres* curl_get(const char* url, char** header, int hlen);
CURL_UTIL_EXP CURLres* curl_get_h(const char* url, struct curl_slist *header);
CURL_UTIL_EXP CURLres* curl_get_h_v(const char* url, struct curl_slist *header,
		int ignore_ssl_verify);
/* send POST HTTP request */
//CURLres* curl_post(const char* url);
//CURLres* curl_post(const char* url, const char* args);
CURL_UTIL_EXP CURLres* curl_post(const char* url, const char* args,
		char** header, int hlen);
CURL_UTIL_EXP CURLres* curl_post_h(const char* url, const char* args,
		struct curl_slist *header);
CURL_UTIL_EXP CURLres* curl_post_h_v(const char* url, const char* args,
		struct curl_slist *header, int ignore_ssl_verify);
#ifdef __cplusplus
}
#endif
#endif /* CURL_UTIL_H_ */

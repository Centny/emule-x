/*
 * curl_util.c
 *
 *  Created on: Sep 4, 2014
 *      Author: cny
 */
#include "curl_util.h"
size_t buf_wdata(void *cbuf, size_t size, size_t nmemb, void *udata) {
	CURLres* res;
	//
	res = (CURLres*) udata;
	if (!res->rdata) {
		res->rdata = (char*) calloc(MAX_RESPONSE_LEN + 1, sizeof(char));
	}
	strcat(res->rdata, cbuf);
	return size * nmemb;
}
size_t buf_whead(void *cbuf, size_t size, size_t nmemb, void *udata) {
	CURLres* res;
	CURLresh* resh;
	res = (CURLres*) udata;
	resh = (CURLresh*) calloc(1, sizeof(CURLresh));
//	puts(cbuf);
	if (res->rcode) {
		sscanf(cbuf, "%[^: \r\n]:%[^:\r\n]", resh->key, resh->val);
		if (strlen(resh->key)) {
			resh->pre = res->h_end;
			res->h_end->next = resh;
			res->h_end = resh;
			if (strcmp("Content-Type", resh->key) == 0) {
				strcpy(res->ctype, resh->val);
			} else if (strcmp("Content-Length", resh->key) == 0) {
				res->clen = atoi(resh->val);
				if (res->clen) {
					res->rdata = (char*) calloc(res->clen + 1, sizeof(char));
				} else {
					res->rdata = (char*) calloc(MAX_RESPONSE_LEN + 1,
							sizeof(char));
				}
			}
		} else {
			free(resh);
			resh = NULL;
		}
	} else {
		char p[30], c[30], r[30];
		sscanf(cbuf, "%[^ ] %[^ ] %[^ \n\r]", p, c, r);
		res->rcode = atoi(c);
		if (res->rcode == 0) {
			res->rcode = -1;
		}
		sscanf(cbuf, "%[^\n\r]", resh->key);
		res->h_start = resh;
		res->h_end = resh;
	}
	return size * nmemb;
}

void curl_free_res(CURLres** res) {
	CURLresh *h, *t;
	//
	if (!res || !*res) {
		return;
	}
	if ((*res)->rdata) {
		free((*res)->rdata);
	}
	if ((*res)->h_start) {
		h = (*res)->h_start, t = NULL;
		do {
			t = h->next;
			free(h);
			h = t;
		} while (h);
		(*res)->h_start = NULL;
	}
	free(*res);
	*res = NULL;
}
CURLresh* curl_res_head(CURLres* res, const char* key) {
	CURLresh *h;
	//
	h = res->h_start;
	while (h) {
		if (strcmp(key, h->key) == 0) {
			return h;
		}
		h = h->next;
	}
	return NULL;
}
CURLresh* curl_sres_head(CURLres* res) {
	CURLresh *h;
	//
	h = res->h_start;
	while (h) {
		printf("H-->%s:%s\n", h->key, h->val);
		h = h->next;
	}
	return NULL;
}
struct curl_slist* curl_to_slist(char** header, int hlen) {
	struct curl_slist *hs = NULL;
	int i = 0;
	for (i = 0; i < hlen; i++) {
		curl_slist_append(hs, header[i]);
	}
	return hs;
}
CURLres* curl_get(const char* url, char** header, int hlen) {
	struct curl_slist *hs;
	CURLres* res;
	//
	hs = curl_to_slist(header, hlen);
	res = curl_get_h(url, hs);
	if (hs) {
		curl_slist_free_all(hs);
	}
	return res;
}

CURLres* curl_get_h(const char* url, struct curl_slist *header) {
	return curl_get_h_v(url, header, 0);
}

CURLres* curl_get_h_v(const char* url, struct curl_slist *header,
		int ignore_ssl_verify) {
	CURLres *res;
	CURL *curl;
	//CURL *curl;
	res = (CURLres*) calloc(1, sizeof(CURLres));
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	if (header) {
		curl_easy_setopt(curl, CURLOPT_PREQUOTE, header);
	}
	if (ignore_ssl_verify) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buf_wdata);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, buf_whead);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, res);
	res->ccode = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

CURLres* curl_post(const char* url, const char* args, char** header, int hlen) {
	struct curl_slist *hs;
	CURLres* res;
	//
	hs = curl_to_slist(header, hlen);
	res = curl_post_h(url, args, hs);
	if (hs) {
		curl_slist_free_all(hs);
	}
	return res;
}
CURLres* curl_post_h(const char* url, const char* args,
		struct curl_slist *header) {
	return curl_post_h_v(url, args, header, 0);
}

CURLres* curl_post_h_v(const char* url, const char* args,
		struct curl_slist *header, int ignore_ssl_verify) {
	CURLres *res;
	CURL *curl;
	res = (CURLres*) calloc(1, sizeof(CURLres));
	curl = curl_easy_init();
	curl_easy_setopt(curl, CURLOPT_URL, url);
	if (args && strlen(args)) {
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, args);
	}
	if (header) {
		curl_easy_setopt(curl, CURLOPT_PREQUOTE, header);
	}
	if (ignore_ssl_verify) {
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
	}
	curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
	curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1);
	//curl_easy_setopt(curl, CURLOPT_HEADER, 1);
	curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, buf_wdata);
	curl_easy_setopt(curl, CURLOPT_WRITEDATA, res);
	curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, buf_whead);
	curl_easy_setopt(curl, CURLOPT_HEADERDATA, res);
	res->ccode = curl_easy_perform(curl);
	curl_easy_cleanup(curl);
	return res;
}

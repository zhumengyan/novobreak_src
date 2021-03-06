/*
 * 
 * Copyright (c) 2011, Jue Ruan <ruanjue@gmail.com>
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
 
#ifndef __STRING_RJ_H
#define __STRING_RJ_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include "list.h"
#include "mem_share.h"

/**
 * String
 */

#ifndef SWAP_TMP
#define SWAP_TMP
#define swap_tmp(a, b, t) { t = a; a = b; b = t; }
#endif


#ifdef HUGE_STRING
typedef long int string_size_t;
#else
typedef int string_size_t;
#endif

typedef struct {
	char *string;
	string_size_t size;
	string_size_t capacity;
} String;

typedef struct {
	char *string;
	string_size_t  size;
} VString;
define_list(VStrv, VString);

#define uc(ch) (((ch) >= 'a' && (ch) <= 'z')? (ch) + 'A' - 'a' : (ch))
#define lc(ch) (((ch) >= 'A' && (ch) <= 'Z')? (ch) + 'a' - 'A' : (ch))

static inline String* init_string(string_size_t cap){
	String *str;
	str = (String*)malloc(sizeof(String));
	str->size = 0;
	str->capacity = (cap&0x1)? cap : cap + 1;
	str->string = (char*)malloc(sizeof(char) * (str->capacity + 1));
	str->string[0] = 0;
	return str;
}

static inline size_t string_obj_desc_cnt(void *obj, int idx){
	return ((String*)obj)->size + 1;
	int idx2 = 0;
	idx2 = idx;
}

static const obj_desc_t string_obj_desc = {sizeof(String), 1, {offsetof(String, string)}, {(obj_desc_t*)&OBJ_DESC_ARRAY}, string_obj_desc_cnt};

static inline unsigned int _string_size_roundup_power2(unsigned int v){
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	return v + 1;
}

static inline void encap_string(String *str, string_size_t inc){
	if(inc + str->size > str->capacity){
		if(inc + str->size < 0xFFFFFFF){
			str->capacity = _string_size_roundup_power2(inc + str->size) + 1;
		} else {
			str->capacity += (((inc + str->size) - str->capacity + 0xFFFFFFF - 1) / 0xFFFFFFF) * 0xFFFFFFF + 1;
		}
		str->string = (char*)realloc(str->string, str->capacity + 1);
	}
}

static inline void uc_string(String *str){
	string_size_t i;
	for(i=0;i<str->size;i++){
		if(str->string[i] >= 'a' && str->string[i] <= 'z') str->string[i] = str->string[i] + 'A' - 'a';
	}
}

static inline void lc_string(String *str){
	string_size_t i;
	for(i=0;i<str->size;i++){
		if(str->string[i] >= 'A' && str->string[i] <= 'Z') str->string[i] = str->string[i] + 'a' - 'A';
	}
}

static inline char* substr(char *string, string_size_t start, string_size_t end, char *dst){
	string_size_t i, size;
	char *str;
	size = strlen(string);
	if(start > size) start = size;
	else if(start < 0) start = 0;
	if(end > size) end = size;
	else if(end < 0) end = 0;
	size = end - start;
	if(size < 0) size = 0;
	if(dst != NULL) str = dst;
	else str = (char*)malloc(sizeof(char) * (size + 1));
	for(i=start;i<end;i++){
		str[i-start] = string[i];
	}
	str[size] = '\0';
	return str;
}

static inline char* catstr(string_size_t n_str, ...){
	char *str, *s;
	string_size_t i, len;
	va_list params;
	
	len = 0;
	str = NULL;
	va_start(params, n_str);
	for(i=0;i<n_str;i++){
		s = va_arg(params, char*);
		len += strlen(s);
		str = realloc(str, len + 1);
		if(i == 0) str[0] = 0;
		strcat(str, s);
	}
	va_end(params);
	return str;
}

static inline void chomp_string(String *str){
	if(str->size && str->string[str->size - 1] == '\n'){
		str->size --;
		str->string[str->size] = 0;
	}
}

static inline void chomp_vstring(VString *str){
	if(str->size && str->string[str->size - 1] == '\n'){
		str->size --;
	}
}

static inline void trim_string(String *str){
	string_size_t i, j;
	i = str->size - 1;
	while(i >= 0 && (str->string[i] == '\n' || str->string[i] == '\t' || str->string[i] == ' ')) i--; 
	str->size = i + 1;
	i = 0;
	while(i < str->size && (str->string[i] == '\n' || str->string[i] == '\t' || str->string[i] == ' ')) i++;
	if(i){
		for(j=i;j<str->size;j++){ str->string[j-i] = str->string[j]; }
		str->size -= i;
	}
	str->string[str->size] = 0;
}

static inline void trim_vstring(VString *str){
	string_size_t i;
	i = str->size - 1;
	while(i >= 0 && (str->string[i] == '\n' || str->string[i] == '\t' || str->string[i] == ' ')) i--; 
	str->size = i + 1;
	i = 0;
	while(i < str->size && (str->string[i] == '\n' || str->string[i] == '\t' || str->string[i] == ' ')) i++;
	str->string += i;
}

static inline void append_string(String *str, char *src, string_size_t offlen){
	encap_string(str, offlen);
	memcpy(str->string + str->size, src, offlen);
	str->size += offlen;
	str->string[str->size] = 0;
}

static inline void append_char_string(String *str, char c, string_size_t num){
	encap_string(str, num);
	while(num-- > 0){ str->string[str->size ++] = c; }
	str->string[str->size] = 0;
}

static inline String* as_string(char *chs){
	string_size_t len;
	String *str;
	len = strlen(chs);
	str = init_string(len);
	append_string(str, chs, len);
	return str;
}

static inline VString* as_vstring(char *chs){
	string_size_t len;
	VString *str;
	len = strlen(chs);
	str = malloc(sizeof(VString));
	str->string = chs;
	str->size = len;
	return str;
}

static inline void add_char_string(String *str, char ch){
	encap_string(str, 1);
	str->string[str->size++] = ch;
	str->string[str->size] = 0;
}

static inline void clear_string(String *str){ str->size = 0; str->string[0] = 0; }

static inline string_size_t split_string(String *str, char separator, VStrv *vstrs){
	VString *vstr;
	string_size_t n_tab, i, s;
	for(i=s=n_tab=0;i<str->size;i++){
		if(str->string[i] == separator){
			if(i > s){
				str->string[i] = '\0';
				vstr = next_ref_VStrv(vstrs);
				vstr->string = str->string + s;
				n_tab ++;
				vstr->size = i - s;
			}
			s = i + 1;
		}
	}
	if(i > s){
		str->string[i] = '\0';
		vstr = next_ref_VStrv(vstrs);
		vstr->string = str->string + s;
		n_tab ++;
		vstr->size = i - s;
	}
	return n_tab;
}

static inline string_size_t split_vstring(VString *str, char separator, VStrv *vstrs, string_size_t cut){
	VString *vstr;
	string_size_t n_tab, i, s;
	for(i=s=n_tab=0;i<str->size;i++){
		if(str->string[i] == separator){
			if(i > s){
				if(cut) str->string[i] = '\0';
				vstr = next_ref_VStrv(vstrs);
				vstr->string = str->string + s;
				n_tab ++;
				vstr->size = i - s;
			}
			s = i + 1;
		}
	}
	if(i > s){
		if(cut) str->string[i] = '\0';
		vstr = next_ref_VStrv(vstrs);
		vstr->string = str->string + s;
		n_tab ++;
		vstr->size = i - s;
	}
	return n_tab;
}

static inline void reverse_string(String *str){
	string_size_t i, j;
	char c;
	i = 0;
	j = str->size - 1;
	while(i < j){
		swap_tmp(str->string[i], str->string[j], c);
		i ++;
		j --;
	}
}

static inline void reverse_str(char *str, string_size_t len){
	string_size_t i, j;
	char c;
	i = 0;
	j = len - 1;
	while(i < j){
		swap_tmp(str[i], str[j], c);
		i ++;
		j --;
	}
}

static inline void tidy_string(String *src, String *dst, char ch){
	string_size_t i;
	encap_string(dst, src->size);
	for(i=0;i<src->size;i++){
		if(src->string[i] != ch){
			dst->string[dst->size ++] = src->string[i];
		}
	}
	dst->string[dst->size] = 0;
}

static inline string_size_t occ_str(char *str, string_size_t len, char c){
	string_size_t i, ret;
	for(i=ret=0;i<len;i++){
		if(str[i] == c) ret ++;
	}
	return ret;
}

static inline void trunc_string(String *str, string_size_t size){
	if(size >= str->size || size < 0) return;
	str->size = size;
	str->string[size] = 0;
}

static inline String* clone_string(String *str){
	String *clone;
	clone = init_string(str->size);
	append_string(clone, str->string, str->size);
	return clone;
}

static inline void free_string(String *str){ free(str->string); free(str); }

static inline void free_vstring(VString *str){ free(str); }

#endif

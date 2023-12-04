#ifndef __COMMON_H__
#define __COMMON_H__

#include <string>

std::string get_file_name_by_url(const std::string &url);
bool calc_file_md5_sha1(const char* filename, char* md5, char* sha1, long* file_size = NULL);
char char_to_upper(const char ch);
void string_to_upper(std::string& s);
std::string trim_string(const std::string& s, char ch = ' ');
#endif


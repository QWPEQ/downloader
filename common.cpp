#include "common.h"
#include "md5.h"
#include "sha1.h"
#include <stdlib.h>
#include <string.h>
#include <algorithm>

// 提取url中的文件名
std::string get_file_name_by_url(const std::string &url)
{
    // 如果存在参数查找参数起始位置
    size_t file_name_pos_end = url.find_first_of('?') != std::string::npos ? url.find_first_of('?') : url.length();
    std::string file_name = "./" + url.substr(url.find_last_of('/') + 1, file_name_pos_end - url.find_last_of('/') - 1);
    return file_name;
}

bool calc_file_md5_sha1(const char* filename, char* md5, char* sha1, long* file_size)
{
    if ((filename == NULL) || (md5 == NULL) || (sha1 == NULL)) {
        return false;
    }

    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        return false;
    }

    md5_ctx_t md5_ctx;
    crypto_md5_init(&md5_ctx);

    sha1_ctx_t sha1_ctx;
    crypto_sha1_init(&sha1_ctx);

    size_t len = 0;
    char buff[2048] = {0};
    while((len = fread(buff, 1, sizeof(buff), fp)) > 0) {
        crypto_md5_update(&md5_ctx, (unsigned char*)buff, len);
        crypto_sha1_update(&sha1_ctx, (unsigned char*)buff, len);
    }
    if (file_size != NULL) {
        *file_size = ftell(fp);
    }

    fclose(fp);
    char tmp[3] = {0};
    unsigned char md5tmp[16] = {0};
    crypto_md5_final(&md5_ctx, md5tmp);
    for (size_t i = 0; i < sizeof(md5tmp); i++) {
        memset(tmp, 0, sizeof(tmp));
        snprintf(tmp, 3, "%02X", md5tmp[i]);
        strncat(md5, tmp, 2);
    }

    unsigned char sha1tmp[20] = {0};
    crypto_sha1_final(&sha1_ctx, sha1tmp);
    for(size_t i = 0; i < sizeof(sha1tmp); i++) {
        memset(tmp, 0, sizeof(tmp));
        snprintf(tmp, 3, "%02X", sha1tmp[i]);
        strncat(sha1, tmp, 2);
    }

    return true;
}

char char_to_upper(const char ch)
{
    // a - z
    if( ch >= 97 && ch <= 122 )
    {
        return ch - 32;
    }
    return ch;
}

void string_to_upper(std::string& str)
{
    std::transform(str.begin(), str.end(), str.begin(), char_to_upper);
}

std::string trim_string(const std::string& str, char ch)
{
    size_t start = 0;
    size_t end = str.size() - 1;
    printf("length:%ld, start:%ld, end:%ld\n", str.size(), start, end);
    // 遍历字符串找到第一个非空格字符的索引
    while (start < str.size() && str[start] == ch) {
        start++;
    }
 
    // 遍历字符串找到最后一个非空格字符的索引
    while (end >= 0 && str[end] == ch) {
        end--;
    }

    printf("length:%ld, start:%ld, end:%ld\n", str.size(), start, end);
    // 如果字符串全为空格，则返回空字符串
    if (start > end) {
        return "";
    }
 
    // 返回去掉前后空格的子串
    return str.substr(start, end - start + 1);
}
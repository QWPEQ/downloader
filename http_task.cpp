
#include "http_task.h"
#include <curl/curl.h>
#include "http_task_mgr.h"
#include "common.h"

#define MAX_DOWNLOAD_RANGE_LENGTH 128
size_t write_data(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    HttpTask *task = (HttpTask *)userdata;
    if (task == NULL) {
        return 0;
    }
    size_t write_length;
    return task->write_file(ptr, size, nmemb);
}

HttpTask::HttpTask():m_offset(0)
{
}

HttpTask::~HttpTask()
{
}

void HttpTask::run()
{
    CURL *curl;
    CURLcode res;
    bool ret;
    curl = curl_easy_init();
    std::string file_name = get_file_name_by_url(m_url);
    char range[MAX_DOWNLOAD_RANGE_LENGTH];
    if (m_segment_no != 0) {
        file_name += ".filepart" + std::to_string(m_segment_no);
        snprintf(range, sizeof(range), "%lld-%lld", m_start + m_offset, m_end);
        curl_easy_setopt(curl, CURLOPT_RANGE, range);
    }
    // printf("run task filename:%s range:%s\n", file_name.c_str(), range);
    m_fp = fopen(file_name.c_str(), "a");
    if (m_fp == NULL) {
        printf("open file error\n");
        return;
    }

    curl_easy_setopt(curl, CURLOPT_URL, m_url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)this);
    curl_easy_setopt(curl, CURLOPT_FORBID_REUSE, 1L);
    res = curl_easy_perform(curl);
    if (CURLE_OK != res) {
        printf("curl error:%d %s location:%d segment url:%s\n", res, curl_easy_strerror(res), m_segment_no, m_url.c_str());
    }

    curl_easy_cleanup(curl);
    fclose(m_fp);
    m_task_mgr->add_finish_task_num();
}

size_t HttpTask::write_file(void *ptr, size_t size, size_t nmemb)
{
    if (m_fp == NULL) {
        return 0;
    }

    size_t write_length = 0;
    if (m_segment_no != 0) {
        fseek(m_fp, m_offset, SEEK_SET);
        write_length = fwrite(ptr, size, nmemb, m_fp);
        m_offset += size * nmemb;
    } else {
        printf("write file %ld\n", size * nmemb);
        write_length = fwrite(ptr, size, nmemb, m_fp);
    }

    if (m_task_mgr) {
        m_task_mgr->add_download_size(write_length);
    }
    return write_length;
}


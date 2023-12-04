#include "http_task_mgr.h"
#include <curl/curl.h>
#include "http_task.h"
#include <unistd.h>
#include "common.h"
#include "task_process.h"
#include <string.h>
#include <stdlib.h>
#include <algorithm>


static int MALLOC_MAX_CACHE_SIZE = 4096;

size_t write_header(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    HttpTaskMgr *task_mgr = (HttpTaskMgr *)userdata;
    if (task_mgr == NULL) {
        return 0;
    }

    std::string header(ptr, size * nmemb);
    // printf("header: %s\n", header.c_str());
    if (header.find("HTTP/") != std::string::npos) {
        if (header.find("200") != std::string::npos) {
            // 请求成功
            task_mgr->set_status(true);
        }
    }

    if (header.find("Content-Length") != std::string::npos || header.find("content-length") != std::string::npos) {
        // 获取文件大小
        task_mgr->set_file_size(atoi(header.substr(header.find(":") + 1).c_str()));
    }

    if (header.find("Accept-Ranges") != std::string::npos || header.find("accept-ranges") != std::string::npos) {
        std::string range = header.substr(header.find(":") + 1);
        if (range.find("bytes") != std::string::npos) {
            task_mgr->set_support_segmentation(true);
        } else {
            task_mgr->set_support_segmentation(false);
        }
    }

    if (header.find("ETag") != std::string::npos || header.find("Etag") != std::string::npos) {
        task_mgr->set_md5(header.substr(header.find(":") + 1), "");
    }

    return size * nmemb;
}

bool HttpTaskMgr::init(const std::string &url, int thread_num)
{
    m_url = url;
    m_thread_num = thread_num;
    CURL *curl = curl_easy_init();
    if (!curl)
    {
        printf("Could not initialize curl\n");
        return false;
    }
    CURLcode res;
   
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, &write_header);
    curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)this);
    curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);

    res = curl_easy_perform(curl);
    if (res != CURLE_OK)
    {
        curl_easy_cleanup(curl);
        printf("Could not acess url:%s\n", url.c_str());
        return false;
    }
    curl_easy_cleanup(curl);
    // printf("url:%s\nfilesize:%lld\nis_support_segmentation:%s, md5:%s\n", m_url.c_str(), m_file_size, m_segmentation ? "true" : "false", m_md5.c_str());
    return true;
}

HttpTaskMgr::HttpTaskMgr() : m_thread_num(0), m_segmentation(false), m_file_size(0), m_request_succ(false), m_download_size(0), m_finish_task_num(0)
{
}

HttpTaskMgr::~HttpTaskMgr()
{
    for (auto& it : m_task_map) {
        if (it.second != NULL) {
            delete it.second;
            it.second = NULL;
        }
    }
}

void HttpTaskMgr::set_md5(const std::string& md5, const std::string& sha1)
{ 
    m_md5 = md5;
    m_md5.erase(std::remove(m_md5.begin(), m_md5.end(), '"'), m_md5.end());
    string_to_upper(m_md5);
    m_sha1 = sha1;
    string_to_upper(m_sha1);
}

bool HttpTaskMgr::split_task()
{
    // 请求失败直接返回
    if (!m_request_succ) {
        printf("HTTP request failed!\n");
        return false;
    }

    long long task_size = m_file_size / (long long)m_thread_num;
    long long start = 0;
    long long end = 0;
    std::string file_name = get_file_name_by_url(m_url);
    // 如果文件存在比对一下 md5 如果一致不下载
    if (access(file_name.c_str(), F_OK) == 0) {
        char md5[33] = {0};
        char sha1[41] = {0};
        bool succ = calc_file_md5_sha1(file_name.c_str(), md5, sha1);
        if (succ && (((m_md5.find(md5) != std::string::npos) || (m_sha1.find(sha1) != std::string::npos)))) {
            printf("the file has been download\n");
            return false;
        }
    }
    if (m_segmentation) { // 支持分段下载
        for (int i = 0; i < m_thread_num; i++) {
            start = i * task_size;
            end = (i + 1) * task_size - 1;
            if (i == m_thread_num - 1) {
                end = m_file_size - 1;
            }

            HttpTask *task = new HttpTask();
            task->set_url(m_url);
            task->set_end(end);
            task->set_segment_no(i+1);
            task->set_task_mgr(this);
            task->set_start(start);
            std::string file_name_tmp = file_name + ".filepart" + std::to_string(i+1);
            // 文件存在根据文件大小计算开始下载位置
            if (access(file_name_tmp.c_str(), F_OK) == 0) {
                FILE* file = fopen(file_name_tmp.c_str(), "rb");
                if (file != NULL) {
                    fseek(file, 0, SEEK_END);
                    long file_size = ftell(file);
                    fclose(file);
                    add_download_size(file_size);
                    if (file_size + start >= end) {
                        add_finish_task_num();
                        continue;
                    }
                    task->set_offset(file_size);
                }
            } else {
                task->set_offset(0);
            }

            m_task_map[i+1] = static_cast<ITask*>(task);
        }
    } else { // 不支持分段下载需要每次下载全部内容
        HttpTask *task = new HttpTask();
        task->set_url(m_url);
        task->set_start(0);
        task->set_end(m_file_size-1);
        task->set_segment_no(0);
        task->set_task_mgr(this);
        task->set_offset(0);
        m_task_map[0] = static_cast<ITask*>(task);;
    }

    return true;
}

void HttpTaskMgr::start_task()
{
    for (auto it = m_task_map.begin(); it != m_task_map.end(); it++) {
        ITask *task = it->second;
        if (task == NULL) {
            continue;
        }

        TaskProcess::get_instance()->run_task(task);
    }
}

bool HttpTaskMgr::merge_file_segmentation()
{
    std::string file_name = get_file_name_by_url(m_url);
    FILE* new_file = fopen(file_name.c_str(), "wb");
    if (new_file == NULL) {
        printf("open file error\n");
        return false;
    }
    
    char* buff = (char*)malloc(MALLOC_MAX_CACHE_SIZE);
    if (buff == NULL) {
        fclose(new_file);
        printf("malloc memory failed!\n");
        return false;
    }
    int pos = 0;
    int new_pos = 0;
    for (int i = 0; i < m_thread_num; i++) {
        std::string file_name_tmp = file_name + ".filepart" + std::to_string(i+1);
        FILE* file = fopen(file_name_tmp.c_str(), "rb");
        if (file != NULL) {
            fseek(file, 0, SEEK_END);
            long file_size = ftell(file);
            pos = 0;
            for (int j = 0; j < int(file_size / MALLOC_MAX_CACHE_SIZE + 1); j++) {
                int lenread = (pos + MALLOC_MAX_CACHE_SIZE) > file_size ? (file_size % MALLOC_MAX_CACHE_SIZE) : MALLOC_MAX_CACHE_SIZE;
                fseek(file, pos, SEEK_SET);
                fread(buff, 1, lenread, file);
                fseek(new_file, new_pos, SEEK_SET);
                fwrite(buff, 1, lenread, new_file);
                pos += MALLOC_MAX_CACHE_SIZE;
                new_pos += lenread;
            }

            fclose(file);
            remove(file_name_tmp.c_str());
        }
    }

    fclose(new_file);
    free(buff);
    printf("download file success\n");
    return true;
}

bool HttpTaskMgr::is_all_task_finish() 
{
    if (m_segmentation) {
        return m_finish_task_num == m_thread_num;
    }

    return m_finish_task_num == 1;
}

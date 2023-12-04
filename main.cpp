#include <stdio.h>
#include <curl/curl.h>
#include <string>
#include <map>
#include <unistd.h>
#include "http_task_mgr.h"

static int MAX_DOWNLOAD_THREAD = 8;
enum {
    INVALID_PROTOCOL = 0,
    PROTOCOL_HTTP = 1,
    PROTOCOL_HTTPS = 2,
    PROTOCOL_FTP = 3
};

// 协议类型
const std::map<std::string, int> g_protocol_type_map = {
    { "http", PROTOCOL_HTTP },
    { "https", PROTOCOL_HTTPS },
    { "ftp", PROTOCOL_FTP }
};

// 获取协议类型
int get_protocol_type(const std::string& url) {
    size_t pos = url.find_first_of("://");
    if (pos == std::string::npos) {
        printf("Invalid URL\n");
        return -1;
    }

    std::string protocol_type = url.substr(0, pos);
    auto iter = g_protocol_type_map.find(protocol_type);
    if (iter != g_protocol_type_map.end()) {
        return iter->second;
    }

    return INVALID_PROTOCOL;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <url>\n", argv[0]);
        return -1;
    }
    curl_global_init(CURL_GLOBAL_ALL);
    std::string url = argv[1];
    int protocol_type = get_protocol_type(url);    
    switch (protocol_type)
    {
    case PROTOCOL_HTTP:
    case PROTOCOL_HTTPS:
        {
            auto http_task_mgr = new HttpTaskMgr();
            if (!http_task_mgr->init(url, MAX_DOWNLOAD_THREAD)) {
                printf("init http task mgr failed\n");
                return -1;
            }
            if (!http_task_mgr->split_task()){
                return -1;
            }
            http_task_mgr->start_task();
            char buf[102] = { 0 };
            int last_index = 0;
            while(true) {
                usleep(1000);

                if (http_task_mgr->get_download_progress() < 0) {
                    break;
                }

                if (http_task_mgr->is_all_task_finish() && http_task_mgr->get_download_progress() < 100) {
                    printf("download failed! please retry!");
                    return -1;
                }

                while(last_index <= http_task_mgr->get_download_progress() && last_index <= 100) {
                    buf[last_index] = '=';
                    last_index++;
                }
                printf("download progress [%-101s][%d%%]\r", buf, last_index);
                fflush(stdout);
                if(http_task_mgr->get_download_progress() >= 100) {
                    while(last_index <= 100) {
                        buf[last_index] = '=';
                        last_index++;
                    }
                    printf("download progress [%-101s][%d%%]\r", buf, 100);
                    printf("\n");
                    break;
                }
            }

            http_task_mgr->merge_file_segmentation();
            if (http_task_mgr) {
                delete http_task_mgr;
                http_task_mgr = NULL;
            }
        }
        break;
    case PROTOCOL_FTP:
        break;
    default:
        printf("Invalid protocol type\n");
        return -1;
        break;
    }



    return 0;
}
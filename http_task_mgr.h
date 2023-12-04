#ifndef __HTTP_TASK_MGR_H__
#define __HTTP_TASK_MGR_H__

#include <string>
#include <map>
#include <atomic>
#include "itask.h"

class HttpTaskMgr {
public:
    HttpTaskMgr();
    ~HttpTaskMgr();
    HttpTaskMgr(const HttpTaskMgr&) = delete;
    HttpTaskMgr& operator=(const HttpTaskMgr&) = delete;
    
    /**
    * 初始化任务管理
    * @return 是否成功 
    */
    bool init(const std::string& url, int thread_num);
    /**
    * 设置文件大小
    */
    void set_file_size(int file_size) { m_file_size = file_size; }
     /**
    * 设置是否支持分片下载
    */
    void set_support_segmentation(bool segmentation) { m_segmentation = segmentation; }
     /**
    * 拆分任务
    * @return 是否成功 
    */
    bool split_task();
    /**
    * 开始任务
    * @return 是否成功 
    */
    void start_task();
    /**
    * 增加下载进度 
    */
    void add_download_size(int size) { m_download_size += size; }
    /**
    * 获取下载进度百分比
    * @return 下载进度百分比 
    */
    int get_download_progress() { return int(m_download_size.load() * 100.0 / (double)m_file_size); }
    /**
    * 合并分段文件
    * @return 是否成功 
    */
    bool merge_file_segmentation();
    /**
    * 设置请求 url header是否成功
    * @return 是否成功 
    */
    void set_status(bool succ) { m_request_succ = succ; }
    /**
    * 设置下载文件 md5
    */
    void set_md5(const std::string& md5, const std::string& sha1);
    /**
    * 增加任务完成数量
    */
    void add_finish_task_num() { m_finish_task_num++; }
    /**
    * 获取任务是否全部处理完成
    * @return 任务是否全部处理完成 
    */
    bool is_all_task_finish();
private:
    std::string m_url;
    std::string m_md5;
    std::string m_sha1;
    int m_thread_num;
    bool m_segmentation;
    long long m_file_size;
    std::atomic_int m_download_size;
    std::atomic_int m_finish_task_num;
    bool m_request_succ;
    std::map<int, ITask*> m_task_map;
};

#endif

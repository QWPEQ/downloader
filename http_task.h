#ifndef __HTTP_TASK_H__
#define __HTTP_TASK_H__

#include "task.h"
#include <string>
class HttpTaskMgr;
class HttpTask : public Task {
public:
    HttpTask();
    virtual ~HttpTask();

    /**
    * 运行任务
    */
    virtual void run();
    /**
    * 写文件
    * @return 写入的数据大小 
    */
    size_t write_file(void* ptr, size_t size, size_t nmemb);
    /**
    * 设置任务管理指针
    */
    void set_task_mgr(HttpTaskMgr* task_mgr) { m_task_mgr = task_mgr; }
    /**
    * 设置文件位置
    */
    void set_offset(int offset) { m_offset = offset; }

private:
    int m_offset;
    HttpTaskMgr* m_task_mgr;
};

#endif

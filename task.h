#ifndef __TASK_H__
#define __TASK_H__
#include "itask.h"

class ITask;
class Task : public ITask {
public:
    Task() {}
    virtual ~Task() {}

    /**
    * 运行任务
    */
    virtual void run() {}
    /**
    * 设置下载地址
    */
    virtual void set_url(std::string url) { m_url = url; }
    /**
    * 设置任务下载起点
    */
    virtual void set_start(long long start) { m_start = start; }
    /**
    * 设置任务下载结束点
    */
    virtual void set_end(long long end) { m_end = end; }
    /**
    * 设置分段序号
    */
    virtual void set_segment_no(int no) { m_segment_no = no; }

protected:
    std::string m_url;
    long long m_start;
    long long m_end;
    int m_segment_no;
    FILE* m_fp;
};

#endif

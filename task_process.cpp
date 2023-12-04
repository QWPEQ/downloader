#include "task_process.h"
#include <thread>

void thread_func(ITask* task)
{
    if (task) {
        task->run();
    }
}

TaskProcess* TaskProcess::instance = NULL;

void TaskProcess::run_task(ITask* task)
{
    std::thread t(thread_func, task);
    t.detach();
}
#ifndef __TASK_PROCESS_H__
#define __TASK_PROCESS_H__

#include "itask.h"

class ITask;
class TaskProcess {
public:
    TaskProcess() = default;
    ~TaskProcess() {
        delete instance;
    };

    TaskProcess(const TaskProcess&) = delete;
    TaskProcess& operator=(const TaskProcess&) = delete;

    static TaskProcess* get_instance() { 
        if (!instance) {
            instance = new TaskProcess();
        }
        return instance;
     }

    void run_task(ITask* task);
private:
    static TaskProcess* instance;
};

#endif

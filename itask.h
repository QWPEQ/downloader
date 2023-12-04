#ifndef __ITASK_H__
#define __ITASK_H__

#include <string>

class ITask {
public:
    ITask() {}
    virtual ~ITask() {}
    virtual void run() = 0;
};

#endif

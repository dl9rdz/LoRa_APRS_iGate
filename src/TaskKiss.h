#ifndef TASK_KISS_H_
#define TASK_KISS_H_

#include <APRSMessage.h>
#include <TaskManager.h>
#include <Timer.h>

class KissTask : public Task {
public:
  explicit KissTask(TaskQueue<std::shared_ptr<APRSMessage>> &toKiss);
  virtual ~KissTask();

  virtual bool setup(System &system) override;
  virtual bool loop(System &system) override;

private:
  TaskQueue<std::shared_ptr<APRSMessage>> &_toKiss;
  //bool connect(const System &system);
};

#endif

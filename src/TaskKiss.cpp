#include <logger.h>

#include "Task.h"
#include "TaskKiss.h"
#include "project_configuration.h"
#include "aprs.h"

KissTask::KissTask(TaskQueue<std::shared_ptr<APRSMessage>> &toKiss) : Task(TASK_KISS, TaskKiss), _toKiss(toKiss) {
}

KissTask::~KissTask() {
}

bool KissTask::setup(System &system) {
  // notthing to do....
  //_aprs_is.setup(system.getUserConfig()->callsign, system.getUserConfig()->aprs_is.passcode, "ESP32-APRS-IS", "0.2");
  return true;
}

bool KissTask::loop(System &system) {
  //aprs_is.getAPRSMessage();

  if (!_toKiss.empty()) {
    std::shared_ptr<APRSMessage> msg = _toKiss.getElement();
    logPrintI("Sending Message: ");
    String c = msg->encode(); //toString();
    char buf[256];
    int len = aprsstr_mon2kiss(c.c_str(), buf, 255);
    buf[len] = 0; 
    Serial.write(buf, len);
    //_aprs_is.sendMessage(msg);
  }

  return true;
}

#if 0
bool KissTask::connect(const System &system) {
  logPrintI("connecting to APRS-IS server: ");
  logPrintI(system.getUserConfig()->aprs_is.server);
  logPrintI(" on port: ");
  logPrintlnI(String(system.getUserConfig()->aprs_is.port));
  if (!_aprs_is.connect(system.getUserConfig()->aprs_is.server, system.getUserConfig()->aprs_is.port)) {
    logPrintlnE("Connection failed.");
    return false;
  }
  logPrintlnI("Connected to APRS-IS server!");
  return true;
}
#endif

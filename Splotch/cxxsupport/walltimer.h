#ifndef PLANCK_WALLTIMER_H
#define PLANCK_WALLTIMER_H

#include <string>
#include <map>

class wallTimer
  {
  private:
    double t_acc, t_started;

  public:
    wallTimer() { t_acc=t_started=0; }
    void start();
    void stop();
    void reset() { t_acc=0.; }
    double acc() const { return t_acc; }
  };

class wallTimerSet
  {
  private:
    std::map<std::string,wallTimer> timers;

  public:
    void start(const std::string &name);
    void stop(const std::string &name);
    void reset(const std::string &name);
    double acc(const std::string &name);

    void report() const;
  };

double elapsedWallTime();

extern wallTimerSet wallTimers;

#endif

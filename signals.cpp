#include <iostream>
#include <signal.h>
#include "signals.h"
#include "Commands.h"

using namespace std;

void ctrlZHandler(int sig_num) {
  cout << "smash: got ctrl-Z" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if(smash.getCurrentJob() != nullptr){
  
  pid_t pid = smash.getCurrentJob()->getPid();
  kill(pid,SIGTSTP);
  if( smash.getJobsList()->getJobByPid(pid) == NULL){
    smash.getJobsList()->addJob(smash.getCurrentJob(),true);
  }
  else{
    smash.getJobsList()->getJobByPid(pid)->Stopped();
    smash.getJobsList()->getJobByPid(pid)->updateTime();
  }
  cout << "smash: process " << smash.getCurrentJob()->getPid() << " was stopped" << endl;
  smash.updateCurrentJob(nullptr);
  return ;
  }
}  

void ctrlCHandler(int sig_num) {
  cout << "smash: got ctrl-C" << endl;
  SmallShell& smash = SmallShell::getInstance();
  if(smash.getCurrentJob() != nullptr){
  pid_t pid = smash.getCurrentJob()->getPid();
  kill(pid,SIGKILL);
  if( smash.getJobsList()->getJobByPid(pid) != NULL)
    smash.getJobsList()->removeJobByPid(pid);
  cout << "smash: process " << smash.getCurrentJob()->getPid() << " was killed" << endl;
  smash.updateCurrentJob(nullptr);
  }
}

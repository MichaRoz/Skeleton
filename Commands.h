#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define HISTORY_MAX_RECORDS (50)


using namespace std;

int _parseCommandLine(const char* cmd_line, char** args) ;

class CommandsHistory {
 protected:
 class CommandHistoryEntry {
   int timestamp;
   char** argList;
   string cmdLine;
   int pos;
   public:
   CommandHistoryEntry(){
   }
   ~CommandHistoryEntry(){
     cmdLine.clear();
   }
   
   CommandHistoryEntry(int timeS, char** arg_List, string cmd_Line, int posLast ) : timestamp(timeS){
    argList = arg_List;
    cmdLine = cmd_Line;
    pos = posLast;
   }
   int getPos(){
     return pos;
   }
   CommandHistoryEntry(const CommandHistoryEntry &toCopy){
    timestamp = toCopy.timestamp;
    argList = toCopy.argList;
    cmdLine = toCopy.cmdLine;
    pos = toCopy.pos;
  }
   int get_timeStamp(){
    return timestamp;
   }
   void timestamp_incr(){
     timestamp++;
   }
  string get_cmd_Line(){
    return cmdLine;
  }
  char** get_arg_List(){
     return argList;
   }
  };
  int counter ;
  vector<CommandHistoryEntry> cmd_his;
 public:
  CommandsHistory(){
    counter = 0;
    cmd_his = vector<CommandHistoryEntry>();
  }
  ~CommandsHistory() {
    cmd_his.clear();
  }
  
  void delete_args(char** toDelete){
    int i = 0;
    while(toDelete[i] != NULL){
      free(toDelete[i]);
      i++;
    }
    delete[] toDelete;
  }
  int cmpArg(char** arg1, char** arg2){
    int i = 0 ; 
    while( arg1[i] != NULL && arg2[i] != NULL){
      if( strcmp(arg1[i],arg2[i]) != 0)
        return -1;
      i++;
    }
    if(arg1[i] == NULL && arg2[i] != NULL){
      return -1;
    }
    if(arg1[i] != NULL && arg2[i] == NULL){
      return -1;
    }
    return 0;
  }
  void incCounter(){
    counter++;
  }
  void decCounter(){
    counter--;
  }
  int getCounter(){
    return counter;
  }
  void deleteLast(){
    decCounter();
    if( cmd_his.size() == 0)
      cmd_his.clear();
    else
      cmd_his.pop_back();
  }
  void addRecord(char** arg_List, const char* cmd_Line){
    if( cmd_his.size() == 0){ // first command to insert
      CommandHistoryEntry toInsert(1,arg_List,string(cmd_Line),0);
      incCounter();
      cmd_his.push_back(toInsert);
      return;
    }
    if(cmpArg(arg_List,cmd_his.back().get_arg_List()) == 0){ // Same commands
      cmd_his.back().timestamp_incr();
      return;
    }
    // Differents commands 
    CommandHistoryEntry toInsert(1,arg_List,string(cmd_Line), cmd_his.back().getPos()+cmd_his.back().get_timeStamp());
    incCounter();
    cmd_his.push_back(toInsert);
  }
  void printHistory();
};


class Command {
  const char* cmdLine;
  CommandsHistory* cmdHistory;
  char** args;
  string strCmdLine;
  pid_t pid;
 public:
  Command(const char* cmd_line, CommandsHistory* cmdHis, char** arg_List, pid_t piD = 0);
  Command(Command& toCopy);
  virtual ~Command();
  virtual void execute() = 0;
  const char* getCmdLine(){
    return cmdLine;
  }
  char** getListArg(){
    return args;
  }
  char* lastArg(){
    char* temp;
    for(int i = 0 ; args[i] != NULL; i++){
      temp = args[i];
    }
    return temp;
  }
  void changeCmdLineToPipe_2(){
    string temp;
    int pipe = 0;
    for(int i =1 ; args[i] != NULL ; i++){
      if( strcmp( args[i] , "|") == 0 ){
        pipe = 1;
        continue;
      }
      if(pipe == 1){
      temp.append(args[i]);
      temp.append(" ");
      }
    }
    temp.pop_back();
    strCmdLine = temp;
  }
  void changeCmdLineToPipe(){
    string temp;
    temp.append(args[0]);
    for(int i =1 ; args[i] != NULL ; i++){
      if( strcmp( args[i] , "|") == 0 ){
        strCmdLine = temp;
        return;
      }
      temp.append(" ");
      temp.append(args[i]);
    }
  }
  void changeCmdLineToRedir(){
    string temp;
    temp.append(args[0]);
    for(int i =1 ; args[i] != NULL ; i++){
      if( strcmp( args[i] , ">") == 0 || strcmp( args[i] , ">>") == 0 ){
        strCmdLine = temp;
        return;
      }
      temp.append(" ");
      temp.append(args[i]);
    }
  }
  bool isBuiltInCmd(){
    char const *list[] = {"pwd", "cd", "history", "jobs", "kill", "showpid", "fg", "bg", "quit," "cp", NULL};
    for(int i = 0 ; list[i] != NULL ; i++){
      if( strcmp( args[0], list[i]) == 0)
        return true;
    }
    return false;
  }
  void updatePID(pid_t toUpd){
    pid = toUpd;
  }
  pid_t getPid(){
    return pid;
  }
  void updateArgs(char** toCopy){
    args = toCopy;
  }
  string getStrCmdLine(){
    return strCmdLine;
  }
  void updateStrCmdLine(string toCopy){
    strCmdLine = toCopy;
  }
  void printArgs(){
    for(int i = 0 ; args[i] != NULL ; i++){
      cout << args[i] << " ";
    }
  }
  CommandsHistory* getcmdHistory(){
    return cmdHistory;
  }
  //virtual void prepare();
  //virtual void cleanup();
  // TODO: Add your extra methods if needed
};

class BuiltInCommand : public Command {
 public:
  BuiltInCommand(const char* cmd_line, CommandsHistory* cmdHis, char** arg_List);
  virtual ~BuiltInCommand() {}
};



class PipeCommand : public Command {
  // TODO: Add your data members
 public:
  PipeCommand(const char* cmd_line, CommandsHistory* listHist,char** arg_List);
  virtual ~PipeCommand() {}
  void execute() override;
};



class ChangeDirCommand : public BuiltInCommand {
  char* lastPwd;
// TODO: Add your data members public:
  public:
  ChangeDirCommand(const char* cmd_line, char* plastPwdArg, CommandsHistory* listH, char** arg_List);
  virtual ~ChangeDirCommand();
  void execute() override;
};

class GetCurrDirCommand : public BuiltInCommand {
 public:
  GetCurrDirCommand(const char* cmd_line, CommandsHistory* cmdHis, char** arg_List);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
  };

class ShowPidCommand : public BuiltInCommand {
 public:
  ShowPidCommand(const char* cmd_line, CommandsHistory* cmdHis, char** arg_List);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

class JobsList;
class QuitCommand : public BuiltInCommand {
  JobsList* jobs_List;
 public:
  QuitCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray, JobsList* jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command {
 bool BackGroundCmd;
 public:
  explicit RedirectionCommand(const char* cmd_line, CommandsHistory* listHist,char** arg_List, bool BackGround);
  virtual ~RedirectionCommand() {}
  void execute() override;
  bool isBG(){
    return BackGroundCmd;
  }
  //void prepare() override;
  //void cleanup() override;
}; 

class ExternalCommand : public Command {
  JobsList* jobs_List;
  pid_t processRunning;
  string strCmdLine;
 public:
  ExternalCommand(const char* cmd_line, CommandsHistory* listHist ,char** arg_List, JobsList* job_list, pid_t process_Running );
  virtual ~ExternalCommand() {}
  void execute() override;
  void updateProRunning(pid_t proRunning){
    processRunning = proRunning;
  }
  string getStrCmdLine() {
    return strCmdLine;
  }
  pid_t getProcesRunning(){
    return processRunning;
  }
};



class HistoryCommand : public BuiltInCommand {
 public:
  HistoryCommand(const char* cmd_line, CommandsHistory* history, char** arg_List);
  virtual ~HistoryCommand();
  void execute() override;
};

class ForegroundCommand : public BuiltInCommand {
 JobsList* jobs_List;
 public:
  ForegroundCommand(const char* cmd_line, CommandsHistory* listHist ,char** arg_List, JobsList* job_list);
  virtual ~ForegroundCommand();
  void execute() override;
};

class JobsList {
 public:
  class JobEntry {
    Command* cmd;
    //string cmdLine;
    //pid_t processId;
    int jobId;
    time_t timeStart;
    bool isStopped;
    //char** argS;
    public:
    JobEntry(){}
    JobEntry(Command* cmD, int job_Id, time_t second_Elapsed = 0, bool is_Stopped = 0) : cmd(cmD), 
     jobId(job_Id), timeStart(second_Elapsed),isStopped(is_Stopped){
      time(&timeStart);
    }
    ~JobEntry(){}
    JobEntry(const JobEntry &toCopy){
      cmd = toCopy.cmd;
      jobId = toCopy.jobId;
      timeStart = toCopy.timeStart;
      isStopped = toCopy.isStopped;
    }
    string getCmdLine(){
      return cmd->getStrCmdLine();
    }
    void printArg(){
      cmd->printArgs();
    }
    Command* getCmd(){
      return cmd;
    }
    pid_t getProcessID(){
      return cmd->getPid();
    }
    time_t getTimeStart(){
      return timeStart;
    }
    void updateTime(){
      time(&timeStart);
    }
    bool isStop(){
      return isStopped;
    }
    void Stopped(){
      if(isStopped == false)
        isStopped = true;
    }
    void Running(){
      if(isStopped == true)
        isStopped = false;
    }
    
    
    string getStopped(){
      if(isStopped == true){
        return string("(stopped)");
      }
      return string("");
    }
    void updateJobId(int posList){
      jobId = posList;
    }
    char** getArgs(){
      return cmd->getListArg();
    }
    int getJobID(){
      return jobId;
    }
    void removeMark(){
      isStopped = false;
    }
  };
  int order;
  vector<JobEntry> jobsList;
 public:
  JobsList(){
    order = 0;
    jobsList = vector<JobEntry>();
  }
  ~JobsList(){}
  vector<JobEntry> getJobsList(){
    return jobsList;
  }
  void decOrder(){
    order--;
  }
  bool isThereStoppedProcess(){
    if(jobsList.size() == 0){
      return false;
    }
      for (vector<JobEntry>::iterator i = jobsList.begin(); i != jobsList.end(); ++i){
        if( (*i).isStop() == true){
          return true;
        }     
      }
      return false;
  }

  bool isRunning(int pid){
  int status;
  int w = waitpid(pid, &status, WNOHANG);
  return !WIFEXITED(status) && w != -1;
  }
  
  void addJob(Command* cmd, bool isStopped = false){
    removeFinishedJobs();
    if(isStopped == false){ // First time 
      if( jobsList.size() ==0){
        JobEntry toInsert(cmd ,1);
        jobsList.push_back(toInsert);
        return;
      }
      else{
        JobEntry toInsert(cmd , jobsList.back().getJobID()+1);
        jobsList.push_back(toInsert);
       return;
      }
    }
    else{
      order++;
      if( jobsList.size() ==0){
        JobEntry toInsert(cmd ,1,0,true);
        jobsList.push_back(toInsert);
        return;
      }
      else{
        JobEntry toInsert(cmd , jobsList.back().getJobID()+1,0,true);
        jobsList.push_back(toInsert);
       return;
      }
    }
  }
  
  void printJobsList(){
    time_t currentTime;
    time(&currentTime);
    for (vector<JobEntry>::iterator i = jobsList.begin(); i != jobsList.end(); ++i){
      if( isRunning( (*i).getProcessID()) == false){
        if(jobsList.size() == 1){
          jobsList.clear();
          return;
        }
        jobsList.erase(i);
      }
    }
    for (vector<JobEntry>::iterator i = jobsList.begin(); i != jobsList.end(); ++i){
      cout << "[" << (*i).getJobID() << "] " << (*i).getCmdLine() << " : " << (*i).getProcessID() <<" "<< difftime(currentTime,(*i).getTimeStart()) << " secs "  << (*i).getStopped()  << endl;
    }
  }
  
  void killAllJobs();
  
  void removeFinishedJobs(){
    for(vector<JobEntry>::iterator it = jobsList.begin() ; it != jobsList.end() ; ++it){
      if( isRunning( (*it).getProcessID()) == false){
        if( jobsList.size() == 1){
          jobsList.clear();
          break;
        }
        jobsList.erase(it);
      }
    }
  }
  JobEntry * getJobById(int jobId){
    for(vector<JobEntry>::iterator it = jobsList.begin() ; it != jobsList.end() ; ++it){
      if( (*it).getJobID() == jobId ){
        return &( (*it));
      }
    }
    return NULL;
  }
  JobEntry * getJobByPid(pid_t pid){
    for(vector<JobEntry>::iterator it = jobsList.begin() ; it != jobsList.end() ; ++it){
      if( (*it).getProcessID() == pid ){
        return &((*it));
      }
    }
    return NULL;
  }
  void removeJobByPid(int pid){
    for(vector<JobEntry>::iterator it = jobsList.begin() ; it != jobsList.end() ; ++it){
      if( (*it).getProcessID() == pid ){
        if( jobsList.size() == 1){
          jobsList.clear();
          return;
        }
        jobsList.erase(it);
        return;
      }
    }
  }
  void removeMarkStopById(int jobId){
    for(vector<JobEntry>::iterator it = jobsList.begin() ; it != jobsList.end() ; ++it){
      if( (*it).getJobID() == jobId){
        (*it).removeMark();
        return;
      }
    }
  }

  JobEntry * getLastJob(){
    return  &( this->jobsList.back() ) ;
  }
  void removeLastJob(){
    jobsList.erase(jobsList.end()-1);
  }
  JobEntry *getLastStoppedJobByID(int *jobId){
    for(vector<JobEntry>::iterator it = jobsList.begin() ; it != jobsList.end() ; ++it){
      if( (*it).isStop() == true && (*it).getJobID() == *jobId){
        JobEntry* outp = new JobEntry( (*it));
        return outp;
      }
      else if( (*it).isStop() == true && (*it).getJobID() != *jobId){
        return NULL;
      }
    }
    return NULL;
  }
  JobEntry *getLastStoppedJob(){
    for(vector<JobEntry>::reverse_iterator it = jobsList.rbegin() ; it != jobsList.rend() ; ++it){
      if( (*it).isStop() == true ){
        JobEntry* outp = new JobEntry( (*it));
        return outp;
      }
    }
    return NULL;
  }

  // TODO: Add extra methods or modify exisitng ones as needed
};
 
class JobsCommand : public BuiltInCommand {
 JobsList* jobsList;
 public:
  JobsCommand(const char* cmd_line, CommandsHistory* listHist, char** argArray, JobsList* jobs);
  virtual ~JobsCommand();
  void execute() override;
};

class KillCommand : public BuiltInCommand {
 JobsList* jobs_List;
 public:
  KillCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray, JobsList* jobs);
  virtual ~KillCommand() {}
  void execute() override;
};


class BackgroundCommand : public BuiltInCommand {
 JobsList* jobs_List;
 public:
  BackgroundCommand(const char* cmd_line, CommandsHistory* listHist ,char** arg_List, JobsList* job_list);
  virtual ~BackgroundCommand() {}
  void execute() override;
};

class CopyCommand : public BuiltInCommand {
 public:
  CopyCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray);
  virtual ~CopyCommand() {}
  void execute() override;
};

class SmallShell {
 private:
 char* last_Dir; //Last direction for commande "cd -"
 CommandsHistory* history_list; 
 JobsList* job_list;
 Command* CurrentJob;
 // pid running 
 public:
  SmallShell();
  Command *CreateCommand(const char* cmd_line);
  SmallShell(SmallShell const&)      = delete; // disable copy ctor
  void operator=(SmallShell const&)  = delete; // disable = operator
  static SmallShell& getInstance() // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  CommandsHistory* getHistList(){
    return history_list;
  }
  void updateCurrentJob(Command* toUpdate){
    CurrentJob = toUpdate;
  }
  Command* getCurrentJob(){
    return CurrentJob;
  }
  JobsList* getJobsList(){
    return job_list;
  }
  ~SmallShell();
  void executeCommand(const char* cmd_line);
  // TODO: add extra methods as needed
};

#endif //SMASH_COMMAND_H_

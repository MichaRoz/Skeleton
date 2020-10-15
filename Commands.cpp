#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"
#include <assert.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


using namespace std;

#if 0
#define FUNC_ENTRY()  \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT()  \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif


const std::string WHITESPACE = " \n\r\t\f\v";
string _ltrim(const std::string& s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string& s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string& s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char* cmd_line, char** args) {
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for(std::string s; iss >> s; ) {
    args[i] = (char*)malloc(s.length()+1);
    memset(args[i], 0, s.length()+1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char* cmd_line) {
  const string whitespace = " \t\n";
  const string str(cmd_line);
  return str[str.find_last_not_of(whitespace)] == '&';
}

bool _isComplexCommand(const char* cmd_line){
  string cmdLine = string(cmd_line);
  for(int i = 0 ; i < int(cmdLine.size()) ; i++){
    if(cmdLine[i] == '*' || cmdLine[i] == '?'){
      return true;
    }
  }
  return false;
}


void _removeBackgroundSign(char* cmd_line) {
  const string whitespace = " \t\n";
  const string str(cmd_line);
  // find last character other than spaces
  size_t idx = str.find_last_not_of(whitespace);
  // if all characters are spaces then return
  if (idx == string::npos) {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&') {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(whitespace, idx-1) + 1] = 0;
}

void _removeKavSign(char* cmd_line) {
  string temp = string(cmd_line);
  int first = temp.find_first_of('-');
  int it = first+1;
  for(int i = 0 ; i < int(temp.size()-first) ;i++){
    cmd_line[i] = temp[it];
    it++;
  }
}

bool isDigit(string toCheck){
  for(string::iterator it = toCheck.begin(); it != toCheck.end() ; ++it){
    if( '0' > (*it) || (*it) > '9')
        return false;
  }
  return true;
}

// TODO: Add your implementation for classes in Commands.h 

/* 
--------------------------------------------------------------------------------------------------------------------------------------------
                                                COMMAND of class COMMAND      
--------------------------------------------------------------------------------------------------------------------------------------------   */


Command::Command(const char* cmd_line, CommandsHistory* cmd_H, char** arg_List, pid_t piD) : cmdLine() , cmdHistory(cmd_H), args(arg_List), pid(piD){
  cmdLine = cmd_line;
  strCmdLine = string(cmd_line);
}
Command::Command(Command& toCopy){
  cmdLine = toCopy.cmdLine;
  cmdHistory = toCopy.cmdHistory;
  args = toCopy.args;
}
Command::~Command() {
}

/*------------------------     CONSTRUCTOR OF SUB CLASS     --------------------------------------------------------------------------------------*/


BuiltInCommand::BuiltInCommand(const char* cmd_line, CommandsHistory* listHist,char** arg_List): Command(cmd_line,listHist,arg_List){}

RedirectionCommand::RedirectionCommand(const char* cmd_line, CommandsHistory* listHist,char** arg_List, bool BackGround) : Command(cmd_line,listHist,arg_List), BackGroundCmd(BackGround){}

PipeCommand::PipeCommand(const char* cmd_line, CommandsHistory* listHist,char** arg_List) : Command(cmd_line,listHist,arg_List){}

GetCurrDirCommand::GetCurrDirCommand(const char* cmd_line, CommandsHistory* listHist,char** arg_List) : BuiltInCommand(cmd_line, listHist, arg_List) {}

ChangeDirCommand::ChangeDirCommand(const char* cmd_line, char* plastPwd, CommandsHistory* listHist, char** argArray) : 
                                              BuiltInCommand(cmd_line,listHist, argArray), lastPwd(plastPwd){
}

ExternalCommand::ExternalCommand(const char* cmd_line,CommandsHistory* listHist,char** arg_List,
JobsList* job_list, pid_t process_Running ): Command(cmd_line, listHist, arg_List), jobs_List(job_list), processRunning(process_Running){
  strCmdLine = string(cmd_line);
}
ShowPidCommand::ShowPidCommand(const char* cmd_line,CommandsHistory* listHist,  char** argArray) : BuiltInCommand(cmd_line,listHist, argArray){}

HistoryCommand::HistoryCommand(const char* cmd_line, CommandsHistory* listHist, char** argArray): BuiltInCommand(cmd_line,listHist, argArray){}

JobsCommand::JobsCommand(const char* cmd_line, CommandsHistory* listHist, char** argArray, JobsList* jobs) : BuiltInCommand(cmd_line,listHist, argArray), jobsList(jobs){}

ForegroundCommand::ForegroundCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray, JobsList* jobs) : BuiltInCommand(cmd_line,listHist, argArray), jobs_List(jobs){}

KillCommand::KillCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray, JobsList* jobs) : BuiltInCommand(cmd_line,listHist, argArray), jobs_List(jobs){}

BackgroundCommand::BackgroundCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray, JobsList* jobs) : BuiltInCommand(cmd_line,listHist, argArray), jobs_List(jobs){}

QuitCommand::QuitCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray, JobsList* jobs) : BuiltInCommand(cmd_line,listHist, argArray), jobs_List(jobs){}

CopyCommand::CopyCommand(const char* cmd_line, CommandsHistory* listHist ,char** argArray) : BuiltInCommand(cmd_line,listHist, argArray){}



/*------------------------       DESTRUCTOR           ----------------------------------------------------------------------------------------------*/


ChangeDirCommand::~ChangeDirCommand(){}

HistoryCommand::~HistoryCommand(){}

JobsCommand::~JobsCommand(){}

ForegroundCommand::~ForegroundCommand(){}

/*------------------------         EXECUTE            ------------------------------------------------------------------------------------------------*/

void CopyCommand::execute(){
  int fd_read = open(this->getListArg()[1], O_RDONLY, 0666);
  if( fd_read == -1){
    perror("smash error: open failed");
    return;
  }
  int fd_write = open(this->getListArg()[2], O_RDWR | O_CREAT, 0666);
  if(  fd_write == -1) {
    close(fd_read);
    perror("smash error: open failed");
    return;
  }
  char buf[10];
  int size ;
  while( ((size = read(fd_read,buf,10)) > 0) ){
      if( write(fd_write,buf,size) < size ){
        perror("smash error: write failed");
      }
      if( size < 10) break;
  }
  if( size == - 1){
    perror("smash error: read failed");
  }
  cout << "smash: " << this->getListArg()[1] << " was copied to " << this->getListArg()[2] << endl;
}

void PipeCommand::execute(){
  int fd[2];
  pipe(fd);
  SmallShell& smash = SmallShell::getInstance();
  pid_t pid ;
  if ((pid = fork()) < 0) {    
    cout << "*** ERROR: forking child process failed\n" << endl;
    exit(1);
  }
  if( pid == 0) {
    dup2(fd[1],1);
    close(fd[0]);
    close(fd[1]);
    if( isBuiltInCmd() == true){
      if( strcmp( this->getListArg()[0] , "history") == 0){
        char cmd_temp[] = {"history -1"};
        smash.executeCommand(cmd_temp);
      }
      else{
        smash.executeCommand(this->getListArg()[0]);
        smash.getHistList()->deleteLast();
      }
    }
    else{ // External command
      this->changeCmdLineToPipe();
      smash.executeCommand( this->getStrCmdLine().c_str());
      smash.getHistList()->deleteLast();
    }
  }
  if(pid > 0) {
    dup2(fd[0],0);
    close(fd[1]);
    close(fd[0]);
    this->changeCmdLineToPipe_2();
    smash.executeCommand( this->getStrCmdLine().c_str());
    smash.getHistList()->deleteLast();
    /* waitpid(pid,&child_status,WUNTRACED); */
  }
  
  close(fd[0]);
  close(fd[1]);

}

void QuitCommand::execute(){
  if (this->getListArg()[1] != NULL && strcmp(this->getListArg()[1],"kill") == 0){
    std::cout << "smash: sending SIGKILL signal to " << jobs_List->jobsList.size() << " jobs:" << endl;
    for (vector<JobsList::JobEntry>::iterator it = jobs_List->jobsList.begin(); it != jobs_List->jobsList.end(); ++it){
      int pid = (*it).getProcessID();
      if (jobs_List->isRunning(pid)){
        cout << pid << ": " << (*it).getCmdLine() << endl;
        kill(pid, SIGKILL);
      }
    }
    exit(0);
  }
  for (vector<JobsList::JobEntry>::iterator it = jobs_List->jobsList.begin(); it != jobs_List->jobsList.end(); ++it){
      int pid = (*it).getProcessID();
      if (jobs_List->isRunning(pid)){
        kill(pid, SIGKILL);
      }
    }
  exit(0);
}

void KillCommand::execute(){
  if( this->getListArg()[1] == NULL || this->getListArg()[3] != NULL){
    cout << "smash error: kill: invalid arguments" << endl;
    return;
  }
  _removeKavSign(this->getListArg()[1]);
  if( this->getListArg()[2] == NULL || (isDigit(string( this->getListArg()[1] )) == false  ) ){
    cout << "smash error: kill: invalid arguments" << endl;
    return;
  }
  if( this->getListArg()[2] != NULL && isDigit( string(this->getListArg()[2]) ) == false){
    cout << "smash error: kill: invalid arguments" << endl;
    return; 
  }  

  if( (atoi(this->getListArg()[2]) > 31 || atoi(this->getListArg()[2])<1 )){
    cout << "smash error: kill: invalid arguments" << endl;
    return; 
  }

  JobsList::JobEntry *jobToFG =  this->jobs_List->getJobById(atoi(this->getListArg()[2]));
  if( jobToFG == NULL){
    cout << "smash error: kill: job-id " << atoi(this->getListArg()[2]) <<" does not exist" << endl;
    return;
  }
  if( kill(jobToFG->getProcessID(),atoi(this->getListArg()[1])) == -1){
    perror("smash error: kill failed");
    return;
  }
  cout << "signal number "<< atoi(this->getListArg()[1]) <<" was sent to pid " << jobToFG->getProcessID() << endl;
  for (vector<JobsList::JobEntry>::iterator i = jobs_List->jobsList.begin(); i != jobs_List->jobsList.end(); ++i){
      if( (*i).getProcessID() == jobToFG->getProcessID()){
        if(jobs_List->jobsList.size() == 1){
          jobs_List->jobsList.clear();
          break;
        }
      jobs_List->jobsList.erase(i);
      break;
      } 
  }
  
}

void ForegroundCommand::execute(){

  this->jobs_List->removeFinishedJobs();
  /* CHECK PARAMETER */
  if( (this->getListArg()[1] == NULL && this->jobs_List->jobsList.size() == 0) ||  this->jobs_List->jobsList.size() == 0){
    cout << "smash error: fg: jobs list is empty" << endl;
    return;
  }


  if( this->getListArg()[1] != NULL && isDigit( string(this->getListArg()[1]) ) == false){
    cout << "smash error: fg: invalid arguments" << endl;
    return; 
  }
  if( this->getListArg()[2] != NULL){
    cout << "smash error: fg: invalid arguments" << endl;
    return; 
  }
  JobsList::JobEntry *jobToFG ;
  if( this->getListArg()[1] == NULL){
     jobToFG = this->jobs_List->getLastJob();
  }
  else{
    int jobId_Input = atoi(this->getListArg()[1]); // CONVERSION ARG TO INT 
    jobToFG = this->jobs_List->getJobById(jobId_Input);
    if(jobToFG == NULL){
      cout << "smash error: fg: job-id " <<  jobId_Input << " does not exist" << endl;
      return;
    }
  }
  cout << jobToFG->getCmdLine() << " : " << jobToFG->getProcessID() << endl;
  int res_kill = kill(jobToFG->getProcessID(),SIGCONT); // IF ZERO = SUCCESS, -1 = ERROR
  if( res_kill)
    cout << "kill did not succeed" << endl;


  SmallShell& smash = SmallShell::getInstance();
  smash.updateCurrentJob(jobToFG->getCmd());
  waitpid(jobToFG->getProcessID(),&res_kill,WUNTRACED);
  
}

void BackgroundCommand::execute(){
this->jobs_List->removeFinishedJobs();
if( this->getListArg()[2] != NULL){
  cout << "smash error: bg: invalid arguments" << endl;
  return; 
}
/* CHECK PARAMETER */
    if( this->getListArg()[1] == NULL ){
        JobsList::JobEntry *jobToBG = this->jobs_List->getLastStoppedJob(); 
        if( jobToBG == NULL){
          cout << "smash error: bg: there is no stopped jobs to resume" << endl;
          return;
        }
        cout << jobToBG->getCmdLine() << " : " << jobToBG->getProcessID() << endl;
        kill(jobToBG->getProcessID(),SIGCONT);
        this->jobs_List->removeMarkStopById(jobToBG->getJobID());
        return;
    }
    if(isDigit( string(this->getListArg()[1]) ) == false){
        cout << "smash error: bg: invalid arguments" << endl;
        return; 
    }
    int jobId_Input = atoi(this->getListArg()[1]); // CONVERSION ARG TO INT 
    JobsList::JobEntry *jobToBG = this->jobs_List->getJobById(jobId_Input);
    if( jobToBG == NULL){
      cout << "smash error: bg: job-id "<< jobId_Input <<" does not exist" << endl;
      return;
    }
    if( jobToBG->isStop() == false){
      cout << "smash error: bg: job-id "<< jobId_Input <<" is already running in the background" << endl;
      return;
    }

    
      if( this->jobs_List->getLastStoppedJobByID(&jobId_Input) == NULL){
        cout << "smash error: fg: job-id " <<  jobId_Input << " does not exist" << endl;
        return;
      }
  
    cout << jobToBG->getCmdLine() << " : " << jobToBG->getProcessID() << endl;
    kill(jobToBG->getProcessID(),SIGCONT);
    this->jobs_List->removeMarkStopById(jobToBG->getJobID());


}

void JobsCommand::execute(){
  this->jobsList->removeFinishedJobs();
  this->jobsList->printJobsList();
}

void GetCurrDirCommand::execute()      //           PWD 
{
  char outp[100];
  if(getcwd(outp,100) == NULL){
    perror("smash error: getcwd failed");
    return;
  }
  std::cout << outp << std::endl;
}

void RedirectionCommand::execute(){    
    int stdout_copy = dup(1);
    close(1);
    int fd_write;
    string cmd_line = string(this->getCmdLine());
    size_t foundAppend = cmd_line.find(">>");
    size_t foundWrite = cmd_line.find(">");
    size_t toCut = (foundAppend == string::npos) ? foundWrite : foundAppend;
    string CmdLine = cmd_line.substr(0, toCut-1);
    if( foundAppend != string::npos){
      fd_write = open(this->lastArg(), O_WRONLY|O_CREAT|O_APPEND, 0666);
    }
    else 
      fd_write = open(this->lastArg(), O_WRONLY|O_CREAT, 0666);
    SmallShell& smash = SmallShell::getInstance();
    
    
    //this->changeCmdLineToRedir();
    if( strcmp( this->getListArg()[0] , "history") == 0){
        char cmd_temp[] = {"history -1"};
        smash.executeCommand(cmd_temp);
    }
    else{
      if( this->isBG() == true){
        CmdLine = CmdLine+" &";
        smash.executeCommand(CmdLine.c_str());
        smash.getJobsList()->getLastJob()->getCmd()->updateStrCmdLine(cmd_line);
        smash.getHistList()->deleteLast();
      }
      else{
        smash.executeCommand(CmdLine.c_str());
        smash.getHistList()->deleteLast();
      }
        //smash.getJobsList()->removeLastJob();
    }
    close(fd_write);
    dup2(stdout_copy,1);
    close(stdout_copy);
}

void ChangeDirCommand::execute(){
    char old_pwd[100];
    if( getcwd(old_pwd,100) == NULL){
      perror("smash error: getcwd failed");
      return;
    }

    string cmd_s = this->getCmdLine() ;
    if((this->getListArg())[1] == NULL){
      return;
    }
    if((this->getListArg())[2] != NULL){
      std::cout << "smash error: cd: too many arguments" << std::endl ;
      return;
    }
    if( strcmp( (this->getListArg())[1],"-") == 0 && strcmp( this->lastPwd,"") == 0){ // error 
      std::cout << "smash error: cd: OLDPWD not set" << std::endl;
      return;
    } 
    if( strcmp( (this->getListArg())[1],"..") == 0 ){
      if( chdir("..") == -1){
        perror("smash error: chdir failed");
      }
      strcpy(this->lastPwd,old_pwd);
      return;
    }
    if( strcmp( (this->getListArg())[1],"-") == 0) {  // go to last cd
      if( chdir(this->lastPwd) == -1){
        perror("smash error: chdir failed");
      }
      strcpy(this->lastPwd,old_pwd);
      return;
    }
    if(chdir((this->getListArg())[1]) == -1){
        perror("smash error: chdir failed");
        return;
    }
    strcpy(this->lastPwd ,old_pwd);
}

void HistoryCommand::execute(){
  this->getcmdHistory()->printHistory();
}

void ShowPidCommand::execute(){
  std::cout << "smash pid is "<< getpid() << std::endl;
}

void ExternalCommand::execute(){
  pid_t  pid;
  int  child_status;

  if ((pid = fork()) < 0) {    
    cout << "*** ERROR: forking child process failed\n" << endl;
    exit(1);
  }
  /*  CHILD PROCESS  */
  if (pid == 0) {                 
      setpgrp();
      if(_isComplexCommand(this->getCmdLine()) == true){
        char temp[100];
        strcpy(temp,this->getCmdLine());
        char bash[] = "bash";
        char c[] = "-c";
        char* Args[] ={bash,c,temp,NULL};
        execvp( "/bin/bash",Args);
      }
      if (execvp(this->getListArg()[0], this->getListArg()) < 0) {     
        perror("smash error: execvp failed");
        exit(1);
      }
  }
  if( pid > 0 ){ /* PARENT PROCESS */

    updateProRunning(pid);
    this->updatePID(pid);

  /* BACKGROUND */
    if(_isBackgroundCommand(this->getCmdLine()) == true){ 
      this->jobs_List->removeFinishedJobs();
      this->jobs_List->addJob(this,false);
      return;
    }

  /* FOREGROUND */
    else{
      SmallShell& smash = SmallShell::getInstance();
      smash.updateCurrentJob(this);
      waitpid(pid,&child_status,WUNTRACED); // O = options. FLAG
    }
  } 
}



void CommandsHistory::printHistory(){
    if( this->cmd_his.size() > HISTORY_MAX_RECORDS){
      vector<CommandHistoryEntry>::iterator i = cmd_his.begin();
      for(int it = 0 ; it < this->getCounter() - HISTORY_MAX_RECORDS; it++)
        i++;
      for (; i != cmd_his.end(); ++i){
        std::cout << right << std::setw(5) << (*i).get_timeStamp()+(*i).getPos() << "  " << (*i).get_cmd_Line() << std::endl;        
      }
      return;
    }
    for (vector<CommandHistoryEntry>::iterator i = cmd_his.begin() ; i != cmd_his.end(); ++i){
        std::cout << right << std::setw(5) << (*i).get_timeStamp()+(*i).getPos() << "  " << (*i).get_cmd_Line() << std::endl;
    }
    
}






/* 
--------------------------------------------------------------------------------------------------------------------------------------------
                                                COMMAND SMALLSHELL      
--------------------------------------------------------------------------------------------------------------------------------------------   */


SmallShell::SmallShell(){
  last_Dir = new char[100];
  history_list = new CommandsHistory();
  job_list = new JobsList();
  CurrentJob = nullptr;
}

SmallShell::~SmallShell() {
  delete(last_Dir);
  this->history_list->~CommandsHistory();
  this->job_list->~JobsList();
}

/**
* Creates and returns a pointer to Command class which matches the given command line (cmd_line)
*/
Command * SmallShell::CreateCommand(const char* cmd_line) {

  char cmdLine_WithoutBG[100];
  strcpy(cmdLine_WithoutBG,cmd_line); 
  bool isBackGroundCmd = _isBackgroundCommand(cmd_line);
  _removeBackgroundSign(cmdLine_WithoutBG);   // COMMAND LINE WITHOUT &

  char** args;
  args = new char* [20];
  _parseCommandLine(cmdLine_WithoutBG, args);
  string cmd_s = string(cmd_line);
  size_t foundAppend = string(cmd_line).find(">>");
  size_t foundWrite = string(cmd_line).find(">");
  size_t foundPipe = string(cmd_line).find("|");

  if( args[0] == NULL){
    return nullptr;
  }
  if (strcmp(args[0], "pwd") == 0) {
    return new GetCurrDirCommand(cmd_line,this->history_list, args);
  }
  else if ( foundAppend != string::npos || foundWrite != string::npos){
    return new RedirectionCommand(cmd_line,this->history_list, args, isBackGroundCmd);
  } 
  else if ( foundPipe != string::npos){
    return new PipeCommand(cmd_line,this->history_list, args); 
  }
  else if (strcmp(args[0], "cd") == 0) {
    return new ChangeDirCommand(cmd_line,last_Dir, this->history_list, args);
  }
  else if ( strcmp(args[0], "history") == 0){
    return new HistoryCommand(cmd_line, this->history_list, args);
  }
  else if ( strcmp(args[0], "showpid") == 0){
    return new ShowPidCommand(cmd_line,this->history_list, args);
  }
  else if ( strcmp(args[0], "jobs") == 0){
    return new JobsCommand(cmd_line,this->history_list, args, this->job_list);
  }
  else if ( strcmp(args[0], "fg") == 0){
    return new ForegroundCommand(cmd_line,this->history_list, args, this->job_list); 
  }
  else if ( strcmp(args[0], "kill") == 0){
    return new KillCommand(cmd_line,this->history_list, args, this->job_list); 
  }
  else if ( strcmp(args[0], "bg") == 0){
    return new BackgroundCommand(cmd_line,this->history_list, args, this->job_list); 
  }
  else if ( strcmp(args[0], "quit") == 0){
    return new QuitCommand(cmd_line,this->history_list, args, this->job_list); 
  }
  else if ( strcmp(args[0], "cp") == 0){
    return new CopyCommand(cmd_line,this->history_list, args); 
  }
  
  else{
    return new ExternalCommand(cmd_line, this->history_list, args, this->job_list,0);
  }
}


void SmallShell::executeCommand(const char *cmd_line) {

  Command* cmd = CreateCommand(cmd_line);
  if(cmd == nullptr) return;
  this->history_list->addRecord(cmd->getListArg(),cmd->getCmdLine());
  if( cmd->getListArg()[1] != NULL && (strcmp(  cmd->getListArg()[1] , "-1") == 0 )){
    this->getHistList()->deleteLast();
  }
  cmd->execute();
}


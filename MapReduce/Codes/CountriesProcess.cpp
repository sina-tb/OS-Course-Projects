#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <sys/wait.h>
#include <vector>
#include <bits/stdc++.h>
#include <dirent.h>
#include <sys/types.h>


using namespace std;

#define WRITE 1
#define READ 0
#define MAX 10000
#define COUNTRY "./country.out"
#define CLUB "./club.out"
#define REDUCE "./reduce.out"

vector <string> tokenize(string chosen_pos,char delimiter)
{
    stringstream ss(chosen_pos); 
    string s; 
    vector <string> str;
    while (getline(ss, s, delimiter)) {    
        str.push_back(s);
    }
    return str; 
}

int create_process(int& write_pipe, string executable)
{
    int pipe_fd[2];
    if (pipe(pipe_fd) == -1) {
        perror("pipe");
    }
    int pid = fork();
    if (pid == 0) {
        // Child process
        dup2(pipe_fd[READ], STDIN_FILENO);
        close(pipe_fd[WRITE]);
        close(pipe_fd[READ]);
        execl(executable.c_str(), executable.c_str(), NULL);
        perror("execl");
    } else if (pid > 0) {
        // Parent process
        close(pipe_fd[READ]);
        write_pipe = pipe_fd[WRITE];
    }else{
        perror("fork");
    }
    return pid;
}

void create_clubs_process(vector <string>& csv_files, string positions,vector <int>& child_pids,string id)
{
    for (int i = 0; i < csv_files.size(); i++){
        int write_pipe;
        int pid= create_process(write_pipe,CLUB);
        string data = csv_files[i] + '-' + positions + '-' + id + to_string(i);
        write(write_pipe, data.c_str(), data.length());
        child_pids.push_back(pid);
        close(write_pipe);
    }
}
vector <string> find_csvs(string path)
{
    vector <string> files;
    vector <string> org_files;
    DIR *dr;
    struct dirent *en;
    char* temp[MAX];
    dr = opendir(path.c_str());
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            files.push_back(en->d_name);
        }
        closedir(dr);
    }
    for (int i = 0; i< files.size(); i++)
    {
        if ( files[i] != "." && files[i] != "..")
        {
            org_files.push_back(files[i]);
        }
    }
    return org_files;
}
void update_path(string path,vector <string>& csvs)
{
    for ( int i =0; i< csvs.size(); i ++)
    {
        csvs[i] = path + '/' + csvs[i];
    }
    return;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    string inp;
    cin >> inp;
    vector <string> commands= tokenize(inp,'-');
    vector <string> csv_files = find_csvs(commands[0]);
    vector <int> child_pids;
    update_path(commands[0],csv_files);
    create_clubs_process(csv_files,commands[1],child_pids,commands[2]);
    int count = 0;
    for(auto& pid : child_pids) {
        int status;
        waitpid(pid, &status, 0);
    }
    return 0;
}
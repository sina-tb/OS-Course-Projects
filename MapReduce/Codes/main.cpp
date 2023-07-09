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
#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

#define FIFO "fifo_"
#define WRITE 1
#define READ 0
#define MAX 10000
#define COUNTRY "./country.out"
#define REDUCE "./reduce.out"
#define POSITION_PATH "clubs/positions.csv"
#define COUNTRY_NUMBER 4
#define CLUB_NUMBER 2

vector <string> find_country_dir(string path)
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
        if ( files[i] != "." && files[i] != ".." && files[i] != "positions.csv")
        {
            org_files.push_back(files[i]);
        }
    }
    return org_files;
}
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
vector <string> read_pos_csv()
{
    fstream fin;
    vector<string> row;
    string line, word, temp;

    fin.open(POSITION_PATH, ios::in);

    getline(fin, line);
    return tokenize(line,',');
}
void show_postions(vector <string> pos)
{
    int count = 0;
    cout<<"All positions:"<<endl;
    while (true)
    {
        cout<<pos[count];
        count ++;
        if (pos.size() == count)
        {
            cout<<endl;
            break;
        }
        cout<<" - ";
    }
    cout << "Enter positions to get stats:"<<endl;
    return;
}
void create_fifo(string fifo_name)
{
    unlink(fifo_name.c_str());
    auto name = fifo_name.c_str();
    if(mkfifo(name, 0666 | S_IRWXU) != 0){
        cerr << "Couldn't create fifo!" << endl;
        return;
    }
}
void create_all_fifo(int pos_size,int country_size,int club_size,vector <string>& fifo)
{
    for (int i = 0; i< pos_size; i++)
    {
        for (int j = 0; j< country_size; j++)
        {
            for( int k = 0; k<club_size; k++)
            {
                string fifo_name = FIFO + to_string(i) + to_string(j) + to_string(k);
                create_fifo(fifo_name);
                fifo.push_back(fifo_name);
            }
        }
    }    
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
string country_data(string folder, string positions,int id)
{
    folder = folder + "-" + positions + "-" + to_string(id);
    return folder;
}

void create_countries_process(const vector<string>& folders, string positions,vector <int>& child_pids)
{
    for (int i = 0; i < folders.size(); i++)
    {
        int write_pipe;
        int pid = create_process(write_pipe,COUNTRY);
        string data = country_data(folders[i],positions,i);
        write(write_pipe, data.c_str(), data.length());
        child_pids.push_back(pid);
        close(write_pipe);
    }
}
string position_data(string position,string pos_size,int country_size,int club_size)
{
    string data = pos_size + ',' + to_string(country_size) + ',' + to_string(club_size);
    data = data + "-" + position;
    return data;
}
void create_positions_process(vector <string>& positions,vector <int>& child_pids)
{
    for (int i = 0; i < positions.size(); i++)
    {
        int write_pipe;
        int pid= create_process(write_pipe,REDUCE);
        string data = position_data(positions[i],to_string(i),4,2);
        write(write_pipe, data.c_str(), data.length());
        child_pids.push_back(pid);
        close(write_pipe);
    }
}

string coded_positions(vector <string> positions)
{
    string result = positions[0];
    for (int i=1; i< positions.size(); i++)
    {
        result = result + ',' + positions[i];
    }
    return result;
}

void make_path(vector <string>& countries,string path)
{
    for(int i=0; i< countries.size(); i++)
    {
        countries[i] = path + "/" + countries[i];
    }
    return; 
}
void unlink_all_fifo(vector <string> fifo)
{
    for( int i = 0; i< fifo.size(); i++)
    {
        unlink(fifo[i].c_str());
    }
}
int main(int argc, char const *argv[])
{
    string temp;
    vector <int> child_pids;
    vector <string> fifo;
    string path= argv[1]; 
    vector <string> countries = find_country_dir(path);
    make_path(countries,path);
    vector <string> pos = read_pos_csv();
    show_postions(pos);
    getline(cin, temp);
    vector <string> chosen_pos = tokenize(temp,' ');
    string coded = coded_positions(chosen_pos);
    create_all_fifo(chosen_pos.size(),COUNTRY_NUMBER,CLUB_NUMBER,fifo);
    create_countries_process(countries,coded,child_pids);
    create_positions_process(chosen_pos,child_pids);
    int count = 0;
    for(auto& pid : child_pids) 
    {
        int status;
        waitpid(pid, &status, 0);
    }
    unlink_all_fifo(fifo);
    return 0;
}

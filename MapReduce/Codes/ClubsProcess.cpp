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

string results_to_string(vector <int> age)
{
    string result = to_string(age[0]);
    for (int i=1; i< age.size()-1; i++)
    {
        result = result + "," + to_string(age[i]); 
    }
    result = result + "," + to_string(age[3]);
    return result;
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

void decode_csv(const char name[],vector <string>& position, vector <int>& age){
    vector <string> results;
    string _name = name, line;
    ifstream file(_name);
    while(getline(file, line)){
        results = tokenize(line,',');
        position.push_back(results[1]);
        age.push_back(stoi(results[2]));
    }
    return;
}
vector <int> calc_results(vector <string> position,vector <int> age,string chosen_pos)
{
    vector <int> results{100,0,0,0};
    for (int i = 0; i< position.size(); i++)
    {
        if(position[i] == chosen_pos)
        {
            if (results[0] > age[i])
            {
                results[0] = age[i]; //min
            }
            if (results[1] < age[i])
            {
                results[1] = age[i]; //max
            }
            results[2] += age[i]; //sum
            results[3] += 1; //count
        }
    }
    return results;
}
void write_results(string token, string fifo_name)
{   
    int pipe_fd;
    pipe_fd = open(fifo_name.c_str(), O_WRONLY);
    write(pipe_fd, token.c_str(), token.length());
    close(pipe_fd);
}

void write_all_results(vector <string>& position, vector <int>& age, vector <string> chosen_pos,string process_id)
{
    for (int i=0; i < chosen_pos.size(); i++)
    {
        string answer;
        vector <int> results = calc_results(position,age,chosen_pos[i]);
        if (results[3] != 0)
        {   
            answer = results_to_string(results);
        }
        if (results[3] == 0)
        {
            answer = "nothing";
        }
        string fifo_name = FIFO + to_string(i) + process_id; 
        write_results(answer,fifo_name);
    }
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    string inp;
    cin >> inp;
    vector <string> commands = tokenize(inp,'-');
    vector <string> chosen_pos = tokenize(commands[1],',');
    vector <string> positions;
    vector <int> age;
    decode_csv(commands[0].c_str(),positions,age);
    write_all_results(positions,age,chosen_pos,commands[2]);
    return 0;
}
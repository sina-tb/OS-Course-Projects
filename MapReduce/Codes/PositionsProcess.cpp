#include <stdio.h>
#include <fcntl.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <string>
#include <sys/wait.h>
#include <bits/stdc++.h>

using namespace std;

#define FIFO "fifo_"
#define WRITE 1
#define READ 0
#define MAX 10000
#define COUNTRY "./country.out"
#define REDUCE "./reduce.out"
#define POSITION_PATH "clubs/positions.csv"

vector <int> int_tokenize(string chosen_pos,char delimiter)
{
    stringstream ss(chosen_pos); 
    string s; 
    vector <int> str;
    while (getline(ss, s, delimiter)) {   
        str.push_back(stoi(s));
    }
    return str; 
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

void update_results(vector <int>& results, vector <int> new_data)
{
    if ( results[0] > new_data[0])
    {
        results[0] = new_data[0];
    }
    if ( results[1] < new_data[1])
    {
        results[1] = new_data[1];
    }
    results[2] += new_data[2];
    results[3] += new_data[3];
}

int read_from_pipe(string fifo_name, vector <int>& results)
{
    int fd = open(fifo_name.c_str(),O_RDONLY);
    if (fd == -1)
    {
        cout << "shit";
    }
    char result[MAX];
    memset(result,0,MAX);
    read(fd, result, MAX);
    string temp = result;
    close(fd);
    if (temp != "nothing")
    {   
        update_results(results,int_tokenize(temp,','));
        return 1;
    }
    return 0;
}

void check_all_pipes(string process_id,vector <int>& results,int countries,int clubs){

    int count = 0;
    for(int i = 0; i < countries; i++)
    {
        for(int j=0; j< clubs ; j++)
        {
            string fifo_name = FIFO + process_id + to_string(i) + to_string(j);
            count+=read_from_pipe(fifo_name,results);        
        }
    }
    if (count == 0)
    {
        results[0] = 0;
        results[1] = 0;
        results[2] = 0;
        results[3] = 0;
    }
}

void print_result(vector <int> results,string position)
{
    double var;
    if ( results[2] != 0 && results[3] != 0)
    {
       var = (double)results[2]/(double)results[3];
    }
    else
    {
        var =0;
    }
    cout<<position<< " min : " << results[0]<<endl;
    cout<<position<< " max : " << results[1]<<endl;
    cout<<position<< " avg : ";
    cout << fixed << setprecision(1) << var<<endl;
    cout <<position<<" count: "<<results[3]<<endl;
}

int main()
{
    ios_base::sync_with_stdio(false);
    cin.tie(NULL);
    string inp, section;
    cin >> inp;
    vector <string> commands = tokenize(inp,'-');
    vector <string> numbers = tokenize(commands[0],',');
    vector <int> results {100,0,0,0};
    check_all_pipes(numbers[0],results,stoi(numbers[1]),stoi(numbers[2]));
    print_result(results,commands[1]);
    return 0;
}
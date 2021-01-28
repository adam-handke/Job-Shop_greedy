//Author: Adam Handke
//Problem: Job-shop
//Algorithm: greedy

#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <chrono>
#include <string>
#include <cstring>
using namespace std;

//function which checks if all JOBs are finished
bool check_all_done(vector<int> time_left_whole_job, int num_jobs){
    for(int i=0; i<num_jobs; i++){
        if(time_left_whole_job[i] > 0){
            return false;
        }
    }
    return true;

}

//struct for storing the machine id and time
struct do_what{
    int machine;
    int time;
};

//struct for current state of a JOB
struct state{
    int executes;
    //executes==-1 -> free JOB
    //executes==N  -> JOB is being done on machine N
    int time_left_current_machine;
    //only meaningful when executes>=0
};

//Beasley version
void Beasley(int argc, char *argv[]){
    ifstream fin(argv[1]);

    int num_jobs, num_machines;
    //tmp variables
    int tmp_machine;
    int tmp_time;
    int tmp_job;
    int liczba_uwzgl_JOBow; //4th argv argument

    fin >> num_jobs >> num_machines;

    //auxiliary vector for noting what is currently done on a machine
    vector<int> job_done_by_machine(num_machines);
    fill(job_done_by_machine.begin(), job_done_by_machine.end(), -1);
    //when job_done_by_machine[j]==-1, then nothing is now done on machine j

    if(argc==5){
        string str(argv[4]);
        liczba_uwzgl_JOBow = stoi(str);
        if((liczba_uwzgl_JOBow < num_jobs) && (liczba_uwzgl_JOBow > 0))
            num_jobs=liczba_uwzgl_JOBow;
    }

    //vector of queues with pairs machine-time
    vector<queue<do_what> > job(num_jobs);

    //remaining completion time for every JOB
    vector<int> time_left_whole_job(num_jobs);
    fill(time_left_whole_job.begin(), time_left_whole_job.end(), 0);

    //auxiliary vector of current JOB states
    vector<state> state_job(num_jobs);

    //loading data from input file
    int i,j;
    do_what element;
    for(i=0; i<num_jobs; i++){
        for(j=0; j<num_machines; j++){
            fin >> element.machine >> element.time;
            time_left_whole_job[i]+=element.time;
            state_job[i].executes=-1;
            state_job[i].time_left_current_machine=0;
            job[i].push(element);
        }
    }
    //current in-program time
    int global_time;
    //vector of FIFO queues with times of machine times for a JOB
    vector<queue<int> > solution(num_jobs);

    //auxiliary vector of the id of the closest (the greates time till end) willing JOB for a machine
    vector<int> job_wants_machine(num_machines);
    //when job_wants_machine[i]==-1, then currently no JOB wants a machine

    //jump_time variable - how much time we can currently jump forwards (when next decision is needed)
    //i.e. minimal time remaining for completing a JOB on any machine
    //very important for the main WHILE loop
    int jump_time;

    //GREEDY ALGORITHM
    //Every time when some JOB can be chosen for a machine
    //then a JOB with the greatest time_left_whole_job is chosen
    global_time=0;
    while(!check_all_done(time_left_whole_job, num_jobs)){
        //in every time point the following 3 things happen:
        
        //1. Computing the job_wants_machine vector
        //We start with an assumption that currently no JOB wants a machine now.
        fill(job_wants_machine.begin(), job_wants_machine.end(), -1);
        //check what JOBs want what machines with they do nothing now
        for(i=0; i<num_jobs; i++){
            if(state_job[i].executes==-1 && job[i].size()>0){
                tmp_machine=job[i].front().machine;
                tmp_time=job[i].front().time;
                //omitting a maching which has a 0 exec time (will not be used at all for a JOB)
                while(tmp_time==0 && job[i].size()>0){
                    solution[i].push(global_time);
                    job[i].pop();
                    tmp_machine=job[i].front().machine;
                    tmp_time=job[i].front().time;
                }
                //checking again if sth is in the queue after getting rid of unused machines for a JOB
                if(job[i].size()>0){
                    //regular checking for a regular maching for a JOB
                    //if a machine is busy now, then we ignore it
                    if(job_done_by_machine[tmp_machine]!=-1)
                        continue;
                    else if(job_wants_machine[tmp_machine]==-1)   //if no JOB wants this machine now, then this JOB is immidiately submitted
                        job_wants_machine[tmp_machine]=i;
                    //if there is an other willing JOB for this machine, then we check which JOB has a longer time to complete
                    //THE GREEDY CHOICE:
                    else if(time_left_whole_job[job_wants_machine[tmp_machine]] < time_left_whole_job[i])
                        job_wants_machine[tmp_machine]=i;
                }
            }
        }
        //now we have completed the job_wants_machine vector - we know which JOB goes to which machine now

        //2. We start JOB execution on machines according to the job_wants_machine vector
        for(i=0; i<num_machines; i++){
            tmp_job=job_wants_machine[i];
            if(tmp_job!=-1){
                state_job[tmp_job].executes=i;
                state_job[tmp_job].time_left_current_machine = job[tmp_job].front().time;
                job_done_by_machine[i]=tmp_job;
                solution[tmp_job].push(global_time);
            }
        }

        //2.5 Computing the jump_time
        jump_time=-1;
        for(i=0; i<num_jobs; i++){
            tmp_time = state_job[i].time_left_current_machine;
            if(state_job[i].executes!=-1 && tmp_time>0){
                if(jump_time == -1){
                    jump_time = tmp_time;
                }
                //the smallest possible
                else if(tmp_time < jump_time){
                    jump_time = tmp_time;
                }
            }
        }

        //3. Jumping to the next time point
        global_time+=jump_time;
        for(i=0; i<num_jobs; i++){
            if(state_job[i].executes != -1){
                state_job[i].time_left_current_machine-=jump_time;
                time_left_whole_job[i]-=jump_time;
                if(state_job[i].time_left_current_machine==0){ //zakonczenie danej maszyny
                    tmp_machine = state_job[i].executes;
                    state_job[i].executes = -1;
                    job_done_by_machine[tmp_machine]=-1;
                    job[i].pop();
                }
            }
        }
    }

    //Writing the solution to txt file
    ofstream fout(argv[2]);
    fout << global_time << "\n";
    for(i=0; i<num_jobs; i++){
        for(j=0; j<num_machines; j++){
            fout << solution[i].front() << " ";
            solution[i].pop();
        }
        fout << "\n";
    }
    fin.close();
    fout.close();

    return;
}

void Taillard(int argc, char *argv[]){
    //The only differences between Taillard() and Beasley() are
    //-input format
    //-numbering machines from 1 instead of 0
    //this could have been done in one function with a B/T parameter but it's too late now :)
    ifstream fin(argv[1]);

    int num_jobs, num_machines;
    int i,j;
    int tmp_machine;
    int tmp_time;
    int tmp_job;
    string tmp; //for reading useless junk from Taillard header
    int liczba_uwzgl_JOBow;

    //reading the num of jobs and machines
    fin >> num_jobs >> num_machines;

    vector<int> job_done_by_machine(num_machines+1);
    fill(job_done_by_machine.begin(), job_done_by_machine.end(), -1);

    vector<queue<do_what> > job(num_jobs);

    vector<int> time_left_whole_job(num_jobs);
    fill(time_left_whole_job.begin(), time_left_whole_job.end(), 0);

    vector<state> state_job(num_jobs);

    vector< vector<int> > tmp_times_table(num_jobs);
    for(i=0; i<num_jobs; i++){
        tmp_times_table[i].resize(num_machines+1);
    }
    //loading data
    fin >> tmp >> tmp >> tmp >> tmp; //4 useless numbers from header

    fin >> tmp; //"Times"
    for(i=0; i<num_jobs; i++){
        for(j=1; j<=num_machines; j++){ //special Taillard reading loop
            fin >> tmp_time;
            tmp_times_table[i][j]=tmp_time;
            time_left_whole_job[i]+=tmp_time;
            state_job[i].executes=-1;
            state_job[i].time_left_current_machine=0;
        }
    }

    fin >> tmp; //"Machines"
    do_what element;
    for(i=0; i<num_jobs; i++){
        for(j=1; j<=num_machines; j++){ //special Taillard reading loop
            fin >> element.machine;
            element.time=tmp_times_table[i][j];
            job[i].push(element);
        }
    }

    if(argc==5){
        string str(argv[4]);
        liczba_uwzgl_JOBow = stoi(str);
        if((liczba_uwzgl_JOBow < num_jobs) && (liczba_uwzgl_JOBow > 0))
            num_jobs=liczba_uwzgl_JOBow;
    }
    //the end of reading Taillard data

    //no comments from now on - everthing is the same as in Beasley
    int global_time;

    vector<queue<int> > solution(num_jobs);

    vector<int> job_wants_machine(num_machines+1);

    int jump_time;

    //GREEDY ALGORITHM
    global_time=0;
    while(!check_all_done(time_left_whole_job, num_jobs)){
        //1.
        fill(job_wants_machine.begin(), job_wants_machine.end(), -1);

        for(i=0; i<num_jobs; i++){
            if(state_job[i].executes==-1 && job[i].size()>0){
                tmp_machine=job[i].front().machine;
                tmp_time=job[i].front().time;

                while(tmp_time==0 && job[i].size()>0){
                    solution[i].push(global_time);
                    job[i].pop();
                    tmp_machine=job[i].front().machine;
                    tmp_time=job[i].front().time;
                }

                if(job[i].size()>0){

                    if(job_done_by_machine[tmp_machine]!=-1)
                        continue;
                    else if(job_wants_machine[tmp_machine]==-1)
                        job_wants_machine[tmp_machine]=i;

                    //THE GREEDY CHOICE
                    else if(time_left_whole_job[job_wants_machine[tmp_machine]] < time_left_whole_job[i])
                        job_wants_machine[tmp_machine]=i;
                }
            }
        }

        //2.
        for(i=1; i<=num_machines; i++){
            tmp_job=job_wants_machine[i];
            if(tmp_job!=-1){
                state_job[tmp_job].executes=i;
                state_job[tmp_job].time_left_current_machine = job[tmp_job].front().time;
                job_done_by_machine[i]=tmp_job;
                solution[tmp_job].push(global_time);
            }
        }

        //2.5
        jump_time=-1;
        for(i=0; i<num_jobs; i++){
            tmp_time = state_job[i].time_left_current_machine;
            if(state_job[i].executes!=-1 && tmp_time>0){
            //jesli jump_time jest nadal w wartosci bazowej -1, to zawsze zastepujemy pierwszym lepszym
                if(jump_time == -1){
                    jump_time = tmp_time;
                }
                //jesli juz jump_time != -1, to przypisujemy mu najmniejszy z dostepnych
                else if(tmp_time < jump_time){
                    jump_time = tmp_time;
                }
            }
        }

        //3.
        global_time+=jump_time;
        for(i=0; i<num_jobs; i++){
            if(state_job[i].executes != -1){
                state_job[i].time_left_current_machine-=jump_time;
                time_left_whole_job[i]-=jump_time;
                if(state_job[i].time_left_current_machine==0){ //zakonczenie danej maszyny
                    tmp_machine = state_job[i].executes;
                    state_job[i].executes = -1;
                    job_done_by_machine[tmp_machine]=-1;
                    job[i].pop();
                }
            }
        }
    }

    //writing the results
    ofstream fout(argv[2]);
    fout << global_time << "\n";
    for(i=0; i<num_jobs; i++){
        for(j=1; j<=num_machines; j++){ //special Taillard loop
            fout << solution[i].front() << " ";
            solution[i].pop();
        }
        fout << "\n";
    }
    fin.close();
    fout.close();
    return;
}




int main(int argc, char *argv[]){
//argv values from the console 
//argv[1] - input file in [B]easley or [T]aillard format
//argv[2] - output file
//argv[3] - input data format: B or T
//argv[4] - OPTIONAL, the number of first N tasks from input file which will be used, when void then all are used

    if(argc<4 || (!strcmp(argv[3],"T") && !strcmp(argv[3],"B"))){
        cout << "Error. Try: " << argv[0] << " [input_file.txt] [output_file.txt] [format_(B)easley/(T)aillard] [OPTIONAL_num_of_first_N_tasks]\n";
    }
    else{
        auto start = std::chrono::system_clock::now(); //time measurement - start
        if(!strcmp(argv[3],"B")){
            Beasley(argc, argv);	//Beasley variant
        }
        else{
            Taillard(argc, argv);	//Taillard variant
        }
        auto end = std::chrono::system_clock::now(); // time measurement - end
        cout << "Execution time: " << ((double)(end-start).count())/10e8 << "\n";
    }

    return 0;
}

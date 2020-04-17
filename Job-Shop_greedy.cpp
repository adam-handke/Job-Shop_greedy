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

//funkcja sprawdzajaca, czy wszytkie JOB-y juz sa zakonczone:
bool check_all_done(vector<int> time_left_whole_job, int num_jobs){
    for(int i=0; i<num_jobs; i++){
        if(time_left_whole_job[i] > 0){
            return false;
        }
    }
    return true;

}

//struktura (ktora maszyna i jaki czas) co w swojej kolejce ma teraz zrobic dany JOB
//podstawowa struktura do przechowywania informacji o maszynach w kolejce dla danego JOB-a
struct do_what{
    int machine;
    int time;
};

//struktura aktualnego stanu danego JOB-a
struct state{
    int executes;
    //executes==-1 -JOB WOLNY
    //executes==0 -JOB WYKONUJE MASZYNE 0 itd.
    int time_left_current_machine;
    //ma tylko sens, gdy executes!=0
};

void Beasley(int argc, char *argv[]){
    ifstream fin(argv[1]);

    int num_jobs, num_machines;
    //zmienne pomocnicze:
    int tmp_machine;
    int tmp_time;
    int tmp_job;
    int liczba_uwzgl_JOBow;
    //argument nr 4 wywolania programu
    //ile JOBow od gory uwzglednic w obliczeniach

    fin >> num_jobs >> num_machines;

    //pomocniczy wektor co jest aktualnie wykonywane na danej maszynie:
    vector<int> job_done_by_machine(num_machines);
    fill(job_done_by_machine.begin(), job_done_by_machine.end(), -1);
    //jesli job_done_by_machine[j]==-1, to nic sie teraz tam nie wykonuje na maszynie j

    if(argc==5){
        string str(argv[4]);
        liczba_uwzgl_JOBow = stoi(str);
        if((liczba_uwzgl_JOBow < num_jobs) && (liczba_uwzgl_JOBow > 0))
            num_jobs=liczba_uwzgl_JOBow;
    }

    //wektor kolejek z parami maszyna-czas
    vector<queue<do_what> > job(num_jobs);

    //dla kazdego JOBa caly czas pozostaly do ostatecznego konca
    vector<int> time_left_whole_job(num_jobs);
    fill(time_left_whole_job.begin(), time_left_whole_job.end(), 0);

    //pomocnieczo - wektor aktualnych stanowow danego JOB-a
    vector<state> state_job(num_jobs);


    //wczytywanie danych
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
    //zmienne do rozwiazania - czasy wykonywania poszczegolnych zadan
    int global_time;
    //oraz wektor kolejek FIFO z czasami wykonywania kolejnych maszyn dla danego JOB-a
    vector<queue<int> > solution(num_jobs);

    //pomocniczy wektor numeru najlepszego (najwieszky czas do konca) chetnego JOB-a na dana maszyne
    vector<int> job_wants_machine(num_machines);
    //jesli job_wants_machine[i]==-1, to w danym momencie zaden JOB nie chce maszyn

    //zmienna jump_time - o ile przeskoczymy czasu, ze bedzie potrzebna kolejna decyzja
    //tzn minimalny czas pozostaly z wykonywania wszystkich aktualnych JOB-ow na aktuyalnych maszynach
    //wykorzystane w punktach 2 i 3 ponizszej pentli while
    int jump_time;

    //ALGORYTM ZACHLANNY (GREEDY)
    //Zawsze gdy moze dobrac JOB-a do wykoniania na danej maszynie
    //to wybiera tego, ktory ma najwiekszy czas time_left_whole_job
    //czyli swoj czas do wykonania wszystkich pozostalych maszyn
    global_time=0;
    while(!check_all_done(time_left_whole_job, num_jobs)){
        //w kazdej chwili czasu wykonuje nastepujace czynnosci:
        //1. zaczynam od zalozenia, ze obecnie zaden JOB nie chce zadnej maszyny
        fill(job_wants_machine.begin(), job_wants_machine.end(), -1);
        //sprawdzam jakie JOBy chca jakie maszyny, jesli nic nie wykonuja teraz
        for(i=0; i<num_jobs; i++){
            if(state_job[i].executes==-1 && job[i].size()>0){
                tmp_machine=job[i].front().machine;
                tmp_time=job[i].front().time;
                //mechanizm pominiecia maszyny, ktora ma miec czas wykonywania 0 (czyli nie byc wcale wykonywana dla danego JOB-a):
                while(tmp_time==0 && job[i].size()>0){
                    solution[i].push(global_time);
                    job[i].pop();
                    tmp_machine=job[i].front().machine;
                    tmp_time=job[i].front().time;
                }
                //ponowne sprawdzenie, czy cokolwiek jest w kolejce po pozbyciu sie nieuzywanych maszyn dla danego JOB-a
                if(job[i].size()>0){
                    //normalne sprawdzanie dla normalnie zadanej maszyny dla JOB-a
                    //jesli ta maszyna obecnie jest zajeta, to mamy gdzies, ze JOB ja chce i idziemy dalej:
                    if(job_done_by_machine[tmp_machine]!=-1)
                        continue;
                    else if(job_wants_machine[tmp_machine]==-1)   //jesli zaden JOB nie chce teraz tej maszyny, to od razu ten JOB jest wpisany jako chetny
                        job_wants_machine[tmp_machine]=i;
                    //jesli juz jakis chce, to sprawdzamy, ktory JOB ma dluzej do wykonania calosci
                    //KLUCZOWY element algorytmu:
                    else if(time_left_whole_job[job_wants_machine[tmp_machine]] < time_left_whole_job[i])
                        job_wants_machine[tmp_machine]=i;
                }
            }
        }
        //teraz mamy juz uzupelniony wektor job_wants_machine tak, by wiedziec co pojdzie do danej maszyny teraz

        //2. Zaczynamy wykonywanie maszyn przez JOBY zgodnie z powyzszym wektorem
        for(i=0; i<num_machines; i++){
            tmp_job=job_wants_machine[i];
            if(tmp_job!=-1){
                state_job[tmp_job].executes=i;
                state_job[tmp_job].time_left_current_machine = job[tmp_job].front().time;
                job_done_by_machine[i]=tmp_job;
                solution[tmp_job].push(global_time);
            }
        }

        //2.5 dopasowanie czasu jump_time do nastepnego zakonczenia jakiegos wykonywania
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

        //3. Przejscie do kolejnej chwili czasu:
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

    //wypisanie rozwiazania do pliku:
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
    //jedyne roznice miedzy Taillard() i Beasley() to:
    //-format wejscia
    //-zmiany dotyczace numeracji maszyn od 1 (petle, wielkosci wektorow itp)
    ifstream fin(argv[1]);

    int num_jobs, num_machines;
    int i,j;
    //zmienne pomocnicze:
    int tmp_machine;
    int tmp_time;
    int tmp_job;
    string tmp; //do wczytywania smieci z naglowka formatu Taillarda
    int liczba_uwzgl_JOBow;
    //argument nr 4 wywolania programu
    //ile JOBow od gory uwzglednic w obliczeniach

    //wczytanie liczby zadan i maszyn
    fin >> num_jobs >> num_machines;

    //pomocniczy wektor co jest aktualnie wykonywane na danej maszynie:
    vector<int> job_done_by_machine(num_machines+1);
    fill(job_done_by_machine.begin(), job_done_by_machine.end(), -1);
    //jesli job_done_by_machine[j]==-1, to nic sie teraz tam nie wykonuje na maszynie j

    //wektor kolejek z parami maszyna-czas
    vector<queue<do_what> > job(num_jobs);

    //dla kazdego JOBa caly czas pozostaly do ostatecznego konca
    vector<int> time_left_whole_job(num_jobs);
    fill(time_left_whole_job.begin(), time_left_whole_job.end(), 0);

    //pomocniczo - wektor aktualnych stanowow danego JOB-a
    vector<state> state_job(num_jobs);


    vector< vector<int> > tmp_times_table(num_jobs); //tabelka do tymczasowego przechowania tabeli Times
    for(i=0; i<num_jobs; i++){
        tmp_times_table[i].resize(num_machines+1);
    }
    //wczytywanie danych
    fin >> tmp >> tmp >> tmp >> tmp; //4 niepotrzebne liczby z naglowka
    //tabela Times
    fin >> tmp; //"Times"
    for(i=0; i<num_jobs; i++){
        for(j=1; j<=num_machines; j++){ //zmieniona petla dla Taillarda
            fin >> tmp_time;
            tmp_times_table[i][j]=tmp_time;
            time_left_whole_job[i]+=tmp_time;
            state_job[i].executes=-1;
            state_job[i].time_left_current_machine=0;
        }
    }
    //tabela Machines i wrzucenie danych do kolejek
    fin >> tmp; //"Machines"
    do_what element; //pomocniczy do wczytywania do kolejek
    for(i=0; i<num_jobs; i++){
        for(j=1; j<=num_machines; j++){ //zmieniona petla dla Taillarda
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
    //koniec wczytywania danych

    //zmienne do rozwiazania - czasy wykonywania poszczegolnych zadan
    int global_time;
    //oraz wektor kolejek FIFO z czasami wykonywania kolejnych maszyn dla danego JOB-a
    vector<queue<int> > solution(num_jobs);

    //pomocniczy wektor numeru najlepszego (najwieszky czas do konca) chetnego JOB-a na dana maszyne
    vector<int> job_wants_machine(num_machines+1);
    //jesli job_wants_machine[i]==-1, to w danym momencie zaden JOB nie chce maszyn

    //zmienna jump_time - o ile przeskoczymy czasu, ze bedzie potrzebna kolejna decyzja
    //tzn minimalny czas pozostaly z wykonywania wszystkich aktualnych JOB-ow na aktuyalnych maszynach
    //wykorzystane w punktach 2 i 3 ponizszej pentli while
    int jump_time;

    //ALGORYTM ZACHLANNY (GREEDY)
    //Zawsze gdy moze dobrac JOB-a do wykoniania na danej maszynie
    //to wybiera tego, ktory ma najwiekszy czas time_left_whole_job
    //czyli swoj czas do wykonania wszystkich pozostalych maszyn
    global_time=0;
    while(!check_all_done(time_left_whole_job, num_jobs)){
        //w kazdej chwili czasu wykonuje nastepujace czynnosci:
        //1. zaczynam od zalozenia, ze obecnie zaden JOB nie chce zadnej maszyny
        fill(job_wants_machine.begin(), job_wants_machine.end(), -1);
        //sprawdzam jakie JOBy chca jakie maszyny, jesli nic nie wykonuja teraz
        for(i=0; i<num_jobs; i++){
            if(state_job[i].executes==-1 && job[i].size()>0){
                tmp_machine=job[i].front().machine;
                tmp_time=job[i].front().time;
                //mechanizm pominiecia maszyny, ktora ma miec czas wykonywania 0 (czyli nie byc wcale wykonywana dla danego JOB-a):
                while(tmp_time==0 && job[i].size()>0){
                    solution[i].push(global_time);
                    job[i].pop();
                    tmp_machine=job[i].front().machine;
                    tmp_time=job[i].front().time;
                }
                //ponowne sprawdzenie, czy cokolwiek jest w kolejce po pozbyciu sie nieuzywanych maszyn dla danego JOB-a
                if(job[i].size()>0){
                    //normalne sprawdzanie dla normalnie zadanej maszyny dla JOB-a
                    //jesli ta maszyna obecnie jest zajeta, to mamy gdzies, ze JOB ja chce i idziemy dalej:
                    if(job_done_by_machine[tmp_machine]!=-1)
                        continue;
                    else if(job_wants_machine[tmp_machine]==-1)   //jesli zaden JOB nie chce teraz tej maszyny, to od razu ten JOB jest wpisany jako chetny
                        job_wants_machine[tmp_machine]=i;
                    //jesli juz jakis chce, to sprawdzamy, ktory JOB ma dluzej do wykonania calosci
                    //KLUCZOWY element algorytmu:
                    else if(time_left_whole_job[job_wants_machine[tmp_machine]] < time_left_whole_job[i])
                        job_wants_machine[tmp_machine]=i;
                }
            }
        }
        //teraz mamy juz uzupelniony wektor job_wants_machine tak, by wiedziec co pojdzie do danej maszyny teraz

        //2. Zaczynamy wykonywanie maszyn przez JOBY zgodnie z powyzszym wektorem
        for(i=1; i<=num_machines; i++){
            tmp_job=job_wants_machine[i];
            if(tmp_job!=-1){
                state_job[tmp_job].executes=i;
                state_job[tmp_job].time_left_current_machine = job[tmp_job].front().time;
                job_done_by_machine[i]=tmp_job;
                solution[tmp_job].push(global_time);
            }
        }

        //2.5 dopasowanie czasu jump_time do nastepnego zakonczenia jakiegos wykonywania
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

        //3. Przejscie do kolejnej chwili czasu:
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

    //wypisanie rozwiazania do pliku:
    ofstream fout(argv[2]);
    fout << global_time << "\n";
    for(i=0; i<num_jobs; i++){
        for(j=1; j<=num_machines; j++){ //zmieniona petla dla Taillarda
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
//argumenty z linii polecen:
//argv[1] - plik z danymi w formacie Beasleya lub Taillarda
//argv[2] - plik do zapisu wyniku
//argv[3] - format danych wejsciowych: B lub T
//argv[4] - OPCJONALNY, liczba pierwszych uwzglednionych JOBow z danych, gdy nie jest podany, to uwzglednia wszystkie

    if(argc<4 || (!strcmp(argv[3],"T") && !strcmp(argv[3],"B"))){
        cout << "Blad. Sprobuj: " << argv[0] << " [input_file.txt] [output_file.txt] [format_(B)easley/(T)aillard] [OPCJONALNE_liczba_uwzgl_pierwszych_JOBow]\n";
    }
    else{
        auto start = std::chrono::system_clock::now(); //pomiar czasu - start
        if(!strcmp(argv[3],"B")){
            Beasley(argc, argv);	//wariant formatu Beasleya
        }
        else{
            Taillard(argc, argv);	//wariant formatu Taillarda
        }
        auto end = std::chrono::system_clock::now(); // pomiar czasu - koniec
        cout << "Execution time: " << ((double)(end-start).count())/10e8 << "\n";
    }

    return 0;
}

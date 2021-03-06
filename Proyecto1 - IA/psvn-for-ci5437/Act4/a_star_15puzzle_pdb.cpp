#include "priority_queue.hpp"
#include <fstream>
#include <sys/time.h>
#include <string>
#include <sstream>

using namespace std;

struct node {
    state_t state;
    int d;
    int hist;
};

bool limit = false;
long long nodes_generated = 0;
time_t begint,endt;
state_map_t *pdb1;
state_map_t *pdb2;       
state_map_t *pdb3;
abstraction_t *abs1;
abstraction_t *abs2;
abstraction_t *abs3;

//Funcion heuristica usando PDB

int pdb(state_t);

void a_star_expand(node expand, PriorityQueue<node> &q) {
    int ruleID;
    ruleid_iterator_t iter;
    state_t child;
    init_fwd_iter(&iter, &expand.state);
    int f;

    while ((ruleID = next_ruleid(&iter)) >= 0){
        if (!fwd_rule_valid_for_history(expand.hist, ruleID)) continue;
        node child;
        child.hist = next_fwd_history(expand.hist, ruleID);
        apply_fwd_rule(ruleID, &expand.state, &child.state);
        child.d = expand.d + 1;
        nodes_generated++;
        f = child.d + pdb(child.state);
        q.Add(f, f, child);
    }
}

int a_star(state_t state, int hist, int d) {
    PriorityQueue<node> q;
    int f = d + pdb(state);
    node to_expand;
    state_t child;
    double secs;
    
    to_expand.state = state;
    to_expand.hist = hist;
    to_expand.d = d;

    q.Add(f, f, to_expand);

    while(!q.Empty()) {
        
        endt = time(NULL);
        secs = difftime(endt,begint);
        if(secs > 300) {    // 10 minutes
            limit = true;
            return 0;
        }

        to_expand = q.Top();
        q.Pop();

        if (is_goal(&to_expand.state)) return to_expand.d;
        a_star_expand(to_expand, q);
    }

    limit = true;
    return 0;

}


int main(int argc, char* argv[]) {
    
    state_t root;
    int aux;
    string state_string;
    char stateArray[256];
    int hist;
    clock_t begin, end;
    double elapsed_secs;
    double gen_per_sec;

    abs1 = read_abstraction_from_file("./PDB/pdb1.abst");
    abs2 = read_abstraction_from_file("./PDB/pdb2.abst");
    abs3 = read_abstraction_from_file("./PDB/pdb3.abst");

    FILE *f = fopen ("./PDB/pdb1.pdb" , "r"); 
    pdb1 = read_state_map(f);
    fclose (f);
    FILE *f2 = fopen ("./PDB/pdb2.pdb" , "r"); 
    pdb2 = read_state_map(f2);
    fclose (f2);

    FILE *f3 = fopen ("./PDB/pdb3.pdb" , "r"); 
    pdb3 = read_state_map(f3);
    fclose (f3);

    ifstream file (argv[1]);
    std::string path = argv[1];
    ofstream outfile (argv[2]);
    outfile << "grupo, algorithm, heuristic, domain, instance, cost, h0, generated, time, gen_per_sec\n";


    if (file.is_open()) {

        while (getline(file, state_string)) {
            strncpy(stateArray, state_string.c_str(), sizeof(stateArray));
            stateArray[sizeof(stateArray) - 1] = 0;
            hist = init_history;
            read_state(stateArray, &root);
            nodes_generated++;

            begin = clock();
            begint = time(NULL);
            aux = a_star(root, hist, 0);
            end = clock();

            elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
            gen_per_sec = double(nodes_generated)/elapsed_secs;

            if (!limit){
                outfile << "x, a_star, pdb, ";
                outfile << path.substr(path.find_last_of("\\/")+1,path.find_last_of(".")) << ", ";
                outfile << "'" << stateArray << "', ";
                outfile << aux << ", ";
                outfile << pdb(root) << ", ";
                outfile << nodes_generated << ", ";
                outfile << elapsed_secs << ", ";
                outfile << gen_per_sec << endl;
            }

            else {
                outfile << "x, a_star, pdb, ";
                outfile << path.substr(path.find_last_of("\\/")+1,path.find_last_of(".")) << ", ";
                outfile << "'" << stateArray << "', ";
                outfile << "na, na, na, na, na "<< endl;
                limit = false; 
            }

            nodes_generated = 0;
        }

        file.close();
        outfile.close();
    }
}

int pdb(state_t state){

	state_t abstrac1;
	state_t abstrac2;
	state_t abstrac3;
	abstract_state(abs1,&state,&abstrac1);
	abstract_state(abs2,&state,&abstrac2);
	abstract_state(abs3,&state,&abstrac3);

    // Calculo del costo de cada pdb
	const int *value1 = state_map_get(pdb1,&abstrac1); 
	const int *value2 = state_map_get(pdb2,&abstrac2);
	const int *value3 = state_map_get(pdb3,&abstrac3);  

	return *value1+*value2+*value3;
}


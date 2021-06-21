#include "ampl/ampl.h"
#include <chrono>
#include <cstdio>
#include <forward_list>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stack>
#include <string>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;

constexpr double TIME_LIMIT = 300; // Tiempo límite de cada ejecución (segundos)


struct Vertice {
    vector<int> destinos;
    bool visited;
    unsigned int pos;

    Vertice() : visited(false), pos(1) {}
    ~Vertice() {}
};

unordered_map<string, bool> connections(unordered_map<int, Vertice>& arcs, int vertice)
{
    stack<int> nodos;
    unordered_map<string, bool> conjunto;

    nodos.push(vertice);
    arcs[vertice].visited = true;
    conjunto.emplace(to_string(vertice), true);

    while (!nodos.empty()) {
        int origen = nodos.top();
        int destino = 0;
        while (destino != -1) {
            destino = -1;
            if (!arcs[origen].visited) {
                nodos.push(origen);
                arcs[origen].visited = true;
                conjunto.emplace(to_string(origen), true);
            }

            Vertice& v = arcs[origen]; //Instantiated only for readibility sake 
            for (unsigned i = 0; i < v.destinos.size(); ++i) {
                unsigned indx = (v.pos - 1) % v.destinos.size();
                if (!arcs.count(v.destinos[indx])) {
                    origen = v.destinos[indx];
                    ++(v.pos);
                    break;
                }
                if (arcs[v.destinos[indx]].visited) {
                    ++(v.pos);
                    continue;
                }
                else {
                    destino = v.destinos[indx];
                    origen = destino;
                    ++(v.pos);
                    break;
                }
            }
        }
        if (!conjunto.count(to_string(origen)))
            conjunto.emplace(to_string(origen), true);
        nodos.pop();
    }
    return conjunto;
}

string ver2str(unordered_map<string, bool> nodos)
{
    string conjunto("{");
    while (!nodos.empty()) {
        nodos.size() == 1 ? conjunto += nodos.begin()->first + "}" :
            conjunto += nodos.begin()->first + ", ";
        nodos.erase(nodos.begin());
    }
    return conjunto;
}

string extract_arcs(unordered_map<int, Vertice> arcos, unordered_map<string, bool> subTour) {
    string conjunto("{");
    for(auto it = subTour.begin(); it != subTour.end(); ++it) {
        int indx = stoi(it->first);
        int aux = -1;
        for(auto it2 = arcos[indx].destinos.begin(); it2 != arcos[indx].destinos.end(); ++it2){
            if( *it2 == aux ) continue;
            char buff[20];
            snprintf(buff, 20, "(%d,%d),", indx, *it2);
            string pareja(buff);
            conjunto += pareja;
            aux = *it2;
        }
    }
    conjunto.pop_back();
    conjunto += "}";
    return conjunto;
}

int getNumArcs(unordered_map<string, bool> subTour, unordered_map<int, Vertice> arcs) {
    unsigned int numArcs = 0;
    for (auto it = subTour.begin(); it != subTour.end(); ++it) {
        unsigned int indx = stoi(it->first);
        int destino = arcs[indx].destinos.front();
        ++numArcs;
        for (auto it2 = arcs[indx].destinos.begin(); it2 != arcs[indx].destinos.end(); ++it2) {
            if(*it2 == destino) continue;
            destino = *it2;
            ++numArcs;
        }
    }
    return numArcs;
}

//Subtour Constraints templates
string EAST_OUT(int numConstraint, string subtour_arcs, string nodeSet, int numArcs) {
    char r[10000]; //Corresponde al número de caracteres de la plantilla de la restricción
    /*snprintf(r, 1000, "s.t. r%d:sum{i in %s, j in Nodes: i<>j and not j in %s} x[i,j] >= 1;",
        numConstraint, nodeSet.c_str(), nodeSet.c_str());*/
    snprintf(r, 10000,
        "s.t. r%d:sum{(i,j) in %s}w[i,j]-sum{i in %s,k in Nodes:i<>k and not k in %s}w[i,k]<=%d-1;",
        numConstraint, subtour_arcs.c_str(), nodeSet.c_str(), nodeSet.c_str(), numArcs);
    string constraint(r);
    return constraint;
}

string EAST_IN(int numConstraint, string subtour_arcs, string nodeSet, int numArcs) {
    char r[10000]; //Corresponde al número de caracteres de la plantilla de la restricción
    /*snprintf(r, 1000, "s.t. r%d:sum{i in %s, j in Nodes: i<>j and not j in %s} x[i,j] >= 1;",
        numConstraint, nodeSet.c_str(), nodeSet.c_str());*/
    snprintf(r, 10000,
        "s.t. r%d:sum{(i,j) in %s}w[i,j]-sum{k in Nodes, i in %s:i<>k and not k in %s}w[k,i]<=%d-1;",
        numConstraint, subtour_arcs.c_str(), nodeSet.c_str(), nodeSet.c_str(), numArcs);
    string constraint(r);
    return constraint;
}

void saveStat( const string& instanceName, vector<float> stats ) {
    ofstream log( "Resultados.txt", std::ios_base::app);
    log << instanceName << ' ';
    for ( auto it = stats.begin(); it != stats.end(); ++it )
        log << setprecision(4) << *it << ' ';
    log << "\n";
    log.close();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        cout << "ERROR: Archivo '.dat' de instancia no especificado\n";
        cout << "Uso de comando: datparse [nombre_archivodat]\n";
        return 1;
    }
    try {
        ampl::AMPL ampl;
        //Paths to files
        string datPath = argv[1];
        string instanceName;
        unsigned pos = datPath.find_last_of("/\\");
        if( pos == string::npos ) instanceName = argv[1];
        else instanceName = datPath.substr(pos + 1);
        datPath += "/SCPM.dat";

        //PERMITIR FLEXIBILIDAD DE SELECCIÓN SOLVER_______________________
        ampl.setOption("solver", "cplex");
        ampl.setBoolOption("presolve", true);

        ampl.setOption("cplex_options", "outlev 1 timelimit 5 return_mipgap 3"); //mipdisplay 2 timelimit 20

        forward_list<float> weights;
        weights.push_front(0.8);
        weights.push_front(0.5);
        weights.push_front(0.2);

        for( auto weight = weights.begin(); weight != weights.end();){
            ampl.read("SCPM.mod");
            ampl.readData(datPath);
            bool existSubtour = true;
            int k = 1, iter = 0; // Used to enumerate the constraints without repeating
            double time_exec;

            // Variables needed to save the important data
            float CTP, RAP, gap, obj, estado;
            vector<float> stats;

            // CHanging the value of parameter "Beta"
            ampl::Parameter beta = ampl.getParameter("Beta");
            float b = weights.front();
            beta.set(b);

            auto t1 = chrono::high_resolution_clock::now();

            do{
                // Solve
                //ampl.solve();
                ampl.getOutput("solve;");

                // Save ampl status
                ampl::DataFrame status = ampl.getData("solve_result_num");
                ampl::DataFrame::iterator it_status = status.begin();
                estado = float((*it_status)[0].dbl());

                // Get the value of the variables and export it to c++ container
                ampl::Variable x_ampl = ampl.getVariable("x");
                ampl::DataFrame df_x = x_ampl.getValues();
                int originNode;
                unordered_map<int, Vertice> arcos;

                for (ampl::DataFrame::iterator it = df_x.begin(); it != df_x.end(); ++it) {
                    int indx1 = int((*it)[0].dbl());
                    int indx2 = int((*it)[1].dbl());
                    int var_value = int((*it)[2].dbl());

                    if (indx1 == 0 && var_value != 0) {
                        //cout << "Nodo inicial: " << indx2 << endl;
                        originNode = indx2;
                        continue;
                    }

                    if (var_value != 0) {
                        if (arcos.count(indx1))
                            arcos[indx1].destinos.insert(arcos[indx1].destinos.end(), var_value, indx2);
                        else {
                            Vertice v;
                            v.destinos.insert(v.destinos.end(), var_value, indx2);
                            arcos.emplace(indx1, v);
                        }
                        //cout << indx1 << " -> " << indx2 << endl;
                    }
                }
                
                //Ruta inicial
                connections(arcos, originNode);
                forward_list<tuple<string, string, int>> constraints;

                //Obtener subtoures
                for (auto it = arcos.begin(); it != arcos.end(); ++it) {
                    if (!it->second.visited) {
                        unordered_map<string, bool> subTour = connections(arcos, it->first);
                        string subtour_arcos = extract_arcs(arcos, subTour);
                        string formattedSubtour = ver2str(subTour);
                        int numArcos = getNumArcs(subTour, arcos);
                        constraints.push_front(make_tuple(subtour_arcos, formattedSubtour, numArcos));
                    }
                }
                
                for (auto it = constraints.begin(); it != constraints.end(); ++it, ++k) {
                    string out = EAST_OUT(k, get<0>(*it), get<1>(*it), get<2>(*it));
                    ampl.eval(out);
                }
                
                auto t2 = chrono::high_resolution_clock::now();
                auto duration = chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() / 1000.0;

                if (duration >= TIME_LIMIT) {
                    existSubtour = false;
                    time_exec = duration;
                }
                if (constraints.empty()) {
                    existSubtour = false;
                    time_exec = duration;
                }

                arcos.clear();
                constraints.clear();
                ++iter;
            } while (existSubtour);
            
            // Obtain CTP indicator
            ampl::DataFrame kpi = ampl.getData("sum{(i,j) in Arcs}C[j]*x[i,j] / NADIR_COST");
            ampl::DataFrame::iterator it_kpi = kpi.begin();
            CTP = float((*it_kpi)[0].dbl());

            // Obtain RAP indicator
            kpi = ampl.getData("sum{(i,j) in Arcs}R[j]*x[i,j] / NADIR_RISK");
            it_kpi = kpi.begin();
            RAP = float((*it_kpi)[0].dbl());
            

            // Get objective entity by AMPL name
            ampl::Objective Fo = ampl.getObjective("z");
            // Save objective value and gap value
            obj = Fo.value();
            gap = Fo.getValues("relmipgap").getRowByIndex(0)[0].dbl();

            // Save ampl status
            stats.push_back(CTP);
            stats.push_back(RAP);
            stats.push_back(b);
            stats.push_back(obj);
            stats.push_back(gap);
            stats.push_back(estado);
            stats.push_back(k);
            stats.push_back(iter);
            stats.push_back(time_exec);
            saveStat(instanceName, stats);
            stats.clear();
            weights.pop_front();
            weight = weights.begin();
            ampl.reset();
        }

        ampl.close();
        return 0;
    }
    catch (const exception& e) {
        cout << "Error: " << e.what() << "\n";
        cin.get();
        return 1;
    }
}
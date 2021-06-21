#include <cmath>
#include <list>
#include <fstream>
#include <iostream>
#include <map>

using std::cout;
using std::cin;
using std::endl;
using std::list;
using std::string;
using std::map;

constexpr int rock_cost = -1;

int main(int argc, char** argv) {

    if (argc < 2) {
        cout << "ERROR: Mapa de costos de instancia no especificado\n";
        cout << "Uso de comando: datparse [nombre_instancia]\n";
        return 1;
    }

    int id = 0;
    map<int, int> costmap;
    string instancePath = argv[1];
    std::ifstream costInfo;
    costInfo.open(instancePath + "\\costmap.txt");
    do {
        int num;
        costInfo >> num >> std::ws;
        costmap.emplace(++id, num);
    } while (!costInfo.eof());

    int n = (int)sqrt(id);
    costInfo.close();

    std::ifstream riskInfo;
    riskInfo.open(instancePath + "\\riskmap.txt");
    map<int, int> riskmap;
    id = 0;
    do {
        int num;
        riskInfo >> num >> std::ws;
        riskmap.emplace(++id, num);
    } while (!riskInfo.eof());
    riskInfo.close();

    list<list<int>> adyacencia, covertura;
    for (auto it = costmap.begin(); it != costmap.end(); ++it) {
        if (it->second == rock_cost) continue;

        list<int> ady_aux, cov_aux;
        ady_aux.push_back(it->first);
        cov_aux.push_back(it->first);

        id = it->first - 1;
        if (costmap.count(id) && costmap[id] != rock_cost && id % n > 0) {
            ady_aux.push_back(id);
            cov_aux.push_back(id);
        }

        id = it->first + 1;
        if (costmap.count(id) && costmap[id] != rock_cost && it->first % n > 0) {
            ady_aux.push_back(id);
            cov_aux.push_back(id);
        }

        id = it->first - n;
        if (costmap.count(id) && costmap[id] != rock_cost) {
            ady_aux.push_back(id);
            cov_aux.push_back(id);
        }

        id = it->first + n;
        if (costmap.count(id) && costmap[id] != rock_cost) {
            ady_aux.push_back(id);
            cov_aux.push_back(id);
        }

        id = it->first - n - 1;
        if (costmap.count(id) && costmap[id] != rock_cost && id % n > 0) cov_aux.push_back(id);

        id = it->first - n + 1;
        if (costmap.count(id) && costmap[id] != rock_cost && it->first % n > 0) cov_aux.push_back(id);

        id = it->first + n + 1;
        if (costmap.count(id) && costmap[id] != rock_cost && it->first % n > 0) cov_aux.push_back(id);

        id = it->first + n - 1;
        if (costmap.count(id) && costmap[id] != rock_cost && id % n > 0) cov_aux.push_back(id);

        adyacencia.push_back(ady_aux);
        covertura.push_back(cov_aux);
        ady_aux.clear();
        cov_aux.clear();
    }

    std::ofstream f;
    f.open(instancePath + "\\SCPM.dat");

    f << "set rocks := ";
    for (auto it = costmap.begin(); it != costmap.end(); ++it)
        if (it->second == rock_cost) f << it->first << " ";
    f << ";\n\n";

    f << "param N := " << n * n << ";\n\n";

    f << "param COV :=\n";
    for (auto it = covertura.begin(); it != covertura.end(); ++it) {
        int node = it->front();
        for (auto j = it->begin(); j != it->end(); ++j)
            f << node << " " << *j << "\t1\n";
    }
    f << ";\n\n";

    f << "param A :=\n";
    for (auto it = adyacencia.begin(); it != adyacencia.end(); ++it) {
        int node = it->front();
        list<int>::iterator j = it->begin();
        ++j;
        for (; j != it->end(); ++j)
            f << node << " " << *j << "\t1\n";
    }
    f << ";\n\n";

    int nadirCost = 0;
    f << "param C :=\n";
    for (auto it = costmap.begin(); it != costmap.end(); ++it) {
        f << it->first << " ";
        if (it->second == rock_cost) f << ".\n";
        else{ 
            f << it->second << endl;
            nadirCost += it->second;
        }
    }
    f << ";\n\n";

    int nadirRisk = 0;
    f << "param R :=\n";
    for (auto it = riskmap.begin(); it != riskmap.end(); ++it){
        f << it->first << " "; 
        if( costmap.count(it->first) && costmap[it->first] == rock_cost )
            f << ".\n";
        else f << it->second << endl;
        nadirRisk += it->second;
    }
    f << ";\n\n";

    f << "param NADIR_COST := " << nadirCost << ";\n";
    f << "param NADIR_RISK := " << nadirRisk << ";";

    f.close();

    return 0;
}
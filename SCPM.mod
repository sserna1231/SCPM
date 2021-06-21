param N integer > 2; # Numero de nodos
param side integer := sqrt(N);
param Beta;
param NADIR_COST;
param NADIR_RISK;
param UTOPIC_COST;
param UTOPIC_RISK;

set cells ordered := {1..N}; # Conjunto de nodos ordenados
set rocks;
set Nodes within cells := {i in cells: i not in rocks};
set Arcs := {i in Nodes union {0}, j in Nodes: i<>j}; # Conjunto de arcos formados por la union de 2 nodos
set Borders within Nodes := {i in Nodes: i <= side or i >= side*(side - 1) or 
	((i mod side) == 0) or ((i mod side) == 1)};

param C{Nodes};
param R{Nodes};
param A{Arcs} default 0;
param COV{Nodes, Nodes} default 0; #Un parámetro que modela tanto la conectividad de aydacencia como el cubrimiento

var x{Arcs} >= 0 integer; # Variable que indica si el arco (i,j) se establece
var y{Nodes} binary; #Variable que determina el inicio de la ruta
var w{Arcs} binary; #Variable que indica si el arco (i,j) se establece al menos una vez

#Minimiza el costo y el riesgo total de viaje
minimize z:((sum{(i,j) in Arcs}C[j]*x[i,j]-UTOPIC_COST)/(NADIR_COST-UTOPIC_COST))*Beta+((sum{(i,j) in Arcs}R[j]*x[i,j]-UTOPIC_RISK)/(NADIR_RISK-UTOPIC_RISK))*(1-Beta);

s.t.
adyacencia{(i,j) in Arcs: i<>0}: x[i,j] <= A[i,j]*w[i,j]*N;
activar_arco{(i,j) in Arcs: i<>0}: x[i,j] >= w[i,j];
origin{p in Borders}: sum{j in Nodes: j<>p}x[p,j] - sum{i in Nodes: i<>p}x[i,p] >= 2*y[p]-1;
visit_origin{p in Nodes}: x[0,p] = y[p];
single_origin: sum{p in Borders} y[p] = 1;
flow{k in Nodes}: sum{j in Nodes: j<>k}x[k,j] - sum{i in Nodes: i<>k}x[i,k] <= y[k];
no_border_exception: sum{k in Nodes: k not in Borders} y[k] = 0;
covering{k in Nodes}: sum{(i,j) in Arcs} COV[k,j]*x[i,j] >= 1;
pairs{(i,j) in Arcs: i<>0}: w[i,j] + w[j,i] - sum{k in Nodes: k<>j and k<>i} w[i,k] - sum{k in Nodes: k<>i and k<>j} w[j,k] <= 1;
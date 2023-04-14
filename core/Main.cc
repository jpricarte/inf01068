#include <iostream>
#include <cmath>
#include "core/Converter.h"
#include "core/Solver.h"
#include "core/SolverTypes.h"
#include "mtl/Vec.h"

using namespace Minisat;
using namespace std;


int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "usage: ./minisat_static LOCKED_INPUT_INDEX < AIG_FILE" << endl;
        exit(1);
    }
    int locked_input_index = atoi(argv[1]);
    Aig aig_instance = aig_from_file(cin);
    Cnf cnf_instance_true = aig_to_cnf(aig_instance);
    Cnf cnf_instance_false = aig_to_cnf(aig_instance, cnf_instance_true.n_vars);
    Cnf cnf_instance = merge_cnf_instances(cnf_instance_true, cnf_instance_false);
    for (int i=0; i<cnf_instance_true.inputs.size(); i++) {
        if (i == locked_input_index)
            continue;
        auto equality_clauses = make_vars_equal_clauses(cnf_instance_true.inputs[i], cnf_instance_false.inputs[i]);
        for (auto clause : equality_clauses) {
            cnf_instance.clauses.push_back(clause);
        }
        cnf_instance.n_clauses += 2;
    }
    for (int i=0; i<int(cnf_instance_true.outputs.size()); i++) {
        auto xor_clauses = create_xor_clauses(abs(cnf_instance_true.outputs[i]), abs(cnf_instance_false.outputs[i]), cnf_instance.n_vars);
        for (auto clause : xor_clauses) {
            cnf_instance.clauses.push_back(clause);
        }
        cnf_instance.n_vars++;
        cout << "XOR OUTPUT: " << cnf_instance.n_vars << endl;
        cnf_instance.clauses.push_back(CnfClause{{cnf_instance.n_vars}});
        cnf_instance.n_clauses += 5;
    }
    cout << locked_input_index << " " << cnf_instance_true.inputs[locked_input_index] << endl;

    cnf_instance.clauses.push_back(CnfClause{{cnf_instance_true.inputs[locked_input_index]}});
    cnf_instance.clauses.push_back(CnfClause{{-cnf_instance_false.inputs[locked_input_index]}});
    cnf_instance.n_clauses += 2;

    // Here, the CNF is ready, now we can pass it as input to the SAT SOLVER or return a .cnf content
    Solver solver{};
    vector<Var> sat_vars{};
    for (int i=0; i<cnf_instance.n_vars; i++) {
        sat_vars.push_back(solver.newVar());
    }
    // Add clauses
    for (auto clause : cnf_instance.clauses) {
        vec<Lit> sat_clause{};
        for (auto cnf_lit : clause.lits) {
            bool sign = cnf_lit < 0;
            int index = get_var_index_from_lit(cnf_lit);
            Lit sat_lit = mkLit(sat_vars[index], sign);
            sat_clause.push(sat_lit);
        }
        solver.addClause(sat_clause);
    }
    solver.simplify();
    bool ret = solver.solve();
    cout << (ret == true ? "// SATISFIABLE" : "// UNSATISFIABLE") << endl;
    
    if (ret)
        for (int i=0; i<cnf_instance.n_vars; i++) {
            auto var_value = solver.model[i];
            cout << i+1 << " = " << ( var_value == l_True ? "True" : var_value == l_False ? "False" : "Undeterminated") << endl;
        }
    else {
        cout << cnf_to_file_content(cnf_instance) << endl;
    }


    return 0;
}
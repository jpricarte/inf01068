#ifndef CONVERTER_H
#define CONVERTER_H

#include <iostream>
#include <vector>

struct AigClause {
    int and_port;
    int input_1;
    int input_2;
};

struct Aig {
    int n_vars;
    int n_input;
    int n_output;
    int n_and;
    std::vector<int> inputs;
    std::vector<int> outputs;
    std::vector<AigClause> ands;
};

Aig aig_from_file(std::istream& input);

struct CnfClause {
    std::vector<int> lits;
};

struct Cnf {
    int n_vars;
    int n_clauses;
    std::vector<CnfClause> clauses;
    std::vector<int> inputs;
    std::vector<int> outputs;
};

int aig_to_cnf_var(int aig_var, int offset=0);

std::vector<CnfClause> aig_to_cnf_clause(AigClause& aig_clause, int offset=0);

Cnf aig_to_cnf(Aig& aig_instance, int offset=0);

std::string cnf_to_file_content(Cnf& cnf_instance);

Cnf merge_cnf_instances(Cnf& instance_1, Cnf& instance_2);

std::vector<CnfClause> create_xor_clauses(int input_1, int input_2, int offset=0);

std::vector<CnfClause> make_vars_equal_clauses(int input_1, int input_2);

int get_var_index_from_lit(int lit);


#endif // CONVERTER_H
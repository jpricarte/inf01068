#include <iostream>
#include <vector>

using namespace std;

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
    vector<int> inputs;
    vector<int> outputs;
    vector<AigClause> ands;
};

Aig aig_from_file(istream& input) {
    string m_s{}, i_s{}, l_s{}, o_s{}, a_s{}, dummy{};
    input >> dummy >> m_s >> i_s >> l_s >> o_s >> a_s;
    int n_vars = stoi(m_s), n_input = stoi(i_s), n_output = stoi(o_s), n_and = stoi(a_s);
    
    vector<int> inputs(n_input, 0);
    for (int i=0; i<n_input; i++) {
        cin >> dummy;
        inputs[i] = stoi(dummy);
    }

    vector<int> outputs(n_output, 0);
    for (int i=0; i<n_output; i++) {
        cin >> dummy;
        outputs[i] = stoi(dummy);
    }

    vector<AigClause> ands(n_and);
    for (int i=0; i<n_and; i++) {
        cin >> dummy;
        int and_port = stoi(dummy);
        cin >> dummy;
        int input_1 = stoi(dummy);
        cin >> dummy;
        int input_2 = stoi(dummy);
        ands[i] = AigClause{and_port, input_1, input_2};
    }
    return Aig{n_vars, n_input, n_output, n_and, inputs, outputs, ands};
}

struct CnfClause {
    vector<int> lits;
};

struct Cnf {
    int n_vars;
    int n_clauses;
    vector<CnfClause> clauses;
    vector<int> inputs;
    vector<int> outputs;
};

int aig_to_cnf_var(int aig_var, int offset=0) {
    return (aig_var%2==0) ? int(aig_var/2)+offset : -int((aig_var-1)/2)-offset;
}

vector<CnfClause> aig_to_cnf_clause(AigClause& aig_clause, int offset=0) {
    int port = aig_to_cnf_var(aig_clause.and_port, offset);
    int a = aig_to_cnf_var(aig_clause.input_1, offset);
    int b = aig_to_cnf_var(aig_clause.input_2, offset);
    vector<CnfClause> output{};
    output.push_back(CnfClause{{-port, a}});
    output.push_back(CnfClause{{-port, b}});
    output.push_back(CnfClause{{port, -a, -b}});
    return output;
}

Cnf aig_to_cnf(Aig& aig_instance, int offset=0) {
    vector<CnfClause> cnf_clauses{};
    for (auto& aig_clause : aig_instance.ands) {
        for (auto& cnf_clause : aig_to_cnf_clause(aig_clause, offset)) {
            cnf_clauses.push_back(cnf_clause);
        }
    }
    vector<int> converted_inputs{};
    for (auto aig_var : aig_instance.inputs) {
        converted_inputs.push_back(aig_to_cnf_var(aig_var, offset));
    }
    vector<int> converted_outputs{};
    for (auto aig_var : aig_instance.outputs) {
        converted_outputs.push_back(aig_to_cnf_var(aig_var, offset));
    }
    return Cnf{aig_instance.n_vars, int(cnf_clauses.size()), cnf_clauses, converted_inputs, converted_outputs};
}

string cnf_to_file_content(Cnf& cnf_instance) {
    string output = "p cnf " + to_string(cnf_instance.n_vars) + " " + to_string(cnf_instance.n_clauses) + "\n";
    for (auto& clause : cnf_instance.clauses) {
        for (auto lit : clause.lits) {
            output += to_string(lit) + " ";
        }
        output += "0\n";
    }
    return output;
}

void change_cnf_var(Cnf& cnf_instance, int old_var, int new_var) {
    for (auto& clause : cnf_instance.clauses) {
        for (int i=0; i<clause.lits.size(); i++) {
            if (clause.lits[i] == old_var) {
                clause.lits[i] = new_var;
            }
        }
    }
}

Cnf merge_cnf_instances(Cnf& instance_1, Cnf& instance_2) {
    Cnf new_instance = Cnf {instance_1.n_vars + instance_2.n_vars, 
                            instance_1.n_clauses + instance_2.n_clauses, 
                            instance_1.clauses, 
                            instance_1.outputs
                            };
    new_instance.clauses.insert(new_instance.clauses.end(), instance_2.clauses.begin(), instance_2.clauses.end());
    new_instance.outputs.insert(new_instance.outputs.end(), instance_2.outputs.begin(), instance_2.outputs.end());
    return new_instance;
}

// Create 4 new clauses and 1 new var
vector<CnfClause> create_xor_clauses(int input_1, int input_2, int offset=0) {
    int output_1 = offset+1;
    vector<CnfClause> clauses{}; // initialize lits empty
    clauses.push_back({{-input_1, -input_2, -output_1}});
    clauses.push_back({{-input_1, input_2, output_1}});
    clauses.push_back({{input_1, -input_2, output_1}});
    clauses.push_back({{input_1, input_2, -output_1}});
    return clauses;
}


vector<CnfClause> make_vars_equal_clauses(int input_1, int input_2) {
    vector<CnfClause> clauses{}; // initialize lits empty
    clauses.push_back({{-input_1, input_2}});
    clauses.push_back({{input_1, -input_2}});
    return clauses;
}


int main(int argc, char** argv) {
    if (argc != 2) {
        cerr << "usage: aig2cnf LOCKED_INPUT_INDEX < AIG_FILE" << endl;
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
    for (int i=0; i<cnf_instance_true.outputs.size(); i++) {
        auto xor_clauses = create_xor_clauses(cnf_instance_true.outputs[i], cnf_instance_false.outputs[i], cnf_instance.n_vars);
        for (auto clause : xor_clauses) {
            cnf_instance.clauses.push_back(clause);
        }
        cnf_instance.n_vars++;
        cnf_instance.n_clauses += 4;
    }
    cnf_instance.clauses.push_back(CnfClause{{cnf_instance_true.inputs[locked_input_index]}});
    cnf_instance.clauses.push_back(CnfClause{{-cnf_instance_false.inputs[locked_input_index]}});
    cnf_instance.n_clauses += 2;
    cout << cnf_to_file_content(cnf_instance);
    return 0;
}
class Solution {
public:
    void remove_duplicates(vector<string>& vec) {
        sort(vec.begin(), vec.end());
        vec.erase(unique(vec.begin(), vec.end()), vec.end());
    }
    vector<string> solve(int &idx, int &n, string &ex){
        stack<string> s;
        int start = idx;
        while (idx < n && ex[idx] != '}'){
            if (isalpha(ex[idx])){
                string word = "";
                while (idx < n && isalpha(ex[idx])){
                    word.push_back(ex[idx]);
                    idx++;
                }
                if (s.empty() || s.top() == "+"){
                    s.push(word);
                    continue;
                }
                vector<string> b;
                while (!s.empty() && s.top() != "+"){
                    b.push_back(s.top() + word);
                    s.pop();
                }
                for (auto &a: b){
                    s.push(a);
                }
            }
            else if (ex[idx] == ','){
                s.push("+");
                idx++;
            }
            else if (ex[idx] == '{'){
                char op = '+';
                if (idx - 1 >= start && ex[idx - 1] != ',' && ex[idx - 1] != '{'){
                    op = '*';
                }
                idx++;
                vector<string> res = solve(idx, n, ex);
                if (op == '+'){
                    while (!res.empty()){
                        s.push(res.back()), res.pop_back();
                    }
                }
                else {
                    vector<string> b;
                    while (!s.empty() && s.top() != "+"){
                        for (auto &a: res){
                            b.push_back(s.top() + a);
                        }
                        s.pop();
                    }
                    for (auto &a: b){
                        s.push(a);
                    }
                }
                idx++;
            }
        }
        vector<string> nright;
        while (!s.empty()){
            string c = s.top(); s.pop();
            if (c == "+"){
                continue;
            }
            nright.push_back(c);
        }
        remove_duplicates(nright);
        return nright;
    }
    vector<string> braceExpansionII(string ex) {
        int n = ex.size(), i = 0;
        vector<string> res = solve(i, n, ex);
        sort(res.begin(), res.end(), [](auto &a, auto &b){
            for (int i = 0; i < min(a.size(), b.size()); i++){
                if (a[i] != b[i]){
                    return a[i] < b[i];
                }
            }
            return a.size() < b.size();
        });
        return res;
    }
};
class Solution {
public:
    vector<int> findSubtreeSizes(vector<int>& parent, string s) {
        int n = parent.size();

        vector<vector<int>>old_adj(n), new_adj(n);

        for(int i=1;i<n;i++) {
            old_adj[parent[i]].push_back(i);
        }

        unordered_map<char, vector<int>>node;
        function<void(int)>dfs = [&](int cur) {
            // we are at node cur 
            // neared ancestor whose value is s[cur]

            if(node[s[cur]].empty()) {
                if(cur > 0) new_adj[parent[cur]].push_back(cur);
            } else {
                new_adj[node[s[cur]].back()].push_back(cur);
            }

            node[s[cur]].push_back(cur);

            for(auto u:old_adj[cur]) {
                dfs(u);
            }

            node[s[cur]].pop_back();
        };

        dfs(0);

        vector<int>ans(n);
        function<void(int)>dfs1 = [&](int cur) {
            ans[cur] = 1;

            for(auto u:new_adj[cur]) {
                dfs1(u);
                ans[cur] += ans[u];
            }
        };

        dfs1(0);

        return ans;
    }
};
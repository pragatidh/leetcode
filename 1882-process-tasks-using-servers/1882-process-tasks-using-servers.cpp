class Solution {
public:
    vector<int> assignTasks(vector<int>& servers, vector<int>& tasks) {
        int n = servers.size();
        int m = tasks.size();

        vector<int> ans(m);

        // Available servers: {weight, index}
        priority_queue<pair<int, int>,
                       vector<pair<int, int>>,
                       greater<pair<int, int>>> pq;

        // Busy servers: {completion_time, index}
        priority_queue<pair<int, int>,
                       vector<pair<int, int>>,
                       greater<pair<int, int>>> tim;

        for (int i = 0; i < n; i++) {
            pq.push({servers[i], i});
        }

        long long t = 0;

        for (int i = 0; i < m; i++) {

            t = max(t, (long long)i);

            // Release all servers that have completed
            while (!tim.empty() && tim.top().first <= t) {
                int ind = tim.top().second;
                tim.pop();

                pq.push({servers[ind], ind});
            }

            // If no server is available, jump to earliest completion
            if (pq.empty()) {
                t = tim.top().first;

                while (!tim.empty() && tim.top().first <= t) {
                    int ind = tim.top().second;
                    tim.pop();

                    pq.push({servers[ind], ind});
                }
            }

            // Select the best available server
            auto p = pq.top();
            pq.pop();

            int weight = p.first;
            int ind = p.second;

            ans[i] = ind;

            // Mark server busy
            tim.push({t + tasks[i], ind});
        }

        return ans;
    }
};
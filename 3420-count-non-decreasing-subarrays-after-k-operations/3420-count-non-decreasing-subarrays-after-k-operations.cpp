class Solution {
public:
    long long countNonDecreasingSubarrays(vector<int>& A, long long k) {
        reverse(A.begin(), A.end());
        long long res = 0;
        deque<int> q;
        for (int j = 0, i = 0; j < A.size(); ++j) {
            while (!q.empty() && A[q.back()] < A[j]) {
                int r = q.back();
                q.pop_back();
                int l = q.empty() ? i - 1 : q.back();
                k -= 1L * (r - l) * (A[j] - A[r]);
            }
            q.push_back(j);
            while (k < 0) {
                k += A[q.front()] - A[i];
                if (q.front() == i) {
                    q.pop_front();
                }
                ++i;
            }
            res += j - i + 1;
        }
        return res;
    }
};
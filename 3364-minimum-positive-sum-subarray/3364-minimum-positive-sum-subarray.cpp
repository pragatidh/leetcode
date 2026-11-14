class Solution {
public:
    int minimumSumSubarray(vector<int>& nums, int l, int r) {
        int n = nums.size();
        int ans = INT_MAX;

        for (int i = 0; i < n; i++) {
            int sum = 0;

            for (int k = i; k < n && k < i + r; k++) {
                sum += nums[k];

                int len = k - i + 1;

                if (len >= l && sum > 0) {
                    ans = min(ans, sum);
                }
            }
        }

        return ans == INT_MAX ? -1 : ans;
    }
};
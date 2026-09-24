class Solution {
public:
    int minOperations(vector<int>& nums, int x) {
        int n = nums.size();

        int total = 0;
        for (int num : nums) total += num;

        if (total < x) return -1; 
        else if (total == x) return n; 

        int start = 0;
        int currSum = 0;
        int maxLen = 0;
        const int TARGET = total - x;

        for (int end = 0; end < n; end++) {
            currSum += nums[end];

            while (currSum > TARGET) {

                currSum -= nums[start++];
            }

            if (currSum == TARGET) {
                maxLen = max(maxLen, end - start + 1);
            }
        }

        if (maxLen == 0) return -1; 

        return n - maxLen;
    }
};
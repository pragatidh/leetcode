class Solution {
public:
    int minimizeSum(vector<int>& nums) {
        int a = nums.size();
        ranges::sort(nums);
        return min({nums[a - 1] - nums[2], nums[a - 2] - nums[1],
                    nums[a - 3] - nums[0]});
    }
};
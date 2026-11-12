class Solution {
public:
    int longestMonotonicSubarray(vector<int>& nums) {
        int increasing = nums[0];
        int ans = 1;
        int ans1 = 1;

        for(int i=1;i<nums.size();i++){
            if(increasing < nums[i]){
                ans1++;
                increasing = nums[i];
                ans = max(ans,ans1);
            }
            else{
                ans1 = 1;
                increasing = nums[i];
            }
        }

        int decreasing = nums[0];
        int ans2 = 1;

        for(int i=1;i<nums.size();i++){
            if(decreasing > nums[i]){
                ans2++;
                decreasing = nums[i];
                ans = max(ans,ans2);
            }
            else{
                ans2 = 1;
                decreasing = nums[i];
            }
        }

        return ans;
    }
};
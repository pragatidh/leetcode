class Solution {
public:
    int numOfSubarrays(vector<int>& arr) {
        int MOD=1e9+7;
        long long cnt=0;
        int even=1,odd=0;
        int sum=0;
    
        
        for(auto& i:arr){
            sum+=i;
            if(sum%2){
                cnt+=even;
                odd++;
            }
            else{
                cnt+=odd;
                even++;
            }
            
            cnt%=MOD;
        }
        return cnt;
    }
};
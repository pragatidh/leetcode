class Solution {
public:
    string maxSumOfSquares(int num, int sum) {
        string ans = "";

        if(sum > num * 9) return "";

        for(int i = 9; i >= 0; i--) {
            while(sum >= i && num > 0 && i > 0) {
                ans += to_string(i);
                sum -= i;
                num--;
            }
        }

        while(num > 0) {
            ans += "0";
            num--;
        }

        return ans;
    }
};
class Solution {
private:
    bool f(int i,int j,int b,vector<vector<char>>& grid,vector<vector<vector<int>>>& dp){
        int n=grid.size();
        int m=grid[0].size();
        if(i>=n || j>=m) return false;

        if(grid[i][j]=='(') b++;
        else
            b--;

        if(b<0) return false;
        if(i==n-1 && j==m-1) return b==0;
        if(dp[i][j][b]!=-1) return dp[i][j][b];


        bool right=false,down=false;
        
        right=f(i,j+1,b,grid,dp);
        down=f(i+1,j,b,grid,dp);

        return dp[i][j][b]=right || down;
    }
public:
    bool hasValidPath(vector<vector<char>>& grid) {
        int n=grid.size();
        int m=grid[0].size();
        if(grid[0][0]==')' || grid[n-1][m-1]=='(')
            return false;

        if((n+m-1)%2) return false;

        vector<vector<vector<int>>>dp(n,vector<vector<int>>(m,vector<int>(n+m,-1)));
        return f(0,0,0,grid,dp);
    }
};
class Solution
{
public:
    pair<int, int> p[4] = {{1, 0}, {0, 1}, {-1, 0}, {0, -1}};
    int countIslands(vector<vector<int>> &grid, int k)
    {
        int row = grid.size();
        int col = grid[0].size();
        int count = 0;
        for (int i = 0; i < row; i++)
        {
            for (int j = 0; j < col; j++)
            {
                if (grid[i][j] != 0)
                {
                    if (dfs(i, j, grid, row, col) % k == 0)
                        count++;
                }
            }
        }
        return count;
    }
    long dfs(int i, int j, vector<vector<int>> &grid, int row, int col)
    {
        long sum = 0;
        sum += grid[i][j];
        grid[i][j] = 0;
        for (const auto &it : p)
        {
            int xnew = i + it.first;
            int ynew = j + it.second;
            if (xnew >= 0 && xnew < row && ynew >= 0 && ynew < col && grid[xnew][ynew] != 0)
            {

                sum += dfs(xnew, ynew, grid, row, col);
            }
        }
        return sum;
    }
};
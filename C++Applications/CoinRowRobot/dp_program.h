// COPYRIGHT Thomas Peterson 2025
#ifndef DP_PROGRAM_H
#define DP_PROGRAM_H

#include <string>
#include <vector>
#include <algorithm>
#include <queue>
#include <stack>

using std::vector;
using std::max; // - TP
using std::reverse; // - TP



typedef struct coin_row_solution{
    int val;
    std::vector< int > coin_indices;

} CRS;

/*
 * Input: vector of coins in order
 * Output: a coin_row_solution, the val set to the value of the optimal solution
 *         the coin_indices (0-indexed) set to the indices of the coins forming the optimal solution
 * Must be done with Dynamic Programming -- no points for recursive solution
 */

 //SOLVE_COIN_ROW
// So the goal is to opimize coin pickup. Like mentioned in class the two conditions being:
// 1. No two Adjacent coins are picked. 2. The total value is maximized.
// So the process is:
//  1. Define the structure that will store final values, coin indexes, and total number of coins.
//  2. Define the two base cases.
//  3. Inti first two places.
//  4. Fill the entire DP table
//  5. Backtrack and record coin indices.
//  6. Output reverse of backtrack (aka turn it the right way round) and set needed final variables.

CRS solve_coin_row(std::vector< int> coin_row){
  CRS soln; // Thing that sotres final value and coin indexes.

  int n = coin_row.size();
  if (n == 0) return {0, {}}; // If no coins, val is 0, there are no indices.
  if (n == 1) return {coin_row[0], {0}}; // If there is a coin, take it.

  vector<int> dp(n);
  vector<bool> take(n, false);

  dp[0] = coin_row[0];
  dp[1] = max(coin_row[0], coin_row[1]);
  take[0] = true;
  take[1] = coin_row[1] > coin_row[0];

  for (int i = 2; i < n; ++i) {
    if (dp[i-1] > dp[i-2] + coin_row[i]) {
      dp[i] = dp[i-1];
      take[i] = false;
    } else {
      dp[i] = dp[i-2] + coin_row[i];
      take[i] = true;
    }
  }

  vector<int> indices; // Backtrack to take coin incides.
  for (int i = n - 1; i >= 0;) {
    if (take[i]) {
      indices.push_back(i);
      i -= 2;
    } else {
      i -= 1;
    }
  }

  reverse(indices.begin(), indices.end()); // std::reverse(...) - From Algorithm library. Reverses the elements in the inputed range.
  soln.val = dp[n - 1];
  soln.coin_indices = indices;

  return soln;

}


/*
 * Robot coin pickup dynamic programming soln
 *
 */

enum Move {Right,Down};

typedef struct robot_coin_solution{
    int n;
    std::vector<Move> moves;
}RCS;

/*
 * Input: vector<vector<bool > > coins : (inner vector is x/column direction, index like a matrix)
 * Output: an RCS (above): n is the max number of coins, moves is a vector of moves
 *    the vector of moves must take the robot from the upper left to the lower right
 *    of course, the moves must also allow the robot to pick up the maximum number of coins
 *    think of the vector of moves that if followed from index 0, 1, ... give a the plan
 *    for the robot starting at the upper left (0,0)
 */

// SOLVE_ROBOT_COIN
// We did the manual version of this in class. Reference the handout for assistnace.
// Just fill out the entire table with the value of each move, and backtrack (from the end) following the most valuable path.
// Algo Process:
//  1. Define variables, vecotrs, etc etc.
//  2. Set up first spot properties, see first col, see first row, fill out rest of table
//  3. Work from finish to start using most valuable path
//  4. Store reverse path for proper solution.
//  5. Assign solution values as needed.

RCS solve_robot_coin(vector<vector<bool> > coins){
  if (coins.empty() || coins[0].empty()) {
    return {0, {}};
  }

  int m = coins.size(); // # rows
  int n = coins[0].size(); // # cols
  vector<vector<int>> dp(m, vector<int>(n));

  dp[0][0] = coins[0][0] ? 1 : 0; // Starting spot

  for (int i = 1; i < m; ++i) { // Column
    dp[i][0] = dp[i-1][0] + (coins[i][0] ? 1 : 0);
  }
  for (int j = 1; j < n; ++j) { // Row
    dp[0][j] = dp[0][j-1] + (coins[0][j] ? 1 : 0);
  }

  for (int i = 1; i < m; ++i) { // Repeat above for rest of table
    for (int j = 1; j < n; ++j) {
      dp[i][j] = max(dp[i-1][j], dp[i][j-1]) + (coins[i][j] ? 1 : 0);
    }
  }

  vector<Move> path; // Backtrack from end to beginning.
  int i = m - 1, j = n - 1;
  while (i > 0 || j > 0) {
    if (i == 0) {
      path.push_back(Right); // push_back() - Used to append an element to the end of a vector.
                             // For this function it's figuring out how it can best get to the finally by storing either a down or a right.
      --j;
    } else if (j == 0) {
      path.push_back(Down);
      --i;
    } else if (dp[i-1][j] >= dp[i][j-1]) {
      path.push_back(Down);
      --i;
    } else {
      path.push_back(Right);
      --j;
    }
  }
  reverse(path.begin(), path.end()); // Reverse() - See above.

  RCS soln;
  soln.n = dp[m-1][n-1];
  soln.moves = path;

  return soln;
}

#endif //DP_PROGRAM_H

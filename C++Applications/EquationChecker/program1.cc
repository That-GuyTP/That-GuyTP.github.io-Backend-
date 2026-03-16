// COPYRIGHT 2024 Thomas Peterson
// I apologize if my code seems a bit messy in some places. Had to do a lot of debugging. Couldn't seem to locate the issue myself.
#include<iostream>
using std::cout;
using std::cin;
using std::endl;

// CHECK IF NUMBER IS PRIME
// This function first checks to see if a number is equal to 1 before moving on to calculate if it can be divided by any iteration of 2 to the power of the inputed number.
bool checkPrime(int num) {
	if (num <= 1) {
		return false;
	}// EOC IF
	for (int i = 2; i * i <= num; i++ ) {
		if (num % i == 0) {
			return false;
		}// EOC INNER IF
	}// EOC FOR
	return true;
}// EOC checkPrime


// PRIME FACTORIZATION
// This function takes the number which prime factors are being checked and calculates what they are.
void primeFactor(int num, int& prime, int& exp) {
  int i = 2;
  exp = 0;
  while (num > 1 && i <= num) {
    exp = 0;
    while (num % i == 0) {
      num /= i;
      exp++;
    }// EOC INNER WHILE
    if (exp > 0) {
      prime = i;
      return;
    }// EOC IF
    i++;
  }// EOC WHILE
}// EOC primeFactor


// MAIN FUNCTION
int main() {
  int num;
  char op;
  int pos_neg;

  // READ INPUT
  cin >> num >> op >> pos_neg;
  
  // CHECK VALID FORMAT
  if (op != '=' || (pos_neg != 1 && pos_neg != -1)) {
    cout << "Invalid input format." << endl;
    return 0;
  }// EOC IF

  int userNum = pos_neg * num; // CALC FULL NUMBER
  bool correct = true;

  // FACTORIZATION LOOP
  while (userNum != 1) {
    int prime = 0, exp = 0;
    primeFactor(abs(userNum), prime, exp); // CALL PRIME FACTORIZATION FUNCTION

    int userPrime = 0, userExp = 0;
    cin >> userPrime >> userExp;

    // CHECK IF FACTOR IS PRIME
    if (!checkPrime(userPrime)) {
      correct = false;
      break;
    }

    // DIVIDE NUMBER BY FACTOR
    for (int i = 0; i < userExp; i++) {
      if (userNum % userPrime != 0) {
        correct = false;
        break;
      }// EOC IF
      userNum /= userPrime;
    }// EOC FOR

    // CHECK NEXT INPUT
    if (cin.peek() == '*') {
      cin.ignore();
    } else if (cin.peek() == '?') {
      cin.ignore();
      break;
    } else {
        correct = false;
        break;
    }// EOC IF-ELSEIF-ELSE
  }// EOC WHILE

  // CHECK INPUT PROCESS
  if (userNum != 1) {
    correct = false;
  }// EOC IF

  // OUTPUT
  if (correct) {
  	cout << "Correct!" << endl;
	} else {

    // SHOW CORRECT FACTORS
  	cout << "Incorrect. " << num << " = ";
    if (num < 0) {
      cout << "-1 * ";
      num = -num;
    } else {
      cout << "1 * ";
    }// EOC INNER IF-ELSE
  
    // GET CORRECT PRIME FACTORS
    bool first = true;
    while (num > 1) {
      int prime = 0, exp = 0;
      primeFactor(num, prime, exp);
      if (!first) cout << " * ";
      cout << prime << "^" << exp;
      first = false;
      for (int i = 0; i < exp; i++) {
        num /= prime;
      }// EOC FOR
    }// EOC WHILE

  }// EOC IF-ELSE
  cout << endl;

  return 0; // END PROGRAM
}// EOC MAIN

  


#include <iostream>
#include <ofstream>

#include "card.h"

void Card::cardRegistration() {
  std::cout << "Card Registration\n";
  std::cout << "Name: ";
  std::cin >> name;
  std::cout << "\nMobile Number: ";
  std::cin >> mobileNum;
  std::cout << "\nAddress: ";
  std::getline(std::cin, address);
  std::cout << "\nEmail Address: ";
  std::getline(std::cin, emailId);
  srand(time(NULL));
  std::cout << "Generating Randomized Card Number ... \n";
  cardNum = rand() % 400000 + 4000000;
  if (cardNum < 0) cardNum *= -1;
  std::cout << "Card Number: " << cardNum;
  std::ofstream fout{"card.dat", std::ios::out | std::ios::app};
  fout << name << " " << mobileNum << " " << address << " " << emailId << " ";
  fout << cardNum;
  std::cout << "Registration Complete. Thank you!";
}
#include <iostream>
#include <fstream>

#include "card.h"

Card::Card() {
  std::cout << "Thank you for your interest in Card Membership!\n";
  std::cout << "Thankyou. Your process has initiated.";
  std::cout << "The card will be mailed to you soon.";
  std::cout << "Please see the benefits"
              "of the "
              "card on the next page. -->";
}

void Card::registration() {
  std::cout << "Card Registration\n Name:";
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

void Card::benefits() {
  // TODO: Display benefits of a card
}

static bool isCardValid(uid_t cardnumber) {
  return true;
}
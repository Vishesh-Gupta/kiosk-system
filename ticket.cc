#include "ticket.h"

#include <iostream>
#include <string>

void hidePassword() {
  char password[3], vh;
  int h;
  puts("Enter the CVV/CVV2");
  while (1) {
    if (h < 0) h = 0;
    vh = getch();
    if (vh == 13) break;
    if (vh == 8) {
      putch(NULL);
      putch(NULL);
      putch(NULL);
      h--;
      continue;
    }
    password[h++] = vh;
    vh = '*';
    putch(vh);
  };
  password[h] == '\0';
}

void Ticket::createTicket() {
  std::cout << "==================================\n";
  std::cout << "Ticket: ";
  std::cout << "==================================\n";
  std::cout << "\n Name 		 :" << name;
  std::cout << "\n Contact Number:" << contactNum;
  std::cout << "\n Show Time     :" << timingOption;
  std::cout << "==================================\n";
}

void creditPurchase(int ticketPrice) {
  time_t t = time(NULL);
  tm* timePtr = localtime(&t);
  std::cout << "Paying :" << ticketPrice << "\n";
  int expirymm, expiryyy;
  std::string name{}, cardNum{};
  std::cout << "Name of the card holder";
  std::cin >> name;
  std::cout << "Enter the card number";
  std::cin >> cardNum;
  std::cout << "Expiry(MM/YYYY)";
  std::cin >> expirymm;
  std::cout << "/";
  std::cin >> expiryyy;
  while (expirymm < (timePtr->tm_mon) || expiryyy < (timePtr->tm_year + 1900)) {
    if (expirymm <= (timePtr->tm_mon)) {
      std::cout << "Enter the month again";
      std::cin >> expirymm;
    }
    if (expiryyy < (timePtr->tm_year + 1900)) {
      std::cout << "Please enter a valid year";
      std::cin >> expiryyy;
    }
  };
}

void Ticket::payment(int numtickets) {
  int showClass, ticketPrice, amount, id, cardId;
  char dtCard{'n'}, cardResponse{'n'};
  std::fstream fin{"card.dat", std::ios::in | std::ios::app};
  std::cout << "Seating Type: \n 1.Normal Class \n 2. Gold Class";
  std::cin >> showClass;
  switch (showClass) {
    case 1:
      std::cout << "You selected for a Normal show";
      ticketPrice = 400;
      break;
    case 2:
      std::cout << "You selected for the Gold Class";
      ticketPrice = 700;
      break;
  }
  amount = ticketPrice * numtickets;
  std::cout << "\n Do you have DTcard(y/n)\n";
  std::cin >> dtCard;
  if (dtCard == 'y') {
    fin.read((char*)&v, sizeof(v));
    std::cout << "Enter the card number";
    if (cardId == id) {
      ticketPrice *= 0.9;
    };
  }
  std::cout << "Want to pay by Card(y/n)";
  if (cardResponse == 'y' || cardResponse == 'Y') {
    creditPurchase(ticketPrice);
  }
}

void Ticket::ticketBooking(int timeingOption) {
  int numTickets{};
  std::cout << "Name: \n";
  std::cin >> name;
  std::cout << "\n Contact Number: \n";
  std::cin >> contactNum;
  std::cout << "\n Number of Tickets: \n";
  std::cin >> numTickets;
  payment(numTickets);
  createTickets();
}
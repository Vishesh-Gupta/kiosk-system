#include "ticket.h"
#include "card.h"
#include <iostream>
#include <string>

#define NORMAL_SEAT 400;
#define GOLD_SEAT 700;
#define VIP_SEAT 1000;

void KioskTicket::generate_ticket(KioskTicket t) {

  std::cout << "==================================\n";
  std::cout << "Ticket: ";
  std::cout << "==================================\n";
  std::cout << "\n Name 		 :" << t.name;
  std::cout << "\n Contact Number:" << t.contactNum;
  std::cout << "\n Show Time     :" << t.showtime;
  std::cout << "==================================\n";
}


int KioskTicket::getTicketCost(int numtickets) {
  int cost{};
  uuid_t cardNumber{};
  int seatType;

  std::cout << "What kind of seating would you like? The options are";
  std::cin >> seatType;

  if (seatType == 0) { cost = NORMAL_SEAT }
  else if (seatType == 1) { cost = GOLD_SEAT; }
  else { cost = VIP_SEAT; }

  if (Card::isCardValid(cardNumber) == true) {
    cost = cost - (cost * CARD_DISCOUNT);
  }

  return cost;

}

void KioskTicket::ticketBooking(int numTickets, int timeingOption) {
  

  std::vector<KioskTicket> tickets;
  for (int i = 0; i < numTickets; i++) {
    KioskTicket t;
    tickets.push_back(t);
  }
}
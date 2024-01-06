#ifndef __TICKET_H__
#define __TICKET_H__

#include <string>

int CARD_DISCOUNT = 0.1;

class KioskTicket {
private:
  std::string name{};
  long unsigned int contactNum{};
  std::string showtime{};

  void generate_ticket(KioskTicket);

public:
  KioskTicket();
  int getTicketCost(int);
  void ticketBooking(int, int);
};

#endif
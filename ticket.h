#ifndef __TICKET_H__
#define __TICKET_H__

#include <string>

class Ticket {
  std::string name{};
  long unsigned int contactNum{};

 public:
  Ticket();
  void createTicket();
  void payment(int);
  void ticketBooking(int);
};

#endif
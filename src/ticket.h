#ifndef __TICKET_H__
#define __TICKET_H__

#include <string>

class KioskTicket {
private:
  std::string name{};                   // name of the ticket holder
  long unsigned int contactNum{};       // contact number of the ticket holder
  std::string showtime{};               // show time for the movie
  std::string movie{};                  // name of the movie

  void generate_ticket(KioskTicket);    // generate one ticket

public:
  int getTicketCost(int);               // gets the ticket cost
  void ticketBooking(int, int);         // handles the ticket booking
};

static int getTicketFromDatabase(uid_t confirmationNumber);       // get a pre-registered online booking from database

#endif
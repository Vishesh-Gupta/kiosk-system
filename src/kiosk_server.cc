#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

#include "ticket.h"
#include "card.h"
#include "helper.h"
#include "movies.h"
#include "db.h"

using namespace kiosk;

void pay(int);
void card();

const char *welcomeText =
  "Welcome Customers!"
  "1. Movie Timings\n 2. Recieving Ticket \n"
  "3. For Information \n 4. DTCard Registration"
  "5. Exit \n Select option number :\t";

int main(int argc, char* argv[]) {

  DB db = DB();

  // db.exec("CREATE TABLE test (id INT PRIMARY KEY) IF NOT EXISTS;");
  db.exec("INSERT INTO test;");
  db.query("SELECT * FROM test;");
  int option{0}, movieNumber{0}, N{0};
  char ans{};
  uid_t confirmationNumber{};

  Card card{};
  getCurrentTime();
  KioskTicket ticket;
  std::vector<Movie> movies = getCurrentlyRunningMovies();
  while(ans == 'y' || ans == 'Y') {
    std::cout << welcomeText;
    std::cin >> option;
    switch (option) {
      case 1: // * Movie Ticket Booking
        // TODO: Write an output operator for movies
        for (auto m: movies) {
          std::cout << m.name << m.year;
        }
        // TODO - Select the show timings
        std::cout << "Enter the number of the movie you want to watch :\t";
        std::cin >> movieNumber;
        // movies[movieNumber].getShowtimings();
        break;
      case 2: // * Online Ticket Processing
        std::cout << "Could you please enter your confirmation number?";
        std::cin >> confirmationNumber;
        // getTicketFromDatabase(confirmationNumber);
        break;
      case 3: // * Card Registration
        // card.registration();
        break;
      case 4: // * Exit system
        exit(0);
        break;
    }
  }
}
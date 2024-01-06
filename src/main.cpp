#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

#include "ticket.h"
#include "card.h"
#include "helper.hpp"

void pay(int);
void card();

const char *welcomeText =
  "Welcome Customers!"
  "1. Movie Timings\n 2. Recieving Ticket \n"
  "3. For Information \n 4. DTCard Registration"
  "5. Exit \n Select option number :\t";






int main(int argc, char* argv[]) {
  int option{0}, movieNumber{0}, N{0};
  char ans{};
  getCurrentTime();
  KioskTicket ticket;
  while(ans == 'y' || ans == 'Y') {
    std::cout << welcomeText;
    std::cin >> option;
    switch (option) {
      case 1:
        // TODO - List of movies
        std::cout << "Enter the number of the movie you want to watch :\t";
        std::cin >> movieNumber;
        std::cout << "\n Show Timings for the film:";
        switch (movieNumber) {
          case 1:
            // TODO - Show timings
            std::cout << "Choose Timings:";
            std::string timingOption{};
            std::cin >> timingOption;
            ticket.ticketBooking(timingOption);
            std::cout << "\n\n Do you want to choose another option(y/n)";
            std::cin >> ans;

            break;
          case 2:
            // TODO - Show timings
            ticketBooking();
            // TODO - Display the apt one
            std::cout << "\n\n Do you want to choose another option(y/n)";
            std::cin >> ans;

            break;
          case 3:
            // TODO - Show timings
            std::cin >> b;
            ticketBooking();
            // TODO - Display the apt one
            std::cout << "\n\n Do you want to choose another option(y/n)";
            std::cin >> ans;
            break;
          case 4:
            // TODO - Show timings
            std::cin >> b;
            ticketBooking();
            // TODO - Display the apt one
            std::cout << "\n\n Do you want to choose another option(y/n)";
            std::cin >> ans;
            break;
          case 5:
            // TODO - Show timings
            std::cin >> b;
            ticketBooking();
            // TODO - Display the apt one
            std::cout << "\n\n Do you want to choose another option(y/n)";
            std::cin >> ans;
            break;
        }
        break;
      case 2:
        std::cout
            << "\n\nThank you for booking the tickets online \n To print out "
               "the tickets please enter your transaction ID in the portal";  // Finding about a prebooked ticket
        std::cout << "\n Enter your transaction id\n (Eg.last five digits of the "
                "transaction id) ";
        // TODO - add instance of bookings
        std::cin >> p.trsnid;
        std::cout << "Enter your name";
        std::cin >> p.name;
        std::cout << "Sorry to say that but you will need to get the print out of "
                "the booking because our database shows no booking by this "
                "name";
        std::cout << "\n Do you want to choose another option(y/n)";
        std::cin >> an;
        break;
      case 3:
        std::cout
            << "For further information about movies you can download our "
               "Application(from the  Google Play Store or from the iOS App "
               "Store) or contact us at our toll-free number 1800-203-2014";  // Finding out more about our std::cinemas
        std::cout << "\n Do you want to choose another option(y/n)";
        std::cin >> an;
        break;
      case 4:
        Card card{};
        card.registration();
        break;
      case 5:
        exit(0);
        break;
    }
  }
}
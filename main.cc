#include <conio>
#include <fstream>
#include <iostream>
#include <string>
#include <time>

void pay(int);
void random();
void card();

void currentTime() {
  time_t t = time(NULL);
  tm* timePtr = localtime(&t);
  cout << "Time of the computer presently:";
  cout << "seconds= " << timePtr->tm_sec << endl;
  cout << "minutes = " << timePtr->tm_min << endl;
  cout << "hours = " << timePtr->tm_hour << endl;
  cout << "day of month = " << timePtr->tm_mday << endl;
  cout << "month of year = " << timePtr->tm_mon << endl;
  cout << "year = " << timePtr->tm_year + 1900 << endl;
  cout << "weekday = " << timePtr->tm_wday << endl;
  cout << "day of year = " << timePtr->tm_yday << endl;
  cout << "daylight savings = " << timePtr->tm_isdst << endl;
}

int main(int argc, char* argv[]) {
  int option{}, movieNumber{}, N{};
  char ans;
  currentTime();
  do {
    std::cout << "Welcome customer!\n";
    std::cout << "1. Movie Timings\n 2. Recieving Ticket \n";
    std::cout << "3. For Information \n 4. DTCard Registration";
    std::cout << "5. Exit \n";
    std::cout << "Select option number :\t";
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
            cout << "Choose Timings:";
            std::string timingOption{};
            cin >> timingOption;
            ticketBooking(timingOption);
            cout << "\n\n Do you want to choose another option(y/n)";
            cin >> ans;
            clrscr();
            break;
          case 2:
            // TODO - Show timings
            ticketBooking();
            // TODO - Display the apt one
            cout << "\n\n Do you want to choose another option(y/n)";
            cin >> ans;
            clrscr();
            break;
          case 3:
            // TODO - Show timings
            cin >> b;
            ticketBooking();
            // TODO - Display the apt one
            cout << "\n\n Do you want to choose another option(y/n)";
            cin >> ans;
            clrscr();
            break;
          case 4:
            // TODO - Show timings
            cin >> b;
            ticketBooking();
            // TODO - Display the apt one
            cout << "\n\n Do you want to choose another option(y/n)";
            cin >> ans;
            clrscr();
            break;
          case 5:
            // TODO - Show timings
            cin >> b;
            ticketBooking();
            // TODO - Display the apt one
            cout << "\n\n Do you want to choose another option(y/n)";
            cin >> ans;
            clrscr();
            break;
        }
        break;
      case 2:
        clrscr();
        cout
            << "\n\nThank you for booking the tickets online \n To print out "
               "the tickets please enter your transaction ID in the portal";  // Finding about a prebooked ticket
        cout << "\n Enter your transaction id\n (Eg.last five digits of the "
                "transaction id) ";
        // TODO - add instance of bookings
        cin >> p.trsnid;
        cout << "Enter your name";
        cin >> p.name;
        cout << "Sorry to say that but you will need to get the print out of "
                "the booking because our database shows no booking by this "
                "name";
        cout << "\n Do you want to choose another option(y/n)";
        cin >> ans;
        clrscr();
        break;
      case 3:
        clrscr();
        cout
            << "For further information about movies you can download our "
               "Application(from the  Google Play Store or from the iOS App "
               "Store) or contact us at our toll-free number 1800-203-2014";  // Finding out more about our cinemas
        cout << "\n Do you want to choose another option(y/n)";
        cin >> ans;
        clrscr();
        break;
      case 4:
        clrscr();
        cout << "Thank you for your interest in Card Membership!\n";
        card();
        cout << "Thankyou. Your process has initiated.";
        std::cout << "The card will be mailed to you soon.";
        std::cout << "Please see the benefits"
                     "of the "
                     "card on the next page. -->";
        char f;
        cout << "\n For selecting the page to go to benefits say (y/n)\n";
        cin >> f;
        if (f == 'y') {
          // TODO - Card registration.txt
        }
        cout << "\n Do you want to choose another option(y/n)";
        cin >> ans;
        if (ans == 'y') {
          clrscr();
          break;
        } else {
          exit(0);
        }
        break;
      case 5:
        exit(0);
        break;
    }
  } while (ans == 'y');
}
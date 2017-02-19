//Header Files
#include <iostream>
#include <fstream>
#include <conio>
#include <string>
#include <math>
#include <stdlib>
#include <stdio>
#include <process>
#include <time>
using namespace std;

//Class definition for ticket
class ticket{
	public:
	char name[10];
	char cno[10];
} t; //object definition for ticket

// Class definition for card
class card :  public ticket{  //inheritence for public ticket
public:
	char address[50];
	char emailid[20];
} v; //object definition for card

//Prototype Call for the functions definitions.
void pay(int);
void random();
void card();

//Main function
void main(){
	clrscr();
	//Integer Declaration
	int ent, a, b, N, x, cardid;
	char ans;
	//To display the system time.
	//Using time header file
	{time_t t = time(NULL);
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
	
	//An exit controlled loop (Do...While)
	do{
		//Menu
		cout<<"\n Welcome customer!";	 																					//Menu for the user
		cout<<"\n 1. Movie Timings";
		cout<<"\n 2. Recieving Ticket";
		cout<<"\n 3. For Information";
		cout<<"\n 4. DTCard Registration";
		cout<<"\n 5. Exit \n";
		cout<<"Enter the number for the option which you want to select :"<<"\t";
		cin>>ent;
	switch(ent)
	{
			//Movie Titles
		case 1: cout<<"\n\nThe Shows are :";
			cout<<"\n\n 1. The Last Witch Hunter";
			cout<<"\n\n 2. Kung Fu Panda 3";
			cout<<"\n\n 3. Deadpool";
			cout<<"\n\n 4. The Fast and the Furious 8";
			cout<<"\n\n 5. Justice League\n";
			cout<<"Enter the number of the movie you want to watch :"<<"\t";
			cin>>a;
			cout<<"\n\n The Timings for the selected show are:";
			switch(a)
			{
				case 1: cout<<"\n\n Select the the timings:";
					cout<<"\n 1. 0800";
					cout<<"\n 2. 1300";
					cout<<"\n 3. 1450";
					cout<<"\n 4. 1800";
					cout<<"\n 5. 2100";
					cout<<"\n 6. 0100 \n";					//Timings of the show
					cout<<"\n\n Please select the timings";
					cin>>b;
					cout<<"\n\n Enter your name";
					cin>>t.name;
					cout<<"\n\n Enter your contact number";
					cin>>t.cno;
					cout<<"\n\n Enter the number of tickets you want to purchase";
					int x;
					cin>>x;
					pay(x);
					cout<<"\n\n\n Your ticket is here:";
					cout<<"\n Name 		:"<<t.name;
					cout<<"\n Contact No	:"<<t.cno;
					cout<<"\n Show timings 	:";
					switch(b)
						{
							case 1:	cout<<"0800";
								break;
							case 2:	cout<<"1300";
								break;
							case 3:	cout<<"1450";
								break;
							case 4:	cout<<"1800";
								break;
							case 5:	cout<<"2100";
								break;
							case 6:	cout<<"0100";
								break;
						}
						cout<<"\n\n Do you want to choose another option(y/n)";
						cin>>ans;
						clrscr();
						break;
				case 2: cout<<"\n\nSelect the the timings:";
					cout<<"\n 1. 0900";
					cout<<"\n 2. 1100";
					cout<<"\n 3. 1250";
					cout<<"\n 4. 1500";
					cout<<"\n 5. 2000";
					cout<<"\n 6. 2200";
					cout<<"\n Please select the timings";
					cin>>b;
					cout<<"\n\n Enter your name";
					cin>>t.name;
					cout<<"\n Enter your contact number";
					cin>>t.cno;
					cout<<"\n Enter the number of tickets you want to purchase";
					cin>>x;
					pay(x);
					cout<<"\n \n Your ticket is here:";
					cout<<"\n Name 		:"<<t.name;
					cout<<"\n Contact No	:"<<t.cno;
					cout<<"\nShow timings 	:";
					switch(b)
					{
							case 1:	cout<<"0800";
								break;
							case 2:	cout<<"1300";
								break;
							case 3:	cout<<"1450";
								break;
							case 4:	cout<<"1800";
								break;
							case 5:	cout<<"2100";
								break;
							case 6:	cout<<"0100";
								break;
					}
					cout<<"\n\n Do you want to choose another option(y/n)";
					cin>>ans;
					clrscr();
					break;
				case 3: cout<<"\n\nSelect the the timings:";
					cout<<"\n 1. 0800";
					cout<<"\n 2. 1300";
					cout<<"\n 3. 1450";
					cout<<"\n 4. 1800";
					cout<<"\n 5. 2100";
					cout<<"\n 6. 0100";
					cout<<"\n Please select the timings";
					cin>>b;
					cout<<"\n Enter your name";
					cin>>t.name;
					cout<<"\n Enter your contact number";
					cin>>t.cno;
					cout<<"\nEnter the number of tickets you want to purchase";
					cin>>x;
					pay(x);
					cout<<"\n\n Your ticket is here:";
					cout<<"\n Name 		:"<<t.name;
					cout<<"\n Contact No	:"<<t.cno;
					cout<<"\n Show timings 	:";
					switch(b)
					{
							case 1:	cout<<"0900";
								break;
							case 2:	cout<<"1300";
								break;
							case 3:	cout<<"1450";
								break;
							case 4:	cout<<"1800";
								break;
							case 5:	cout<<"2100";
								break;
							case 6:	cout<<"0100";
								break;
					}
					cout<<"\n\n Do you want to choose another option(y/n)";
					cin>>ans;
					clrscr();
					break;
				case 4: cout<<"\n\nSelect the the timings:";
					cout<<"\n 1. 0800";
					cout<<"\n 2. 1300";
					cout<<"\n 3. 1450";
					cout<<"\n 4. 1800";
					cout<<"\n 5. 2100";
					cout<<"\n 6. 0100";
					cout<<"\n Please select the timings";
					cin>>b;
					cout<<"\n Enter your name";
					cin>>t.name;
					cout<<"\n Enter your contact number";
					cin>>t.cno;
					cout<<"\n Enter the number of tickets you want to purchase";
					cin>>x;
					pay(x);
					cout<<"\n\n Your ticket is here:";
					cout<<"\n Name 		:"<<t.name;
					cout<<"\n Contact No	:"<<t.cno;
					cout<<"\n Show timings 	:";
					switch(b)
					{
							case 1:	cout<<"0800";
								break;
							case 2:	cout<<"1300";
								break;
							case 3:	cout<<"1450";
								break;
							case 4:	cout<<"1800";
								break;
							case 5:	cout<<"2100";
								break;
							case 6:	cout<<"0100";
								break;
					}
						cout<<"\n\n Do you want to choose another option(y/n)";
						cin>>ans;
						clrscr();
						break;
				case 5: cout<<"\n\nSelect the the timings:";
					cout<<"\n 1. 0800";
					cout<<"\n 2. 1300";
					cout<<"\n 3. 1450";
					cout<<"\n 4. 1800";
					cout<<"\n 5. 2100";
					cout<<"\n 6. 0100";
					cout<<"\n Please select the timings";
					cin>>b;
					cout<<"\n Enter your name";
					cin>>t.name;
					cout<<"\n Enter your contact number";
					cin>>t.cno;
					cout<<"\n Enter the number of tickets you want to purchase";
					cin>>x;
					pay(x);
					cout<<"\n \n Your ticket is here:";
					cout<<"\n Name 		:"<<t.name;
					cout<<"\n Contact No	:"<<t.cno;
					cout<<"\n Show timings 	:";
					switch(b)
					{
							case 1:	cout<<"0800";
								break;
							case 2:	cout<<"1300";
								break;
							case 3:	cout<<"1450";
								break;
							case 4:	cout<<"1800";
								break;
							case 5:	cout<<"2100";
								break;
							case 6:	cout<<"0100";
								break;
					}
					cout<<"\n\n Do you want to choose another option(y/n)";
					cin>>ans;
					clrscr();
					break;
			}break;
		case 2:	clrscr();
				cout<<"\n\nThank you for booking the tickets online \n To print out the tickets please enter your transaction ID in the portal";					//Finding about a prebooked ticket
				struct pre
				{
				int trsnid;
				char name[10];
				} p;
				cout<<"\n Enter your transaction id\n (Eg.last five digits of the transaction id) ";
				cin>>p.trsnid;
				cout <<"Enter your name";
				cin>>p.name;
				cout<<"Sorry to say that but you will need to get the print out of the booking because our database shows no booking by this name";
				cout<<"\n Do you want to choose another option(y/n)";
				cin>>ans;
				clrscr();
				break;
		case 3: clrscr();
				cout<<"For further information about movies you can download our Application(from the  Google Play Store or from the iOS App Store) or contact us at our toll-free number 1800-203-2014";			//Finding out more about our cinemas
				cout<<"\n Do you want to choose another option(y/n)";
				cin>>ans;
				clrscr();
				break;
		case 4: clrscr();
				cout<<"Good Morning/Evnening \n Welcome to start a new journey with our cinemas \n";																												//card membership
				card();
				cout<<"Thankyou. \n It will take us a week for completing your registration for the card. \n Please see the benefits of the card on the next page. -->";
				char f;
				cout<<"\n For selecting the page to go to benefits say (y/n)\n";
				cin>>f;
				if(f=='y')
				{
					cout<<"Thank you for registeration once again \n The priveleges provided with this card are as follows:";
					cout<<"\n 1. For every purchase of a movie ticket you get 25 points(1point = 1Rs.) so after 16 movies you get a free movie ticket.";
					cout<<"\n 2. You are provided with regular updates regarding the movie and the showtimings.";
					cout<<"\n 3. Anytime prebook tickets for the upcoming movie and preffered seats will be provided.";
				}
				cout<<"\n Do you want to choose another option(y/n)";
				cin>>ans;
				if(ans=='y')
				{
					clrscr();
					break;
				}
				else
				{
					exit(0);
				}
				break;
		case 5: exit(0);
				break;
	}
	}while(ans=='y');
	}

//Function Declaration for Card
void card()
{
	int cardid;
	cout<<"Welcome to register for card facility in our cinemas";
	cout<<" \n Enter your name";
	cin>>v.name;
	cout<<"Enter your mobile number";
	cin>>v.cno;
	cout<<"Enter the address";
	gets(v.address);
	cout<<"Enter the mail id";
	gets(v.emailid);
	system("cls");
	int ID;
	srand (time(NULL));
	ID = rand() % 400000 + 4000000;
	if (ID<0)
	ID=(ID*-1);
	cout<<"Your new card number is - :" <<"\t"<<ID;
	fstream fout;
	fout.open("card.dat", ios::out|ios::app);
	fout<<"\n Name :"<<v.name<<"\n"<<"\n Mobile No. :"<<v.cno<<"\n"<<"\n Address :"<<v.address<<"\n"<<"\n Mail ID :"<<v.emailid<<"\n"<<"\nCard Number:"<<ID;
	fout.close();
	cout<<"Thank you for the registeration for the card. \n";
}

//Payment system for the interface
void pay(int a)
{
	int normal, gold, amt[2],id;
	time_t t = time(NULL);							//time setup
	tm* timePtr = localtime(&t);
	fstream fin;
	fin.open("card.dat", ios::in|ios::app);
	fin>>id;
	cout<<"Thank you for selecting the show. Now we request you to select your type of seating \n 1.Normal Class \n OR \n 2. Gold Class";
	int c;
	cin>>c;
	if(c==1)
	{
		cout<<"You selected for a Normal show";
		amt[1] = a * 400;
		char final;
		cout<<"\n Do you have DTcard(y/n)";
		cin>>final;
		if(final=='y')
		{
			int cid;
			fin.read((char*) &v, sizeof(v));
			
			cout<<"Enter the card number";
			
			if(cid==id)
			{
				amt[1]=amt[1] - (0.1*amt[1]);
			};
		}
		cout<<"Want to pay by Card(y/n)";
		char rep;
		cin>>rep;
		cout<<"\n"<<"Paying :"<<amt[1]<<"\n";
		if (rep=='y'||rep=='Y')
		{
			cout<<"Name of the card holder";
			char n[10];
			gets(n);
			cout<<"Enter the card number";
			char Card[16];
			gets(Card);
			cout<<"Expiry(MM/YYYY)";
			int expirymm,expiryyy;
			cin>>expirymm;
			cout<<"/";
			cin>>expiryyy;
			while(expirymm<(timePtr->tm_mon) || expiryyy<(timePtr->tm_year + 1900)){
				if(expirymm<=(timePtr->tm_mon))
					{
						cout<<"Enter the month again";
						cin>>expirymm;
					}
				if(expiryyy<(timePtr->tm_year + 1900))
				{
					cout<<"Please enter a valid year";
					cin>>expiryyy;
				}
			};
			char password[3],vh;
			int h;
			puts("Enter the CVV/CVV2");
			while (1)
			{
				if (h<0)
					h=0;
				vh=getch();
				if (vh==13)
					break;
				if (vh==8)
				{
					putch(NULL);
					putch(NULL);
					putch(NULL);
					h--;
					continue;
				}
				password[h++]=vh;
				vh='*';
				putch(vh);
			};
			password[h]=='\0';
		}
	}
	else
	{
		cout<<"You selected for the Gold Class";
		amt[2] = a * 700;
		char final;
		cout<<"\n Do you have DTcard(y/n)";
		cin>>final;
		if(final=='y')
		{
			int cid;
			cout<<"Enter the card number";
			cin>>cid;
			if(cid==id)
			{
				amt[1]=amt[1] - (0.1*amt[1]);
			};
		}
		cout<<"Want to pay by Card(y/n)";
		char rep;
		cin>>rep;
		cout<<"\n"<<"Paying :"<<amt[2]<<"\n";
		if (rep=='y'||rep=='Y')
		{
			cout<<"Name of the card holder";
			char n[10];
			gets(n);
			cout<<"Enter the card number";
			char Card[16];
			gets(Card);
			cout<<"Expiry(MM/YY)";
			int expirymm, expiryyy;
			cin>>expirymm;
			cout<<"/";
			cin>>expiryyy;
			while(expirymm<(timePtr->tm_mon) || expiryyy<(timePtr->tm_year + 1900)){
				if(expirymm<=(timePtr->tm_mon))
					{
						cout<<"Enter the month again";
						cin>>expirymm;
					}
				if(expiryyy<(timePtr->tm_year + 1900))
				{
					cout<<"Please enter a valid year";
					cin>>expiryyy;
				}
			};
			char password[3],vh;
			int h;
			puts("Enter the CVV/CVV2");
			while (1)
			{
				if (h<0)
					h=0;
				vh=getch();
				if (vh==13)
					break;
				if (vh==8)
				{
					putch(NULL);
					putch(NULL);
					putch(NULL);
					h--;
					continue;
				}
				password[h++]=vh;
				vh='*';
				putch(vh);
			};
			password[h]=='\0';
			}
	};
}

-- Create table for movies
CREATE TABLE Movies (
    MovieID INT PRIMARY KEY,
    Title VARCHAR(255) NOT NULL,
    Genre VARCHAR(50),
    ReleaseYear INT,
    Rating DECIMAL(3, 1)
);

-- Create table for movie showtimes
CREATE TABLE Showtimes (
    ShowtimeID INT PRIMARY KEY,
    MovieID INT,
    StartTime DATETIME,
    TheaterNumber INT,
    FOREIGN KEY (MovieID) REFERENCES Movies(MovieID)
);

-- Create table for tickets
CREATE TABLE Tickets (
    TicketID INT PRIMARY KEY,
    ShowtimeID INT,
    SeatNumber VARCHAR(10),
    Price DECIMAL(8, 2),
    Status VARCHAR(20) DEFAULT 'Available', -- Status can be 'Available', 'Sold', etc.
    FOREIGN KEY (ShowtimeID) REFERENCES Showtimes(ShowtimeID)
);

-- Create table for customers
CREATE TABLE Customers (
    CustomerID INT PRIMARY KEY,
    FirstName VARCHAR(50) NOT NULL,
    LastName VARCHAR(50) NOT NULL,
    Email VARCHAR(100) UNIQUE
);

-- Create table for purchases
CREATE TABLE Purchases (
    PurchaseID INT PRIMARY KEY,
    CustomerID INT,
    TicketID INT,
    PurchaseDate DATE,
    FOREIGN KEY (CustomerID) REFERENCES Customers(CustomerID),
    FOREIGN KEY (TicketID) REFERENCES Tickets(TicketID)
);

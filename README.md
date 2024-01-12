# Kiosk System

<img src="https://img.freepik.com/free-vector/modern-exhibition-icon_1284-66424.jpg?w=1380&t=st=1704563769~exp=1704564369~hmac=68e55b385bb7ee0496a41827caac25d849b75a04ad47425a7e1d158c85fb3415" height="150" width="150">

A generic purpose kiosk system, that uses industry C++ concepts. It uses Conan for package management, gRPC with protobuf for communication with a client, PostgreSQL for store, buf for Buf Schema Registry to maintain proto definitions

### History

This used to be a Movie Ticketing Kiosk System, but then I wanted to generalize this into a kiosk system for anything depending on any source of data.

I built it as my Grade 12 Project Submission, and then improved it mid university but never had time. In Janaury 2024, I decided I wanted to spend more time with C++ and build a high level kiosk system with database interactions and a UI.


## Installation Guide

### Tooling

1. Python 3.11 (and Pip)
2. Conan (C++ Package Manager)

### Steps

1. Clone the repository from GitHub
2. Run the following commands to build the code

```sh
$ cd cmake/build
$ cmake -DCMAKE_PREFIX_PATH=$MY_INSTALL_DIR ..
$ make
```

```sh
conan install . --output-folder=build --build=missing
cd build/
cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

## Operations
### Movie Ticket Example
- ListMovies: List all currently running movies.
- GetMovie: Select the movie and show all the show timings for the movie.
- PostMovie: Select the movie, with show timings, number of tickets
  - This reaches out to `stripe-mock` with a data response for doing a mock payment process
- DeleteMovie: Cancels a movie ticket.
- GetMovieTicket: Gets a movie ticket that has been stored in the database.

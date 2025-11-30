-- Movie table
CREATE TABLE IF NOT EXISTS movie (
  id            SERIAL PRIMARY KEY,
  name          TEXT NOT NULL,
  release_year  INT NOT NULL,
  description   TEXT,
  duration      INT NOT NULL,
  rating        VARCHAR(10),
  created_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
  updated_at    TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE INDEX IF NOT EXISTS idx_movie_name ON movie(name);
CREATE INDEX IF NOT EXISTS idx_movie_release_year ON movie(release_year);

INSERT INTO movie (name, release_year, description, duration, rating) VALUES
('The Matrix', 1999, 'A computer hacker learns about the true nature of reality', 136, 'R'),
('Inception', 2010, 'A thief enters peoples dreams to steal their secrets', 148, 'PG-13'),
('Interstellar', 2014, 'Explorers travel through a wormhole in space', 169, 'PG-13'),
('The Dark Knight', 2008, 'Batman faces the Joker in Gotham', 152, 'PG-13'),
('Pulp Fiction', 1994, 'The lives of two mob hitmen intertwine', 154, 'R')
ON CONFLICT DO NOTHING;

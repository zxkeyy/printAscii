#ifndef CAST_PLAYER_H
#define CAST_PLAYER_H

/**
 * Plays an Asciinema v2 (.cast) file directly in the terminal.
 * 
 * @param path The path to the .cast file to play.
 * @return 0 on success, < 0 on failure.
 */
int cast_player_play(const char* path);

#endif // CAST_PLAYER_H

/*
 * Copyright 2010, Mikhail "borman" Borisov <borisov.mikhail@gmail.com>
 *
 * This file is part of borman's management game server.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <string.h>

#include "commands.h"

const struct CommandDescription
{
  enum Command code;
  const char *str;
  enum ClientState allowed_states;
} commands[] = 
{
  {CMD_IDENTIFY, "identify", CL_CONNECTED},
  {CMD_READY, "ready", CL_IN_LOBBY},
  {CMD_NOTREADY, "notready", CL_IN_LOBBY_ACK},
  {CMD_QUIT, "quit", CL_IN_LOBBY | CL_IN_LOBBY_ACK | CL_IN_GAME | CL_SUPERVISOR}
};
const int n_commands = sizeof(commands)/sizeof(struct CommandDescription);


enum Command command_resolve(enum ClientState client_state, const char *cmd_str)
{
  unsigned int i;
  for (i=0; i<n_commands; i++)
    if (strcmp(cmd_str, commands[i].str) == 0)
    {
      if (commands[i].allowed_states & client_state)
        return commands[i].code;
      else
        return CMD_INVALID;
    }
  return CMD_INVALID;
}

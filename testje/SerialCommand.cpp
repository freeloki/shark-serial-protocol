/**
 * https://github.com/kroimon/Arduino-SerialCommand
 * SerialCommand - A Wiring/Arduino library to tokenize and parse commands
 * received over a serial port.
 *
 * Copyright (C) 2012 Stefan Rado
 * Copyright (C) 2011 Steven Cogswell <steven.cogswell@gmail.com>
 *                    http://husks.wordpress.com
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "SerialCommand.h"

/**
 * Constructor makes sure some things are set.
 */
SerialCommand::SerialCommand()
    : commandList(NULL),
      commandCount(0),
      defaultHandler(NULL)
{
}

/**
 * Adds a "command" and a handler function to the list of available commands.
 * This is used for matching a found token in the buffer, and gives the pointer
 * to the handler function to deal with it.
 */
void SerialCommand::addCommand(byte command, void (*function)(byte*, byte)) {
    commandList = (SerialCommandCallback *) realloc(commandList, (commandCount + 1) * sizeof(SerialCommandCallback));
    commandList[commandCount].command = command;
    commandList[commandCount].function = function;
    commandCount++;
}

/**
 * This sets up a handler to be called in the event that the received command string
 * isn't in the list of commands.
 */
void SerialCommand::setDefaultHandler(void (*function)(const byte)) {
    defaultHandler = function;
}


void SerialCommand::readSerial(byte* packet, byte length) {
    bool matched=false;
    for (int i = 0; i < commandCount; i++) {
        // Compare the found command against the list of known commands for a match
        if (packet[0]==commandList[i].command) {
            // Execute the stored handler function for the command
            (*commandList[i].function)(packet+1, length-1);
            matched = true;
            break;
        }
    }
    if (!matched && (defaultHandler != NULL)) {
        (*defaultHandler)(packet[0]);
    }
}

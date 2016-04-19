#include "busCommand.h"
 
/**
 * Constructor makes sure some things are set.
 */
busCommand::busCommand()
  : commandList(NULL),
    commandCount(0),
    defaultHandler(NULL),
    transmissionEnd(';'),         // default terminator for transmission
    commandDelim(','),
    argDelim(' '),
    last(buffer),
    isI2C(false),
    isFinished(false),
    readyToParse(false)
{
  clearBuffer();
}
 
/**
 * Adds a "command" and a handler function to the list of available commands.
 * This is used for matching a found token in the buffer, and gives the pointer
 * to the handler function to deal with it.
 */
void busCommand::addCommand(const char *command, void (*function)()) {
  #ifdef busCommand_DEBUG
    Serial.print("Adding command (");
    Serial.print(commandCount);
    Serial.print("): ");
    Serial.println(command);
  #endif
 
  commandList = (busCommandCallback *) realloc(commandList, (commandCount + 1) * sizeof(busCommandCallback));
  strncpy(commandList[commandCount].command, command, busCommand_MAXCOMMANDLENGTH);
  commandList[commandCount].function = function;
  commandCount++;
}
 
/**
 * This sets up a handler to be called in the event that the receveived command string
 * isn't in the list of commands.
 */
void busCommand::setDefaultHandler(void (*function)(const char *)) {
  defaultHandler = function;
}
 
 
/**
 * This checks the Serial stream for characters, and assembles them into a buffer.
 * When the terminator character (default '\n') is seen, it starts parsing the
 * buffer for a prefix command, and calls handlers setup by addCommand() member
 */
void busCommand::readBus() {
   
  while (isAvailable() > 0) {
    char inChar = busRead();   // Read single available character, there may be more waiting
    if(inChar == '\r' || inChar == '\n') continue; //Skip CR and NEWLINE
    #ifdef busCommand_DEBUG
      Serial.print(inChar);   // Echo back to serial stream
    #endif
    if (inChar == transmissionEnd) {     // Check for the terminator (default ';')
      readyToParse = true;
    }
    if (bufPos < busCommand_BUFFER) {
      buffer[bufPos++] = inChar;  // Put character into buffer
      buffer[bufPos] = '\0';      // Null terminate
    } else {
      #ifdef busCommand_DEBUG
        Serial.println("Line buffer is full - increase busCommand_BUFFER");
      #endif
    }
  }
}
 
char* busCommand::getCommand(){
 
  hasArgs = true;
  if(isFinished) 
    return NULL;
 
  while(*last == argDelim || *last == commandDelim){
    *last = '\0';
    last++;   //Jump over junk
  }
  if(*last == transmissionEnd){
    clearBuffer();
    isFinished = true;
    return NULL;
  }
 
  char *beg = last;
 
  while(*last != commandDelim && *last != argDelim && *last != transmissionEnd ) last++;
 
  if(*last == commandDelim || *last == transmissionEnd) 
    hasArgs = false;
   
  if(*last == transmissionEnd){
    isFinished = true;
  }
  *last = '\0'; 
  last++;
  return beg;
}
 
char* busCommand::getArg(){
  if(hasArgs == false || isFinished)
    return NULL;
  while(*last == commandDelim){
    *last = '\0';
    last++;
  }
 
  char *beg = last;
 
  while(*last != commandDelim && *last != argDelim && *last != transmissionEnd) last++;
 
  if(*last == commandDelim || *last == transmissionEnd) hasArgs = false;
 
  if(*last == transmissionEnd)
    isFinished = true;
 
  *last = '\0'; 
  last++;
 
  return beg;
}
 
void busCommand::Parse(){
 
  #ifdef busCommand_DEBUG
      Serial.print("Received: ");
      Serial.println(buffer);
  #endif
 
  last = buffer;
  isFinished = false;
  char *command = getCommand();   // Search for command 
  while (command != NULL) {
    boolean matched = false;
    for (int i = 0; i < commandCount; i++) {
      #ifdef busCommand_DEBUG
        Serial.print("Comparing [");
        Serial.print(command);
        Serial.print("] to [");
        Serial.print(commandList[i].command);
        Serial.println("]");
      #endif
 
      // Compare the found command against the list of known commands for a match
      if (strncmp(command, commandList[i].command, busCommand_MAXCOMMANDLENGTH) == 0) {
        #ifdef busCommand_DEBUG
          Serial.print("Matched Command: ");
          Serial.println(command);
        #endif
 
        // Execute the stored handler function for the command
        (*commandList[i].function)();
        matched = true;
        break;
      }
    }
    if (!matched && (defaultHandler != NULL)) {
      (*defaultHandler)(command);
    }
    command = getCommand();
  }
  clearBuffer();
 
  readyToParse = false;
}
 
/*
 * Clear the input buffer.
 */
void busCommand::clearBuffer() {
  buffer[0] = '\0';
  bufPos = 0;
}

void busCommand::switchToI2C(){
  isI2C = true;
}
 
void busCommand::switchToSerial(){
  isI2C = false;
}
 
int busCommand::isAvailable(){
  if(isI2C)
    return Wire.available();
  if(customSerial)
    return customSerial->available();
  return Serial.available();
}
 
char busCommand::busRead(){
  if(isI2C)
    return Wire.read();
  if(customSerial)
    return customSerial->read();
  return Serial.read();
}

void busCommand::setSoftwareSerial(SoftwareSerial *serial){
  customSerial = serial;
}

boolean busCommand::isSerial(){
  return !isI2C;
}
 
boolean busCommand::isReadyToParse(){
  return readyToParse;
}
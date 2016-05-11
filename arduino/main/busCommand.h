#ifndef busCommand_h
#define busCommand_h
 
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <string.h>
#include <Wire.h>
 
// Size of the input buffer in bytes
#define busCommand_BUFFER 64
// Maximum length of a command excluding the terminating null
#define busCommand_MAXCOMMANDLENGTH 8
 
// Uncomment the next line to run the library in debug mode (verbose messages)
//#define busCommand_DEBUG
 
 
class busCommand {
  public:
    busCommand();      // Constructor
    void addCommand(const char *command, void(*function)());  // Add a command to the processing dictionary.
    void setDefaultHandler(void (*function)(const char *));   // A handler to call when no valid command received.
 
    void readBus();    // Main entry point.
    void Parse();
    char *getArg();
    char *getCommand();
    void clearBuffer();   // Clears the input buffer.
 
    void switchToI2C();
    void switchToSerial();
    void setSoftwareSerial(SoftwareSerial *);
 
    boolean isSerial();
    boolean isReadyToParse();
 
    //For printing and reading
    template<typename T> int print(const T &value){
        const byte * p = (const byte*) &value; 
        unsigned int i;
        char *num = (char*)malloc((sizeof value) + 1);
        for (i = 0; i < sizeof value; i++){
            num[i] = *p++; 
             
        }
        num[i] = '\0';
        if(isI2C){
            if(i < 31){
                Wire.write(num);
                Serial.println(num);
            }
        }
        else
            Serial.println(num);
        free(num);
        return i;
    }
 
    template<typename T> int read(T &value){
        byte * p = ( byte*) &value; 
        unsigned int i;
        for (i = 0; i < sizeof value; i++){
            *p++ = Wire.read(); 
        }
        Serial.println(value);
        return i;
    }
 
  private:
    // Command/handler dictionary
    struct busCommandCallback {
      char command[busCommand_MAXCOMMANDLENGTH + 1];
      void (*function)();
    };                                    // Data structure to hold Command/Handler function key-value pairs
    busCommandCallback *commandList;   // Actual definition for command/handler array
    byte commandCount;
 
    // Pointer to the default handler function
    void (*defaultHandler)(const char *);
 
    char commandDelim;
    char argDelim;
    char transmissionEnd;    
 
    char buffer[busCommand_BUFFER + 1]; // Buffer of stored characters while waiting for terminator character
    byte bufPos;                        // Current position in the buffer
    char *last;                         // State variable used by strtok_r during processing
    
    SoftwareSerial *customSerial;

    boolean hasArgs;
    boolean isI2C;
    boolean readyToParse;
    boolean isFinished;                 // If parsing is finished
 
    int isAvailable();
    char busRead();
};
 
#endif //busCommand_h

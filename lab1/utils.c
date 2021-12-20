#include "utils.h"

void logError(char *msg){
    char buf[MAX_SIZE];
    sprintf(buf, "\033[0;31m>>>ERROR:\t%s\n\033[0m", msg);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void logSuccess(char *msg){
    char buf[MAX_SIZE];
    sprintf(buf, "\033[0;32m>>>SUCCESS:\t%s\n\033[0m", msg);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void logWarning(char *msg){
    char buf[MAX_SIZE];
    sprintf(buf, "\033[0;33m>>>WARNING:\t%s\n\033[0m", msg);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void logInfo(char *msg){
    char buf[MAX_SIZE];
    sprintf(buf, ">>>Info:\t%s\n", msg);
    write(STDOUT_FILENO, buf, strlen(buf));
}

void logUsage(){
    char buf[MAX_SIZE];
    sprintf(buf, "Usage:\t./application <SerialPort>\n\tex: ./application /dev/ttyS10\n");
    write(STDOUT_FILENO, buf, strlen(buf));
    
}

void timeoutHandler()                   // atende alarme
{
	timeout=1;
	timeoutCount++;
	char buf[MAX_SIZE];
	sprintf(buf, "Time-out achieved, count = %i\n", timeoutCount);
	logWarning(buf);

}

void disconnectTimeout()                   // atende alarme
{
	timeout=1;
}


void startTimeElapsed(){
    timeElapsed.start = clock();
}

void endTimeElapsed(){
    timeElapsed.end = clock();
    timeElapsed.timeTaken = ((double) (timeElapsed.end - timeElapsed.start)) / CLOCKS_PER_SEC;
}

void logStats(){
    char buf[MAX_SIZE];
    sprintf(buf, "Transmission Time: %fs\nTransmission Rate: %fb/s\n", timeElapsed.timeTaken, BAUDRATE, dataFile.filesize/timeElapsed.timeTaken);
    write(STDOUT_FILENO, buf, strlen(buf));
}
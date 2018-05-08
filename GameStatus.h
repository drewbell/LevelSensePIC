#ifndef GAMESTATUS_H
#define	GAMESTATUS_H

typedef enum {
    RED,
    BLUE
} GS_BotColor_t;

typedef enum {
    NEU = 0,
    BLUE = 1,
    RED = 2,
    ERR = 3
} GS_PollStatus_t;

/* Updates the polling station data in the active GameStatus buffer 
 * pollIndex is */
void setPollingStationStatus(uint8_t pollIndex, GS_PollStatus_t pollStatus);
void setAttack(GS_BotColor_t, bool attackStatus);
void setGameRunning(bool isRunning);

void incrementActiveBuffer(void);

uint8_t * getStatus(void);

#endif	/* GAMESTATUS_H */


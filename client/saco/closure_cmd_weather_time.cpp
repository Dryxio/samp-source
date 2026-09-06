#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
extern CGame *pGame;
extern CChatWindow *pChatWindow;

void cmdSetWeather(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: set_weather (weather number)");
		return;
	}	
	int iWeatherID = atoi(szCmd);
	pGame->SetWorldWeather(iWeatherID);
}

void cmdSetTime(PCHAR szCmd)
{
	if(!strlen(szCmd)){
		pChatWindow->AddDebugMessage("Usage: set_time (hour) (minute)");
		return;
	}
		
	int	iHour=0,iMinute=0;

	sscanf(szCmd,"%d%d",&iHour,&iMinute);

	if ((iHour >= 0 && iHour <= 23) && (iMinute >= 0 && iMinute <= 59)) {
		pGame->SetWorldTime(iHour,iMinute);
	}
	else {
		pChatWindow->AddDebugMessage("Invalid Time. Use /set_time (hour 0-23) (minute 0-59)");
	}
}

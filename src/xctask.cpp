#include <Arduino.h>
#include <stddef.h>

#include "xctask.h"
#include "jsmn.h"

#define stricmp(a, b) strcasecmp(a, b)
#define strnicmp(a, b, c) strncasecmp(a, b, c)

XcTaskResult XcTask2Xml(char *text, char *taskfile, char *wptfile) {
#define XCTASK_VERSION2_PRE "XCTSK:"
	XcTaskResult result = XcTask_WrongFormat;
	char *js = text;
	jsmn_parser parser;
	jsmn_init(&parser);
	
	if (strncmp(text, XCTASK_VERSION2_PRE, strlen(XCTASK_VERSION2_PRE)) == 0)
		js = &text[strlen(XCTASK_VERSION2_PRE)];
	
	char *strName = NULL, *strValue = NULL;
	XcTask xcTask = {0};
	
	int cnt_tokens = jsmn_parse(&parser, js, strlen(js), NULL, 0);
	if (cnt_tokens > 0)
	{
		jsmntok_t *tokens = (jsmntok_t*)malloc(cnt_tokens * sizeof(jsmntok_t));
		jsmn_init(&parser);
		cnt_tokens = jsmn_parse(&parser, js, strlen(js), tokens, cnt_tokens);

		// 0tes Objekt ist root
		if (tokens[0].type != JSMN_OBJECT)
			return XcTask_WrongFormat;
		
		for (int i = 1; i < cnt_tokens; i+=2) {
			strName = strValue = NULL;
			int intVal = 0;
			jsmntok_t *tokName = &tokens[i];
			jsmntok_t *tokValue = &tokens[i+1];
			
			if (tokName->type != JSMN_STRING)
				goto Error;
			
			jsonGetString(js, tokName, &strName);
			
			if (stricmp(strName, "taskType") == 0)
			{
				if (tokValue->type != JSMN_STRING)
					goto Error;
				jsonGetString(js, tokValue, &xcTask.taskType);
			}
			else if (stricmp(strName, "version") == 0)
			{
				if (tokValue->type != JSMN_PRIMITIVE)
					goto Error;
				jsonGetInt(js, tokValue, &xcTask.version);
			}
			else if (stricmp(strName, "earthModel") == 0 || stricmp(strName, "e") == 0)
			{
				//xcTask.earthModel = WGS84;	//preinit
				if (tokValue->type == JSMN_STRING)
				{
					jsonGetString(js, tokValue, &strValue);
					if (stricmp(strValue, "FAI_SPHERE") == 0)
						xcTask.earthModel = FAI_SPHERE;
					free(strValue);	
				}
				else if (tokValue->type == JSMN_PRIMITIVE)
				{
					jsonGetInt(js, tokValue, &intVal);
					if (intVal)
						xcTask.earthModel = FAI_SPHERE;
				} else
					goto Error;
			}
			else if (stricmp(strName, "turnpoints") == 0 || stricmp(strName, "t") == 0)
			{
				if (tokValue->type != JSMN_ARRAY)
					goto Error;
				// Array of turnpoint objects
				xcTask.cntTurnpoints = tokValue->size;
				xcTask.turnpoints = (XcTaskTurnpoint*)malloc(sizeof(XcTaskTurnpoint) * tokValue->size);
				for (int tp = 0; tp < tokValue->size; tp++, i++)
				{
					jsmntok_t *tokTpObj = &tokens[i];
					for (int tpi = 0; tpi < tokTpObj->size; tpi++, i++)
					{
						jsmntok_t *tokTpName = &tokens[i];
						jsmntok_t *tokTpValue = &tokens[i+1];
						
						char *strTpTokName = NULL;
						jsonGetString(js, tokTpName, &strTpTokName);
						
						if (stricmp(strTpTokName, "type") == 0)
							jsonGetString(js, tokTpValue, &xcTask.turnpoints[tp].type);
						else if (stricmp(strTpTokName, "radius") == 0)
							jsonGetFloat(js, tokTpValue, &xcTask.turnpoints[tp].radius);
						
						// inside waypoint
						else if (stricmp(strTpTokName, "name") == 0)
							jsonGetString(js, tokTpValue, &xcTask.turnpoints[tp].name);
						else if (stricmp(strTpTokName, "description") == 0)
							jsonGetString(js, tokTpValue, &xcTask.turnpoints[tp].description);
						else if (stricmp(strTpTokName, "lat") == 0)
							jsonGetFloat(js, tokTpValue, &xcTask.turnpoints[tp].lat);
						else if (stricmp(strTpTokName, "lon") == 0)
							jsonGetFloat(js, tokTpValue, &xcTask.turnpoints[tp].lon);
						else if (stricmp(strTpTokName, "alt") == 0)
							jsonGetFloat(js, tokTpValue, &xcTask.turnpoints[tp].alt);
					}
				}
			}
			else if (stricmp(strName, "takeoff") == 0 || stricmp(strName, "") == 0)
			{
				if (tokValue->type != JSMN_OBJECT)
					goto Error;
			}
			else if (stricmp(strName, "sss") == 0 || stricmp(strName, "") == 0)
			{
				if (tokValue->type != JSMN_OBJECT)
					goto Error;
			}
			else if (stricmp(strName, "goal") == 0 || stricmp(strName, "") == 0)
			{
				if (tokValue->type != JSMN_OBJECT)
					goto Error;

				// goal object
				for (int goali = 0; goali < tokValue->size; goali++)
				{
					jsmntok_t *tokGoalName = &tokens[i+=2];
					jsmntok_t *tokGoalValue = &tokens[i+1];
					
					if (tokGoalName->type == JSMN_STRING && tokGoalValue->type == JSMN_STRING)
					{
						char *strGoalTokName = NULL;
						jsonGetString(js, tokGoalName, &strGoalTokName);
					
						if (stricmp(strGoalTokName, "deadline") == 0)
							jsonGetString(js, tokGoalValue, &xcTask.goal.deadline);
						else if (stricmp(strGoalTokName, "type") == 0)
							jsonGetString(js, tokGoalValue, &xcTask.goal.type);
						
						free(strGoalTokName);
					}
					else
					{
						//Unknown data
						log_e("Unknown data!");
						
					}
				}
			}
			/*
			else if (stricmp(strName, "") == 0  || stricmp(strName, "") == 0)
			{
			}
			*/
			else
			{
				// Unknown object or array, step over
				log_e("Unknown ");
			}
			
			log_d("Type: %d   size: %5d   Name: %s\n", tokName->type, tokName->size, strName);
			free(strName);
			//jsonGetString(js, tokValue, &strValue);
			//sprintf(info, "Type: %d   size: %5d   Value: %s\n", tokValue->type, tokValue.size, strValue);
			//free(strValue);
		}
		result = XcTask_NoError;

Error:		
		free(tokens);
		free(strName);
		free(strValue);
		
		free(xcTask.taskType);
		for (int tp = 0; tp < xcTask.cntTurnpoints; tp++) {
			free (xcTask.turnpoints[tp].type);
			free (xcTask.turnpoints[tp].name);
			free (xcTask.turnpoints[tp].description);
		}
		free(xcTask.turnpoints);
		free(xcTask.goal.deadline);
		free(xcTask.goal.type);
	}
	return result;
}



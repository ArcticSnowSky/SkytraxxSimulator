#ifndef XCTASK_H
#define XCTASK_H

typedef enum enEarthModel {
	WGS84,
	FAI_SPHERE
} enEarthModel;

typedef struct struXcTaskTurnpoint {
	char *type;
	float radius;
	
	// inside waypoint
	char *name;
	char *description;
	float lat;
	float lon;
	float alt;
} XcTaskTurnpoint;

typedef struct struXcTask {
	int version;
	char *taskType;
	enEarthModel earthModel;
	int cntTurnpoints;
	XcTaskTurnpoint *turnpoints;
	
	struct {
		char *deadline;
		char *type;
	} goal;
	
} XcTask;


typedef enum {
	XcTask_NoError,
	XcTask_WrongFormat
} XcTaskResult;

XcTaskResult XcTask2Xml(char *text, char *taskfile, char *wptfile);

#endif // XCTASK_H
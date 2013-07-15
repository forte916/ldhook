#include "debug.h"
#include "storeMessage.h"

int storeMsgToString(struct store_msg *sm)
{
	DEBUG_PRINT("sm->cmd = %d\n", sm->cmd);
	DEBUG_PRINT("sm->pid = %d\n", sm->pid);
	DEBUG_PRINT("sm->fd = %d\n", sm->fd);
	DEBUG_PRINT("sm->path = %s\n", sm->path);
	DEBUG_PRINT("sm->result = %d\n", sm->result);
    return 0;
}


// Original vendor plugin registration and packet producer methods.
#include <stdlib.h>
#include "../raknet/RakPeer.h"
#include "../raknet/PluginInterface.h"
Packet *AllocPacket(unsigned dataSize, unsigned char *data)
{
	Packet *p = (Packet *)malloc(sizeof(Packet));
	p->data=NULL;
	p->length=0;
	p->deleteData=false;
	if(dataSize != 0)
	{
		if(data != NULL)
		{
			p->data=data;
			p->length=dataSize;
			p->deleteData=true;
		}
	}
	return p;
}

void RakPeer::AttachPlugin( PluginInterface *plugin )
{
	if (messageHandlerList.GetIndexOf(plugin)==MAX_UNSIGNED_LONG)
	{
		messageHandlerList.Insert(plugin);
		plugin->OnAttach(this);
	}
}

void RakPeer::DetachPlugin( PluginInterface *plugin )
{
	if (plugin==0)
		return;

	unsigned int index;
	index = messageHandlerList.GetIndexOf(plugin);
	if (index!=MAX_UNSIGNED_LONG)
	{
		messageHandlerList[index]->OnDetach(this);
		// Unordered list so delete from end for speed
		messageHandlerList[index]=messageHandlerList[messageHandlerList.Size()-1];
		messageHandlerList.Del();
	}
}


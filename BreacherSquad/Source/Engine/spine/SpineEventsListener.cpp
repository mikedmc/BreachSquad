#include "dxstdafx.h"
#include "SpineEventsListener.h"

void Spine_AnimEventsCallback(AnimationState* state, EventType type, TrackEntry* entry, Event* event)
{
	const String& animationName = (entry && entry->getAnimation()) ? entry->getAnimation()->getName() : String("");

	switch (type)
	{
		case EventType_Start:
			//LOG("%d start: %s\n", entry->getTrackIndex(), animationName.buffer());
			break;
		case EventType_Interrupt:
			//LOG("%d interrupt: %s\n", entry->getTrackIndex(), animationName.buffer());
			break;
		case EventType_End:
			//LOG("%d end: %s\n", entry->getTrackIndex(), animationName.buffer());
			break;
		case EventType_Complete:
			//LOG("%d complete: %s\n", entry->getTrackIndex(), animationName.buffer());
			break;
		case EventType_Dispose:
			//LOG("%d dispose: %s\n", entry->getTrackIndex(), animationName.buffer());
			break;
		case EventType_Event:
		{
			CStringHash shEventName(animationName.buffer());
			ESpineEvent evt = (ESpineEvent)GetListIndexByNameHash(shEventName.textHash, ESpineEventNames, K_SD_EVENTS_CNT);
			switch (evt)
			{
			case K_SD_EVENT_SHOOT:
				// should send event to the actor or to the animation state or somewhere 
				break;
			case K_SD_EVENT_FOOTSTEP:
				break;
			default:
				break;
			}
			//LOG("%d event: %s, %s: %d, %f, %s\n", entry->getTrackIndex(), animationName.buffer(), event->getData().getName().buffer(), event->getIntValue(), event->getFloatValue(), event->getStringValue().buffer());
		}
		break;
	}
}



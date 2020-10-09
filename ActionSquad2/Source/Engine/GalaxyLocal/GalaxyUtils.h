#pragma once

template <class T> 
inline void RegisterAsGalaxyListener (T* listener)
{
	galaxy::api::ListenerRegistrar()->Register(T::GetListenerType(), listener);
}

template <class T>
inline void UnregisterAsGalaxyListener (T* listener)
{
	galaxy::api::ListenerRegistrar()->Unregister(T::GetListenerType(), listener);
}

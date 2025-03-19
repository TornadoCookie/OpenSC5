#include "rlWebkitClient.h"

#include <iostream>


 void GLWebkitClient::DebugLog(EA::WebKit::DebugLogInfo& l) 
{
   std::cout << l.mType << ": " << l.mpLogText << std::endl;
}

 void GLWebkitClient::GetLocalizedString(EA::WebKit::LocalizedStringInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::GetLocalizedString(info);
}

 void GLWebkitClient::LoadUpdate(EA::WebKit::LoadInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::LoadUpdate(info);
}

 void GLWebkitClient::NetworkError(EA::WebKit::NetworkErrorInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::NetworkError(info);
}


 void GLWebkitClient::CreateView(EA::WebKit::CreateViewInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::CreateView(info);
}


 void GLWebkitClient::DestroyView(EA::WebKit::DestroyViewInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl; 
   EA::WebKit::EAWebKitClient::DestroyView(info);
}


 void GLWebkitClient::ViewUpdate(EA::WebKit::ViewUpdateInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::ViewUpdate(info);
}


 void GLWebkitClient::CursorChanged(EA::WebKit::CursorChangeInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::CursorChanged(info);
}


 void GLWebkitClient::CursorMoved(EA::WebKit::CursorMovedInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::CursorMoved(info);
}


 void GLWebkitClient::LinkNotification(EA::WebKit::LinkNotificationInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::LinkNotification(info);
}


 void GLWebkitClient::NavigationActionsUpdate(EA::WebKit::NavigationActionsInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::NavigationActionsUpdate(info);
}


 void GLWebkitClient::ViewProcessStatus(EA::WebKit::ViewProcessInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::ViewProcessStatus(info);
}


 void GLWebkitClient::TextInputState(EA::WebKit::TextInputStateInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::TextInputState(info);
}


 void GLWebkitClient::ClipboardEvent(EA::WebKit::ClipboardEventInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::ClipboardEvent(info);
}


 void GLWebkitClient::ProcessEvents(EA::WebKit::ProcessEventsInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::ProcessEvents(info);
}


 void GLWebkitClient::ReportJSCallstack(EA::WebKit::ReportJSCallstackInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::ReportJSCallstack(info);
}


 void GLWebkitClient::JSMessageBox(EA::WebKit::JSMessageBoxInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::JSMessageBox(info);
}


 void GLWebkitClient::MediaUpdate(EA::WebKit::MediaUpdateInfo& info) 
{
   //std::cout << __FUNCTION__ << std::endl;
   EA::WebKit::EAWebKitClient::MediaUpdate(info);
}


//void GLWebkitClient::WatchDogNotification(EA::WebKit::WatchDogNotificationInfo& info) 
//{
//   //std::cout << __FUNCTION__ << std::endl;
//   EA::WebKit::EAWebKitClient::WatchDogNotification(info);
//}

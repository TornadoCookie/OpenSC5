#pragma once

#include <EAWebKit/EAWebKitClient.h>

class GLWebkitClient : public EA::WebKit::EAWebKitClient
{
public:
   virtual void DebugLog(EA::WebKit::DebugLogInfo& l) override;

   virtual void GetLocalizedString(EA::WebKit::LocalizedStringInfo& info) override;

   virtual void LoadUpdate(EA::WebKit::LoadInfo& info) override;

   virtual void NetworkError(EA::WebKit::NetworkErrorInfo& info) override;

   virtual void CreateView(EA::WebKit::CreateViewInfo& info) override;

   virtual void DestroyView(EA::WebKit::DestroyViewInfo& info) override;

   virtual void ViewUpdate(EA::WebKit::ViewUpdateInfo& info) override;

   virtual void CursorChanged(EA::WebKit::CursorChangeInfo& info) override;

   virtual void CursorMoved(EA::WebKit::CursorMovedInfo& info) override;

   virtual void LinkNotification(EA::WebKit::LinkNotificationInfo& info) override;

   virtual void NavigationActionsUpdate(EA::WebKit::NavigationActionsInfo& info) override;

   virtual void ViewProcessStatus(EA::WebKit::ViewProcessInfo& info) override;

   virtual void TextInputState(EA::WebKit::TextInputStateInfo& info) override;

   virtual void ClipboardEvent(EA::WebKit::ClipboardEventInfo& info) override;

   virtual void ProcessEvents(EA::WebKit::ProcessEventsInfo& info) override;

   virtual void ReportJSCallstack(EA::WebKit::ReportJSCallstackInfo& info) override;

   virtual void JSMessageBox(EA::WebKit::JSMessageBoxInfo& info) override;

   virtual void MediaUpdate(EA::WebKit::MediaUpdateInfo& info) override;

   //virtual void WatchDogNotification(EA::WebKit::WatchDogNotificationInfo& info) override;

protected:
};

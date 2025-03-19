/*
Copyright (C) 2008, 2009, 2010, 2011, 2012 Electronic Arts, Inc.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

1.  Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
3.  Neither the name of Electronic Arts, Inc. ("EA") nor the names of
    its contributors may be used to endorse or promote products derived
    from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY ELECTRONIC ARTS AND ITS CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL ELECTRONIC ARTS OR ITS CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "config.h"
#include <internal/include/EAWebKitViewNavigationDelegate.h>

#include <EAWebKit/EAWebKit.h>
#include <EAWebKit/EAWebkitSTLWrapper.h>
#include <EAWebKit/EAWebKitClient.h>

#include <internal/include/EAWebkitEASTLHelpers.h>
#include <Scrollbar.h>
#include <NamedNodeMap.h>
#include <FrameView.h>
#include <internal/include/InputBinding/EAWebKitDocumentNavigator.h>
#include <internal/include/InputBinding/EAWebKitPolarRegion.h>
#include <internal/include/InputBinding/EAWebKitUtils.h>
#include <internal/include/InputBinding/EAWebKitDocumentNavigationDelegates.h>
#include <internal/include/InputBinding/EAWebKitDOMWalker.h>

#include "Frame.h"

namespace EA
{
namespace WebKit
{
bool ViewNavigationDelegate::JumpToNearestElement(EA::WebKit::JumpDirection direction)
{
	//We try to find an element in the current visible area. If not found, we scroll the view in the intended direction.
	bool foundElement = JumpToNearestElement(direction, true);

	// If not found, we try one more time since newer elements may be visible now. In our case, it is necessary to do so(unlike some other implementation where only
	// scroll may happen). Imagine a player navigating through the music track listing and on the edge. If the user presses down, not only he is expecting the new
	// item to become visible, he is also expecting to be able to jump to it.
	if(!foundElement)
		foundElement = JumpToNearestElement(direction, false);

	return foundElement;
}

bool ViewNavigationDelegate::JumpToNearestElement(EA::WebKit::JumpDirection direction, bool scrollIfElementNotFound)
{

	// Note by Arpit Baldeva:
	// We have a problem here. mpModalInputClient object is supposed to be used for Modal input only however the only class using this object 
	// is html SELECT element(implemented as a popup). But in reality, html SELECT element is NOT modal. So it is little ill-conceived. 
	// For example, in all the browsers, if you scroll the mouse wheel on the frame, the SELECT element disappears and the actual frame scrolls.

	// For any modal input needs on a web page, the users are advised to use the Z-layer technique with Javascript/CSS - http://jqueryui.com/demos/dialog/#modal-confirmation.

	// The problem we want to solve here is have the SELECT element respond to the controller input correctly(select element one by one).
	// But the button event information is lost by the time we are in the EA::WebKit::View. For the foreseeable future, there is no candidate
	// other than html SELECT element which is implemented as a modal popup inside EAWebKit. So inside EA::WebKit::View, we create a dummy
	// button event from the Jump direction and make SELECT respond to it. If any other object starts using the modal input, this would need to be
	// revisited. But then, we'll need to solve a plethora of issues. So we do minimum work here to not break other things.

	IOverlayInputClient* pOverlayInputClient = mView->GetOverlayInputClient();

	bool handledByOverlayInputClient = false;
	if(pOverlayInputClient)
	{
		EA::WebKit::ButtonEvent btnEvent;
		switch(direction)
		{
			/*
			case EA::WebKit::JumpLeft:
			{
			btnEvent.mID = EA::WebKit::kButton0;
			handledByOverlayInputClient = pOverlayInputClient->OnButtonEvent(btnEvent);
			}
			*/
		case EA::WebKit::JumpUp:
			{
				btnEvent.mID = EA::WebKit::kButton1;
				handledByOverlayInputClient =  pOverlayInputClient->OnButtonEvent(btnEvent);
				break;
			}
			/*
			case EA::WebKit::JumpRight:
			{
			btnEvent.mID = EA::WebKit::kButton2;
			handledByOverlayInputClient =  pOverlayInputClient->OnButtonEvent(btnEvent);
			}
			*/
		case EA::WebKit::JumpDown:
			{
				btnEvent.mID = EA::WebKit::kButton3;
				handledByOverlayInputClient =  pOverlayInputClient->OnButtonEvent(btnEvent);
				break;
			}
		default:
			// We don't return and allow any other button press to go to the main View. At the same time, we make the SELECT element lose focus.
			{
				pOverlayInputClient->OnFocusChangeEvent(false);
				break;
			}
		}
	}

	if(handledByOverlayInputClient)
		return true;

	int lastX, lastY;
	mView->GetCursorPosition(lastX, lastY);

	// Following is a shortcut to drive navigation from a page.
	switch (direction)
	{
	case EA::WebKit::JumpRight:
		if (GetFixedString(mCachedNavigationRightId)->compare(""))
		{
			if (!GetFixedString(mCachedNavigationRightId)->compare("ignore"))
			{
				return false;
			}

			if (JumpToId(GetFixedString(mCachedNavigationRightId)->c_str()))
			{
				return true;
			}
		}
		break;

	case EA::WebKit::JumpDown:
		if (GetFixedString(mCachedNavigationDownId)->compare(""))
		{
			if (!GetFixedString(mCachedNavigationDownId)->compare("ignore"))
			{
				return false;
			}

			if (JumpToId(GetFixedString(mCachedNavigationDownId)->c_str()))
			{
				return true;
			}
		}
		break;

	case EA::WebKit::JumpLeft:
		if (GetFixedString(mCachedNavigationLeftId)->compare(""))
		{
			if (!GetFixedString(mCachedNavigationLeftId)->compare("ignore"))
			{
				return false;
			}

			if (JumpToId(GetFixedString(mCachedNavigationLeftId)->c_str()))
			{
				return true;
			}
		}
		break;

	case EA::WebKit::JumpUp:
		if (GetFixedString(mCachedNavigationUpId)->compare(""))
		{
			if (!GetFixedString(mCachedNavigationUpId)->compare("ignore"))
			{
				return false;
			}

			if (JumpToId(GetFixedString(mCachedNavigationUpId)->c_str()))
			{
				return true;
			}
		}
		break;

	default:
		EAW_FAIL_MSG("Should not have got here\n");
	}


	// Iterate over all the frames and find the closest element in any of all the frames.
	WebCore::Frame* pFrame		= mView->GetFrame();
	float currentRadialDistance = FLT_MAX; // A high value to start with so that the max distance between any two elements in the surface is under it.
	WebCore::Node* currentBestNode = NULL;
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		if(document)
		{
			WebCore::FrameView* pFrameView = document->view();
			WebCore::IntPoint scrollOffset;
			if(pFrameView)
			{
 				scrollOffset.setX(pFrameView->scrollOffset().width());
 				scrollOffset.setY(pFrameView->scrollOffset().height());
			}

			// We figure out the start position(It is center of the currently hovered element almost all the time but can be slightly different 
			// due to scroll sometimes).
			mCentreX = lastX + scrollOffset.x();
			mCentreY = lastY + scrollOffset.y();

			DocumentNavigator navigator(mView, document, direction, WebCore::IntPoint(mCentreX, mCentreY), mBestNodeX, mBestNodeY, mBestNodeWidth, mBestNodeHeight, mJumpNavigationParams.mNavigationTheta, mJumpNavigationParams.mStrictAxesCheck, currentRadialDistance);
			navigator.FindBestNode(document);

			if(navigator.GetBestNode())
			{
				currentBestNode			= navigator.GetBestNode();
				currentRadialDistance	= navigator.GetBestNodeRadialDistance();
			}

		}

		pFrame = pFrame->tree()->traverseNext();
	}

	bool foundSomething = false;
	if (currentBestNode) //We found the node to navigate. Move the cursor and we are done.
	{
		foundSomething = true;
		MoveMouseCursorToNode(currentBestNode, false);
	}
	else if(scrollIfElementNotFound)// Node is not found. 
	{
		// Based on the intended direction of movement, scroll so that some newer elements are visible.
		
		int cursorPosBeforeScrollX, cursorPosBeforeScrollY;
		mView->GetCursorPosition(cursorPosBeforeScrollX, cursorPosBeforeScrollY);

		switch(direction)
		{
		case EA::WebKit::JumpDown:
			{
				ScrollOnJump(true, -120, mJumpNavigationParams.mNumLinesToAutoScroll);
				break;
			}

		case EA::WebKit::JumpUp:
			{
				ScrollOnJump(true, 120, mJumpNavigationParams.mNumLinesToAutoScroll);
				break;
			}
		case EA::WebKit::JumpRight:
			{
				ScrollOnJump(false, -120, mJumpNavigationParams.mNumLinesToAutoScroll);
				break;
			}
		case EA::WebKit::JumpLeft:
			{
				ScrollOnJump(false, 120, mJumpNavigationParams.mNumLinesToAutoScroll);
				break;
			}
		default:
			{
				EAW_ASSERT_MSG(false, "Should not reach here\n");
			}
		}

		// We move the mouse cursor back to the location where the last best node was found. This is so that we don't end up with the cursor being in no man's land. While that may work 
		// for ordinary sites, it may not work well with customized pages that leverage CSS to visually indicate current position rather than a cursor graphic.
		// We don't call MoveMouseCursorToNode() with last cached node as there are edge cases where we may be holding an invalid node. Using a cached frame and checking against the
		// current valid frames safeguards against that.

		WebCore::IntSize scrollOffset;
		WebCore::Frame* pFrame1	= mView->GetFrame();
		while(pFrame1)
		{
			if(pFrame1 == mBestNodeFrame)//Find the frame where last best node existed.
			{
				if(pFrame1->view())
				{
					scrollOffset = pFrame1->view()->scrollOffset();//We read scroll offset here as it could have changed in the switch statement above.
					break;
				}
			}
			pFrame1 = pFrame1->tree()->traverseNext();
		}
		
		int targetcursorPosAfterScrollX, targetcursorPosAfterScrollY;
		targetcursorPosAfterScrollX = mBestNodeX + mBestNodeWidth / 2 - scrollOffset.width();
		targetcursorPosAfterScrollY = mBestNodeY + mBestNodeHeight/ 2 - scrollOffset.height();

		EA::WebKit::MouseMoveEvent moveEvent;
		memset( &moveEvent, 0, sizeof(moveEvent) );
		
		const int cursorInset = 5;// Make cursor stay inside 5 pixels from boundaries. No known issues but added this as a safety measure so that we do not lose cursor ever.
		
		int width = mView->GetSize().mWidth;
		int height = mView->GetSize().mHeight;

		moveEvent.mX	= Clamp( cursorInset, targetcursorPosAfterScrollX, width - cursorInset );
		moveEvent.mY	= Clamp( cursorInset, targetcursorPosAfterScrollY, height - cursorInset );


		mView->OnMouseMoveEvent(moveEvent);
		// We intentionally don't call JumpToNearestElement(direction, false) here to avoid recursion. We do it in the overloaded function above.
	}
		
	return foundSomething;
}

bool ViewNavigationDelegate::JumpToId(const char* jumpToId)
{
	WebCore::Frame* pFrame = mView->GetFrame();
	// Search for the desired element in all the frames
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		if (document)
		{
			JumpToElementWithIdDelegate delegate1(mView, jumpToId);

			DOMWalker<JumpToElementWithIdDelegate> walker(document, delegate1);		
			WebCore::Element* element = delegate1.FoundElement();
			if (element)
			{
				return true;
			}
		}
		pFrame = pFrame->tree()->traverseNext();
	}

	return false;
}

bool ViewNavigationDelegate::ClickElementById(const char* id)
{
	EAW_ASSERT(id);
	if(!id || !id[0])
		return false;

	IOverlayInputClient* pOverlayInputClient = mView->GetOverlayInputClient();


	if(pOverlayInputClient)
		pOverlayInputClient->OnFocusChangeEvent(false);


	bool elementClicked = false;

	WebCore::Frame* pFrame = mView->GetFrame();
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		if (document)
		{
			WebCore::Element* element = document->getElementById(id);

			if (element && element->isHTMLElement())
			{    
				WebCore::HTMLElement* htmlElement = (WebCore::HTMLElement*)element;
				htmlElement->click();
				elementClicked = true;
			}
		}

		if(elementClicked)
			break;

		pFrame = pFrame->tree()->traverseNext();
	}

	return elementClicked;
}


bool ViewNavigationDelegate::ClickElementsByClass(const char* className)
{
	EAW_ASSERT(className);
	if(!className || !className[0])
		return false;

	IOverlayInputClient* pOverlayInputClient = mView->GetOverlayInputClient();

	if(pOverlayInputClient)
		pOverlayInputClient->OnFocusChangeEvent(false);


	bool elementClicked = false;

	WebCore::Frame* pFrame = mView->GetFrame();
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		ClickElementsByIdOrClassDelegate delegate(className, false);
		if (document)
		{
			DOMWalker<ClickElementsByIdOrClassDelegate> walker(document, delegate);	
		}

		elementClicked = delegate.GetReturnValue();

		if(elementClicked)
			break;

		pFrame = pFrame->tree()->traverseNext();
	}

	return elementClicked;
}

bool ViewNavigationDelegate::ClickElementsByIdOrClass(const char* idOrClassName)
{
	EAW_ASSERT(idOrClassName);
	if(!idOrClassName || !idOrClassName[0])
		return false;

	IOverlayInputClient* pOverlayInputClient = mView->GetOverlayInputClient();

	if(pOverlayInputClient)
		pOverlayInputClient->OnFocusChangeEvent(false);

	bool elementClicked = false;

	WebCore::Frame* pFrame = mView->GetFrame();
	while(pFrame)
	{
		WebCore::Document* document = pFrame->document();
		EAW_ASSERT(document);

		ClickElementsByIdOrClassDelegate delegate(idOrClassName, true);
		DOMWalker<ClickElementsByIdOrClassDelegate> walker(document, delegate);	
		elementClicked = delegate.GetReturnValue();

		if(elementClicked)
			break;

		pFrame = pFrame->tree()->traverseNext();
	}

	return elementClicked;
}

bool ViewNavigationDelegate::Click()
{
	//Do not deal with modal input here since OnMouseButtonEvent deals with it.
	EA::WebKit::MouseButtonEvent event;
	memset(&event, 0, sizeof(event));
	event.mId = EA::WebKit::kMouseLeft;
	event.mClickCount = 1;
	mView->GetCursorPosition(event.mX, event.mY);

	event.mbDepressed = true;
	mView->OnMouseButtonEvent(event);
	event.mbDepressed = false;
	mView->OnMouseButtonEvent(event);

	return true;
}

void ViewNavigationDelegate::MoveMouseCursorToNode(WebCore::Node* node, bool scrollIfNecessary)
{
	if (node)
	{
		WebCore::HTMLElement* element = (WebCore::HTMLElement*)node;

		WebCore::IntRect rect = element->getRect();
		WebCore::IntPoint frameOffset;
		WebCore::IntPoint scrollOffset;

		WebCore::FrameView* pFrameView = element->document()->view(); //Can be NULL
		if(pFrameView)
		{
			//Use move here instead of setX/setY as it results in 1 call instead of two and takes advantage that ctor sets x,y to 0.
			frameOffset.move(pFrameView->x(), pFrameView->y());
			scrollOffset.move(pFrameView->scrollOffset().width(), pFrameView->scrollOffset().height());
		}

		int width = mView->GetSize().mWidth;
		int height = mView->GetSize().mHeight;

		// This will be true if this function is called from anywhere except JumpToNearestElement(). This enables us to not lose the cursor during
		// arbitrary jumping of elements from either code or webpage using the Navigation APIs.
		if(scrollIfNecessary)
		{
			int cursorX, cursorY;
			mView->GetCursorPosition(cursorX, cursorY);

			WebCore::IntPoint targetCursorLocation(frameOffset.x()+rect.x()+rect.width()/2  - scrollOffset.x(), frameOffset.y()+rect.y()+rect.height()/2  - scrollOffset.y());
			// Added 1 in all the line delta below resulting in a better visual behavior when the element happens to be at the edge.
			if(targetCursorLocation.y() > height)
			{
                float numLinesDelta = (((targetCursorLocation.y() - cursorY)/WebCore::Scrollbar::pixelsPerLineStep())+1);
				ScrollOnJump(true, -120, numLinesDelta);
			}
			if(targetCursorLocation.y()< 0.0f)
			{
				float numLinesDelta = (((cursorY - targetCursorLocation.y())/WebCore::Scrollbar::pixelsPerLineStep())+1);
				ScrollOnJump(true, 120, numLinesDelta);
			}
			if(targetCursorLocation.x() > width)
			{
				float numLinesDelta = (((targetCursorLocation.x() - cursorX)/WebCore::Scrollbar::pixelsPerLineStep())+1);
				ScrollOnJump(false, -120, numLinesDelta);
			}
			if(targetCursorLocation.x()< 0.0f)
			{
				float numLinesDelta = (((cursorX - targetCursorLocation.x())/WebCore::Scrollbar::pixelsPerLineStep())+1);
				ScrollOnJump(false, 120, numLinesDelta);
			}

			// Read the scroll offset again as it may have changed.
			if(pFrameView) 
			{
				scrollOffset.setX(pFrameView->scrollOffset().width());
				scrollOffset.setY(pFrameView->scrollOffset().height());
			}
		}

		mBestNodeFrame	= element->document()->frame(); //Can be NULL
		mBestNodeX		= frameOffset.x()  + rect.x();
		mBestNodeY		= frameOffset.y()  + rect.y();
		mBestNodeWidth	= rect.width();
		mBestNodeHeight = rect.height();

		int lastX, lastY;
		mView->GetCursorPosition(lastX, lastY);
		int newX		= mBestNodeX + rect.width()/2  - scrollOffset.x();
		int newY		= mBestNodeY + rect.height()/2 - scrollOffset.y();

		EA::WebKit::MouseMoveEvent moveEvent;
		memset( &moveEvent, 0, sizeof(moveEvent) );
		const int cursorInset = 5;// Make cursor stay inside 5 pixels from boundaries. No known issues but added this as a safety measure so that we do not lose cursor ever.
		moveEvent.mX	= Clamp( cursorInset, newX, width - cursorInset );
		moveEvent.mY	= Clamp( cursorInset, newY, height - cursorInset );

		mView->OnMouseMoveEvent( moveEvent );

		if(EAWebKitClient* const pClient = GetEAWebKitClient())
		{
			CursorMovedInfo cmInfo(mView, mView->GetUserData(), moveEvent.mX, moveEvent.mY);
			pClient->CursorMoved(cmInfo);
		}

		UpdateCachedHints(node);

	}
}


void ViewNavigationDelegate::UpdateCachedHints(WebCore::Node* node)
{
	if (node && node->hasAttributes())
	{
		WebCore::NamedNodeMap* attributeMap = node->attributes();

		WebCore::Attribute* upAttribute		= attributeMap->getAttributeItem(WebCore::QualifiedName	(""	,"navigationup"		,""));
		WebCore::Attribute* downAttribute	= attributeMap->getAttributeItem(WebCore::QualifiedName	(""	,"navigationdown"	,""));
		WebCore::Attribute* leftAttribute	= attributeMap->getAttributeItem(WebCore::QualifiedName	(""	,"navigationleft"	,""));
		WebCore::Attribute* rightAttribute	= attributeMap->getAttributeItem(WebCore::QualifiedName	(""	,"navigationright"	,""));

		if ( upAttribute )
		{
			EA::WebKit::ConvertToString8(upAttribute->value().string(),*GetFixedString(mCachedNavigationUpId));
		}
		else
		{
			GetFixedString(mCachedNavigationUpId)->assign("");
		}

		if ( downAttribute )
		{
			EA::WebKit::ConvertToString8(downAttribute->value().string(),*GetFixedString(mCachedNavigationDownId));
		}
		else
		{
			GetFixedString(mCachedNavigationDownId)->assign("");
		}

		if ( leftAttribute )
		{
			EA::WebKit::ConvertToString8(leftAttribute->value().string(),*GetFixedString(mCachedNavigationLeftId));
		}
		else
		{
			GetFixedString(mCachedNavigationLeftId)->assign("");
		}

		if ( rightAttribute )
		{
			EA::WebKit::ConvertToString8(rightAttribute->value().string(),*GetFixedString(mCachedNavigationRightId));
		}
		else
		{
			GetFixedString(mCachedNavigationRightId)->assign("");
		}
	}
	else
	{
		GetFixedString(mCachedNavigationUpId)->assign("");
		GetFixedString(mCachedNavigationDownId)->assign("");
		GetFixedString(mCachedNavigationLeftId)->assign("");
		GetFixedString(mCachedNavigationRightId)->assign("");
	}
}

void ViewNavigationDelegate::ScrollOnJump(bool vertical, int zDelta, float numLines)
{
	EA::WebKit::MouseWheelEvent mouseWheelEvent;
	mView->GetCursorPosition( mouseWheelEvent.mX, mouseWheelEvent.mY );

	if(!vertical)
		mouseWheelEvent.mModifiers = kModifierMaskShift;

	mouseWheelEvent.mZDelta = zDelta; 
	mouseWheelEvent.mNumLines = numLines;
	mView->OnMouseWheelEvent(mouseWheelEvent);
}

}// WebKit
}// EA
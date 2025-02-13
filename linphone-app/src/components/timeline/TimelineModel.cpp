/*
 * Copyright (c) 2010-2020 Belledonne Communications SARL.
 *
 * This file is part of linphone-desktop
 * (see https://www.linphone.org).
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "components/core/CoreManager.hpp"
#include "components/settings/AccountSettingsModel.hpp"
#include "components/sip-addresses/SipAddressesModel.hpp"
#include "components/chat-room/ChatRoomModel.hpp"
#include "utils/Utils.hpp"
#include "app/App.hpp"

#include "TimelineModel.hpp"
#include "TimelineListModel.hpp"

#include "../calls/CallsListModel.hpp"

#include <QDebug>
#include <qqmlapplicationengine.h>
#include <QTimer>


// =============================================================================
std::shared_ptr<TimelineModel> TimelineModel::create(std::shared_ptr<linphone::ChatRoom> chatRoom, const std::list<std::shared_ptr<linphone::CallLog>>& callLogs, QObject *parent){
	if((!chatRoom || chatRoom->getState() != linphone::ChatRoom::State::Terminated)  && (!CoreManager::getInstance()->getTimelineListModel() || !CoreManager::getInstance()->getTimelineListModel()->getTimeline(chatRoom, false)) ) {
		std::shared_ptr<TimelineModel> model = std::make_shared<TimelineModel>(chatRoom, parent);
		if(model && model->getChatRoomModel()){
			model->mSelf = model;
			chatRoom->addListener(model);
			// Get Max updatetime from chat room and last call event
			auto timelineChatRoom = model->getChatRoomModel();
			std::shared_ptr<linphone::CallLog> lastCall = nullptr;
			QString peerAddress = timelineChatRoom->getParticipantAddress();
			std::shared_ptr<const linphone::Address> lLocalAddress = chatRoom->getLocalAddress();
			QString localAddress = Utils::coreStringToAppString(lLocalAddress->asStringUriOnly());
			
			if(callLogs.size() == 0) {
				auto callHistory = CallsListModel::getCallHistory(peerAddress, localAddress);
				if(callHistory.size() > 0)
					lastCall = callHistory.front();
			}else{// Find the last call in list
				std::shared_ptr<linphone::Address> lPeerAddress = Utils::interpretUrl(peerAddress);
				if( lPeerAddress && lLocalAddress){
					auto itCallLog = std::find_if(callLogs.begin(), callLogs.end(), [lPeerAddress, lLocalAddress](std::shared_ptr<linphone::CallLog> c){
						return c->getLocalAddress()->weakEqual(lLocalAddress) && c->getRemoteAddress()->weakEqual(lPeerAddress);
					});
					if( itCallLog != callLogs.end())
						lastCall = *itCallLog;
					}
			}
				
			if(lastCall){
				auto callDate = lastCall->getStartDate();
				if( lastCall->getStatus() == linphone::Call::Status::Success )
					callDate += lastCall->getDuration();
				timelineChatRoom->setLastUpdateTime(QDateTime::fromMSecsSinceEpoch(std::max(chatRoom->getLastUpdateTime(), callDate )*1000));
			}else
				timelineChatRoom->setLastUpdateTime(QDateTime::fromMSecsSinceEpoch(chatRoom->getLastUpdateTime()*1000));
			return model;
		}
	}
	return nullptr;
}

TimelineModel::TimelineModel (std::shared_ptr<linphone::ChatRoom> chatRoom, QObject *parent) : QObject(parent) {
	App::getInstance()->getEngine()->setObjectOwnership(this, QQmlEngine::CppOwnership);// Avoid QML to destroy it when passing by Q_INVOKABLE
	mChatRoomModel = ChatRoomModel::create(chatRoom);
	if( mChatRoomModel ){
		CoreManager::getInstance()->handleChatRoomCreated(mChatRoomModel);
		QObject::connect(this, &TimelineModel::selectedChanged, this, &TimelineModel::updateUnreadCount);
		QObject::connect(CoreManager::getInstance()->getAccountSettingsModel(), &AccountSettingsModel::defaultAccountChanged, this, &TimelineModel::onDefaultAccountChanged);
	}
	mSelected = false;
}

TimelineModel::~TimelineModel(){
	//mChatRoomModel->getChatRoom()->removeListener(mChatRoomModel);
}

QString TimelineModel::getFullPeerAddress() const{
	return mChatRoomModel->getFullPeerAddress();
}
QString TimelineModel::getFullLocalAddress() const{
	return mChatRoomModel->getLocalAddress();
}


QString TimelineModel::getUsername() const{
	return mChatRoomModel->getUsername();
}

QString TimelineModel::getAvatar() const{
	return "";
}

int TimelineModel::getPresenceStatus() const{
	return 0;
}

ChatRoomModel *TimelineModel::getChatRoomModel() const{
	return mChatRoomModel.get();
}

void TimelineModel::setSelected(const bool& selected){
	if(selected != mSelected){
		mSelected = selected;
		if(mSelected){
			qInfo() << "Chat room selected : Subject :" << mChatRoomModel->getSubject()
				<< ", Username:" << mChatRoomModel->getUsername()
				<< ", GroupEnabled:"<< mChatRoomModel->isGroupEnabled()
				<< ", isConference:"<< mChatRoomModel->isConference()
				<< ", isOneToOne:"<< mChatRoomModel->isOneToOne()
				<< ", Encrypted:"<< mChatRoomModel->haveEncryption()
				<< ", ephemeralEnabled:" << mChatRoomModel->isEphemeralEnabled()
				<< ", isAdmin:"<< mChatRoomModel->isMeAdmin()
				<< ", canHandleParticipants:"<< mChatRoomModel->canHandleParticipants()
				<< ", isReadOnly:" << mChatRoomModel->isReadOnly()
				<< ", state:" << mChatRoomModel->getState();
		}else
			mChatRoomModel->resetEntries();// Cleanup leaving chat room
		emit selectedChanged(mSelected);
	}
}

void TimelineModel::updateUnreadCount(){
	if(!mSelected){// updateUnreadCount is called when selected has changed;: So if mSelected is false then we are going out of it.
		mChatRoomModel->resetMessageCount();// The reset will appear when the chat room has "mark as read enabled", that means that we should have read messages when going out.
	}
}
void TimelineModel::onDefaultAccountChanged(){
	if( mSelected && !mChatRoomModel->isCurrentAccount())
		setSelected(false);
}
//----------------------------------------------------------
//------				CHAT ROOM HANDLERS
//----------------------------------------------------------

void TimelineModel::onIsComposingReceived(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::Address> & remoteAddress, bool isComposing){
}
void TimelineModel::onMessageReceived(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<linphone::ChatMessage> & message){}
void TimelineModel::onNewEvent(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onChatMessageReceived(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onChatMessageSending(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onChatMessageSent(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onParticipantAdded(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onParticipantRemoved(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onParticipantAdminStatusChanged(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onStateChanged(const std::shared_ptr<linphone::ChatRoom> & chatRoom, linphone::ChatRoom::State newState){
	if(newState == linphone::ChatRoom::State::Created && CoreManager::getInstance()->getTimelineListModel()->mAutoSelectAfterCreation) {
		CoreManager::getInstance()->getTimelineListModel()->mAutoSelectAfterCreation = false;
		QTimer::singleShot(200, [=](){// Delay process in order to let GUI time for Timeline building/linking before doing actions
				setSelected(true);
			});
	}
}
void TimelineModel::onSecurityEvent(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onSubjectChanged(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog)
{
	emit usernameChanged();
}
void TimelineModel::onUndecryptableMessageReceived(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<linphone::ChatMessage> & message){}
void TimelineModel::onParticipantDeviceAdded(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onParticipantDeviceRemoved(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onConferenceJoined(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){
}
void TimelineModel::onConferenceLeft(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){
	
}
void TimelineModel::onEphemeralEvent(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onEphemeralMessageTimerStarted(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onEphemeralMessageDeleted(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::EventLog> & eventLog){}
void TimelineModel::onConferenceAddressGeneration(const std::shared_ptr<linphone::ChatRoom> & chatRoom){}
void TimelineModel::onParticipantRegistrationSubscriptionRequested(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::Address> & participantAddress){}
void TimelineModel::onParticipantRegistrationUnsubscriptionRequested(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<const linphone::Address> & participantAddress){}
void TimelineModel::onChatMessageShouldBeStored(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<linphone::ChatMessage> & message){}
void TimelineModel::onChatMessageParticipantImdnStateChanged(const std::shared_ptr<linphone::ChatRoom> & chatRoom, const std::shared_ptr<linphone::ChatMessage> & message, const std::shared_ptr<const linphone::ParticipantImdnState> & state){}

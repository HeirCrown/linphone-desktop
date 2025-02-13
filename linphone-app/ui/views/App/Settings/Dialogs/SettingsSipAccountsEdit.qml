import QtQuick 2.7

import Common 1.0

import App.Styles 1.0

import 'SettingsSipAccountsEdit.js' as Logic

// =============================================================================

DialogPlus {
	id: dialog
	
	property var account // Optional.
	
	property bool _sipAddressOk: false
	property bool _serverAddressOk: false
	property bool _routeOk: true
	property bool _conferenceUriOk: true
	
	buttons: [
		TextButtonA {
			text: qsTr('cancel')
			
			onClicked: exit(0)
		},
		TextButtonB {
			enabled: Logic.formIsValid()
			text: qsTr('confirm')
			
			onClicked: Logic.validAccount(dialog.account ? dialog.account.account : null)
		}
	]
	
	buttonsAlignment: Qt.AlignCenter
	
	height: SettingsSipAccountsEditStyle.height
	width: SettingsSipAccountsEditStyle.width
	
	// ---------------------------------------------------------------------------
	
	Component.onCompleted: Logic.initForm(account)
	
	// ---------------------------------------------------------------------------
	
	TabContainer {
		anchors.fill: parent
		
		Column {
			width: parent.width
			
			Form {
				title: qsTr('mainSipAccountSettingsTitle')
				width: parent.width
				
				FormLine {
					FormGroup {
						label: qsTr('sipAddressLabel') + '*'
						
						TextField {
							id: sipAddress
							placeholderText: 'sip:name@sip.example.net'
							
							error: dialog._sipAddressOk ? '' : qsTr('invalidSipAddress')
							
							onTextChanged: Logic.handleSipAddressChanged(text)
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('serverAddressLabel') + '*'
						
						TextField {
							id: serverAddress
							placeholderText: 'sip:sip.example.net'
							error: dialog._serverAddressOk ? '' : qsTr('invalidServerAddress')
							onActiveFocusChanged: if(!activeFocus && dialog._serverAddressOk) Logic.handleTransportChanged(transport.model[transport.currentIndex])
							onTextChanged: Logic.handleServerAddressChanged(text)
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('registrationDurationLabel')
						
						NumericField {
							id: registrationDuration
							Keys.onEnterPressed:  route.forceActiveFocus()
							Keys.onReturnPressed:  route.forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('transportLabel')
						
						ComboBox {
							id: transport
							
							enabled: dialog._serverAddressOk
							model: [ 'UDP', 'TCP', 'TLS' ]
							
							onActivated: Logic.handleTransportChanged(model[index])
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('routeLabel')
						
						TextField {
							id: route
							
							error: dialog._routeOk ? '' : qsTr('invalidRoute')
							
							onTextChanged: Logic.handleRouteChanged(text)
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						//: "Conference URI" : Label of a text edit for filling Conference URI
						label: qsTr('conferenceURI')
						
						TextField {
							id: conferenceUri
							//: "invalid conference URI" : Error text about conference URI
							error: dialog._conferenceUriOk ? '' : qsTr("invalidConferenceURI")
							
							onTextChanged: Logic.handleConferenceUriChanged(text)
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('contactParamsLabel')
						
						TextField {
							id: contactParams
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('avpfIntervalLabel')
						
						NumericField {
							id: avpfInterval
							
							maxValue: 5
							minValue: 1
							Keys.onEnterPressed:  focus=false
							Keys.onReturnPressed:  focus=false
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('registerEnabledLabel')
						
						Switch {
							id: registerEnabled
							
							onClicked: checked = !checked
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('publishPresenceLabel')
						
						Switch {
							id: publishPresence
							
							onClicked: checked = !checked
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('avpfEnabledLabel')
						
						Switch {
							id: avpfEnabled
							
							onClicked: checked = !checked
						}
					}
				}
			}
			
			// -----------------------------------------------------------------------
			// NAT and Firewall.
			// -----------------------------------------------------------------------
			
			Form {
				title: qsTr('natAndFirewallTitle')
				width: parent.width
				
				FormLine {
					FormGroup {
						label: qsTr('enableIceLabel')
						
						Switch {
							id: iceEnabled
							
							onClicked: checked = !checked
						}
					}
					
					FormGroup {
						label: qsTr('stunServerLabel')
						
						TextField {
							id: stunServer
							placeholderText: 'stun.example.net'
							
							readOnly: !iceEnabled.checked
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {
						label: qsTr('enableTurnLabel')
						
						Switch {
							id: turnEnabled
							
							enabled: iceEnabled.checked
							
							onClicked: checked = !checked
						}
					}
					
					FormGroup {
						label: qsTr('turnUserLabel')
						
						TextField {
							id: turnUser
							
							readOnly: !turnEnabled.checked || !turnEnabled.enabled
							Keys.onEnterPressed:  nextItemInFocusChain().forceActiveFocus()
							Keys.onReturnPressed:  nextItemInFocusChain().forceActiveFocus()
						}
					}
				}
				
				FormLine {
					FormGroup {}
					
					FormGroup {
						label: qsTr('turnPasswordLabel')
						
						TextField {
							id: turnPassword
							readOnly: !turnEnabled.checked || !turnEnabled.enabled || !turnUser.text.length
						}
					}
				}
			}
		}
	}
}

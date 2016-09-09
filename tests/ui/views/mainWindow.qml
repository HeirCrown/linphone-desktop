import QtQuick 2.7
import QtQuick.Controls 2.0
import QtQuick.Layouts 1.3

import 'qrc:/ui/components/form'

ApplicationWindow {
    header: ToolBar {
        background: Rectangle {
            color: '#EAEAEA'
        }
        height: 70

        RowLayout {
            anchors.fill: parent
            anchors.rightMargin: 10

            // Collapse.
            Collapse {
                image: 'qrc:/imgs/collapse.svg'
                onCollapsed: {
                    mainWindow.height = collapsed ? 480 : 70
                }
            }

            // User info.
            // TODO

            // User actions.
            ToolBarButton {
                onClicked: {
                    var component = Qt.createComponent('qrc:/ui/views/manageAccounts.qml');
                    if (component.status !== Component.Ready) {
                        console.debug('Window not ready.')
                        if(component.status === Component.Error) {
                            console.debug('Error:' + component.errorString())
                        }
                    } else {
                        component.createObject(mainWindow).show()
                    }
                }
            }

            // Search.
            TextField {
                signal searchTextChanged (string text)

                background: Rectangle {
                    color: '#FFFFFF'
                    implicitHeight: 30
                }
                id: searchText
                Layout.fillWidth: true
                onTextChanged: searchTextChanged(text)
                placeholderText: qsTr('mainSearchBarPlaceholder')
            }

            // Start conference.
            ToolBarButton {
                image: 'qrc:/imgs/start_conference.svg'
            }
        }
    }
    id: mainWindow
    minimumHeight: 70
    minimumWidth: 640
    title: 'Linphone'
    visible: true
}
